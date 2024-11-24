cmake_minimum_required(VERSION 3.15)
project(MyCProject C)

# C 표준 설정
set(CMAKE_C_STANDARD 99)

# 소스 파일 설정
set(SOURCES
        OSS_TeamProject/main.c
        OSS_TeamProject/Tetris.c
        OSS_TeamProject/2048.c
        OSS_TeamProject/BP.c
        OSS_TeamProject/mine.c
)

# 실행 파일 생성
add_executable(MyCExecutable ${SOURCES}
        OSS_TeamProject/main.c)

# pkg-config를 사용하여 GTK, Cairo, GLib 찾기
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)

# 헤더와 라이브러리 경로 추가
include_directories(${GTK3_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS} ${CAIRO_LIBRARY_DIRS} ${GLIB2_LIBRARY_DIRS} /opt/homebrew/lib)

# 컴파일 및 링커 플래그 설정
add_definitions(${GTK3_CFLAGS_OTHER} ${CAIRO_CFLAGS_OTHER} ${GLIB2_CFLAGS_OTHER})

# 라이브러리 연결
target_link_libraries(MyCExecutable ${GTK3_LIBRARIES} ${CAIRO_LIBRARIES} ${GLIB2_LIBRARIES})

# 디버깅용 메시지
message(STATUS "GTK3 Include dirs: ${GTK3_INCLUDE_DIRS}")
message(STATUS "GTK3 Library dirs: ${GTK3_LIBRARY_DIRS}")
message(STATUS "GTK3 Libraries: ${GTK3_LIBRARIES}")
message(STATUS "GLIB2 Include dirs: ${GLIB2_INCLUDE_DIRS}")
message(STATUS "GLIB2 Library dirs: ${GLIB2_LIBRARY_DIRS}")
message(STATUS "GLIB2 Libraries: ${GLIB2_LIBRARIES}")