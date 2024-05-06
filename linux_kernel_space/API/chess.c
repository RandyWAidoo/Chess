#ifndef CHESS_C
#define CHESS_C

//*
#include <linux/random.h>
//*/

// Game parameters
#define BOARD_SIZE 8
#define ROW_STARTC '8'
#define COL_STARTC 'a'
#define MOVE_NOTATION_LENGTH 13

// Player colors
#define NO_COLOR '*'
#define BLACK 'B'
#define WHITE 'W'

// Color-based positional/movement constants
#define WHITE_START_ROW BOARD_SIZE - 1
#define BLACK_START_ROW 0
#define FORWARD_WHITE -1
#define FORWARD_BLACK 1

// Player-Color-Related Utilities
#define is_valid_color(c) (c == BLACK || c == WHITE)
#define other_color(c) ((c == BLACK) ? WHITE:BLACK)

// Piece Rank Labels
#define NO_RANK '*'
#define KING 'K'
#define QUEEN 'Q'
#define BISHOP 'B'
#define KNIGHT 'N'
#define ROOK 'R'
#define PAWN 'P'

// Gain-Related Values
//  Piece values(in aquisition or loss)
#define QUEEN_VALUE 9
#define BISHOP_VALUE 3
#define KNIGHT_VALUE 3
#define ROOK_VALUE 5
#define PAWN_VALUE 1
#define KING_VALUE ((QUEEN_VALUE+BISHOP_VALUE+KNIGHT_VALUE+ROOK_VALUE+PAWN_VALUE)*32)

// Helpful gain value(s)
#define MAX_GAIN KING_VALUE
#define NULL_GAIN (-KING_VALUE)

// Piece-Rank-Related Utilities
#define is_valid_rank(c) \
	(c == KING || c == QUEEN \
	 || c == BISHOP || c == KNIGHT \
	 || c == ROOK || c == PAWN)

// Game-Parameter-Related Utilities
#define get_rowc(n) (ROW_STARTC - n)
#define get_rown(c) ((int)(ROW_STARTC - c))
#define get_colc(n) (COL_STARTC + n)
#define get_coln(c) ((int)(c - COL_STARTC))
#define is_valid_rown(n) (n < BOARD_SIZE && n >= 0)
#define is_valid_coln(n) is_valid_rown(n)
#define is_valid_rowc(c) (c <= ROW_STARTC && c > ROW_STARTC - BOARD_SIZE)
#define is_valid_colc(c) (c < COL_STARTC + BOARD_SIZE && c >= COL_STARTC)

// Movement & Movement Side-Effect labels
#define MOVEMENT '-'
#define NO_MOVEMENT '\0'
#define NULL_EFFECT '\0'
#define NO_EFFECT '*'
#define CAPTURE 'x'
#define PROMOTION 'y'

// Movement Side-Effect utilities
#define is_valid_effect(c) (c == CAPTURE || c == PROMOTION || c == NO_EFFECT)

// Dimensional constants(row or column values)
#define NULL_DIMN -1
#define NULL_DIMC '\0'

// Classes of loss calculation for the `optimal_move` functions defined later
#define NO_LOSS 0
#define SELF_LOSS 1
#define ALL_LOSS 2

// Useful functions
unsigned int rand(void) {
    unsigned int rand_num;
    //*
    get_random_bytes(&rand_num, sizeof(rand_num));
    //*/
    return rand_num;
}

// Classes/enums/typedefs and associated functions
//  byte typedef
typedef unsigned char byte;

/*  Bool enum
typedef enum bool{
	false=0, 
	true=1
} bool;//*/

//  Move class
typedef struct Move{
	byte notation[MOVE_NOTATION_LENGTH + 1];
	char subject_color;
	int start[2];
	int stop[2];
	bool capture; 
	char captured_color;
	char captured_rank;
	bool promotion;
	char subject_prev_rank;
	char subject_next_rank;
	int gain;
} Move;

//  Piece class
typedef struct Piece{
	byte color;
	byte rank;
} Piece;

//  Chess_Game class
typedef struct Chess_Game{
	Piece board[BOARD_SIZE][BOARD_SIZE];
	bool check;
	char checked_color;
	bool checkmate;
	char checkmated_color;
	char player_color;
	char cpu_color;
	int cpu_king_loc[2];
	int player_king_loc[2];
	Move last_move;
} Chess_Game;

//  Piece_Attributes class
typedef struct Piece_Attributes{
	int piece_value;
	// Check if a piece can move from [r][c] to [r2][c2]
	bool (*can_move)(Chess_Game*, int, int, int, int); 
	// Check if a piece can capture [r2][c2] from [r][c]
	bool (*can_capture)(Chess_Game*, int, int, int, int);  
	// Find the highest value move for a piece
	Move (*optimal_move)(Chess_Game*, int, int, byte); 
} Piece_Attributes;

