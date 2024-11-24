cmake_minimum_required(VERSION 3.15)
project(MyCProject C)

set(CMAKE_C_STANDARD 99)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(CAIRO REQUIRED cairo)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

add_executable(MyCExecutable main.c
    Tetris.c
    2048.c
    mine.c
    BP.c
        OSS_TeamProject/Tetris.c
)

target_link_libraries(MyCExecutable ${GTK3_LIBRARIES} ${CAIRO_LIBRARIES})