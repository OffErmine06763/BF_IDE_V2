cmake_minimum_required(VERSION 3.15)
project(BF_Compiler LANGUAGES CXX)

# Set standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files
file(GLOB_RECURSE PROJECT_FILES "Src/*.cpp" "Src/*.h" "Src/*.hpp")
add_executable(${PROJECT_NAME} ${PROJECT_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE Src)

# Platform-specific settings
if (WIN32)
	message(STATUS "Configuring for Windows...")

	# Optional: Set Windows-specific compile definitions
	target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_WINDOWS)

	# Optional: Set MSVC flags
	if (MSVC)
		target_compile_options(${PROJECT_NAME} PRIVATE /W4)
	endif()

	if (CMAKE_GENERATOR MATCHES "Visual Studio")
		# Set startup project
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
		# Group by folder automatically in VS filters
		source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files" FILES ${PROJECT_FILES})
	endif()
elseif (UNIX)
	message(STATUS "Configuring for Linux...")

	target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_LINUX)

	# GCC/Clang warnings
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