// Move functions
//  An almost null Move that is default initialized, but
//   makes everything as non-null as possible so a player can write over
//   its notation without the result being interpreted as a null and thus invaalid move
Move Move_template(void){
	Move m;
	// Notation
	m.notation[0] = NO_COLOR;
	m.notation[1] = NO_RANK;
	m.notation[2] = NULL_DIMC;
	m.notation[3] = NULL_DIMC;
	m.notation[4] = MOVEMENT;
	m.notation[5] = NULL_DIMC;
	m.notation[6] = NULL_DIMC;
	m.notation[7] = NO_EFFECT;
	m.notation[8] = NO_COLOR;
	m.notation[9] = NO_RANK;
	m.notation[10] = NO_EFFECT;
	m.notation[11] = NO_COLOR;
	m.notation[12] = NO_RANK;
	m.notation[MOVE_NOTATION_LENGTH] = 0;
	// Indexes
	{
		int i;
		for (i=0; i<2; ++i){
			m.start[i] = NULL_DIMN;
			m.stop[i] = NULL_DIMN;
		}
	}
	// Effects
	m.capture = false;
	m.captured_color = NO_COLOR;
	m.captured_rank = NO_RANK;
	m.promotion = false;
	// Movement subject data
	m.subject_color = m.notation[0];
	m.subject_prev_rank = m.notation[1];
	m.subject_next_rank = m.notation[11];
	// Gain
	m.gain = NULL_GAIN;
	
	return m;
}

//  Other Move constructors
Move Move_init0(void){
	Move m;
	// Notation
	byte* notation = m.notation;
	notation[0] = NO_COLOR;
	notation[1] = NO_RANK;
	notation[2] = NULL_DIMC;
	notation[3] = NULL_DIMC;
	notation[4] = NO_MOVEMENT;
	notation[5] = NULL_DIMC;
	notation[6] = NULL_DIMC;
	notation[7] = NULL_EFFECT;
	notation[8] = NO_COLOR;
	notation[9] = NO_RANK;
	notation[10] = NULL_EFFECT;
	notation[11] = NO_COLOR;
	notation[12] = NO_RANK;
	notation[MOVE_NOTATION_LENGTH] = 0;
	// Indexes
	{
		int i;
		for (i=0; i<2; ++i){
			m.start[i] = NULL_DIMN;
			m.stop[i] = NULL_DIMN;
		}
	}
	// Effects
	m.capture = false;
	m.captured_color = NO_COLOR;
	m.captured_rank = NO_RANK;
	m.promotion = false;
	// Movement subject data
	m.subject_color = m.notation[0];
	m.subject_prev_rank = m.notation[1];
	m.subject_next_rank = m.notation[11];
	// Gain
	m.gain = NULL_GAIN;
	
	return m;
}

Move Move_init2(byte notation[MOVE_NOTATION_LENGTH], int gain){
	Move m;
	int i;
	
	// Notation
	//  Formatting so the notation is properly analyzable
	{
		if (notation[7] == PROMOTION){
			for (i=7; i<10; ++i){
				notation[i+3] = notation[i];
			}
			notation[7] = NO_EFFECT;
			notation[8] = NO_COLOR;
			notation[9] = NO_RANK;
		}
	}
	//  Validatate the notation
	if (!is_valid_color(notation[0]) 
		|| !is_valid_rank(notation[1])
		|| !is_valid_colc(notation[2]) 
		|| !is_valid_rowc(notation[3])
		|| (notation[4] != MOVEMENT)
		|| !is_valid_colc(notation[5])
		|| !is_valid_rowc(notation[6]) 
		|| (notation[7] != NO_EFFECT && notation[7] != CAPTURE)
		|| (notation[7] == CAPTURE 
			&& (!is_valid_color(notation[8]) || !is_valid_rank(notation[9]))
			)
		|| (notation[10] != NO_EFFECT && notation[10] != PROMOTION)
		|| (notation[10] == PROMOTION 
			&& (!is_valid_color(notation[11]) || !is_valid_rank(notation[12])
				|| notation[1] != PAWN || notation[12] == notation[1] 
				|| notation[12] == KING) 
			)
		|| (notation[2] == notation[5] && notation[3] == notation[6])
		|| (notation[0] == notation[8])
		|| (notation[10] == PROMOTION && notation[0] != notation[11])
		)
	{
		return Move_init0();
	}
	//  Copy after validation
	for (i=0; i<MOVE_NOTATION_LENGTH; ++i){
		m.notation[i] = notation[i];
	}
	m.notation[MOVE_NOTATION_LENGTH] = 0;
	// Indexes
	m.start[0] = get_rown(m.notation[3]);
	m.start[1] = get_coln(m.notation[2]);
	m.stop[0] = get_rown(m.notation[6]);
	m.stop[1] = get_coln(m.notation[5]);
	// Effects
	m.capture = (m.notation[7] == CAPTURE);
	m.captured_color = m.notation[8];
	m.captured_rank = m.notation[9];
	m.promotion = (m.notation[10] == PROMOTION);
	// Movement subject data
	m.subject_color = m.notation[0];
	m.subject_prev_rank = m.notation[1];
	m.subject_next_rank = m.notation[12];
	// Total gain from the move's 
	//  effects(captures vs potential losses vs promotions)
	m.gain = gain;
	
	return m;
}

Move Move_init1(byte notation[MOVE_NOTATION_LENGTH]){
	return Move_init2(notation, 0);
}

Move Move_init10(
	Piece p, int row1, int col1, int row2, int col2, 
	bool capture, Piece captured, 
	bool promotion, Piece promoted,
	int gain
){
	// Notation
	byte notation[MOVE_NOTATION_LENGTH];
	notation[0] = p.color;
	notation[1] = p.rank;
	notation[2] = get_colc(col1);
	notation[3] = get_rowc(row1);
	notation[4] = MOVEMENT;
	notation[5] = get_colc(col2);
	notation[6] = get_rowc(row2);
	notation[7] = capture ? CAPTURE:NO_EFFECT;
	notation[8] = captured.color;
	notation[9] = captured.rank;
	notation[10] = promotion ? PROMOTION:NO_EFFECT;
	notation[11] = promoted.color;
	notation[12] = promoted.rank;
	
	// Other categories
	return Move_init2(notation, gain);
}

