# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/youngmin/datalab/mySimul

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/youngmin/datalab/mySimul/build

# Include any dependencies generated for this target.
include lib/CMakeFiles/cxlEC.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include lib/CMakeFiles/cxlEC.dir/compiler_depend.make

# Include the progress variables for this target.
include lib/CMakeFiles/cxlEC.dir/progress.make

# Include the compile flags for this target's objects.
include lib/CMakeFiles/cxlEC.dir/flags.make

lib/CMakeFiles/cxlEC.dir/cxlEC.cpp.o: lib/CMakeFiles/cxlEC.dir/flags.make
lib/CMakeFiles/cxlEC.dir/cxlEC.cpp.o: ../lib/cxlEC.cpp
lib/CMakeFiles/cxlEC.dir/cxlEC.cpp.o: lib/CMakeFiles/cxlEC.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/youngmin/datalab/mySimul/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/CMakeFiles/cxlEC.dir/cxlEC.cpp.o"
	cd /home/youngmin/datalab/mySimul/build/lib && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT lib/CMakeFiles/cxlEC.dir/cxlEC.cpp.o -MF CMakeFiles/cxlEC.dir/cxlEC.cpp.o.d -o CMakeFiles/cxlEC.dir/cxlEC.cpp.o -c /home/youngmin/datalab/mySimul/lib/cxlEC.cpp

lib/CMakeFiles/cxlEC.dir/cxlEC.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cxlEC.dir/cxlEC.cpp.i"
	cd /home/youngmin/datalab/mySimul/build/lib && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/youngmin/datalab/mySimul/lib/cxlEC.cpp > CMakeFiles/cxlEC.dir/cxlEC.cpp.i

lib/CMakeFiles/cxlEC.dir/cxlEC.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cxlEC.dir/cxlEC.cpp.s"
	cd /home/youngmin/datalab/mySimul/build/lib && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/youngmin/datalab/mySimul/lib/cxlEC.cpp -o CMakeFiles/cxlEC.dir/cxlEC.cpp.s

lib/CMakeFiles/cxlEC.dir/switch.cpp.o: lib/CMakeFiles/cxlEC.dir/flags.make
lib/CMakeFiles/cxlEC.dir/switch.cpp.o: ../lib/switch.cpp
lib/CMakeFiles/cxlEC.dir/switch.cpp.o: lib/CMakeFiles/cxlEC.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/youngmin/datalab/mySimul/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object lib/CMakeFiles/cxlEC.dir/switch.cpp.o"
	cd /home/youngmin/datalab/mySimul/build/lib && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT lib/CMakeFiles/cxlEC.dir/switch.cpp.o -MF CMakeFiles/cxlEC.dir/switch.cpp.o.d -o CMakeFiles/cxlEC.dir/switch.cpp.o -c /home/youngmin/datalab/mySimul/lib/switch.cpp

lib/CMakeFiles/cxlEC.dir/switch.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cxlEC.dir/switch.cpp.i"
	cd /home/youngmin/datalab/mySimul/build/lib && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/youngmin/datalab/mySimul/lib/switch.cpp > CMakeFiles/cxlEC.dir/switch.cpp.i

lib/CMakeFiles/cxlEC.dir/switch.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cxlEC.dir/switch.cpp.s"
	cd /home/youngmin/datalab/mySimul/build/lib && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/youngmin/datalab/mySimul/lib/switch.cpp -o CMakeFiles/cxlEC.dir/switch.cpp.s

# Object files for target cxlEC
cxlEC_OBJECTS = \
"CMakeFiles/cxlEC.dir/cxlEC.cpp.o" \
"CMakeFiles/cxlEC.dir/switch.cpp.o"

# External object files for target cxlEC
cxlEC_EXTERNAL_OBJECTS =

lib/libcxlEC.a: lib/CMakeFiles/cxlEC.dir/cxlEC.cpp.o
lib/libcxlEC.a: lib/CMakeFiles/cxlEC.dir/switch.cpp.o
lib/libcxlEC.a: lib/CMakeFiles/cxlEC.dir/build.make
lib/libcxlEC.a: lib/CMakeFiles/cxlEC.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/youngmin/datalab/mySimul/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libcxlEC.a"
	cd /home/youngmin/datalab/mySimul/build/lib && $(CMAKE_COMMAND) -P CMakeFiles/cxlEC.dir/cmake_clean_target.cmake
	cd /home/youngmin/datalab/mySimul/build/lib && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cxlEC.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/CMakeFiles/cxlEC.dir/build: lib/libcxlEC.a
.PHONY : lib/CMakeFiles/cxlEC.dir/build

lib/CMakeFiles/cxlEC.dir/clean:
	cd /home/youngmin/datalab/mySimul/build/lib && $(CMAKE_COMMAND) -P CMakeFiles/cxlEC.dir/cmake_clean.cmake
.PHONY : lib/CMakeFiles/cxlEC.dir/clean

lib/CMakeFiles/cxlEC.dir/depend:
	cd /home/youngmin/datalab/mySimul/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/youngmin/datalab/mySimul /home/youngmin/datalab/mySimul/lib /home/youngmin/datalab/mySimul/build /home/youngmin/datalab/mySimul/build/lib /home/youngmin/datalab/mySimul/build/lib/CMakeFiles/cxlEC.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/CMakeFiles/cxlEC.dir/depend
