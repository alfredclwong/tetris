#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <ncurses.h>
#include "tetris.h"

void fill_bag(LinkedPiece **bag) {
    // generate random perm
    int perm[NUM_PIECES];
    for (int i=0; i<NUM_PIECES; i++)
        perm[i] = i;
    for (int i=0; i<NUM_PIECES; i++) {
        int j, tmp;
        j = rand() % (NUM_PIECES-i) + i;
        tmp = perm[j];
        perm[j] = perm[i];
        perm[i] = tmp;
    }

    // fill bag according to perm
    *bag = (LinkedPiece*) malloc(sizeof(LinkedPiece));
    LinkedPiece *end = *bag;
    for (int i=0; i<NUM_PIECES; i++) {
        end->piece = PIECES[perm[i]];
        end->next = (LinkedPiece*) malloc(sizeof(LinkedPiece));
        end = end->next;
    }
    end = NULL;
}

Piece pop_next(LinkedPiece **next, LinkedPiece **bag) {
    // require non-empty bag for repopulating next after pop
    if (*bag == NULL)
        fill_bag(bag);

    // edge case: first game loop has empty next
    if (*next == NULL) {
        *next = *bag;
        for (int i=0; i<NEXT-1; i++)
            *bag = (*bag)->next;
        LinkedPiece *tmp = (*bag)->next;
        (*bag)->next = NULL;
        *bag = tmp;
    }

    // pop from next for retval, pop from bag for next next
    Piece next_piece = (*next)->piece;
    *next = (*next)->next;
    LinkedPiece *end = *next;
    while (end->next)
        end = end->next;
    end->next = *bag;
    *bag = (*bag)->next;
    end->next->next = NULL;
    return next_piece;
}

void draw(int matrix[COLS][ROWS], Point loc, Piece *fall, Piece *hold, LinkedPiece *next, LinkedPiece *bag) {
    // draw matrix
    for (int x=0; x<COLS; x++)
        for (int y=0; y<ROWS; y++)
            mvprintw(BUFFER+ROWS-1-y, 5+x, "%d", matrix[x][y]);
    // draw fall
    for (int i=0; i<4; i++)
        mvprintw(BUFFER+ROWS-1-(loc.y+fall->points[i].y), 5+loc.x+fall->points[i].x, "1");
    // draw next
    mvprintw(BUFFER-2, 5+COLS+1, "NEXT");
    LinkedPiece *end = next;
    for (int i=0; end; i++) {
        for (int j=0; j<4; j++)
            mvprintw(BUFFER+i*3+1+end->piece.points[j].y, 5+COLS+2+end->piece.points[j].x, "1");
        end = end->next;
    }
    // draw hold
    mvprintw(BUFFER-2, 0, "HOLD");
    if (hold != NULL) {
        for (int i=0; i<4; i++)
            mvprintw(BUFFER+1+hold->points[i].y, 1+hold->points[i].x, "1");
    }
    // draw bag
    mvprintw(BUFFER-2, 5+COLS+1+5, "BAG");
    end = bag;
    for (int i=0; end; i++) {
        for (int j=0; j<4; j++)
            mvprintw(BUFFER+i*3+1+end->piece.points[j].y, 5+COLS+2+5+end->piece.points[j].x, "1");
        end = end->next;
    }
}

long usec_diff(struct timeval *a, struct timeval *b) {
    return (a->tv_sec-b->tv_sec)*1000000 + a->tv_usec-b->tv_usec;
}

int can_fall(Piece *fall, Point *loc, int matrix[COLS][ROWS]) {
    for (int i=0; i<4; i++) {
        if (loc->y + fall->points[i].y - 1 < 0)
            return 0;
        if (matrix[loc->x + fall->points[i].x][loc->y + fall->points[i].y - 1])
            return 0;
    }
    return 1;
}

int main() {
    srand(time(NULL));

    int matrix[COLS][ROWS] = {{0}};
    Point loc, spawn = {COL_SPAWN, ROW_SPAWN};
    Piece *fall = NULL, *ghost = NULL, *hold = NULL;
    LinkedPiece *next = NULL, *bag = NULL;

    initscr();
    noecho();
    curs_set(0);
    timeout(0);
    keypad(stdscr, TRUE);

    int done = 0;
    while (!done) {
        /********************************************/
        /*              GENERATION PHASE            */
        /********************************************/
        loc = spawn;
        Piece next_piece = pop_next(&next, &bag);
        fall = &next_piece;

        /********************************************/
        /*              FALLING/LOCK PHASE          */
        /********************************************/
        int c, dx, held = 0, falling = 1, speed = SPEED, tick = 33334;
        struct timeval prev, curr, lock_start;
        gettimeofday(&prev, NULL);
        prev.tv_sec--;
        while (!done) {
            // deal with inputs
            c = getch();
            switch (c) {
                case 'q':
                    done = 1;
                    continue;
                case KEY_LEFT: // try to move left
                case KEY_RIGHT: // try to move right
                    dx = c == KEY_LEFT ? -1 : 1;
                    for (int i=0; i<4; i++) {
                        int new_x = loc.x + fall->points[i].x + dx;
                        if (new_x<0 || new_x>=COLS || matrix[new_x][loc.y + fall->points[i].y]) {
                            dx = 0;
                            break;
                        }
                    }
                    loc.x += dx;
                    draw(matrix, loc, fall, hold, next, bag);
                    break;
                case KEY_UP: // TODO hard drop
                    break;
                case KEY_DOWN: // TODO soft drop
                    break;
                case 'z': // rotate ccw
                case 'x': // rotate cw
                    break;
                case 'c': // hold
                    if (held)
                        break;
                    if (hold) {
                        Piece *tmp = hold;
                        hold = fall;
                        fall = tmp;
                    } else {
                        hold = fall;
                        Piece next_piece = pop_next(&next, &bag);
                        fall = &next_piece;
                    }
                    loc = spawn;
                    held = 1;
                    draw(matrix, loc, fall, hold, next, bag);
                    break;
            }

            gettimeofday(&curr, NULL);
            // lock timer
            if (!falling) {
                if (usec_diff(&curr, &lock_start) < LOCK_US)
                    break;
                mvprintw(BUFFER+ROWS-1, 0, "%.3f", (LOCK_US-usec_diff(&curr, &lock_start))/1000000.0);
                refresh();
                if (can_fall(fall, &loc, matrix))
                    falling = 1;
                else
                    continue;
            }
            
            // tick code
            if (curr.tv_usec-prev.tv_usec + (curr.tv_sec-prev.tv_sec)*1000000 < speed)
                continue;
            prev = curr;

            if (can_fall(fall, &loc, matrix))
                loc.y--;
            else {
                falling = 0;
                gettimeofday(&lock_start, NULL);
            }

            clear();
            draw(matrix, loc, fall, hold, next, bag);
            for (int i=0; i<4; i++)
                mvprintw(BUFFER+ROWS+i, 0, "(%d,%d)", fall->points[i].x, fall->points[i].y);
            refresh();
        }
        for (int i=0; i<4; i++)
            matrix[loc.x + fall->points[i].x][loc.y + fall->points[i].y] = 1;
        fall = NULL;
    }
    endwin();
}
