@echo off
REM ===========================================================================
REM  build.bat - Parallel build helper
REM  Usage:  build.bat               (build all targets in parallel)
REM          build.bat target_name   (build a specific target)
REM          build.bat --clean       (clean and rebuild)
REM          build.bat --configure   (reconfigure cmake)
REM ===========================================================================

setlocal
set BUILD_DIR=%~dp0build
set CORES=%NUMBER_OF_PROCESSORS%

if /I "%~1"=="--clean" (
    echo [build] Cleaning...
    cmake --build "%BUILD_DIR%" -j %CORES% --clean-first
    goto :eof
)

if /I "%~1"=="--configure" (
    echo [build] Reconfiguring...
    cmake -S "%~dp0." -B "%BUILD_DIR%"
    goto :eof
)

if /I "%~1"=="--help" (
    echo Usage: build.bat [options]
    echo.
    echo Options:
    echo   target_name     Build only the specified target
    echo   --clean         Clean and rebuild
    echo   --configure     Reconfigure CMake before building
    echo   --help          Show this help
    echo.
    echo Default: build all targets in parallel (%CORES% cores)
    goto :eof
)

if "%~1"=="" (
    echo [build] cmake --build "%BUILD_DIR%" -j %CORES%
    cmake --build "%BUILD_DIR%" -j %CORES%
) else (
    echo [build] cmake --build "%BUILD_DIR%" -j %CORES% --target %*
    cmake --build "%BUILD_DIR%" -j %CORES% --target %*
)

if %ERRORLEVEL% neq 0 (
    echo [build] FAILED (error code %ERRORLEVEL%)
    exit /b %ERRORLEVEL%
)
echo [build] Done.