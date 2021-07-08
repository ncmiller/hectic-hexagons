cmake_minimum_required(VERSION 3.0)

set(project_name hectic-hexagons)
project(${project_name})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(
    src/include
    ${SDL2_INCLUDE_DIR}
    ${SDL2_TTF_INCLUDE_DIR}
    ${SDL2_IMAGE_INCLUDE_DIR}
)

set(source_files
    src/main.c
    src/graphics.c
    src/input.c
    src/game.c
    src/text.c
    src/window.c
    src/time_now.c
    src/cursor.c
    src/hex.c
    src/constants.c
    src/statistics.c
)


set(CMAKE_C_FLAGS "-std=gnu11 -Wall -Werror -Wno-unused-function")

set(ENABLE_DEBUG 1)
if(ENABLE_DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
else() # Release
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
endif()

add_executable(${project_name} ${source_files})
target_link_libraries(${project_name} PRIVATE
    ${SDL2_LIBRARY}
    ${SDL2_IMAGE_LIBRARY}
    ${SDL2_TTF_LIBRARY}
    m
)