//  Piece functions
Piece Piece_init0(void){
	return (Piece){NO_COLOR, NO_RANK};
}
Piece Piece_init2(byte color, byte rank){
	return (Piece){color, rank};
}
bool are_allies(Piece p, Piece p2){
	return (
		is_valid_color(p.color) 
		&& is_valid_color(p2.color)
		&& p.color == p2.color
	);
}
bool are_enemies(Piece p, Piece p2){
	return (
		is_valid_color(p.color) 
		&& is_valid_color(p2.color)
		&& p.color != p2.color
	);
}

//  Piece_Attributes functions
//   Forward declarations
//    King function(s)
bool king_can_move(Chess_Game* game, int row, int col, int row2, int col2);
bool king_can_capture(Chess_Game* game, int row, int col, int row2, int col2);
Move king_optimal_move(Chess_Game* game, int row, int col, byte loss_class);
//    Queen function(s)
bool queen_can_move(Chess_Game* game, int row, int col, int row2, int col2);
bool queen_can_capture(Chess_Game* game, int row, int col, int row2, int col2);
Move queen_optimal_move(Chess_Game* game, int row, int col, byte loss_class);
//    Bishop function(s)
bool bishop_can_move(Chess_Game* game, int row, int col, int row2, int col2);
bool bishop_can_capture(Chess_Game* game, int row, int col, int row2, int col2);
Move bishop_optimal_move(Chess_Game* game, int row, int col, byte loss_class);
//    Knight function(s)
bool knight_can_move(Chess_Game* game, int row, int col, int row2, int col2);
bool knight_can_capture(Chess_Game* game, int row, int col, int row2, int col2);
Move knight_optimal_move(Chess_Game* game, int row, int col, byte loss_class);
//    Rook function(s)
bool rook_can_move(Chess_Game* game, int row, int col, int row2, int col2);
bool rook_can_capture(Chess_Game* game, int row, int col, int row2, int col2);
Move rook_optimal_move(Chess_Game* game, int row, int col, byte loss_class);
//    Pawn function(s)
bool pawn_can_move(Chess_Game* game, int row, int col, int row2, int col2);
bool pawn_can_capture(Chess_Game* game, int row, int col, int row2, int col2);
Move pawn_optimal_move(Chess_Game* game, int row, int col, byte loss_class);

//   Constructors
Piece_Attributes Piece_Attributes_init0(void){
	return (Piece_Attributes){
		0,
		NULL, NULL, NULL
	};
}
Piece_Attributes Piece_Attributes_init1(char rank){
	Piece_Attributes pa;
	switch (rank){
		case KING:
			pa.piece_value = KING_VALUE;
			pa.optimal_move = king_optimal_move;
			pa.can_capture = king_can_capture;
			pa.can_move = king_can_move;
			break;
		case QUEEN:
			pa.piece_value = QUEEN_VALUE;
			pa.optimal_move = queen_optimal_move;
			pa.can_capture = queen_can_capture;
			pa.can_move = queen_can_move;
			break;
		case BISHOP:
			pa.piece_value = BISHOP_VALUE;
			pa.optimal_move = bishop_optimal_move;
			pa.can_capture = bishop_can_capture;
			pa.can_move = bishop_can_move;
			break;
		case KNIGHT:
			pa.piece_value = KNIGHT_VALUE;
			pa.optimal_move = knight_optimal_move;
			pa.can_capture = knight_can_capture;
			pa.can_move = knight_can_move;
			break;
		case ROOK:
			pa.piece_value = ROOK_VALUE;
			pa.optimal_move = rook_optimal_move;
			pa.can_capture = rook_can_capture;
			pa.can_move = rook_can_move;
			break;
		case PAWN:
			pa.piece_value = PAWN_VALUE;
			pa.optimal_move = pawn_optimal_move;
			pa.can_capture = pawn_can_capture;
			pa.can_move = pawn_can_move;
			break;
		default: // null piece(no rank)
			pa = Piece_Attributes_init0();
			break;
	}
	return pa;
}

//   Helpers
bool can_promote(Chess_Game* game, int row, int col, int row2, int col2){
	bool invalid_params = (
		!is_valid_rown(row) || !is_valid_coln(col)
		|| !is_valid_rown(row2) || !is_valid_coln(col2)
	);
	int end_row = invalid_params 
				  ? NULL_DIMN
				  : (game->board[row][col].color == WHITE) ? 0:BOARD_SIZE-1;
	return (
		!invalid_params
		&& (game->board[row][col].rank == PAWN)
		&& (row2 == end_row)
	);
}

bool can_capture(Chess_Game* game, int row, int col){
	int i, j;
	Piece_Attributes pa;
	for (i=0; i<BOARD_SIZE; ++i){
		for(j=0; j<BOARD_SIZE; ++j){
			// Skip non-enemies
			if (!are_enemies(game->board[i][j], game->board[row][col])){
				continue;
			}
			// Check if the piece is capturable
			pa = Piece_Attributes_init1(game->board[i][j].rank);
			if (pa.can_capture(game, i, j, row, col)){
				return true;
			}
		}
	}
	return false;
}

