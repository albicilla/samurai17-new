# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/albicilla/programming/samurAI/samurai17/player

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/albicilla/programming/samurAI/samurai17/player/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/player.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/player.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/player.dir/flags.make

CMakeFiles/player.dir/greedyC.cpp.o: CMakeFiles/player.dir/flags.make
CMakeFiles/player.dir/greedyC.cpp.o: ../greedyC.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/albicilla/programming/samurAI/samurai17/player/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/player.dir/greedyC.cpp.o"
	/usr/local/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/player.dir/greedyC.cpp.o -c /Users/albicilla/programming/samurAI/samurai17/player/greedyC.cpp

CMakeFiles/player.dir/greedyC.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/player.dir/greedyC.cpp.i"
	/usr/local/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/albicilla/programming/samurAI/samurai17/player/greedyC.cpp > CMakeFiles/player.dir/greedyC.cpp.i

CMakeFiles/player.dir/greedyC.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/player.dir/greedyC.cpp.s"
	/usr/local/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/albicilla/programming/samurAI/samurai17/player/greedyC.cpp -o CMakeFiles/player.dir/greedyC.cpp.s

CMakeFiles/player.dir/greedyC.cpp.o.requires:

.PHONY : CMakeFiles/player.dir/greedyC.cpp.o.requires

CMakeFiles/player.dir/greedyC.cpp.o.provides: CMakeFiles/player.dir/greedyC.cpp.o.requires
	$(MAKE) -f CMakeFiles/player.dir/build.make CMakeFiles/player.dir/greedyC.cpp.o.provides.build
.PHONY : CMakeFiles/player.dir/greedyC.cpp.o.provides

CMakeFiles/player.dir/greedyC.cpp.o.provides.build: CMakeFiles/player.dir/greedyC.cpp.o


CMakeFiles/player.dir/raceState.cpp.o: CMakeFiles/player.dir/flags.make
CMakeFiles/player.dir/raceState.cpp.o: ../raceState.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/albicilla/programming/samurAI/samurai17/player/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/player.dir/raceState.cpp.o"
	/usr/local/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/player.dir/raceState.cpp.o -c /Users/albicilla/programming/samurAI/samurai17/player/raceState.cpp

CMakeFiles/player.dir/raceState.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/player.dir/raceState.cpp.i"
	/usr/local/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/albicilla/programming/samurAI/samurai17/player/raceState.cpp > CMakeFiles/player.dir/raceState.cpp.i

CMakeFiles/player.dir/raceState.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/player.dir/raceState.cpp.s"
	/usr/local/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/albicilla/programming/samurAI/samurai17/player/raceState.cpp -o CMakeFiles/player.dir/raceState.cpp.s

CMakeFiles/player.dir/raceState.cpp.o.requires:

.PHONY : CMakeFiles/player.dir/raceState.cpp.o.requires

CMakeFiles/player.dir/raceState.cpp.o.provides: CMakeFiles/player.dir/raceState.cpp.o.requires
	$(MAKE) -f CMakeFiles/player.dir/build.make CMakeFiles/player.dir/raceState.cpp.o.provides.build
.PHONY : CMakeFiles/player.dir/raceState.cpp.o.provides

CMakeFiles/player.dir/raceState.cpp.o.provides.build: CMakeFiles/player.dir/raceState.cpp.o


# Object files for target player
player_OBJECTS = \
"CMakeFiles/player.dir/greedyC.cpp.o" \
"CMakeFiles/player.dir/raceState.cpp.o"

# External object files for target player
player_EXTERNAL_OBJECTS =

../beamZ/player: CMakeFiles/player.dir/greedyC.cpp.o
../beamZ/player: CMakeFiles/player.dir/raceState.cpp.o
../beamZ/player: CMakeFiles/player.dir/build.make
../beamZ/player: CMakeFiles/player.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/albicilla/programming/samurAI/samurai17/player/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../beamZ/player"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/player.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/player.dir/build: ../beamZ/player

.PHONY : CMakeFiles/player.dir/build

CMakeFiles/player.dir/requires: CMakeFiles/player.dir/greedyC.cpp.o.requires
CMakeFiles/player.dir/requires: CMakeFiles/player.dir/raceState.cpp.o.requires

.PHONY : CMakeFiles/player.dir/requires

CMakeFiles/player.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/player.dir/cmake_clean.cmake
.PHONY : CMakeFiles/player.dir/clean

CMakeFiles/player.dir/depend:
	cd /Users/albicilla/programming/samurAI/samurai17/player/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/albicilla/programming/samurAI/samurai17/player /Users/albicilla/programming/samurAI/samurai17/player /Users/albicilla/programming/samurAI/samurai17/player/cmake-build-debug /Users/albicilla/programming/samurAI/samurai17/player/cmake-build-debug /Users/albicilla/programming/samurAI/samurai17/player/cmake-build-debug/CMakeFiles/player.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/player.dir/depend
