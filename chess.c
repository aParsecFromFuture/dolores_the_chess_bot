#include "chess.h"

chess_board BOARD;
pthread_t THREAD_ID[64];
pthread_mutex_t LOCK_1, LOCK_2;

const int color[13] = {NONE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK};
const int mobility[13] = {0, 4, 8, 28, 28, 56, 10, 4, 8, 28, 28, 56, 10};
const int iterable[13] = {0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0};

const int eval_offset[] = {0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768};
const int legal_offset[] = {0, 0, 256, 768, 2560, 4352, 7936, 8576, 8832, 9344, 11136, 12928, 16512};
const int movement_offset[] = {0, 0, 4, 12, 40, 68, 124, 134, 138, 146, 174, 202, 258};

int iterate[56];
short eval[832];
short legal[17152];
int movement[268];

void* thread_white(void *args)
{
    int position, attack, status, score, *arg_data, *best_score, *best_position, *best_attack;
	unsigned char record[12];
    chess_board tmp_board;
	enum chess_piece piece;
    
    chess_copy(BOARD, tmp_board);

    arg_data = (int*)(args);
    position = arg_data[0];
    arg_data[4] = 0;
    best_position = &arg_data[1];
    best_attack = &arg_data[2];
    best_score = &arg_data[3];
    
    piece = tmp_board[position];
    for(FIRST_ATTACK(attack); IS_ATTACK(piece, attack); NEXT_ATTACK(piece, attack, status)){
        if(DO_PLAY(tmp_board, position, attack, status, record)){
            if(status == 3){
                pthread_mutex_lock(&LOCK_1);
                *best_score = POS_INF;
                *best_position = position;
                *best_attack = attack;
                pthread_mutex_unlock(&LOCK_1);
                TAKEBACK(tmp_board, record);
                return 0;
            }
            score = chess_min_max(tmp_board, BLACK, arg_data[5] - 1, NEG_INF, POS_INF);
            if(score > *best_score){
                pthread_mutex_lock(&LOCK_2);
                *best_score = score;
                *best_position = position;
                *best_attack = attack;
                pthread_mutex_unlock(&LOCK_2);
            }
            TAKEBACK(tmp_board, record);
            if(status == 4) TAKEBACK(tmp_board, (&record[6]));
        }
    }
    return 0;
}

void* thread_black(void *args)
{
    int position, attack, status, score, *arg_data, *best_score, *best_position, *best_attack;
	unsigned char record[12];
    chess_board tmp_board;
	enum chess_piece piece;
    
    chess_copy(BOARD, tmp_board);

    arg_data = (int*)(args);
    position = arg_data[0];
    arg_data[4] = 0;
    best_position = &arg_data[1];
    best_attack = &arg_data[2];
    best_score = &arg_data[3];
    
    piece = tmp_board[position];
    for(FIRST_ATTACK(attack); IS_ATTACK(piece, attack); NEXT_ATTACK(piece, attack, status)){
        if(DO_PLAY(tmp_board, position, attack, status, record)){
            if(status == 3){
                pthread_mutex_lock(&LOCK_1);
                *best_score = NEG_INF;
                *best_position = position;
                *best_attack = attack;
                pthread_mutex_unlock(&LOCK_1);
                TAKEBACK(tmp_board, record);
                return 0;
            }
            score = chess_min_max(tmp_board, WHITE, arg_data[5] - 1, NEG_INF, POS_INF);
            if(score < *best_score){
                pthread_mutex_lock(&LOCK_2);
                *best_score = score;
                *best_position = position;
                *best_attack = attack;
                pthread_mutex_unlock(&LOCK_2);
            }
            TAKEBACK(tmp_board, record);
            if(status == 4) TAKEBACK(tmp_board, (&record[6]));
        }
    }
    return 0;
}

void chess_init(chess_board board)
{
    for (int i = 0; i < 72; i++) board[i] = EMPTY;
    
	board[8] = board[9] = board[10] = board[11] = board[12] = board[13] = board[14] = board[15] = B_PAWN;
	board[0] = board[7] = B_ROOK;
	board[1] = board[6] = B_KNIGHT;
	board[2] = board[5] = B_BISHOP;
	board[3] = B_QUEEN;
	board[4] = B_KING;

	board[48] = board[49] = board[50] = board[51] = board[52] = board[53] = board[54] = board[55] = W_PAWN;
	board[56] = board[63] = W_ROOK;
	board[57] = board[62] = W_KNIGHT;
	board[58] = board[61] = W_BISHOP;
	board[59] = W_QUEEN;
	board[60] = W_KING;
}

void chess_load()
{
	FILE *fp;
	int i;
    
    for(i = 0; i < 56; i++) iterate[i] = i - (i % 7) + 7;
	
	fp = fopen("../eval.txt", "r");
    if(fp){
        for(i = 0; i < 832; i++) fscanf(fp, "%hd", &eval[i]);
        fclose(fp);
    }

	fp = fopen("../movement.txt", "r");
    if(fp){
        for(i = 0; i < 268; i++) fscanf(fp, "%d", &movement[i]);
        fclose(fp);
    }

	fp = fopen("../legal.txt", "r");
    if(fp){
        for(i = 0; i < 17152; i++) fscanf(fp, "%hd", &legal[i]);
        fclose(fp);
    }
}