int calc_loss(Chess_Game* game, char target_color){
	int enemy_gain = 0;
	int i, j;
	int curr_gain;
	Piece_Attributes pa;
	for (i=0; i<BOARD_SIZE && enemy_gain < MAX_GAIN; ++i){
		for (j=0; j<BOARD_SIZE && enemy_gain < MAX_GAIN; ++j){
			// Skip null or ally pieces
			if (game->board[i][j].color == NO_COLOR
				|| game->board[i][j].color == target_color)
			{
				continue;
			}
			// Calculate the most optimal enemy Move's gain
			pa = Piece_Attributes_init1(game->board[i][j].rank);
			curr_gain = pa.optimal_move(game, i, j, NO_LOSS).gain;
			enemy_gain = (enemy_gain < curr_gain) ? curr_gain:enemy_gain;
		}
	} 
	return enemy_gain;
}

Move piece_optimal_move(
	Chess_Game* game, int row, int col,
	int max_pos_offsets, int (*pos_offsets)[2], byte loss_class
){
	// Consider total gain(potential gains minus potential losses) from 
	//  moving to a certain position
	//  Gain primarily comes from captures and special cases like promotion.
	//  Loss is simply total enemy gain
	//  The maximum gain wil be used to decide values
	//  for variables that will be used to construct an optimal Move object

	//  Setup of Move object builders/variables
	int optimal_offsets[2] = {0, 0};
	int max_gain = NULL_GAIN;
	//   Always-applicable variables
	bool capture = false;
	//   Special case variables
	bool promoted = false;
	char promoted_rank = NO_RANK;
	//   Game terminating variables
	bool won = false;
	
	// Iterate over positional/capture offsets
	//  to find the optimal values for the Move object builders 
	//  by considering the total gain from every new position. Stop
	//  when the max gain is found(meaning we found a checkmate-creating move)
	int row2, col2;
	Piece self;
	Piece captured;
	Piece promoted_piece;
	
	Piece_Attributes pa = (
		Piece_Attributes_init1(game->board[row][col].rank)
	);
	Piece_Attributes pa2;
	int loss;
	int i;
	int gain;
	bool promotion;
	
	for (i=0; i<max_pos_offsets; ++i){
		int curr_r = row + pos_offsets[i][0];
		int curr_c = col + pos_offsets[i][1];
		
		// Ensure that the piece can move there
		if (!pa.can_move(game, row, col, curr_r, curr_c)){
			continue;
		}
		gain = 0;
		// Find and handle gain at this capture position
		pa2 = (
			Piece_Attributes_init1(game->board[curr_r][curr_c].rank)
		);
		promotion = false;
		
		// Check for and handle terminal non-losing cases like a king's capture
		won = (
			are_enemies(game->board[row][col], game->board[curr_r][curr_c]) 
			&& pa2.piece_value == KING_VALUE
		);
		if (won){
			gain = MAX_GAIN;
		}else{ // Look for gain in other forms
			// Check gain from captures
			gain += pa2.piece_value;
			// Special cases
			//  Check for gain from promotions
			if ((promotion = can_promote(game, row, col, curr_r, curr_c))){
				gain += QUEEN_VALUE - PAWN_VALUE; // Always promote to queen		
			}
			// Check for loss/enemy gain
			if (loss_class != NO_LOSS){
				// Temporarily enact the move
				Piece orig_at_origin = game->board[row][col];
				Piece orig_at_dest = game->board[curr_r][curr_c];
				game->board[row][col] = Piece_init0();
				game->board[curr_r][curr_c] = orig_at_origin;
				loss = 0;
				// Calculate loss/enemy gain
				if (loss_class == SELF_LOSS){ // Only calculate loss from this piece being captured
					loss = can_capture(game, curr_r, curr_c) ? pa.piece_value:0;
				}else{ // Calculate maximum loss considering the full game state
					loss = calc_loss(game, orig_at_origin.color);
				}
				if (loss == MAX_GAIN){ // We can undo and skip because this move is invalid
					game->board[curr_r][curr_c] = orig_at_dest;
					game->board[row][col] = orig_at_origin;
					continue; 
				}else{
					gain -= loss;
				}

				// Undo the move
				game->board[curr_r][curr_c] = orig_at_dest;
				game->board[row][col] = orig_at_origin;
			}
		}
		
		// Reassign Move variables if the gain is the new max
		//  or somewhat randomly if it is equal to the max
		if (gain > max_gain || (gain == max_gain && rand() % max_pos_offsets + 1 == 1)){
			max_gain = gain;
			optimal_offsets[0] = pos_offsets[i][0];
			optimal_offsets[1] = pos_offsets[i][1];
			capture = are_enemies(game->board[row][col], game->board[curr_r][curr_c]);
			promoted = promotion;
			promoted_rank = promotion ? QUEEN:NO_RANK;
		}
	}
	
	// Build and return the optimal move
	row2 = row + optimal_offsets[0];
	col2 = col + optimal_offsets[1];
	self = game->board[row][col];
	captured = capture ? game->board[row2][col2]:Piece_init0();
	promoted_piece = promoted
					 ? Piece_init2(self.color, promoted_rank):Piece_init0();
	return Move_init10(
		self, row, col, row2, col2, 
		capture, captured, 
		promoted, promoted_piece, 
		max_gain
	);
}

