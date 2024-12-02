#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define GRID_WIDTH 10
#define GRID_HEIGHT 20

int grid[GRID_HEIGHT][GRID_WIDTH] = {0}; // 게임판 데이터
int current_tetromino[4][4];             // 현재 도형
int current_x = GRID_WIDTH / 2 - 2;      // 도형의 X 위치
int current_y = 0;                       // 도형의 Y 위치
int game_speed = 500;                    // 게임 속도 (ms)
int score_Tetris = 0;                    // 점수
bool game_over_Tetris = false;           // 게임 오버 상태
void clear_lines();                      // 꽉 찬 줄 제거

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
void spawn_new_tetromino()
{
    int random_index = rand() % 7;
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            current_tetromino[y][x] = tetrominos[random_index][y][x];
        }
    }
    current_x = GRID_WIDTH / 2 - 2;
    current_y = 0;

    // 게임 오버 확인
    if (check_collision(current_x, current_y, current_tetromino))
    {
        game_over_Tetris = true;
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
gboolean game_loop(GtkWidget *widget)
{
    if (game_over_Tetris)
        return FALSE; // 게임 루프 종료

    if (!check_collision(current_x, current_y + 1, current_tetromino))
    {
        current_y++;
    }
    else
    {
        // 고정
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                if (current_tetromino[y][x] == 1)
                {
                    grid[current_y + y][current_x + x] = 1;
                }
            }
        }
        clear_lines();

        // 게임 오버 체크
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            if (grid[0][x] != 0)
            {
                game_over_Tetris = true;
                break;
            }
        }
        if (!game_over_Tetris)
        {
            spawn_new_tetromino(); // 게임 오버가 아닐 경우에만 새 도형 생성
        }
    }

    gtk_widget_queue_draw(widget);
    return TRUE;
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    switch (event->keyval)
    {
    case GDK_KEY_Left: // 왼쪽 이동
        if (!check_collision(current_x - 1, current_y, current_tetromino))
        {
            current_x--;
        }
        break;

    case GDK_KEY_Right: // 오른쪽 이동
        if (!check_collision(current_x + 1, current_y, current_tetromino))
        {
            current_x++;
        }
        break;

    case GDK_KEY_Down:                                                        // 아래로 빠르게 이동
        while (!check_collision(current_x, current_y + 1, current_tetromino)) // 바닥에 도달할 때까지
        {
            current_y++;
        }

        // 도형을 고정하고 줄 삭제
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                if (current_tetromino[y][x] == 1)
                {
                    grid[current_y + y][current_x + x] = 1;
                }
            }
        }

        clear_lines(); // 줄 삭제 호출

        // 게임 오버 확인
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            if (grid[0][x] != 0)
            {
                game_over_Tetris = true;
                break;
            }
        }

        // 새 도형 생성
        if (!game_over_Tetris)
        {
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

void clear_lines()
{
    int cleared_lines = 0; // 삭제된 줄 수

    for (int y = GRID_HEIGHT - 1; y >= 0; y--) // 아래에서 위로 검사
    {
        bool full_line = true;

        // 현재 줄이 가득 찼는지 확인
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            if (grid[y][x] == 0)
            {
                full_line = false;
                break;
            }
        }

        // 가득 찬 줄 처리
        if (full_line)
        {
            cleared_lines++;

            // 해당 줄을 위로 당김
            for (int ny = y; ny > 0; ny--)
            {
                for (int nx = 0; nx < GRID_WIDTH; nx++)
                {
                    grid[ny][nx] = grid[ny - 1][nx];
                }
            }

            // 가장 위쪽 줄 초기화
            for (int nx = 0; nx < GRID_WIDTH; nx++)
            {
                grid[0][nx] = 0;
            }

            // 줄이 내려왔으므로 같은 줄을 다시 검사
            y++;
        }
    }

    // 점수 계산 (한 번에 여러 줄 삭제 시 가중치)
    if (cleared_lines > 0)
    {
        score_Tetris += cleared_lines * 100; // 1줄당 100점
    }
}

// 게임 시작
void start_tetris_game()
{
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tetris Game");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 600);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 300, 600);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_game_board), NULL);
    g_timeout_add(game_speed, (GSourceFunc)game_loop, drawing_area);

    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    gtk_widget_show_all(window);

    spawn_new_tetromino();
    gtk_main();
}