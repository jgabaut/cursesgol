// jgabaut @ github.com/jgabaut
// SPDX-License-Identifier: GPL-3.0-only
/*
    Copyright (C) 2026 jgabaut

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

void free_matrix(int** matrix, size_t height)
{
    for (size_t y = 0; y < height; y++) {
        free(matrix[y]);
    }
    free(matrix);
}

void clear_matrix(int** matrix, size_t height, size_t width)
{
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            matrix[y][x] = 0;
        }
    }
}

void draw_matrix(int** matrix, size_t height, size_t width)
{
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            if (matrix[y][x]) {
                attron(COLOR_PAIR(1));
                mvaddch(y, x, '*');
                attroff(COLOR_PAIR(1));
            } else {
                mvaddch(y, x, '.');
            }
        }
    }
}

int count_neighbours(int **matrix, int height, int width, int y, int x) {
    int count = 0;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dy == 0 && dx == 0) continue; // skip the cell itself

            int ny = y + dy;
            int nx = x + dx;

            // bounds check
            if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                if (matrix[ny][nx]) {
                    count++;
                }
            }
        }
    }

    return count;
}

void update_matrix(int **matrix, size_t height, size_t width) {
    // allocate a new matrix for next generation
    int **next = calloc(height, sizeof(int*));
    for (size_t y = 0; y < height; y++) {
        next[y] = calloc(width, sizeof(int));
    }

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            int nc = count_neighbours(matrix, height, width, y, x);
            int cell = matrix[y][x];

            if (cell) {
                // Survival: 2 or 3 neighbors
                if (nc == 2 || nc == 3) next[y][x] = 1;
                else next[y][x] = 0;
            } else {
                // Birth: exactly 3 neighbors
                if (nc == 3) next[y][x] = 1;
            }
        }
    }

    // copy next into matrix
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            matrix[y][x] = next[y][x];
        }
        free(next[y]);
    }
    free(next);
}

int main(void)
{
    initscr();                         // Start curses mode
    nodelay(stdscr, TRUE);             // Make getch() non-blocking
    noecho();                          // Don't echo pressed keys
    keypad(stdscr, TRUE);              // Enable function keys + mouse
    mousemask(BUTTON1_PRESSED, NULL);  // Prepare for mouse events
    timeout(16);                       // getch() waits 16ms before returning ERR
    curs_set(FALSE);                   // Hide the cursor
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    int width = COLS;
    int height = LINES;
    //printw("Hello, World!"); // Print to the screen at the current cursor
    int** matrix = calloc(height, sizeof(int*));
    for (size_t y = 0; y < height; y++) {
        matrix[y] = calloc(width, sizeof(int));
    }

    MEVENT mouse_event;
    int live = 0;
    int ch = '0';
    while(true) {
        if (live) update_matrix(matrix, height, width);
        draw_matrix(matrix, height, width);
        refresh();            // Update the screen to show the output
        ch = getch();              // Wait for user input
        if (ch == KEY_MOUSE) {
            if (getmouse(&mouse_event) == OK) {
                if (mouse_event.bstate & BUTTON1_PRESSED) {  // only left click
                    int y = mouse_event.y;
                    int x = mouse_event.x;
                    if (y >= 0 && y < height && x >= 0 && x < width) {
                        matrix[y][x] = !matrix[y][x];
                    }
                }
            }
        }
        if (ch == 'q') break; // quit with q
        if (ch == 'p') live = !live;
        if (ch == 'c') clear_matrix(matrix, height, width);
    }
    endwin();             // End curses mode
    free_matrix(matrix, height);
    return 0;
}