bool correct_direction(int forward, int row, int row2){
	return ((row2 - row)/forward >= 0);
}

bool vertical_obstructed(Chess_Game* game, int row, int col, int row2, int col2){
	int r_inc;
	if (!is_valid_rown(row) || !is_valid_coln(col)
		|| !is_valid_rown(row2) || !is_valid_coln(col2)
		|| row == row2
		|| col != col2)
	{
		return true;
	}
	r_inc = (row < row2) ? 1:-1;
	for (row+=r_inc; row!=row2; row+=r_inc){
		if (game->board[row][col].color != NO_COLOR){
			return true;
		}
	}
	return false;
}
bool horizontal_obstructed(Chess_Game* game, int row, int col, int row2, int col2){
	int c_inc;
	if (!is_valid_rown(row) || !is_valid_coln(col)
		|| !is_valid_rown(row2) || !is_valid_coln(col2)
		|| col == col2
		|| row != row2)
	{
		return true;
	}
	c_inc = (col < col2) ? 1:-1;
	for (col+=c_inc; col!=col2; col+=c_inc){
		if (game->board[row][col].color != NO_COLOR){
			return true;
		}
	}
	return false;
}
bool diagonal_obstructed(Chess_Game* game, int row, int col, int row2, int col2){
	int r_inc;
	int c_inc;
	if (!is_valid_rown(row) || !is_valid_coln(col)
		|| !is_valid_rown(row2) || !is_valid_coln(col2)
		|| row == row2
		|| col == col2
		|| ((row2 - row)*(row2 - row) != (col2 - col)*(col2 - col))
		)
	{
		return true;
	}
	r_inc = (row < row2) ? 1:-1; 
	c_inc = (col < col2) ? 1:-1;
	row += r_inc;
	col += c_inc;
	for (; row!=row2; row+=r_inc){
		if (game->board[row][col].color != NO_COLOR){
			return true;
		}
		col += c_inc;
	}
	return false;
}

//   King function(s)
bool king_can_move(Chess_Game* game, int row, int col, int row2, int col2){
	bool invalid_params = (
		!is_valid_rown(row) 
		|| !is_valid_coln(col)
		|| !is_valid_rown(row2) 
		|| !is_valid_coln(col2)
	);
	return (
		!invalid_params
		&& (game->board[row2][col2].color != game->board[row][col].color)
		&& ((row2 - row)*(row2 - row) <= 1)
		&& ((col2 - col)*(col2 - col) <= 1)
	);
}
bool king_can_capture(Chess_Game* game, int row, int col, int row2, int col2){
	return king_can_move(game, row, col, row2, col2);
}

Move king_optimal_move(Chess_Game* game, int row, int col, byte loss_class){
	int max_pos_offsets;
	int pos_offsets[8][2] = {
		{1,-1}, {1,0}, {1,1},
		{0,-1}, {0,1}, 
		{-1,-1}, {-1,0}, {-1,1}
	};
	
	// Return a null move upon invalid parameters
	if (!is_valid_rown(row) || !is_valid_coln(col)){
		return Move_init0();
	}
	
	// Return an optimal move based on movement and capture
	//  offset options
	max_pos_offsets = 8; 
	return piece_optimal_move(
		game, row, col,
		max_pos_offsets, pos_offsets, loss_class
	);
}
//   Queen function(s)
bool queen_can_move(Chess_Game* game, int row, int col, int row2, int col2){
	return (
		bishop_can_move(game, row, col, row2, col2)
		|| rook_can_move(game, row, col, row2, col2)
	);
}
bool queen_can_capture(Chess_Game* game, int row, int col, int row2, int col2){
	return queen_can_move(game, row, col, row2, col2);
}

Move queen_optimal_move(Chess_Game* game, int row, int col, byte loss_class){
	Move bishop_opt_move;
	Move rook_opt_move;
	// Return a null move upon invalid parameters
	if (!is_valid_rown(row) || !is_valid_coln(col)){
		return Move_init0();
	}
	
	// Return an optimal move based on movement and capture
	//  offset options
	bishop_opt_move = bishop_optimal_move(game, row, col, loss_class);
	rook_opt_move = rook_optimal_move(game, row, col, loss_class);
	return (
		bishop_opt_move.gain > rook_opt_move.gain
		? bishop_opt_move:rook_opt_move
	);
}
//   Bishop function(s)
bool bishop_can_move(Chess_Game* game, int row, int col, int row2, int col2){
	bool invalid_params = (
		!is_valid_rown(row) 
		|| !is_valid_coln(col)
		|| !is_valid_rown(row2) 
		|| !is_valid_coln(col2)
	);
	return (
		!invalid_params
		&& (game->board[row2][col2].color != game->board[row][col].color)
		&& !diagonal_obstructed(game, row, col, row2, col2)
	);
}
bool bishop_can_capture(Chess_Game* game, int row, int col, int row2, int col2){
	return bishop_can_move(game, row, col, row2, col2);
}

