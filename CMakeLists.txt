# CMake version
cmake_minimum_required(VERSION 3.21)

# C++ standard version
set(CMAKE_CXX_STANDARD 20)

# updating git submodules (external libraries)
message(STATUS "updating submodules")
execute_process(COMMAND git submodule update --init --recursive
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_RESULT
)
if(NOT GIT_RESULT EQUAL "0")
    message(FATAL_ERROR "Failed to update submodules, ${GIT_RESULT}")
endif()

set(PROJECT_NAME pbr)

# initiating project given its name
project(${PROJECT_NAME})

add_subdirectory(internal)
add_subdirectory(src)



# build events
add_custom_command(TARGET ${BUILD_NAME}
    POST_BUILD
    COMMAND
)


# installing build (using cmake --install command)
install(TARGETS ${BUILD_NAME})
# installing the include files
install(
    DIRECTORY include/
    DESTINATION include
    FILES_MATCHING
    PATTERN *.h
    PATTERN *.hpp
    PATTERN *.inl
)