# clean.ps1 — 清理构建产物 & 临时文件
#
# 用法:
#   .\clean.ps1              # 删除: build*/ / cmake 缓存 / CMakeFiles / *.exe ~/编译中间文件 等等
#   .\clean.ps1 all          # 同上 + 清掉 out/algorithms 下的 PNG 产物 (保留 README)
#   .\clean.ps1 out          # 只清 out/algorithms PNG
#   .\clean.ps1 cmake        # 只清 cmake 缓存与 CMakeFiles
#   .\clean.ps1 -DryRun      # 只打印会删什么, 不实际删
#
[CmdletBinding()]
param(
    [ValidateSet("build", "all", "out", "cmake")]
    [string]$Mode = "build",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $RepoRoot

function Rm-IfExists($path, $desc, $recurse = $true) {
    if (Test-Path $path) {
        $full = (Resolve-Path $path).Path
        if ($DryRun) {
            Write-Host "[DRY] $desc`: $full" -ForegroundColor DarkYellow
        } else {
            Write-Host "[DEL] $desc`: $full" -ForegroundColor Red
            if ($recurse) {
                Remove-Item -Recurse -Force $full -ErrorAction SilentlyContinue
            } else {
                Remove-Item -Force $full -ErrorAction SilentlyContinue
            }
        }
    }
}

Write-Host "==> clean.ps1  Mode=$Mode  DryRun=$DryRun  Root=$RepoRoot" -ForegroundColor Cyan

# ---- 公共: cmake 缓存散落在根的情况 ----
$cmakeJunk = @(
    "CMakeCache.txt", "CMakeFiles", "CMakeScripts", "Testing",
    "Makefile", "cmake_install.cmake", "install_manifest.txt",
    "compile_commands.json", "CTestTestfile.cmake",
    "_deps", "CMakeUserPresets.json", "CMakePresets.json"
)

if ($Mode -eq "build" -or $Mode -eq "cmake" -or $Mode -eq "all") {
    Write-Host ""
    Write-Host "--- 清理构建目录 build* ---"
    Get-ChildItem -Directory $RepoRoot -Filter "build*" | ForEach-Object {
        # 注意: mingw-build 是项目硬约束保留目录, 决不能碰
        if ($_.Name -like "mingw-build") { return }
        Rm-IfExists $_.FullName "build 目录"
    }
    # 散落在根的 CMake 垃圾 (用户直接 cmake . 会生成)
    Write-Host ""
    Write-Host "--- 清理 CMake 缓存 (散落根目录) ---"
    foreach ($j in $cmakeJunk) {
        Rm-IfExists (Join-Path $RepoRoot $j) "CMake 垃圾 $j"
    }
    # notes/image_process 内独立 build 目录
    Rm-IfExists (Join-Path $RepoRoot "notes\image_process\build")      "notes/imgproc build"
    Rm-IfExists (Join-Path $RepoRoot "notes\image_process\build_*")    "notes/imgproc build_*"
}

# ---- out/ 清理 ----
if ($Mode -eq "out" -or $Mode -eq "all") {
    Write-Host ""
    Write-Host "--- 清理 out/ 算法产物 (保留 README.md) ---"
    $outRoot = Join-Path $RepoRoot "out"
    if (Test-Path $outRoot) {
        # out/* 下除 README.md 的所有子项
        Get-ChildItem -Force $outRoot -Exclude "README.md" | ForEach-Object {
            if ($_.PSIsContainer -and $_.Name -eq "algorithms") {
                # algorithms/ 下保留 README.md, 其余全清
                Get-ChildItem -Force $_.FullName -Exclude "README.md" | ForEach-Object {
                    Rm-IfExists $_.FullName ("out/algorithms/" + $_.Name)
                }
            } else {
                Rm-IfExists $_.FullName ("out/" + $_.Name)
            }
        }
    }
}

Write-Host ""
Write-Host "==> Clean finished ($Mode)." -ForegroundColor Green
if ($DryRun) { Write-Host "    (DRY RUN - 上面只是打印, 实际未删除. 去掉 -DryRun 再运行可真删)" }
