# build.ps1 — Windows 一键构建脚本 (PowerShell)
#
# 用法:
#   .\build.ps1                    # 默认: Release, 只构建 BUILD_MAIN (legacy openCv exe)
#   .\build.ps1 -Target main       # 同上, 显式指定
#   .\build.ps1 -Target learn -Layer L2
#   .\build.ps1 -Target algorithms -Module ALL
#   .\build.ps1 -Target all        # main + learn(ALL) + algorithms(ALL) 一起编
#   .\build.ps1 -Target notes      # 构建 notes/image_process legacy 子项目
#   .\build.ps1 -Config Debug      # 编 Debug 版
#
# 约定:
#   - 生成目录: build_<target>[_suffix] (build_main / build_learn_L2 / build_algo_xxx …)
#   - 用 MinGW Makefiles 生成器, 自动 -j <核心数>
#   - 如果 mingw32-make / cmake 不在 PATH, 会尝试从常见位置找一下并提示
#
[CmdletBinding()]
param(
    [ValidateSet("main", "learn", "algorithms", "notes", "all")]
    [string]$Target = "main",

    # learn 专属
    [ValidateSet("ALL", "L0", "L1", "L2", "L3", "L4", "L5")]
    [string]$Layer = "ALL",

    # algorithms 专属: "ALL" 或分号分隔列表: hdr;denoise_single
    [string]$Module = "ALL",

    [ValidateSet("Release", "Debug", "RelWithDebInfo")]
    [string]$Config = "Release",

    [string]$Generator = "MinGW Makefiles",
    [switch]$NoBuild    # 只 cmake configure, 不 build
    [switch]$List       # 只列出可选 Target/Layer/Module, 不构建
)

$ErrorActionPreference = "Stop"
# 兼容 -File / dot-source / 内嵌调用: 优先 $PSScriptRoot, 退而取 MyInvocation
$RepoRoot = if ($PSScriptRoot) { $PSScriptRoot } else { Split-Path -Parent $MyInvocation.MyCommand.Path }
if ($RepoRoot -and (Test-Path $RepoRoot)) {
    Set-Location $RepoRoot
    Write-Host "==> Repo root: $RepoRoot" -ForegroundColor Cyan
} else {
    Write-Warning "无法确定 Repo root, 跳过 Set-Location (请确保在仓库根目录运行此脚本)"
}

# ---- 仅列出可选目标（在工具探测前，弱依赖）----
if ($List) {
    Write-Host ""
    Write-Host "=== 可用的构建目标 (Target) ===" -ForegroundColor Cyan
    Write-Host "  main | learn | algorithms | notes | all" -ForegroundColor White
    Write-Host ""
    Write-Host "=== learn 层级 (Layer) ===" -ForegroundColor Cyan
    Write-Host "  ALL | L0 | L1 | L2 | L3 | L4 | L5" -ForegroundColor White
    Write-Host ""
    Write-Host "=== algorithms 模块 (Module, ALL 或分号分隔) ===" -ForegroundColor Cyan
    Write-Host "  ALL" -ForegroundColor White
    Write-Host "  denoise_single | denoise_multi | hdr | night_scene | beauty | watermark" -ForegroundColor White
    Write-Host "  edge_detection | morphology | segmentation | feature_detection | stereo" -ForegroundColor White
    Write-Host "  deblur | template_matching | inpaint | hough_transform | frequency_domain | optical_flow" -ForegroundColor White
    Write-Host ""
    Write-Host "=== 常用组合示例 ===" -ForegroundColor Cyan
    Write-Host "  .\build.ps1 -List" -ForegroundColor DarkGray
    Write-Host "  .\build.ps1 -Target algorithms -Module ALL" -ForegroundColor DarkGray
    Write-Host "  .\build.ps1 -Target algorithms -Module `"hdr;denoise_single;beauty`"" -ForegroundColor DarkGray
    Write-Host "  .\build.ps1 -Target learn -Layer L2" -ForegroundColor DarkGray
    Write-Host "  .\build.ps1 -Target all" -ForegroundColor DarkGray
    exit 0
}

# ---- 工具探测 ----
function Find-Exe($name) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    return $null
}
$cmake = Find-Exe "cmake"
$make  = Find-Exe "mingw32-make"
if (-not $cmake) { Write-Error "cmake.exe 不在 PATH, 请先安装并加入 PATH"; exit 1 }
if (-not $make -and $Generator -like "*MinGW*") {
    Write-Warning "mingw32-make 不在 PATH; 若使用 MinGW 生成器可能会失败"
}

# ---- 按 Target 拼 CMake 开关 ----
function Invoke-CMakeBuild($buildDir, $cmakeArgs) {
    Write-Host ""
    Write-Host "==> Configure:  cmake -B $buildDir -G `"$Generator`" @cmakeArgs ..." -ForegroundColor Green
    & $cmake -B $buildDir -G $Generator @cmakeArgs
    if ($LASTEXITCODE -ne 0) { Write-Error "cmake configure failed (exit=$LASTEXITCODE)"; exit $LASTEXITCODE }

    if ($NoBuild) { Write-Host "==> -NoBuild 已指定, 跳过 build"; return }
    $parallel = [Environment]::ProcessorCount
    Write-Host "==> Build:  cmake --build $buildDir -j $parallel ($Config)" -ForegroundColor Green
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    & $cmake --build $buildDir -j $parallel --config $Config
    $sw.Stop()
    if ($LASTEXITCODE -ne 0) { Write-Error "build failed (exit=$LASTEXITCODE)"; exit $LASTEXITCODE }
    Write-Host ("==> Build time: {0:F1}s (parallel={1})" -f $sw.Elapsed.TotalSeconds, $parallel) -ForegroundColor Yellow
}

