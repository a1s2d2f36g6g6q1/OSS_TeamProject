#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "games.h"

#define GRID_WIDTH 10
#define GRID_HEIGHT 20

GtkLabel* tetris_score_label;  // 전역 변수로 score_label 선언

int grid[GRID_HEIGHT][GRID_WIDTH] = {0}; // 게임판 데이터
int current_tetromino[4][4];             // 현재 도형
int current_x = GRID_WIDTH / 2 - 2;      // 도형의 X 위치
int current_y = 0;                       // 도형의 Y 위치
int game_speed = 500;                    // 게임 속도 (ms)
int score_Tetris = 0;                    // 점수
bool game_over_Tetris = false;           // 게임 오버 상태


static guint game_loop_id = 0;
GtkWidget* next_block_area = NULL;


int next_tetromino[4][4]; // 다음 블록 저장
int tetrominos[7][4][4] = {
    // I 블록
    {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    // O 블록
    {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    // T 블록
    {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    // S 블록
    {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    // Z 블록
    {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    // L 블록
    {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    // J 블록
    {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};


void clear_lines();                      // 꽉 찬 줄 제거
void update_tetris_score(int score);     // 점수 갱신
void spawn_new_tetromino();              // 새로운 도형 생성
bool check_collision(int new_x, int new_y, int new_tetromino[4][4]);  // 충돌 검사





// 충돌 검사
bool check_collision(int new_x, int new_y, int new_tetromino[4][4]) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (new_tetromino[y][x] == 1) {
                int grid_x = new_x + x;
                int grid_y = new_y + y;

                // 배열 경계 초과 검사
                if (grid_x < 0 || grid_x >= GRID_WIDTH || grid_y < 0 || grid_y >= GRID_HEIGHT) {
                    return true; // 충돌 발생
                }

                if (grid[grid_y][grid_x] != 0) {
                    return true; // 충돌 발생
                }
            }
        }
    }
    return false;
}

// 도형 회전
void rotate_tetromino()
{
    int rotated[4][4] = {0};

    // 시계 방향 회전
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            rotated[x][3 - y] = current_tetromino[y][x];
        }
    }

    if (!check_collision(current_x, current_y, rotated))
    {
        // 회전 반영
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                current_tetromino[y][x] = rotated[y][x];
            }
        }
    }
}

// 새로운 도형 생성
void spawn_new_tetromino() {
    memcpy(current_tetromino, next_tetromino, sizeof(current_tetromino));

    int random_index = rand() % 7;
    memcpy(next_tetromino, tetrominos[random_index], sizeof(next_tetromino));

    current_x = GRID_WIDTH / 2 - 2;
    current_y = 0;

    if (check_collision(current_x, current_y, current_tetromino)) {
        game_over_Tetris = true;
        return;
    }

    if (GTK_IS_WIDGET(next_block_area)) {
        gtk_widget_queue_draw(next_block_area); // 다음 블록 갱신
    } else {
        g_warning("spawn_new_tetromino: next_block_area가 유효하지 않습니다.");
    }
}

// draw_game_board 수정
gboolean draw_game_board(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    double cell_size = 30;

    // 배경
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // 고정된 블록
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x] != 0) {
                if (game_over_Tetris) {
                    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); // 빨간색
                } else {
                    cairo_set_source_rgb(cr, 0.0, 0.0, 0.5); // 짙은 파란색
                }
                cairo_rectangle(cr, x * cell_size, y * cell_size, cell_size - 1, cell_size - 1);
                cairo_fill(cr);
            }
        }
    }

    // 현재 도형
    if (!game_over_Tetris) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (current_tetromino[y][x] == 1) {
                    cairo_set_source_rgb(cr, 0.5, 0.8, 1.0); // 하늘색
                    cairo_rectangle(cr, (current_x + x) * cell_size, (current_y + y) * cell_size, cell_size - 1, cell_size - 1);
                    cairo_fill(cr);
                }
            }
        }
    }

    // 게임 오버 메시지
    if (game_over_Tetris) {
        cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); // 빨간색
        cairo_set_font_size(cr, 40);
        cairo_move_to(cr, 50, 250);
        cairo_show_text(cr, "Game Over");
    }

    return FALSE;
}


