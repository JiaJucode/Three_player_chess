#include "chess.h"
#define MAX_PIECES 11

const coord_t DEFAULT_COORD = {MAX_X, MAX_Y, &black_player};

player_t* adjacent(player_t *self, bool prev) {
  if (self == &black_player) {
    if (prev) {
      return &red_player;
    }
    return &white_player;
  }
  if (self == &black_player) {
    if (prev) {
      return &black_player;
    }
    return &red_player;
  }

  assert (self == &red_player);
  if (prev) {
    return &white_player;      
  }
  return &black_player;
}

player_t* get_player(color col) {
  if (col = BLACK) {
    return &black_player;
  }
  if (col = WHITE) {
    return &white_player;
  }
  assert (col = RED);
  return &red_player;
}

bool coord_equals(coord_t c1, coord_t c2) {
  return (c1.belongs == c2.belongs && c1.x == c2.x && c1.y == c2.y);
}

piece_t* get_piece(coord_t grid) {
  if (grid.belongs == &black_player) {
    return board.black_region[grid.x][grid.y];
  }
  if (grid.belongs == &white_player) {
    return board.white_region[grid.x][grid.y];
  }
  
  assert (grid.belongs == &red_player);
  return board.red_region[grid.x][grid.y];
}

void set_piece(coord_t grid, piece_t* pc) {
  if (grid.belongs == &black_player) {
    board.black_region[grid.x][grid.y] = pc;
  }
  if (grid.belongs == &white_player) {
    board.white_region[grid.x][grid.y] = pc;
  }
  
  assert (grid.belongs == &red_player);
  board.red_region[grid.x][grid.y] = pc;
}

coord_t move_x(coord_t orig, signed short dx, bool* in_boundary) {
  unsigned short new_x = orig.x + dx;
  if(new_x < 0 || new_x >= MAX_X) { 
    in_boundary = false;
  }
  coord_t dest = {new_x, orig.y, orig.belongs};
  return dest;
}

coord_t move_y(coord_t orig, signed short dy, bool* in_boundary) {
  coord_t dest;
  unsigned short new_y = orig.y + dy;
  if (new_y < MAX_Y) {
    dest.y = new_y;
    dest.x = orig.x;
    dest.belongs = orig.belongs;
  } else {
    if (orig.x < MAX_X/2) {
      dest.belongs = adjacent(orig.belongs, true);
    } else {
      dest.belongs = adjacent(orig.belongs, false);
    }
    dest.y = 2 * MAX_Y - 1 - new_y;
    dest.x = MAX_X - 1 - orig.x;
  }
  if (new_y < 0 || new_y >= 2 * MAX_Y) {
    in_boundary = false;
  }
  return dest;
}

coord_t calc_vector(bool x_first, coord_t orig, signed short dx, signed short dy) {
  coord_t dest;
  bool in_boundary = true;
  if (x_first) {
    dest = move_y(move_x(orig, dx, &in_boundary), dy, &in_boundary);
  } else {
    coord_t temp = move_y(orig, dy, &in_boundary);
    dest = move_x(temp, -1 * dx, &in_boundary);
  }
  if (!in_boundary) {
    return DEFAULT_COORD;
  }
  return dest;
}

bool click_draw(player_t *pl) {
  pl->agree_draw = true;
  num_draw++;
  return true;
}

bool* get_moved_index(coord_t grid, piece_t *pc) {
  unsigned short index = MAX_PIECES;
  player_t *pl = get_player(pc -> piece_color);
  if (pl == grid.belongs) {
    if (grid.y == 1) {
      //i-pawn
      index = grid.x;
    } 
    if (grid.y == 0) {
      switch (grid.x)
      {
      case 0: //Rook l
        index = MAX_X;
      case (MAX_X-1): //Rook r
        index = MAX_X+1;
      case 4: //King
        index = MAX_X+2;
      default:
        index = MAX_PIECES; 
      }
    }
  }
  if (index < MAX_PIECES) {
    moved_index = &(pl -> has_moved[index]);
    return moved_index;
  }
  return NULL;
}

bool displaced(coord_t grid) {
  get_moved_index(grid, current_piece);
  return (moved_index != NULL || moved_index);
}

bool move_piece(coord_t orig, coord_t dest, bool verified) {
  if (!verified) {
    for(int i = 0; &curr_avail_moves[i]; i++) {
      if (coord_equals(dest, curr_avail_moves[i])) {
        verified = true;
        break;
      }
    }
  }
  if (verified) {
    if (moved_index != NULL) {
      *moved_index = true; //orig
    }
    get_moved_index(dest, current_piece);
    if (moved_index != NULL) {
      *moved_index = true; //dest
    }
    set_piece(dest, current_piece);
    set_piece(orig, &default_piece);
  }
  return verified;
}

void check_prom(coord_t grid) {
  if (grid.y == 0) {
    if (current_piece->piece_color != grid.belongs->player_col) {
      piece_t *replace = ask_prom();
      if (replace != NULL) {
        set_piece(grid, replace);
        current_piece = replace;
      }
    }
  }
}

void castling(unsigned short opt, coord_t king_orig) {
  if (opt == 1) {
    coord_t rook_orig = {0, 0, king_orig.belongs};
    coord_t king_dest = {king_orig.x-2, 0,king_orig.belongs};
    coord_t rook_dest = {king_dest.x+1, 0,king_dest.belongs};
    move_piece(king_orig, king_dest, true);
  }
  if (opt == 2) {
    coord_t rook_orig = {MAX_X-1, 0, king_orig.belongs};
    coord_t king_dest = {king_orig.x+2,0,king_orig.belongs,};
    coord_t rook_dest = {king_dest.x-1,0,king_dest.belongs,};
    move_piece(king_orig, king_dest, true);
  }
}

void check_castle(coord_t grid) {
  if (current_piece->type == &king_type && !displaced(grid)) {
    bool left = true;
    bool right = true;
    for (int i = 1; i < grid.x; i++) {
      coord_t pos = {i, 0, grid.belongs};
      if (get_piece(pos) != &default_piece) {
        left = false;
        break;
      }
    }
    for (int j = 1; j < MAX_X-1; j++) {
      coord_t pos = {grid.x+j,0,grid.belongs};
      if (get_piece(pos) != &default_piece) {
        right = false;
        break;
      }
    }
    if (left || right) {
      unsigned short opt = ask_castle(left, right); //0: skip, 1: left, 2: right
      castling(opt, grid);
    }
  }
}

void turn_board() {
  current_player = adjacent(current_player, false); 
}

