# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.18

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = E:\lyh\cmake-3.18.1-win64-x64\bin\cmake.exe

# The command to remove a file.
RM = E:\lyh\cmake-3.18.1-win64-x64\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = E:\lyh\opencv\opencv\sources

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:\lyh\opencv\opencv\mingw-build

# Include any dependencies generated for this target.
include apps/annotation/CMakeFiles/opencv_annotation.dir/depend.make

# Include the progress variables for this target.
include apps/annotation/CMakeFiles/opencv_annotation.dir/progress.make

# Include the compile flags for this target's objects.
include apps/annotation/CMakeFiles/opencv_annotation.dir/flags.make

apps/annotation/CMakeFiles/opencv_annotation.dir/opencv_annotation.cpp.obj: apps/annotation/CMakeFiles/opencv_annotation.dir/flags.make
apps/annotation/CMakeFiles/opencv_annotation.dir/opencv_annotation.cpp.obj: apps/annotation/CMakeFiles/opencv_annotation.dir/includes_CXX.rsp
apps/annotation/CMakeFiles/opencv_annotation.dir/opencv_annotation.cpp.obj: E:/lyh/opencv/opencv/sources/apps/annotation/opencv_annotation.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\lyh\opencv\opencv\mingw-build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object apps/annotation/CMakeFiles/opencv_annotation.dir/opencv_annotation.cpp.obj"
	cd /d E:\lyh\opencv\opencv\mingw-build\apps\annotation && C:\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\opencv_annotation.dir\opencv_annotation.cpp.obj -c E:\lyh\opencv\opencv\sources\apps\annotation\opencv_annotation.cpp

apps/annotation/CMakeFiles/opencv_annotation.dir/opencv_annotation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/opencv_annotation.dir/opencv_annotation.cpp.i"
	cd /d E:\lyh\opencv\opencv\mingw-build\apps\annotation && C:\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\lyh\opencv\opencv\sources\apps\annotation\opencv_annotation.cpp > CMakeFiles\opencv_annotation.dir\opencv_annotation.cpp.i

apps/annotation/CMakeFiles/opencv_annotation.dir/opencv_annotation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/opencv_annotation.dir/opencv_annotation.cpp.s"
	cd /d E:\lyh\opencv\opencv\mingw-build\apps\annotation && C:\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\lyh\opencv\opencv\sources\apps\annotation\opencv_annotation.cpp -o CMakeFiles\opencv_annotation.dir\opencv_annotation.cpp.s

# Object files for target opencv_annotation
opencv_annotation_OBJECTS = \
"CMakeFiles/opencv_annotation.dir/opencv_annotation.cpp.obj"

# External object files for target opencv_annotation
opencv_annotation_EXTERNAL_OBJECTS =

bin/opencv_annotation.exe: apps/annotation/CMakeFiles/opencv_annotation.dir/opencv_annotation.cpp.obj
bin/opencv_annotation.exe: apps/annotation/CMakeFiles/opencv_annotation.dir/build.make
bin/opencv_annotation.exe: lib/libopencv_highgui452.dll.a
bin/opencv_annotation.exe: lib/libopencv_videoio452.dll.a
bin/opencv_annotation.exe: lib/libopencv_imgcodecs452.dll.a
bin/opencv_annotation.exe: lib/libopencv_imgproc452.dll.a
bin/opencv_annotation.exe: lib/libopencv_core452.dll.a
bin/opencv_annotation.exe: apps/annotation/CMakeFiles/opencv_annotation.dir/linklibs.rsp
bin/opencv_annotation.exe: apps/annotation/CMakeFiles/opencv_annotation.dir/objects1.rsp
bin/opencv_annotation.exe: apps/annotation/CMakeFiles/opencv_annotation.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=E:\lyh\opencv\opencv\mingw-build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ..\..\bin\opencv_annotation.exe"
	cd /d E:\lyh\opencv\opencv\mingw-build\apps\annotation && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\opencv_annotation.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
apps/annotation/CMakeFiles/opencv_annotation.dir/build: bin/opencv_annotation.exe

.PHONY : apps/annotation/CMakeFiles/opencv_annotation.dir/build

apps/annotation/CMakeFiles/opencv_annotation.dir/clean:
	cd /d E:\lyh\opencv\opencv\mingw-build\apps\annotation && $(CMAKE_COMMAND) -P CMakeFiles\opencv_annotation.dir\cmake_clean.cmake
.PHONY : apps/annotation/CMakeFiles/opencv_annotation.dir/clean

apps/annotation/CMakeFiles/opencv_annotation.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" E:\lyh\opencv\opencv\sources E:\lyh\opencv\opencv\sources\apps\annotation E:\lyh\opencv\opencv\mingw-build E:\lyh\opencv\opencv\mingw-build\apps\annotation E:\lyh\opencv\opencv\mingw-build\apps\annotation\CMakeFiles\opencv_annotation.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : apps/annotation/CMakeFiles/opencv_annotation.dir/depend

