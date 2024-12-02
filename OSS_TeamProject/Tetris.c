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

// 새 도형 생성
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

    // 새 도형이 배치되자마자 충돌하면 게임 오버
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (current_tetromino[y][x] == 1 && grid[current_y + y][current_x + x] != 0)
            {
                game_over_Tetris = true;
                return;
            }
        }
    }
}

// 줄 삭제
void clear_lines()
{
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        bool full_line = true;
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            if (grid[y][x] == 0)
            {
                full_line = false;
                break;
            }
        }

        if (full_line)
        {
            // 점수 추가
            score_Tetris += 100;

            // 줄 위로 당기기
            for (int ny = y; ny > 0; ny--)
            {
                for (int nx = 0; nx < GRID_WIDTH; nx++)
                {
                    grid[ny][nx] = grid[ny - 1][nx];
                }
            }

            // 가장 위 줄은 초기화
            for (int nx = 0; nx < GRID_WIDTH; nx++)
            {
                grid[0][nx] = 0;
            }
        }
    }
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

    // 충돌 체크
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            int new_x = current_x + x;
            int new_y = current_y + y;

            if (rotated[y][x] == 1)
            {
                if (new_x < 0 || new_x >= GRID_WIDTH || new_y >= GRID_HEIGHT || grid[new_y][new_x] != 0)
                {
                    return; // 충돌 시 회전 취소
                }
            }
        }
    }

    // 회전 반영
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            current_tetromino[y][x] = rotated[y][x];
        }
    }
}

// 게임판 그리기
gboolean draw_game_board(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    double cell_size = 30;

    // 게임판
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            cairo_set_source_rgb(cr, grid[y][x] ? 0.3 : 0.9, 0.3, 0.9);
            cairo_rectangle(cr, x * cell_size, y * cell_size, cell_size - 1, cell_size - 1);
            cairo_fill(cr);
        }
    }

    // 현재 도형
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (current_tetromino[y][x] == 1)
            {
                cairo_set_source_rgb(cr, 0.2, 0.6, 0.8);
                cairo_rectangle(cr, (current_x + x) * cell_size, (current_y + y) * cell_size, cell_size - 1, cell_size - 1);
                cairo_fill(cr);
            }
        }
    }

    // 점수 표시
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, 10, 10);
    cairo_show_text(cr, g_strdup_printf("Score: %d", score_Tetris));

    if (game_over_Tetris)
    {
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_move_to(cr, 30, 250);
        cairo_show_text(cr, "Game Over!");
    }

    return FALSE;
}

// 게임 루프
gboolean game_loop(GtkWidget *widget)
{
    if (game_over_Tetris)
        return FALSE;

    gboolean can_move = TRUE;

    // 도형 이동 가능 여부 체크
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (current_tetromino[y][x] == 1)
            {
                int new_x = current_x + x;
                int new_y = current_y + y + 1;

                if (new_y >= GRID_HEIGHT || grid[new_y][new_x] != 0)
                {
                    can_move = FALSE;
                    break;
                }
            }
        }
        if (!can_move)
            break;
    }

    if (can_move)
    {
        current_y++;
    }
    else
    {
        // 도형 고정
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

        // 줄 삭제
        clear_lines();

        // 새 도형 생성
        spawn_new_tetromino();
    }

    gtk_widget_queue_draw(widget);
    return TRUE;
}

// 키 입력 처리
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    switch (event->keyval)
    {
    case GDK_KEY_Left:
        current_x--;
        break;
    case GDK_KEY_Right:
        current_x++;
        break;
    case GDK_KEY_Up:
        rotate_tetromino();
        break;
    case GDK_KEY_Down:
        current_y++;
        break;
    }
    gtk_widget_queue_draw(widget);
    return TRUE;
}

// 게임 시작
void start_tetris_game()
{
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tetris Game");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 600);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 300, 600);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_game_board), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    g_timeout_add(game_speed, (GSourceFunc)game_loop, drawing_area);

    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    gtk_widget_show_all(window);

    spawn_new_tetromino();
    gtk_main();
}