Move bishop_optimal_move(Chess_Game* game, int row, int col, byte loss_class){
	int max_pos_offsets;
	int pos_offsets[32][2];
	int i, j;
	
	// Return a null move upon invalid parameters
	if (!is_valid_rown(row) || !is_valid_coln(col)){
		return Move_init0();
	}
	
	// Return an optimal move based on movement and capture
	//  offset options
	max_pos_offsets = 32;
	{
		i = 0; 
		j = 0;
		for (i=0; i<max_pos_offsets/4; ++i){
			pos_offsets[j][0] = i + 1;
			pos_offsets[j][1] = i + 1;
			++j;
		}
		for (i=0; i<max_pos_offsets/4; ++i){
			pos_offsets[j][0] = i + 1;
			pos_offsets[j][1] = -(i + 1);
			++j;
		}
		for (i=0; i<max_pos_offsets/4; ++i){
			pos_offsets[j][0] = -(i + 1);
			pos_offsets[j][1] = -(i + 1);
			++j;
		}
		for (i=0; i<max_pos_offsets/4; ++i){
			pos_offsets[j][0] = -(i + 1);
			pos_offsets[j][1] = i + 1;
			++j;
		}
	} 
	return piece_optimal_move(
		game, row, col,
		max_pos_offsets, pos_offsets, loss_class
	);
}
//   Knight function(s)
bool knight_can_move(Chess_Game* game, int row, int col, int row2, int col2){
	bool invalid_params = (
		!is_valid_rown(row) 
		|| !is_valid_coln(col)
		|| !is_valid_rown(row2) 
		|| !is_valid_coln(col2)
	);
	return (
		!invalid_params
		&& (game->board[row2][col2].color != game->board[row][col].color)
		&& (
			((row2 - row)*(row2 - row) == 4 && (col2 - col)*(col2 - col) == 1)
			|| ((row2 - row)*(row2 - row) == 1 && (col2 - col)*(col2 - col) == 4)
		)
	);
}
bool knight_can_capture(Chess_Game* game, int row, int col, int row2, int col2){
	return knight_can_move(game, row, col, row2, col2);
}

Move knight_optimal_move(Chess_Game* game, int row, int col, byte loss_class){
	int max_pos_offsets;
	int pos_offsets[8][2] = {
		{2,-1}, {2,1}, {-2,-1}, {-2,1},
		{1,-2}, {1,2}, {-1,-2}, {-1,2}
	};
	
	// Return a null move upon invalid parameters
	if (!is_valid_rown(row) || !is_valid_coln(col)){
		return Move_init0();
	}
	
	// Return an optimal move based on movement and capture
	//  offset options
	max_pos_offsets = 8;
	return piece_optimal_move(
		game, row, col,
		max_pos_offsets, pos_offsets, loss_class
	);
}
//   Rook function(s)
bool rook_can_move(Chess_Game* game, int row, int col, int row2, int col2){
	bool invalid_params = (
		!is_valid_rown(row) 
		|| !is_valid_coln(col)
		|| !is_valid_rown(row2) 
		|| !is_valid_coln(col2)
	);
	bool (*has_obstruction)(Chess_Game*, int, int, int, int) = (
		(row != row2) ? vertical_obstructed:horizontal_obstructed
	);
	return (
		!invalid_params
		&& (game->board[row2][col2].color != game->board[row][col].color)
		&& ((row != row2) ^ (col != col2)) 
		&& !has_obstruction(game, row, col, row2, col2)
	);
}
bool rook_can_capture(Chess_Game* game, int row, int col, int row2, int col2){
	return rook_can_move(game, row, col, row2, col2);
}

Move rook_optimal_move(Chess_Game* game, int row, int col, byte loss_class){
	int max_pos_offsets;
	int pos_offsets[32][2] = {0};
	int i, j;
	
	// Return a null move upon invalid parameters
	if (!is_valid_rown(row) || !is_valid_coln(col)){
		return Move_init0();
	}
	
	// Return an optimal move based on movement and capture
	//  offset options
	max_pos_offsets = 32; 
	{
		i = 0; 
		j = 0;
		for (i=0; i<max_pos_offsets/4; ++i){
			pos_offsets[j][0] = i + 1;
			pos_offsets[j][1] = 0;
			++j;
		}
		for (i=0; i<max_pos_offsets/4; ++i){
			pos_offsets[j][0] = -(i + 1);
			pos_offsets[j][1] = 0;
			++j;
		}
		for (i=0; i<max_pos_offsets/4; ++i){
			pos_offsets[j][0] = 0;
			pos_offsets[j][1] = i + 1;
			++j;
		}
		for (i=0; i<max_pos_offsets/4; ++i){
			pos_offsets[j][0] = 0;
			pos_offsets[j][1] = -(i + 1);
			++j;
		}
	}
	return piece_optimal_move(
		game, row, col,
		max_pos_offsets, pos_offsets, loss_class
	);
}
//   Pawn function(s)
bool pawn_can_move(Chess_Game* game, int row, int col, int row2, int col2){
	bool invalid_params = (
		!is_valid_rown(row) 
		|| !is_valid_coln(col)
		|| !is_valid_rown(row2) 
		|| !is_valid_coln(col2)
	);
	int forward = invalid_params 
				  ? 0
				  : (game->board[row][col].color == WHITE) 
				  	? FORWARD_WHITE:FORWARD_BLACK;
	return (
		(!invalid_params
		&& correct_direction(forward, row, row2)
		&& (col == col2)
		&& (row != row2)
		&& game->board[row2][col].color == NO_COLOR
		&& ((row2 - row)*(row2 - row) <= 4)
		&& !vertical_obstructed(game, row, col, row2, col2))
		|| pawn_can_capture(game, row, col, row2, col2)
	);
}
bool pawn_can_capture(Chess_Game* game, int row, int col, int row2, int col2){
	bool invalid_params = (
		!is_valid_rown(row) 
		|| !is_valid_coln(col)
		|| !is_valid_rown(row2) 
		|| !is_valid_coln(col2)
	);
	int forward = invalid_params 
				  ? 0
				  : (game->board[row][col].color == WHITE) 
				  	? FORWARD_WHITE:FORWARD_BLACK;
	return (
		!invalid_params
		&& correct_direction(forward, row, row2)
		&& game->board[row2][col2].color != NO_COLOR
		&& game->board[row][col].color != game->board[row2][col2].color
		&& ((col2 - col)*(col2 - col) == 1)
		&& ((row2 - row)*(row2 - row) == 1)
	);
}