int chess_play(chess_board board, int pos, int mov_id, unsigned char *record)
{
	enum chess_piece from_piece, to_piece;
	int new_pos;
    
	from_piece = board[pos];
    new_pos = pos + movement[movement_offset[from_piece] + mov_id];
    to_piece = board[new_pos];
    if(COLOR_OF(from_piece) == COLOR_OF(to_piece)) return 0;
    
    switch(from_piece){
        case W_PAWN :
            if(mov_id == 1 && board[new_pos + 8] != EMPTY) return 0;
            if((mov_id == 0 || mov_id == 1) && to_piece != EMPTY) return 0;
            if((mov_id == 2 || mov_id == 3) && to_piece == EMPTY) return 0;
            break;
        case B_PAWN :
            if(mov_id == 1 && board[new_pos - 8] != EMPTY) return 0;
            if((mov_id == 0 || mov_id == 1) && to_piece != EMPTY) return 0;
            if((mov_id == 2 || mov_id == 3) && to_piece == EMPTY) return 0;
            break;
        case W_KING :
            if(mov_id == 8){
                if(board[56] != W_ROOK || board[57] != EMPTY || board[58] != EMPTY || board[59] != EMPTY) return 0;
                for(int i = 57; i < 61; i++) if(!IS_CHECK_SAFE(board, W_KING, 60, i)) return 0;
                
                if(record != 0){
                    SAVE(record, W_KING, 60, EMPTY, 58, board[70], board[71]);
                    SAVE((&record[6]), W_ROOK, 56, EMPTY, 59, board[70], board[71]);
                }
                MAKEMOVE(board, W_KING, 60, 58);
                MAKEMOVE(board, W_ROOK, 56, 59);
                SCORE(board) += 3;
                return 4;
            }else if(mov_id == 9){
                if(board[63] != W_ROOK || board[61] != EMPTY || board[62] != EMPTY) return 0;
                for(int i = 60; i < 63; i++) if(!IS_CHECK_SAFE(board, W_KING, 60, i)) return 0;
                
                if(record != 0){
                    SAVE(record, W_KING, 60, EMPTY, 62, board[70], board[71]);
                    SAVE((&record[6]), W_ROOK, 63, EMPTY, 61, board[70], board[71]);
                }
                MAKEMOVE(board, W_KING, 60, 62);
                MAKEMOVE(board, W_ROOK, 63, 61);
                SCORE(board) += 6;
                return 4;
            }
        case B_KING :
            if(mov_id == 8){
                if(board[0] != B_ROOK || board[1] != EMPTY || board[2] != EMPTY || board[3] != EMPTY) return 0;
                
                for(int i = 1; i < 5; i++) if(!IS_CHECK_SAFE(board, B_KING, 4, i)) return 0;
                
                if(record != 0){
                    SAVE(record, B_KING, 4, EMPTY, 2, board[70], board[71]);
                    SAVE((&record[6]), B_ROOK, 0, EMPTY, 3, board[70], board[71]);
                }
                MAKEMOVE(board, B_KING, 4, 2);
                MAKEMOVE(board, B_ROOK, 0, 3);
                SCORE(board) -= 3;
                return 4;
            }else if(mov_id == 9){
                if(board[7] != B_ROOK || board[5] != EMPTY || board[6] != EMPTY) return 0;
                
                for(int i = 4; i < 7; i++) if(!IS_CHECK_SAFE(board, B_KING, 4, i)) return 0;
                
                if(record != 0){
                    SAVE(record, B_KING, 4, EMPTY, 6, board[70], board[71]);
                    SAVE((&record[6]), B_ROOK, 7, EMPTY, 5, board[70], board[71]);
                }
                MAKEMOVE(board, B_KING, 4, 6);
                MAKEMOVE(board, B_ROOK, 7, 5);
                SCORE(board) -= 6;
                return 4;
            }
        default:
            break;
    }

    if(record != 0) SAVE(record, from_piece, pos, to_piece, new_pos, board[70], board[71]);
    MAKEMOVE(board, from_piece, pos, new_pos);
    
    if(board[new_pos] == W_PAWN && new_pos < 8) board[new_pos] = W_QUEEN;
    if(board[new_pos] == B_PAWN && new_pos > 55) board[new_pos] = B_QUEEN;
    if(from_piece == W_KING) SCORE(board) -= 4;
    if(from_piece == B_KING) SCORE(board) += 4;
    
    switch(to_piece){
        case EMPTY :
            SCORE(board) += (EVALUATE(board[new_pos], new_pos) - EVALUATE(from_piece, pos));
            return 1;
        case W_KING :
        case B_KING :
            return 3;
        default :
            SCORE(board) += (EVALUATE(board[new_pos], new_pos) - EVALUATE(from_piece, pos) - EVALUATE(to_piece, new_pos));
            return 2;
    }
}