gboolean on_key_press(GtkWidget* widget, GdkEventKey* event, gpointer data) {
    GtkWidget* score_label = GTK_WIDGET(data); // 전달된 score_label 가져오기

    switch (event->keyval) {
    case GDK_KEY_Left: // 왼쪽 이동
        if (!check_collision(current_x - 1, current_y, current_tetromino)) {
            current_x--;
        }
        break;

    case GDK_KEY_Right: // 오른쪽 이동
        if (!check_collision(current_x + 1, current_y, current_tetromino)) {
            current_x++;
        }
        break;

    case GDK_KEY_Down: // 아래로 빠르게 이동
        if (!check_collision(current_x, current_y + 1, current_tetromino)) {
            current_y++;
        }

        // 도형을 고정하고 줄 삭제
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (current_tetromino[y][x] == 1) {
                    grid[current_y + y][current_x + x] = 1;
                }
            }
        }

        clear_lines(); // 줄 삭제 호출

        // 게임 오버 확인
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[0][x] != 0) {
                game_over_Tetris = true;
                break;
            }
        }

        // 새 도형 생성
        if (!game_over_Tetris) {
            spawn_new_tetromino();
        }
        break;

    case GDK_KEY_Up: // 도형 회전
        rotate_tetromino();
        break;
    }

    gtk_widget_queue_draw(widget); // 화면 갱신 요청
    return TRUE;
}


void update_tetris_score(int score) {
    if (!GTK_IS_LABEL(tetris_score_label)) {
        g_warning("update_tetris_score: tetris_score_label이 유효하지 않습니다!");
        return;
    }

    char score_text[16];
    snprintf(score_text, sizeof(score_text), "Score: %d", score);
    gtk_label_set_text(GTK_LABEL(tetris_score_label), score_text);

    // 서버와 점수 동기화
    if (username[0] != '\0' && !is_guest_mode) {
        g_message("점수 동기화: %d", score);
        send_game_score(username, "tetris", score);
    } else {
        g_message("점수 동기화 생략 (게스트 모드 활성화)");
    }
}



void clear_lines() {
    int cleared_lines = 0;

    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
        bool full_line = true;

        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x] == 0) {
                full_line = false;
                break;
            }
        }

        if (full_line) {
            cleared_lines++;
            for (int ny = y; ny > 0; ny--) {
                for (int nx = 0; nx < GRID_WIDTH; nx++) {
                    grid[ny][nx] = grid[ny - 1][nx];
                }
            }
            for (int nx = 0; nx < GRID_WIDTH; nx++) {
                grid[0][nx] = 0;
            }

            y++; // 한 줄 내려온 만큼 다시 검사
        }
    }

    if (cleared_lines > 0) {
        score_Tetris += cleared_lines * 100;  // 1줄 삭제당 100점
        update_tetris_score(score_Tetris);  // 점수 갱신
    }
}

gboolean draw_next_tetromino(GtkWidget* widget, cairo_t* cr, gpointer data) {
    double cell_size = 20;

    // 배경색
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // 다음 블록 그리기
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (next_tetromino[y][x] == 1) {
                cairo_set_source_rgb(cr, 0.5, 0.8, 1.0); // 하늘색
                cairo_rectangle(cr, x * cell_size, y * cell_size, cell_size - 1, cell_size - 1);
                cairo_fill(cr);
            }
        }
    }

    return FALSE;
}




