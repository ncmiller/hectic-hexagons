cmake_minimum_required(VERSION 3.0)

set(project_name hectic-hexagons)
project(${project_name})

include_directories(src)

set(sources
    src/main.c
)
set(CMAKE_C_FLAGS "-std=c99 -Wall -Werror")

find_package(SDL2 REQUIRED)

add_executable(${project_name} ${sources})
target_include_directories(${project_name} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${project_name} PRIVATE ${SDL2_LIBRARIES})
