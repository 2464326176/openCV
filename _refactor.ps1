$ErrorActionPreference = 'Stop'
$root = "d:\visionProcess\githubDesktop\openCV"

# 1. 收集待处理文件: notes/ learn/ algorithms/ docs/ common/ 以及根目录 OUTLINE.md/README.md
$files = @()
foreach ($d in @("notes", "learn", "algorithms", "docs", "common")) {
    $files += Get-ChildItem -Path "$root\$d" -Recurse -File -Include "*.md", "*.cpp", "*.h", "*.hpp", "*.json", "*.py"
}
foreach ($n in @("OUTLINE.md", "README.md", "main.cpp", "makefile.sh", "CMakeLists.txt")) {
    $p = Join-Path $root $n
    if (Test-Path $p) { $files += Get-Item $p }
}

# 2. (regex_pattern, replacement) pairs - all case-sensitive (-creplace)
# Order matters: specific/compound first, generic/bare last.
$pairs = @(
    # ---------- CMake option names (compound, BEFORE bare names) ----------
    @{old='BUILD_OPEN_CV_MAIN';    new='BUILD_MAIN'},
    @{old='BUILD_LEARN_OPENCV';    new='BUILD_LEARN'},
    @{old='BUILD_OPENCV_ALGO_DEV'; new='BUILD_ALGORITHMS'},

    # ---------- build_algo / opencvAlgoDev runtime path (compound first) ----------
    # 老的运行时路径 build_algo/opencvAlgoDev 已重命名为 out/algorithms
    @{old='build_algo\\opencvAlgoDev'; new='out\algorithms'},
    @{old='build_algo/opencvAlgoDev';  new='out/algorithms'},
    @{old='opencvAlgoDev/output_algo/'; new='out/algorithms/'},
    @{old='\.\./output_algo/';          new='../out/algorithms/'},
    @{old='output_algo/';               new='out/algorithms/'},

    # ---------- C++ 文件名 (specific, BEFORE folder-slash patterns) ----------
    @{old='facedetect\.cpp';                  new='face_detect.cpp'},
    @{old='singleFrameProcess\.cpp';          new='single_frame_process.cpp'},
    @{old='imageProcess\.cpp';                new='image_process.cpp'},
    @{old='cornerHarris\.cpp';                new='corner_harris.cpp'},
    @{old='basicLinerTransformsTrackbar\.cpp';new='basic_linear_transforms_trackbar.cpp'},
    @{old='calcHist\.cpp';                    new='calc_hist.cpp'},
    @{old='compareHist\.cpp';                 new='compare_hist.cpp'},
    @{old='equalizeHist\.cpp';                new='equalize_hist.cpp'},
    @{old='hsHist\.cpp';                      new='hs_hist.cpp'},
    @{old='matchTemplate\.cpp';               new='match_template.cpp'},
    @{old='SURF_detection\.cpp';              new='surf_detection.cpp'},
    @{old='addImage\.cpp';                    new='add_image.cpp'},
    @{old='colorReduce\.cpp';                 new='color_reduce.cpp'},
    @{old='displayImage\.cpp';                new='display_image.cpp'},
    @{old='readImg\.cpp';                     new='read_image.cpp'},
    @{old='regionOfInterest\.cpp';            new='region_of_interest.cpp'},
    @{old='splitMerge\.cpp';                  new='split_merge.cpp'},
    @{old='matOperate\.cpp';                  new='mat_operate.cpp'},
    @{old='imageOperation1\.cpp';             new='image_operation1.cpp'},
    @{old='imageOperation2\.cpp';             new='image_operation2.cpp'},
    @{old='filterProcess\.cpp';               new='filter_process.cpp'},
    @{old='morphOpProcess\.cpp';              new='morph_op_process.cpp'},
    @{old='findOutline\.cpp';                 new='find_outline.cpp'},
    # case-sensitive (避免匹配 OpenCV 自带 imageSegmentation.cpp)
    @{old='\bSegmentation\.cpp';              new='segmentation.cpp'},

    # ---------- 头文件 include (代码引用, 必须改) ----------
    @{old='<opencvUnits\.h>';    new='<opencv_utils.h>'},
    @{old='"opencvUnits\.h"';    new='"opencv_utils.h"'},

    # ---------- 单位工具源文件 (units/opencvUnits.cpp) ----------
    @{old='opencvUnits\.cpp';   new='opencv_utils.cpp'},

    # ---------- 文件夹路径 (带尾斜杠; 涵盖路径与链接文本中的子路径) ----------
    @{old='imageProcess/';        new='image_process/'},
    @{old='imageTransformation/'; new='image_transformation/'},
    @{old='imageSegmentation/';   new='image_segmentation/'},
    @{old='histogramsMatch/';     new='histogram_match/'},
    @{old='highGui/';             new='highgui/'},
    @{old='detectHarris/';        new='harris_detect/'},
    @{old='facedetect/';          new='face_detect/'},
    @{old='features2D/';          new='features2d/'},
    @{old='basicDrawing/';        new='basic_drawing/'},
    @{old='imageAlgo/';           new='image_algo/'},

    # ---------- 链接文本 [name] ----------
    @{old='\[imageProcess\]';        new='[image_process]'},
    @{old='\[imageTransformation\]'; new='[image_transformation]'},
    @{old='\[imageSegmentation\]';   new='[image_segmentation]'},
    @{old='\[histogramsMatch\]';     new='[histogram_match]'},
    @{old='\[highGui\]';             new='[highgui]'},
    @{old='\[detectHarris\]';        new='[harris_detect]'},
    @{old='\[facedetect\]';          new='[face_detect]'},
    @{old='\[features2D\]';          new='[features2d]'},
    @{old='\[basicDrawing\]';        new='[basic_drawing]'},
    @{old='\[imageAlgo\]';           new='[image_algo]'},

    # ---------- tutorialDoc → docs ----------
    @{old='tutorialDoc/normalize_docs\.py'; new='docs/scripts/normalize_docs.py'},
    @{old='tutorialDoc/';                   new='docs/'},
    @{old='tutorialDoc';                    new='docs'},

    # ---------- units/opencvUnits → common/opencv_utils (header path) ----------
    @{old='units/opencvUnits\.h';   new='common/opencv_utils.h'},

    # ---------- 顶层文件夹名 (带尾斜杠, 在 bare 名之前) ----------
    @{old='learnOpenCV/';      new='learn/'},
    @{old='opencvAlgoDev/';    new='algorithms/'},

    # ---------- bare 名 (最后处理) ----------
    @{old='learnOpenCV';   new='learn'},
    @{old='opencvAlgoDev'; new='algorithms'},

    # ---------- build_algo (bare, 最后) ----------
    @{old='build_algo';    new='out'}
)

# 3. 批量替换
$count = 0
$changed = @()
foreach ($file in $files) {
    $content = Get-Content $file.FullName -Raw
    if ($null -eq $content) { continue }
    $original = $content
    foreach ($pair in $pairs) {
        $content = $content -creplace $pair.old, $pair.new
    }
    if ($content -ne $original) {
        [System.IO.File]::WriteAllText($file.FullName, $content, (New-Object System.Text.UTF8Encoding $false))
        $count++
        $rel = $file.FullName.Substring($root.Length + 1)
        $changed += $rel
    }
}
Write-Host "Total files updated: $count"
$changed | ForEach-Object { Write-Host "  - $_" }
