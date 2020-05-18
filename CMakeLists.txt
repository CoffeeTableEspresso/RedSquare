cmake_minimum_required(VERSION 3.10.2)
project(sdl-example)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "-Werror")

add_executable(sdl-example
        main.c player.c player.h constants.h vect.c vect.h vect.c)

target_link_libraries(sdl-example SDL2)
target_link_libraries(sdl-example SDL2_image)
target_link_libraries(sdl-example SDL2_ttf)
target_link_libraries(sdl-example yaslapi)
target_link_libraries(sdl-example m)
