#include "chess.h"

#define NAME_SIZE 16

unsigned short player_num(player_t *pl) {
    if (pl == &black_player) {
        return BLACK;
    }
    if (pl == &white_player) {
        return WHITE;
    }
    if(pl == &red_player) {
        return RED;
    }
    assert (pl == NULL);
    return 0;
}

player_t* to_player(unsigned short code) {
    switch (code) {
        case BLACK: return &black_player;
        case WHITE: return &white_player;
        default: {
            assert (code = RED);
            return &red_player;
        }
    }
    return &red_player;
}

coord_t send_recv_coord(bool send) {
    coord_t pos;
    char msg[MSG_SIZE] = {7,0,0};
    if (send) {
        send_msg(msg, MSG_SIZE*sizeof(char));
    }
    receive_msg(msg, MSG_SIZE*sizeof(char));
    pos.x = msg[0];
    pos.y = msg[1];
    pos.belongs = to_player(msg[2]);
    return pos;
}

bool send_avail_moves() {
    char msg[MAX_MOVES*MSG_SIZE];
    for (int i = 0; &curr_avail_moves[i]; i++) {
        coord_t pos = curr_avail_moves[i];
        unsigned short index = MSG_SIZE*i;
        msg[index] = pos.x;
        msg[index+1] = pos.y;
        msg[index+2] = player_num(pos.belongs);
    }
    return send_msg(msg, MAX_MOVES*MSG_SIZE*sizeof(char));
}

bool check_valid(piece_t *pc) {
    return (pc != &default_piece && pc->piece_color == current_player->player_col);
}

int main() {
    int status = system("\"path\" width=640 height=480 isWindowedMode=true");

    char names[MSG_SIZE*NAME_SIZE];
    receive_msg(names, MSG_SIZE*NAME_SIZE*sizeof(char));

    char *players[MSG_SIZE];
    for (int i = 0; i < MSG_SIZE; i++) {
        memcpy(players[i], &names[i*NAME_SIZE], NAME_SIZE); 
    }

    init_players(players);
    while (1) {
        init_chess_boards();
        current_player = &white_player;
        player_t *win1 = NULL;
        player_t *win2 = NULL;
        while (1) {

            game_status = game_state(win1, win2);
            //check if game has ended
            if (game_status != GAME) {
                //display game result
                char msg_result[MSG_SIZE];
                if (game_status == CHECKMATE) {
                    msg_result[0] = 6;
                    msg_result[1] = player_num(win1);
                    msg_result[2] = player_num(win2);
                } else {
                    msg_result[0] = 5;
                    if (game_status == STALEMATE) {
                        msg_result[1] = 0;
                    } else {
                        assert (game_status == DRAW);
                        msg_result[1] = 1;
                    }
                }
                send_msg(msg_result, MSG_SIZE);
                break;
            }

            coord_t orig_grid;
            coord_t dest_grid;   

            while (1) {
                orig_grid = send_recv_coord(true);
                current_piece = get_piece(orig_grid);
                if (check_valid(current_piece)){
                    curr_avail_moves = show_avail_move(orig_grid);
                    send_avail_moves();
                    dest_grid = send_recv_coord(false);

                    if (movable(dest_grid, curr_avail_moves)) {
                        move_piece(orig_grid, dest_grid, NULL, NULL);
                        //check castling
                        if (current_piece->type == &king_type) {
                            signed short dx = dest_grid.x - orig_grid.x;
                            if (dx == -2) {
                                castling(orig_grid, true);
                            }
                            if (dx == 2) {
                                castling(orig_grid, false);
                            }
                        }
                        //check promotion
                        check_prom(dest_grid);
                        break;
                    }                   
                }
            }
            next_player();
        }
        //display scores
        char msg_score[MSG_SIZE+1] = 
        {1,black_player.score,white_player.score,red_player.score};
        send_msg(msg_score, MSG_SIZE+1);

        //continue next round or quit
        char ctn = 0;
        receive_msg(&ctn, sizeof(char));
        if (ctn == 0) {
            //end game
            break;
        }
        assert (ctn == 1);
        //next round
    }
    
    terminate();
}
