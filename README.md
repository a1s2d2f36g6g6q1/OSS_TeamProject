# OSS_TeamProject

- Mac Clion -> CLion -> Settings -> Build, Execution, Deployment -> CMake
  - Build Type, Debug 또는 Release 선택 (Release 로 해야 직접 실행 가능)
- Toolchains -> Cmake, Make, Debugger 확인
- CMakeLists.txt 파일에서 add_executable(파일명, 소스파일명) 추가


gcc로 강제 컴파일
```
gcc -o OSS_TeamProject_executable \
./OSS_TeamProject/2048.c \
./OSS_TeamProject/BP.c \
./OSS_TeamProject/Tetris.c \
./OSS_TeamProject/login.c \
./OSS_TeamProject/main.c \
./OSS_TeamProject/mine.c \
./OSS_TeamProject/ScoreBoard.c \
./OSS_TeamProject/setting.c \
./OSS_TeamProject/sign_up.c \
-I./OSS_TeamProject \
-I/opt/homebrew/Cellar/curl/8.11.0_1/include/curl \
-L/opt/homebrew/Cellar/curl/8.11.0_1/lib \
-lcurl \
`pkg-config --cflags --libs gtk+-3.0 json-c`
```

이후 실행
```
./OSS_TeamProject_executable
```