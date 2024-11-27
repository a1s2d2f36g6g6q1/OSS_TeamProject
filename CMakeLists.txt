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

# 실행 파일 생성
add_compile_options(-Wno-incompatible-function-pointer-types -Wno-deprecated-declarations -Wno-return-type)
add_executable(MyCExecutable ${SOURCES})

# GTK+3, Cairo, Curl, JSON-C 찾기
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(JSONC REQUIRED json-c)

# include 디렉토리 설정
include_directories(
        ${GTK3_INCLUDE_DIRS}
        ${CAIRO_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
        ${JSONC_INCLUDE_DIRS}
        /opt/homebrew/opt/curl/include # libcurl 헤더 경로 수동 추가
)

# 링크 디렉토리 설정
link_directories(
        ${GTK3_LIBRARY_DIRS}
        ${CAIRO_LIBRARY_DIRS}
        ${CURL_LIBRARY_DIRS}
        ${JSONC_LIBRARY_DIRS}
        /opt/homebrew/opt/curl/lib # libcurl 라이브러리 경로 수동 추가
)

# 필요한 플래그 추가
add_definitions(${GTK3_CFLAGS_OTHER} ${CAIRO_CFLAGS_OTHER} ${CURL_CFLAGS_OTHER} ${JSONC_CFLAGS_OTHER})

# 링크 라이브러리 설정
target_link_libraries(MyCExecutable
        ${GTK3_LDFLAGS}
        ${CAIRO_LDFLAGS}
        ${CURL_LIBRARIES}
        ${JSONC_LIBRARIES}
        gtk-3 gdk-3 pangocairo-1.0 pango-1.0 harfbuzz atk-1.0 cairo-gobject
        gdk_pixbuf-2.0 gio-2.0 gobject-2.0 glib-2.0 intl
        /opt/homebrew/opt/curl/lib/libcurl.dylib # libcurl 수동 링크
)