int chess_min_max(chess_board board, int is_maximize, int depth, int alpha, int beta)
{
	int position, attack, status, score, best_score;
	unsigned char record[12];
	enum chess_piece piece;

	if(depth == 0) return SCORE(board);

	if(is_maximize){
		best_score = NEG_INF;
		for(position = 0; position < 64; position++){
			piece = board[position];
			if(COLOR_OF(piece) != WHITE) continue;
			for(FIRST_ATTACK(attack); IS_ATTACK(piece, attack); NEXT_ATTACK(piece, attack, status)){
				if(DO_PLAY(board, position, attack, status, record)){
                    if(status == 3){
                        TAKEBACK(board, record);
                        return depth * 1000;
                    }
                    score = chess_min_max(board, BLACK, depth - 1, alpha, beta);
                    TAKEBACK(board, record);
                    if(status == 4) TAKEBACK(board, (&record[6]));
					if(score > best_score) best_score = score;
					if(alpha < best_score) alpha = best_score;;
					if(alpha >= beta) return best_score;
				}
			}
		}
	}else{
		best_score = POS_INF;
		for(position = 0; position < 64; position++){
			piece = board[position];
			if(COLOR_OF(piece) != BLACK) continue;
			for(FIRST_ATTACK(attack); IS_ATTACK(piece, attack); NEXT_ATTACK(piece, attack, status)){
				if(DO_PLAY(board, position, attack, status, record)){
                    if(status == 3){
                        TAKEBACK(board, record);
                        return depth * -1000;
                    }
                    score = chess_min_max(board, WHITE, depth - 1, alpha, beta);
					TAKEBACK(board, record);
                    if(status == 4) TAKEBACK(board, (&record[6]));
					if(score < best_score) best_score = score;
					if(beta > best_score) beta = best_score;
					if(beta <= alpha) return best_score;
				}
			}
		}
	}
	return best_score;
}

int chess_play_ai(chess_board board, int is_maximize, int depth, unsigned char *record)
{
	int i, arg_data[6];
    arg_data[0] = 0;
    arg_data[5] = depth;

    if(is_maximize){
        arg_data[3] = NEG_INF;
        for(i = 0; i < 64; i++){
            if(COLOR_OF(board[i]) == WHITE){
                arg_data[0] = i;
                pthread_create(&THREAD_ID[i], 0, thread_white, arg_data);
                while(arg_data[4]);
                arg_data[4] = 1;
            }
        }
        for(i = 0; i < 64; i++) if(COLOR_OF(board[i]) == WHITE) pthread_join(THREAD_ID[i], 0);
    }else{
        arg_data[3] = POS_INF;
        for(i = 0; i < 64; i++){
            if(COLOR_OF(board[i]) == BLACK){
                arg_data[0] = i;
                pthread_create(&THREAD_ID[i], 0, thread_black, arg_data);
                while(arg_data[4]);
                arg_data[4] = 1;
            }
        }
        for(i = 0; i < 64; i++) if(COLOR_OF(board[i]) == BLACK) pthread_join(THREAD_ID[i], 0);
    }
	return chess_play(board, arg_data[1], arg_data[2], record);
}

int chess_play_manual(chess_board board, int pos, int new_pos, unsigned char *record)
{
    enum chess_piece piece = board[pos];
    int attack, status;

    for(FIRST_ATTACK(attack); IS_ATTACK(piece, attack); NEXT_ATTACK(piece, attack, status)){
        if(DO_PLAY(board, pos, attack, status, record)){
            if(record[3] == new_pos){
                if(chess_is_checkmate(board, !color[piece])){
                    TAKEBACK(board, record);
                    if(status == 4) TAKEBACK(board, (&record[6]));
                    return 0;
                }
                return status;
            }
            TAKEBACK(board, record);
            if(status == 4) TAKEBACK(board, (&record[6]));
        }
    }
    return 0;
}

int chess_is_checkmate(chess_board board, int opponent)
{
    int position, attack, status;
	unsigned char record[12];
	enum chess_piece piece;

    for(position = 0; position < 64; position++){
        piece = board[position];
        if(COLOR_OF(piece) != opponent) continue;
        
        for(FIRST_ATTACK(attack); IS_ATTACK(piece, attack); NEXT_ATTACK(piece, attack, status)){
            if((piece == W_KING || piece == B_KING) && (attack == 8 || attack == 9)) continue;
            if(DO_PLAY(board, position, attack, status, record)){
                if(status == 3){
                    TAKEBACK(board, record);
                    return 1;
                }
                TAKEBACK(board, record);
            }
        }
    }
    return 0;
}

void chess_copy(chess_board board1, chess_board board2)
{
    for(int i = 0; i < 72; i++) board2[i] = board1[i];
}