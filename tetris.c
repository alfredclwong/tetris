#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <ncurses.h>
#include "tetris.h"

int main() {
    srand(time(NULL));

    int matrix[COLS][ROWS] = {{0}};
    Point loc, spawn = {COL_SPAWN, ROW_SPAWN};
    Piece *falling = NULL, *ghost = NULL, *hold = NULL;
    LinkedPiece *next = NULL, *bag = NULL;

    initscr();
    noecho();
    curs_set(0);
    timeout(0);
    keypad(stdscr, TRUE);

    int c, quit = 0, speed = 1000000, tick = 33334;
    struct timeval prev, curr;
    gettimeofday(&prev, NULL);
    prev.tv_sec--;
    // TODO contain input/tick loop within falling phase
    while (!quit) {
        // deal with inputs
        c = getch();
        switch (c) {
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_UP:
            case KEY_DOWN:
                break;
            case 'q':
                quit = 1;
                break;
        }

        // tick code
        gettimeofday(&curr, NULL);
        if (curr.tv_usec-prev.tv_usec + (curr.tv_sec-prev.tv_sec)*1000000 < speed)
            continue;
        prev = curr;

        // GENERATION PHASE
        // refill bag if empty
        if (bag == NULL) {
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
            // populate linked list
            bag = (LinkedPiece*) malloc(sizeof(LinkedPiece));
            LinkedPiece *end = bag;
            for (int i=0; i<NUM_PIECES; i++) {
                end->piece = PIECES[perm[i]];
                if (i < NUM_PIECES-1) {
                    end->next = (LinkedPiece*) malloc(sizeof(LinkedPiece));
                    end = end->next;
                } else
                    end->next = NULL;
            }
        }
        
        // first iteration - fill next from bag
        if (next == NULL) {
            next = bag;
            for (int i=0; i<NEXT-1; i++)
                bag = bag->next;
            LinkedPiece *tmp = bag->next;
            bag->next = NULL;
            bag = tmp;
        }

        // pop falling from next and next next from bag
        if (falling == NULL) {
            falling = &(next->piece);
            next = next->next;
            loc = spawn;
            LinkedPiece *end = next;
            while (end->next)
                end = end->next;
            end->next = bag;
            bag = bag->next;
            end->next->next = NULL;
        }

        // FALLING PHASE
        a
        loc.y--;

        // draw matrix
        clear();
        for (int x=0; x<COLS; x++)
            for (int y=0; y<ROWS; y++)
                mvprintw(BUFFER+y, 5+x, "%d", matrix[x][y]);
        // draw falling
        for (int i=0; i<4; i++)
            mvprintw(BUFFER+ROWS-1-(loc.y+falling->points[i].y), 5+loc.x+falling->points[i].x, "1");
        // draw next
        mvprintw(BUFFER-2, 5+COLS+1, "NEXT");
        LinkedPiece *end = next;
        for (int i=0; i<NEXT; i++) {
            for (int j=0; j<4; j++) {
                mvprintw(BUFFER+i*3+1-end->piece.points[j].y, 5+COLS+2+end->piece.points[j].x, "1");
            }
            end = end->next;
        }
        // draw hold
        mvprintw(BUFFER-2, 0, "HOLD");
        if (hold != NULL) {
            ;
        }
        refresh();
    }
    endwin();
}
