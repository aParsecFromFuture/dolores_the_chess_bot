#ifndef CHESS_H
#define CHESS_H
#include <stdio.h>
#include <pthread.h>

#define EVALUATE(x, y) (eval[eval_offset[(x)] + (y)])
#define SCORE(board) (*(short*)&board[70])
#define COLOR_OF(piece) (color[piece])
#define IS_LEGAL(piece, position, attack) (legal[legal_offset[piece] + mobility[piece] * position + attack])

#define IS_CHECK_SAFE(board, piece, from, to) (MAKEMOVE(board, piece, from, to), (chess_is_checkmate(board, !color[piece])? (MAKEMOVE(board, piece, to, from), 0) : (MAKEMOVE(board, piece, to, from), 1)))

#define IS_CHECKED(board, opponent) (chess_play_ai(board, opponent, 1) == 3)? 1 : 0)

#define SAVE(record, a, b, c, d, e, f) (record[0] = a, record[1] = b, record[2] = c, record[3] = d, record[4] = e, record[5] = f)
#define TAKEBACK(board, record) (board[record[1]] = record[0], board[record[3]] = record[2], board[70] = record[4], board[71] = record[5])
#define MAKEMOVE(board, a, b, c) (board[b] = EMPTY, board[c] = a)

#define FIRST_ATTACK(attack_id) (attack_id = 0)
#define NEXT_ATTACK(piece, attack, status) ((status == 1 || !iterable[piece])? (attack++) : (attack = iterate[attack]))
#define IS_ATTACK(piece, attack) (attack < mobility[piece]) 

#define DO_PLAY(b, i, j, status, record) ((status = IS_LEGAL(b[i], i, j))? (status = chess_play(b, i, j, record)) : 0)

#define POS_INF 10000
#define NEG_INF -10000

enum chess_piece{EMPTY, W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING};
enum chess_player{NONE = 2, WHITE = 1, BLACK = 0};

typedef unsigned char chess_board[72];

extern chess_board BOARD;
extern pthread_t THREAD_ID[64];
extern pthread_mutex_t LOCK_1, LOCK_2;

extern const int color[13];
extern const int mobility[13];
extern const int iterable[13];

extern const int eval_offset[];
extern const int legal_offset[];
extern const int movement_offset[];

extern int iterate[56];
extern short eval[832];
extern short legal[17152];
extern int movement[268];

extern void* thread_white(void*);

void chess_load();
void chess_init(chess_board);
int chess_play(chess_board, int, int, unsigned char*);
int chess_min_max(chess_board, int, int, int, int);
int chess_play_ai(chess_board, int, int, unsigned char*);
int chess_play_manual(chess_board, int, int, unsigned char*);
int chess_is_checkmate(chess_board, int);
void chess_copy(chess_board, chess_board);
#endif