Move pawn_optimal_move(Chess_Game* game, int row, int col, byte loss_class){
	char piece_color;
	int forward;
	int max_pos_offsets;
	int min_pos_offsets;
	int pos_offsets[4][2];
	
	// Return a null move upon invalid parameters
	if (!is_valid_rown(row) || !is_valid_coln(col)){
		return Move_init0();
	}
	
	// Return an optimal move based on movement and capture
	//  offset options
	piece_color = game->board[row][col].color;
	forward = (game->board[row][col].color == WHITE) 
			  ? FORWARD_WHITE:FORWARD_BLACK;
	pos_offsets[0][0] = 1*forward;
	pos_offsets[1][0] = 2*forward;
	pos_offsets[0][1] = pos_offsets[1][1] = 0;
	max_pos_offsets = 4; 
	min_pos_offsets = 2; // When no captures can happen
	{
		if (is_valid_rown(row + 1*forward)){
			int fwd_row = row + 1*forward;
			int left_col = col - 1;
			int right_col = col + 1;
			
			if (is_valid_coln(left_col)
				&& are_enemies(game->board[row][col], game->board[fwd_row][left_col]))
			{
				pos_offsets[min_pos_offsets][0] = 1*forward;
				pos_offsets[min_pos_offsets][1] = -1;
				min_pos_offsets += 1;
			}
			if (is_valid_coln(right_col)
				&& are_enemies(game->board[row][col], game->board[fwd_row][right_col]))
			{
				pos_offsets[min_pos_offsets][0] = 1*forward;
				pos_offsets[min_pos_offsets][1] = 1;
				min_pos_offsets += 1;
			}
		}
		max_pos_offsets = min_pos_offsets;
	}
	return piece_optimal_move(
		game, row, col,
		max_pos_offsets, pos_offsets, loss_class
	);
}

//  Chess_Game functions
//   Forward declarations
Move Chess_Game_cpu_move(Chess_Game* game, char color, bool no_render);

//   Definitions
Chess_Game Chess_Game_init2(char player_color, char cpu_color){
	// Initialize and return the game object
	Chess_Game game;
	int i, j;
	
	//  Initialize the board to contain the pieces in their starting positions
    //   Place black pieces at the top
    game.board[0][0] = Piece_init2(BLACK, ROOK);
    game.board[0][1] = Piece_init2(BLACK, KNIGHT);
    game.board[0][2] = Piece_init2(BLACK, BISHOP);
    game.board[0][3] = Piece_init2(BLACK, QUEEN);
    game.board[0][4] = Piece_init2(BLACK, KING);
    game.board[0][5] = Piece_init2(BLACK, BISHOP);
    game.board[0][6] = Piece_init2(BLACK, KNIGHT);
    game.board[0][7] = Piece_init2(BLACK, ROOK);
    for (i=0; i<BOARD_SIZE; i++) {
        game.board[1][i] = Piece_init2(BLACK, PAWN);
    }
    //  Place white pieces at the bottom
    game.board[7][0] = Piece_init2(WHITE, ROOK);
    game.board[7][1] = Piece_init2(WHITE, KNIGHT);
    game.board[7][2] = Piece_init2(WHITE, BISHOP);
    game.board[7][3] = Piece_init2(WHITE, QUEEN);
    game.board[7][4] = Piece_init2(WHITE, KING);
    game.board[7][5] = Piece_init2(WHITE, BISHOP);
    game.board[7][6] = Piece_init2(WHITE, KNIGHT);
    game.board[7][7] = Piece_init2(WHITE, ROOK);
    for (i=0; i<BOARD_SIZE; i++) {
        game.board[6][i] = Piece_init2(WHITE, PAWN);
    }
    // Initialize the rest of the board with empty pieces
    for (i=2; i<6; i++) {
        for (j=0; j<BOARD_SIZE; j++) {
            game.board[i][j] = Piece_init0();
        }
    }
	// Set the check statuses and the location of the 2 kings
	game.cpu_king_loc[0] = 0;
	game.player_king_loc[0] = 7;
	game.cpu_king_loc[1] = game.player_king_loc[1] = 4;
	game.checked_color = game.checkmated_color = NO_COLOR;
	// Set the colors of the player and cpu
	game.player_color = player_color;
	game.cpu_color = cpu_color;
	// Initialize the last move to a null move
	game.last_move = Move_init0();

	// Return the board
	return game;
}

