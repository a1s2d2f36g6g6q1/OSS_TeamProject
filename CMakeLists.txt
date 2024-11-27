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
        OSS_TeamProject/games.h
)

add_executable(MyCExecutable ${SOURCES})

# GTK+3 및 Cairo 찾기
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(CAIRO REQUIRED cairo)

# include 및 라이브러리 경로 설정
include_directories(${GTK3_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS} ${CURL_IN})
link_directories(${GTK3_LIBRARY_DIRS} ${CAIRO_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER} ${CAIRO_CFLAGS_OTHER})

# 추가된 curl 및 json-c 라이브러리 설정
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(JSONC REQUIRED json-c)

include_directories(${CURL_INCLUDE_DIRS} ${JSONC_INCLUDE_DIRS})
link_directories(${CURL_LIBRARY_DIRS} ${JSONC_LIBRARY_DIRS})

# Windows 환경 처리
if(WIN32)
    target_link_libraries(MyCExecutable ws2_32) # WinSock2 사용
endif()

# 모든 환경에서 공통으로 필요한 라이브러리 연결
target_link_libraries(MyCExecutable
        ${GTK3_LDFLAGS}
        ${CAIRO_LDFLAGS}
        ${CURL_LIBRARIES}
        ${JSONC_LIBRARIES}
        gtk-3 gdk-3 pangocairo-1.0 pango-1.0 harfbuzz atk-1.0 cairo-gobject
        gdk_pixbuf-2.0 gio-2.0 gobject-2.0 glib-2.0 intl
)

# 디버깅용 메시지
message(STATUS "GTK3 Include dirs: ${GTK3_INCLUDE_DIRS}")
message(STATUS "GTK3 Library dirs: ${GTK3_LIBRARY_DIRS}")
message(STATUS "GTK3 Libraries: ${GTK3_LIBRARIES}")
message(STATUS "CURL Include dirs: ${CURL_INCLUDE_DIRS}")
message(STATUS "CURL Libraries: ${CURL_LIBRARIES}")
message(STATUS "JSON-C Include dirs: ${JSONC_INCLUDE_DIRS}")
message(STATUS "JSON-C Libraries: ${JSONC_LIBRARIES}")