void initialize_tetris_ui(GtkWidget* score_label_widget, GtkWidget* next_block_widget) {
    if (!GTK_IS_LABEL(score_label_widget)) {
        g_warning("initialize_tetris_ui: score_label_widget이 유효하지 않습니다!");
        return;
    }
    gtk_label_set_text(GTK_LABEL(score_label_widget), "Score: 0");

    if (!GTK_IS_WIDGET(next_block_widget)) {
        g_warning("initialize_tetris_ui: next_block_widget이 유효하지 않습니다!");
        return;
    }
    gtk_widget_queue_draw(next_block_widget);
}










gboolean game_loop(GtkWidget* widget, gpointer data) {
    // 게임 오버 상태면 종료
    if (game_over_Tetris) {
        return FALSE;
    }

    // 도형 이동
    if (!check_collision(current_x, current_y + 1, current_tetromino)) {
        current_y++;
    } else {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (current_tetromino[y][x] == 1) {
                    grid[current_y + y][current_x + x] = 1;
                }
            }
        }
        clear_lines();  // 줄 삭제 및 점수 갱신
        spawn_new_tetromino();

        if (game_over_Tetris) {
            if (GTK_IS_LABEL(tetris_score_label)) {
                gtk_label_set_text(GTK_LABEL(tetris_score_label), "Game Over");
            }
            return FALSE;
        }
    }

    gtk_widget_queue_draw(widget);
    return TRUE;
}

void on_start_button_clicked(GtkWidget* widget, gpointer data) {
    if (game_loop_id > 0) {
        g_source_remove(game_loop_id);  // 중복 타이머 제거
        game_loop_id = 0;
    }

    // 전역 변수 tetris_score_label을 사용
    if (!GTK_IS_LABEL(tetris_score_label)) {
        g_warning("on_start_button_clicked: tetris_score_label이 유효하지 않습니다!");
        return;
    }

    // UI 초기화
    initialize_tetris_ui(GTK_WIDGET(tetris_score_label), next_block_area);

    // 게임 상태 초기화
    memset(grid, 0, sizeof(grid));
    score_Tetris = 0;
    game_over_Tetris = false;

    // 새 도형 생성 및 게임 루프 시작
    spawn_new_tetromino();
    game_loop_id = g_timeout_add(game_speed, (GSourceFunc)game_loop, NULL);
}

void update_ui(GtkWidget* drawing_area, GtkWidget* score_label) {
    if (GTK_IS_WIDGET(drawing_area)) {
        gtk_widget_queue_draw(drawing_area);
    }

    if (GTK_IS_LABEL(score_label)) {
        char score_text[16];
        snprintf(score_text, sizeof(score_text), "Score: %d", score_Tetris);
        gtk_label_set_text(GTK_LABEL(score_label), score_text);
    }
}


GtkWidget* create_tetris_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

    // 점수 표시 라벨
    tetris_score_label = GTK_LABEL(gtk_label_new("Score: 0"));  // 전역 변수로 저장
    GtkWidget* score_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(score_container, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(score_container), GTK_WIDGET(tetris_score_label), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), score_container, FALSE, FALSE, 0);

    // 게임 보드
    GtkWidget* drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 300, 600);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 5);

    // 다음 블록 표시 영역
    next_block_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(next_block_area, 100, 100);
    gtk_box_pack_start(GTK_BOX(vbox), next_block_area, FALSE, FALSE, 5);
    g_signal_connect(next_block_area, "draw", G_CALLBACK(draw_next_tetromino), NULL);

    // Start 버튼
    GtkWidget* start_button = gtk_button_new_with_label("Start");
    gtk_box_pack_start(GTK_BOX(vbox), start_button, FALSE, FALSE, 5);
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_button_clicked), NULL);

    // Back 버튼
    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    gtk_box_pack_start(GTK_BOX(vbox), back_button, FALSE, FALSE, 5);
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_game_board), NULL);
    g_signal_connect(drawing_area, "key-press-event", G_CALLBACK(on_key_press), NULL);

    return vbox;
}

void start_tetris_game(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "tetris_screen");
}