@echo off
REM ===========================================================================
REM  build.bat - Parallel build helper
REM  Usage:  build.bat               (build all targets in parallel)
REM          build.bat target_name   (build a specific target)
REM          build.bat learn         (group targets: main/learn/algorithms/notes)
REM          build.bat --list        (list all available targets)
REM          build.bat --clean       (clean and rebuild)
REM          build.bat --configure   (reconfigure cmake)
REM          build.bat --help        (show help)
REM ===========================================================================

setlocal
set BUILD_DIR=%~dp0build
set CORES=%NUMBER_OF_PROCESSORS%

if /I "%~1"=="--configure" (
    echo [build] Reconfiguring...
    cmake -S "%~dp0." -B "%BUILD_DIR%"
    goto :done
)

REM Smart auto-configure: run cmake if the build dir is missing or stale
if not exist "%BUILD_DIR%\Makefile" (
    echo [build] Build dir missing or not configured - configuring first...
    cmake -S "%~dp0." -B "%BUILD_DIR%"
    if errorlevel 1 exit /b 1
)

if /I "%~1"=="--help" (
    echo Usage: build.bat [options]
    echo.
    echo Options:
    echo   target_name     Build only the specified target(s)
    echo   learn ^| algorithms ^| notes ^| main    Build a whole group
    echo   --list          List all available targets
    echo   --clean         Clean and rebuild
    echo   --configure     Reconfigure CMake before building
    echo   --help          Show this help
    echo.
    echo Default: build all targets in parallel (%CORES% cores)
    goto :eof
)

if /I "%~1"=="--list" (
    cmake --build "%BUILD_DIR%" --target help
    goto :eof
)

if /I "%~1"=="--clean" (
    echo [build] Cleaning...
    cmake --build "%BUILD_DIR%" -j %CORES% --clean-first
    goto :done
)

if "%~1"=="" (
    echo [build] cmake --build "%BUILD_DIR%" -j %CORES%
    cmake --build "%BUILD_DIR%" -j %CORES%
) else (
    echo [build] cmake --build "%BUILD_DIR%" -j %CORES% --target %*
    cmake --build "%BUILD_DIR%" -j %CORES% --target %*
)

:done
if errorlevel 1 (
    echo [build] FAILED
    exit /b %errorlevel%
)
echo [build] Done.
