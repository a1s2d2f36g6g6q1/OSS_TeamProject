#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "games.h"

#define GRID_WIDTH 10
#define GRID_HEIGHT 20

int grid[GRID_HEIGHT][GRID_WIDTH] = {0}; // 게임판 데이터
int current_tetromino[4][4];             // 현재 도형
int current_x = GRID_WIDTH / 2 - 2;      // 도형의 X 위치
int current_y = 0;                       // 도형의 Y 위치
int game_speed = 500;                    // 게임 속도 (ms)
int score_Tetris = 0;                    // 점수
bool game_over_Tetris = false;           // 게임 오버 상태
void clear_lines(GtkWidget* score_label);                      // 꽉 찬 줄 제거

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


// 점수 연동
void update_tetris_score(int score, GtkWidget* score_label) {
    char score_text[16];
    snprintf(score_text, sizeof(score_text), "Score: %d", score);
    gtk_label_set_text(GTK_LABEL(score_label), score_text);

    if (!is_guest_mode) {
        send_game_score(username, "tetris", score); // 서버 동기화
    }
}



// 충돌 검사
bool check_collision(int new_x, int new_y, int new_tetromino[4][4])
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (new_tetromino[y][x] == 1)
            {
                int grid_x = new_x + x;
                int grid_y = new_y + y;

                if (grid_x < 0 || grid_x >= GRID_WIDTH || grid_y >= GRID_HEIGHT || grid[grid_y][grid_x] != 0)
                {
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
    // 현재 블록을 다음 블록으로 대체
    memcpy(current_tetromino, next_tetromino, sizeof(current_tetromino));

    // 다음 블록 생성
    int random_index = rand() % 7;
    memcpy(next_tetromino, tetrominos[random_index], sizeof(next_tetromino));

    current_x = GRID_WIDTH / 2 - 2;
    current_y = 0;

    // 게임 오버 확인
    if (check_collision(current_x, current_y, current_tetromino)) {
        game_over_Tetris = true;
    }

    // 다음 블록 그리기 갱신
    if (GTK_IS_WIDGET(next_block_area)) {
        gtk_widget_queue_draw(next_block_area);
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
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            if (grid[y][x] != 0)
            {
                if (game_over_Tetris)
                {
                    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); // 빨간색
                }
                else
                {
                    cairo_set_source_rgb(cr, 0.0, 0.0, 0.5); // 짙은 파란색
                }
                cairo_rectangle(cr, x * cell_size, y * cell_size, cell_size - 1, cell_size - 1);
                cairo_fill(cr);
            }
        }
    }

    // 현재 도형
    if (!game_over_Tetris) // 게임 오버 시 현재 도형 그리지 않음
    {
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                if (current_tetromino[y][x] == 1)
                {
                    cairo_set_source_rgb(cr, 0.5, 0.8, 1.0); // 하늘색
                    cairo_rectangle(cr, (current_x + x) * cell_size, (current_y + y) * cell_size, cell_size - 1, cell_size - 1);
                    cairo_fill(cr);
                }
            }
        }
    }

    // 점수 표시
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, 10, 20);
    cairo_show_text(cr, g_strdup_printf("Score: %d", score_Tetris));

    // 게임 오버
    if (game_over_Tetris)
    {
        cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); // 빨간색
        cairo_set_font_size(cr, 40);
        cairo_move_to(cr, 50, 250);
        cairo_show_text(cr, "Game Over");
    }

    return FALSE;
}

// game_loop 수정
gboolean game_loop(GtkWidget* widget, gpointer data) {
    GtkWidget* score_label = GTK_WIDGET(data); // 전달된 score_label 가져오기

    if (game_over_Tetris)
        return FALSE; // 게임 루프 종료

    if (!check_collision(current_x, current_y + 1, current_tetromino)) {
        current_y++;
    } else {
        // 도형 고정
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (current_tetromino[y][x] == 1) {
                    grid[current_y + y][current_x + x] = 1;
                }
            }
        }

        // 줄 삭제 및 점수 업데이트
        clear_lines(score_label); // score_label 전달

        // 게임 오버 체크
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[0][x] != 0) {
                game_over_Tetris = true;
                break;
            }
        }

        if (!game_over_Tetris) {
            spawn_new_tetromino(); // 새 도형 생성
        }
    }

    gtk_widget_queue_draw(widget); // 화면 갱신
    return TRUE;
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

        clear_lines(score_label); // 줄 삭제 호출

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

void clear_lines(GtkWidget* score_label) {
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

    // 점수 계산 및 UI 업데이트
    if (cleared_lines > 0) {
        score_Tetris += cleared_lines * 100;  // 1줄 삭제당 100점
        char score_text[16];
        snprintf(score_text, sizeof(score_text), "Score: %d", score_Tetris);
        gtk_label_set_text(GTK_LABEL(score_label), score_text);  // UI 점수 갱신
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

void on_start_button_clicked(GtkWidget* widget, gpointer data) {
    GtkWidget* drawing_area = GTK_WIDGET(data);

    if (game_loop_id != 0) {
        g_source_remove(game_loop_id);
        game_loop_id = 0;
    }

    memset(grid, 0, sizeof(grid)); // 그리드 초기화
    score_Tetris = 0;
    game_over_Tetris = false;

    GtkWidget* score_label = g_object_get_data(G_OBJECT(drawing_area), "score_label");
    gtk_label_set_text(GTK_LABEL(score_label), "Score: 0");

    spawn_new_tetromino();
    gtk_widget_queue_draw(drawing_area);

    game_loop_id = g_timeout_add(game_speed, (GSourceFunc)game_loop, score_label); // game_loop에 score_label 전달
}

GtkWidget* create_tetris_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

    // Score Label 추가
    GtkWidget* score_label = gtk_label_new("Score: 0");
    gtk_box_pack_start(GTK_BOX(vbox), score_label, FALSE, FALSE, 5);

    // 게임 보드
    GtkWidget* drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 300, 600);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 5);

    // Drawing Area에 Score Label 저장
    g_object_set_data(G_OBJECT(drawing_area), "score_label", score_label);

    // Start 버튼
    GtkWidget* start_button = gtk_button_new_with_label("Start");
    gtk_box_pack_start(GTK_BOX(vbox), start_button, FALSE, FALSE, 5);
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_button_clicked), drawing_area);

    // Back to Main Menu 버튼
    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    gtk_box_pack_start(GTK_BOX(vbox), back_button, FALSE, FALSE, 5);
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);

    // Signal 연결
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_game_board), NULL);
    g_signal_connect(drawing_area, "key-press-event", G_CALLBACK(on_key_press), score_label);

    return vbox;
}

void start_tetris_game(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "tetris_screen");
}