$commonBase = @(
    "-DCMAKE_BUILD_TYPE=$Config"
)

# --- main ---
if ($Target -eq "main" -or $Target -eq "all") {
    $dir = "build_main"
    $args = $commonBase + @(
        "-DBUILD_MAIN=ON",
        "-DBUILD_LEARN=OFF",
        "-DBUILD_ALGORITHMS=OFF"
    )
    Invoke-CMakeBuild $dir $args
    Write-Host "  → exe: $dir\openCv.exe" -ForegroundColor Cyan
}

# --- learn ---
if ($Target -eq "learn" -or $Target -eq "all") {
    $suffix = if ($Layer -eq "ALL") { "ALL" } else { $Layer }
    $dir = "build_learn_$suffix"
    $args = $commonBase + @(
        "-DBUILD_MAIN=OFF",
        "-DBUILD_LEARN=ON",
        "-DLEARN_LAYER=$Layer",
        "-DBUILD_ALGORITHMS=OFF"
    )
    Invoke-CMakeBuild $dir $args
    Write-Host "  → exes in: $dir\learn\learn_*.exe" -ForegroundColor Cyan
}

# --- algorithms ---
if ($Target -eq "algorithms" -or $Target -eq "all") {
    $suffix = if ($Module -eq "ALL") { "ALL" } else { ($Module -replace ';', '_') }
    $dir = "build_algo_$suffix"
    $args = $commonBase + @(
        "-DBUILD_MAIN=OFF",
        "-DBUILD_LEARN=OFF",
        "-DBUILD_ALGORITHMS=ON",
        "-DALGO_MODULE=$Module"
    )
    Invoke-CMakeBuild $dir $args
    Write-Host "  → exes in: $dir\algorithms\algo_*.exe" -ForegroundColor Cyan
    Write-Host "  → output : out\algorithms\*.png (运行 exe 后生成)" -ForegroundColor DarkCyan
}

# --- notes ---
if ($Target -eq "notes") {
    $dir = "build_notes_imgproc"
    # notes/image_process 用 standalone 方式构建, 不通过根 CMake 开关
    Push-Location notes\image_process
    try {
        $args = $commonBase  # 子项目内部会自己推断 OpenCV_DIR
        Write-Host ""
        Write-Host "==> Configure notes/image_process → $dir" -ForegroundColor Green
        & $cmake -B $dir -G $Generator @args
        if ($LASTEXITCODE -ne 0) { Write-Error "cmake configure failed"; exit $LASTEXITCODE }
        if (-not $NoBuild) {
            $parallel = [Environment]::ProcessorCount
            & $cmake --build $dir -j $parallel --config $Config
        }
    } finally {
        Pop-Location
    }
    Write-Host "  → exes in: notes\image_process\$dir\note_imgproc_*.exe" -ForegroundColor Cyan
}

Write-Host ""
Write-Host "==> Build finished successfully. 目标=$Target 配置=$Config" -ForegroundColor Green
Write-Host "    若要运行可执行, 请 cd 到对应 build/<sub>/ 目录下再 exe, 以保证相对路径 ../../data 可解析"
