cmake_minimum_required(VERSION 3.15)
project(OSS_TeamProject C)

set(CMAKE_C_STANDARD 99)

# 소스 파일 정의
set(SOURCES
        OSS_TeamProject/main.c
        OSS_TeamProject/2048.c
        OSS_TeamProject/BP.c
        OSS_TeamProject/Tetris.c
        OSS_TeamProject/mine.c
)

add_executable(MyCExecutable ${SOURCES})

# GTK+3 및 Cairo 찾기
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(CAIRO REQUIRED cairo)

# include 및 라이브러리 경로 설정
include_directories(${GTK3_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS} ${CAIRO_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER} ${CAIRO_CFLAGS_OTHER})

# 라이브러리 연결
target_link_libraries(MyCExecutable ${GTK3_LIBRARIES} ${CAIRO_LIBRARIES})

# 디버깅용 메시지
message(STATUS "GTK3 Include dirs: ${GTK3_INCLUDE_DIRS}")
message(STATUS "GTK3 Library dirs: ${GTK3_LIBRARY_DIRS}")
message(STATUS "GTK3 Libraries: ${GTK3_LIBRARIES}")