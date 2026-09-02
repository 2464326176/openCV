#!/usr/bin/env bash
#
# build.sh — Linux one-stop build/clean script (bash, mirrors Windows build.ps1)
#
# Usage:
#   ./build.sh                                  # default: Release, build main only
#   ./build.sh main                             # same as above, explicit
#   ./build.sh learn L2                         # specify layer (L0..L5 / ALL)
#   ./build.sh algorithms ALL                   # or semicolon list, e.g. hdr;denoise_single
#   ./build.sh notes                            # build notes/image_process (via root BUILD_NOTES)
#   ./build.sh all                              # main + learn(ALL) + algorithms(ALL) + notes
#   ./build.sh build main L2 ALL Release        # positional form equivalent to above
#   NO_BUILD=1 ./build.sh all                   # cmake configure only, skip build
#
#   ./build.sh clean                            # remove build/ + CMake caches + obsolete notes leftovers (keep out/)
#   ./build.sh clean all                        # also clean out/ algorithm outputs (keep README.md)
#   ./build.sh clean out                        # clean out/ outputs only
#   ./build.sh clean cmake                      # clean CMake caches only
#   DRY_RUN=1 ./build.sh clean                  # print what would be deleted, do not actually delete
#
# Conventions:
#   - Build dirs live under build/ as per-target sub-directories:
#       build/main | build/learn | build/algorithms | build/notes
#   - Uses the Unix Makefiles generator, auto -j $(nproc)
#   - OpenCV lookup: see CMakeLists.txt (user -DOpenCV_DIR > local mingw-build > system find_package)
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_ROOT"

# ---- Action parsing ----
ACTION="build"
if [ "${1:-}" = "clean" ]; then
    ACTION="clean"; shift || true
elif [ "${1:-}" = "build" ]; then
    shift || true
fi

# ============================ CLEAN ============================
if [ "$ACTION" = "clean" ]; then
    MODE="${1:-build}"
    DRY_RUN="${DRY_RUN:-0}"

    rm_if_exists() {
        local path="$1"
        if [ -e "$path" ]; then
            if [ "$DRY_RUN" = "1" ]; then
                echo "[DRY] $path"
            else
                echo "[DEL] $path"
                rm -rf "$path"
            fi
        fi
    }

    echo "==> build.sh clean  Mode=$MODE  DryRun=$DRY_RUN  Root=$REPO_ROOT"

    CMAKE_JUNK=(CMakeCache.txt CMakeFiles CMakeScripts Testing Makefile cmake_install.cmake
                install_manifest.txt compile_commands.json CTestTestfile.cmake _deps
                CMakeUserPresets.json CMakePresets.json)

    if [ "$MODE" = "build" ] || [ "$MODE" = "cmake" ] || [ "$MODE" = "all" ]; then
        echo ""
        echo "--- Cleaning build dirs (build*) ---"
        # Safety: mingw-build holds the project's bundled OpenCV libs; never touch it.
        # (It does not start with "build", so the build* glob already skips it.)
        for d in build build_*; do
            [ -e "$d" ] || continue
            case "$d" in mingw-build*) continue ;; esac
            rm_if_exists "$d"
        done
        echo ""
        echo "--- Cleaning CMake caches (scattered in root) ---"
        for j in "${CMAKE_JUNK[@]}"; do
            rm_if_exists "$j"
        done
        # Obsolete standalone leftovers: build dirs inside notes/image_process
        rm_if_exists notes/image_process/build
        rm_if_exists notes/image_process/build_
    fi

    if [ "$MODE" = "out" ] || [ "$MODE" = "all" ]; then
        echo ""
        echo "--- Cleaning out/ algorithm outputs (keep README.md) ---"
        if [ -d out ]; then
            if [ "$DRY_RUN" = "1" ]; then
                find out -mindepth 1 ! -name README.md -print
            else
                find out -mindepth 1 ! -name README.md -exec rm -rf {} +
            fi
        fi
    fi

    echo ""
    echo "==> Clean finished ($MODE)."
    if [ "$DRY_RUN" = "1" ]; then echo "    (DRY RUN - printed only, nothing deleted. Re-run without DRY_RUN=1 to actually delete)"; fi
    exit 0
fi

# ============================ BUILD ============================
TARGET="${1:-main}"        # main | learn | algorithms | notes | all
LAYER="${2:-ALL}"          # ALL | L0 | L1 | L2 | L3 | L4 | L5
MODULE="${3:-ALL}"         # ALL or semicolon-separated list
CONFIG="${4:-Release}"

CMAKE_BIN="$(command -v cmake || true)"
if [ -z "$CMAKE_BIN" ]; then echo "ERROR: cmake not installed or not in PATH" >&2; exit 1; fi
if ! command -v make >/dev/null 2>&1; then echo "WARNING: make not in PATH, Unix Makefiles generator may fail" >&2; fi

GENERATOR="Unix Makefiles"
JOBS="$(nproc 2>/dev/null || echo 4)"
NO_BUILD="${NO_BUILD:-0}"

invoke_cmake() {
    local dir="$1"; shift
    echo ""
    echo "==> Configure: cmake -B $dir -G \"$GENERATOR\" $*"
    cmake -B "$dir" -G "$GENERATOR" "$@"
    if [ "$NO_BUILD" = "1" ]; then echo "==> NO_BUILD set, skipping build"; return; fi
    echo "==> Build: cmake --build $dir -j $JOBS ($CONFIG)"
    cmake --build "$dir" -j "$JOBS" --config "$CONFIG"
}

COMMON=(-DCMAKE_BUILD_TYPE="$CONFIG")

# --- main ---
if [ "$TARGET" = "main" ] || [ "$TARGET" = "all" ]; then
    invoke_cmake build/main "${COMMON[@]}" \
        -DBUILD_MAIN=ON -DBUILD_LEARN=OFF -DBUILD_ALGORITHMS=OFF -DBUILD_NOTES=OFF
    echo "  -> exe: build/main/openCv"
fi

# --- learn ---
if [ "$TARGET" = "learn" ] || [ "$TARGET" = "all" ]; then
    invoke_cmake "build/learn" "${COMMON[@]}" \
        -DBUILD_MAIN=OFF -DBUILD_LEARN=ON -DLEARN_LAYER="$LAYER" \
        -DBUILD_ALGORITHMS=OFF -DBUILD_NOTES=OFF
    echo "  -> exes in: build/learn/<stem>   (each = a .cpp filename)"
fi

# --- algorithms ---
if [ "$TARGET" = "algorithms" ] || [ "$TARGET" = "all" ]; then
    invoke_cmake "build/algorithms" "${COMMON[@]}" \
        -DBUILD_MAIN=OFF -DBUILD_LEARN=OFF -DBUILD_ALGORITHMS=ON \
        -DALGO_MODULE="$MODULE" -DBUILD_NOTES=OFF
    echo "  -> exes in: build/algorithms/<module>   (each = <module>.cpp filename)"
    echo "  -> output : out/algorithms/*.png (generated after running)"
fi

# --- notes ---
if [ "$TARGET" = "notes" ] || [ "$TARGET" = "all" ]; then
    invoke_cmake build/notes "${COMMON[@]}" \
        -DBUILD_MAIN=OFF -DBUILD_LEARN=OFF -DBUILD_ALGORITHMS=OFF -DBUILD_NOTES=ON
    echo "  -> exes in: build/notes/<stem>   (each = a .cpp filename)"
fi

echo ""
echo "==> Build finished successfully. Target=$TARGET Config=$CONFIG"
echo "    To run an executable, cd into the matching build/<sub>/ dir first so the relative path ../../data resolves correctly"
