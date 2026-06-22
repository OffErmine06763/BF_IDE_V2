cmake_minimum_required(VERSION 3.15)
project(BF_Compiler LANGUAGES CXX)

# Set standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files
file(GLOB_RECURSE PROJECT_FILES "Src/*.cpp" "Src/*.h" "Src/*.hpp" "MainComp.cpp")
add_executable(${PROJECT_NAME} ${PROJECT_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE Src)

# Platform-specific settings
if (WIN32)
	message(STATUS "Configuring for Windows...")

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

	# GCC/Clang warnings
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

	# Create Scripts dir in the build dir
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory
			$<TARGET_FILE_DIR:${PROJECT_NAME}>/Scripts
	)
	# Move the script files there
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_CURRENT_SOURCE_DIR}/Scripts/CheckRequirements.sh
			${CMAKE_CURRENT_SOURCE_DIR}/Scripts/Assemble.sh
			${CMAKE_CURRENT_SOURCE_DIR}/Scripts/LinkObj.sh
			$<TARGET_FILE_DIR:${PROJECT_NAME}>/Scripts
	)
	# Make them executable
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND chmod +x
	        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Scripts/CheckRequirements.sh
	        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Scripts/Assemble.sh
	        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Scripts/LinkObj.sh
	)
endif()