Move Chess_Game_render_move(
	Chess_Game* game, Move m, bool no_self_check, 
	char color, bool validate
){
	bool must_capture;
	bool must_promote;
	int* king_loc;
	bool checkmate;
	bool check;
	int i;
	char colors[2] = {color, other_color(color)};
	int* king_locs[2] = {
		(color == game->player_color) 
		? game->player_king_loc:game->cpu_king_loc,
		(color == game->player_color) 
		? game->cpu_king_loc:game->player_king_loc
	};
	Piece_Attributes pa;

	// Don't process null moves
	if (m.subject_color == NO_COLOR){
		return Move_init0();
	}

	// Return a null move when the given move is invalid. 
	//  Cpu moves are assumed to be valid
	if (validate){
		// Validate the move
		//  Ensure that the ranks and colors described for the 
		//   given locations are correct
		if (game->board[m.start[0]][m.start[1]].color != m.subject_color
			|| game->board[m.start[0]][m.start[1]].rank != m.subject_prev_rank
			|| game->board[m.stop[0]][m.stop[1]].rank != m.captured_rank)
		{
			return Move_init0();
		}
		// Get data on the piece
		pa = Piece_Attributes_init1(game->board[m.start[0]][m.start[1]].rank); 
		//  Ensure that positional changes are valid
		if (!pa.can_move // Null when the piece was null
			|| !pa.can_move(game, m.start[0], m.start[1], m.stop[0], m.stop[1])
			)
		{
			return Move_init0();
		}
		//  Ensure captures are valid or enforce the 
		//   notation of a capture if it occurred but was not explicitly recorded
		must_capture = (
			are_enemies(game->board[m.start[0]][m.start[1]], game->board[m.stop[0]][m.stop[1]])
			&& pa.can_capture(game, m.start[0], m.start[1], m.stop[0], m.stop[1])
		);
		if (m.capture != must_capture){ 
			return Move_init0();
		}
		//  Ensure promotions are valid or enforce the 
		//   notation of a promotion if it occurred but was not explicitly recorded
		must_promote = can_promote(game, m.start[0], m.start[1], m.stop[0], m.stop[1]);
		if (m.promotion != must_promote){ 
			return Move_init0();
		}
	}

	// Render the move
	//  Render the actual movement and possible capture
	game->board[m.stop[0]][m.stop[1]] = game->board[m.start[0]][m.start[1]];
	game->board[m.start[0]][m.start[1]] = Piece_init0();
	//  Render promotions
	if (m.promotion){
		game->board[m.stop[0]][m.stop[1]] = (
			Piece_init2(m.subject_color, m.subject_next_rank)
		);
	}

	// Update the appropriate king's location to be used later if the king was moved
	if (m.subject_prev_rank == KING){
		king_locs[0][0] = m.stop[0];
		king_locs[0][1] = m.stop[1];
	}

	// For each color, determine/update its statuses if it is checkmated or in check
	{
		for (i=0; i<2; ++i){
			king_loc = king_locs[i];
			check = can_capture(game, king_loc[0], king_loc[1]);
			// If the player puts themself in check, it's essentially checkmate as the cpu
			//  will take their king on its turn. This may or may not be allowed
			if (check && m.subject_color == colors[i]){
				// If we aren't allowed to check themselves, thensimply undo the move
				//  and return a null move
				if (no_self_check){
					game->board[m.start[0]][m.start[1]] = Piece_init2(m.subject_color, m.subject_prev_rank);
					game->board[m.stop[0]][m.stop[1]] = Piece_init2(m.captured_color, m.captured_rank);
					return Move_init0();
				}
				// Otherwise, declare checkmate
				else{
					checkmate = true;
				}
			}
			// Update the game statuses for check, checkmate, etc
			checkmate = (
				Chess_Game_cpu_move(game, color, true).gain == NULL_GAIN
			);
			game->check = check;
			game->checkmate = checkmate;
			game->checkmated_color = game->checkmate ? colors[i]:NO_COLOR;
			game->checked_color = game->check ? colors[i]:NO_COLOR;
			// Stop once a check[mate] state has ocurred
			if (game->check || game->checkmate){ 
				break;
			}
		}
	}

	// Update the game's last move
	game->last_move = m;

	// Return the move as it came in, thereby indicating its validity
	return m;
}

Move Chess_Game_cpu_move(Chess_Game* game, char color, bool no_render){
	// Find an optimal move.
	//  Choose the first Move with the highest gain
	Move optimal_move = Move_init0();
	int i, j;
	Piece_Attributes pa;
	Move curr_move;
	
	for (i=0; i<BOARD_SIZE && optimal_move.gain < MAX_GAIN; ++i){
		for (j=0; j<BOARD_SIZE && optimal_move.gain < MAX_GAIN; ++j){
			// Skip non-ally pieces
			if (game->board[i][j].color != color){
				continue;
			}
			// Find an optimal move based on maximum gain.
			//  When moves are equally optimal, switch to the other move
			//  somewhat randomly
			pa = Piece_Attributes_init1(
				game->board[i][j].rank
			);
			curr_move = pa.optimal_move(game, i, j, ALL_LOSS);
			if (curr_move.gain > optimal_move.gain 
				|| (curr_move.gain == optimal_move.gain 
					&& rand() % 32 + 1 == 1))
			{
				optimal_move = curr_move;
			}
		}
	}
	
	// Simpmly return the move if that is desired
	if (no_render){
		return optimal_move;
	}
	
	// Simulate and return the result
	return Chess_Game_render_move(game, optimal_move, false, color, false);
}

#endif //CHESS_C