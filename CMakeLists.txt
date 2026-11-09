cmake_minimum_required(VERSION 3.16)
project(chip8 C)

find_package(SDL3 REQUIRED CONFIG)

add_executable(chip8 main.c chip.c display.c)
target_link_libraries(chip8 PRIVATE SDL3::SDL3)

# Copy SDL3.dll next to the exe after each build
add_custom_command(TARGET chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:chip8>)
