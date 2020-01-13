#define COLS 10
#define ROWS 20
#define BUFFER 4
#define SPEED 200000
#define LOCK_US 500000
#define NEXT 4
#define NUM_PIECES 7
#define COL_SPAWN 4 // spawn on 4th (-1) to 7th (2) cols
#define ROW_SPAWN 20 // spawn on 21st (0) and 22nd (1) rows

/********************************************/
/*          CORE TETRIS                     */
/********************************************/
typedef struct {
    int x, y;
} Point;

typedef struct {
    Point points[4];
} Piece;

typedef struct LinkedPiece LinkedPiece;
struct LinkedPiece {
    Piece piece;
    LinkedPiece *next;
};

static const Piece PIECES[7] = {
    { .points = {{ 0,  0}, { 0,  1}, { 1,  0}, { 1,  1}} }, // O
    { .points = {{-1,  0}, { 0,  0}, { 1,  0}, { 2,  0}} }, // I
    { .points = {{-1,  0}, { 0,  0}, { 1,  0}, { 0,  1}} }, // T
    { .points = {{-1,  0}, { 0,  0}, { 1,  0}, { 1,  1}} }, // L
    { .points = {{-1,  0}, { 0,  0}, { 1,  0}, {-1,  1}} }, // J
    { .points = {{-1,  0}, { 0,  0}, { 0,  1}, { 1,  1}} }, // S
    { .points = {{ 0,  0}, { 1,  0}, {-1,  1}, { 0,  1}} }, // Z
};
