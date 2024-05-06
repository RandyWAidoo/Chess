#include "chess.c"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

bool streq(const char* s, const char* s2, int start, int end){
	int i = start;
	for (; i<end && s[i] && s2[i] && s[i] == s2[i]; ++i){}
	return (i == end || (!s[i] && !s2[i]));
}

int main(){
	// Setup
	bool debug = false;
	Chess_Game game;
	bool game_started = false;
	bool turn = 0;
	bool player_turn = 0;
	bool cpu_turn = 1;
	char player_color;
	char cpu_color;

	const int cmd_content_offset = 3;
	bool no_self_check = true;
	unsigned char input_buff[cmd_content_offset + MOVE_NOTATION_LENGTH + 1];
	Move move_template = Move_template();
	{
		int i;
		for (i=0; i<cmd_content_offset; ++i){
			input_buff[i] = 0;
		}
		for (; i<cmd_content_offset + MOVE_NOTATION_LENGTH; ++i){
			input_buff[i] = move_template.notation[i - cmd_content_offset];
		}
		input_buff[cmd_content_offset + MOVE_NOTATION_LENGTH] = 0;
	}
	
	// Main loop
	while (true){
		// Get the player's move
		printf("Command: ");
		if (!fgets(input_buff, cmd_content_offset + MOVE_NOTATION_LENGTH + 1, stdin)){
			perror("Could not receive input");
			continue;
		}
		
		// Find the cmd len and undo the '\n'
		int cmd_len = 0;
		{
			int i;
			for (i=0; i<cmd_content_offset + MOVE_NOTATION_LENGTH; ++i){
				if (input_buff[i] == '\n'){
					break;
				}else{++cmd_len;}
			}

			if (!cmd_len){// Error on 0 length cmds 
				printf("UNKCMD\n");
				continue;
			}

			for (i=(i<cmd_content_offset) ? cmd_content_offset:i; 
				 i<cmd_content_offset + MOVE_NOTATION_LENGTH; 
				 ++i)
			{
				input_buff[i] = move_template.notation[i - cmd_content_offset];
			}
			input_buff[cmd_content_offset + MOVE_NOTATION_LENGTH] = 0; //In case '\n' is at the end or the user entered too much
		}
		
		// Handle resignation and quiting
		if (streq(input_buff, "04", 0, cmd_len)){
			printf("OK\n");
			game_started = false;
		}else if (streq(input_buff, "quit", 0, cmd_len)){
			printf("OK\n");
			break;
		}

		// Handle a request for the CPU to make a move
		else if (streq(input_buff, "03", 0, 2)){
			if (game_started){
				if (turn == cpu_turn){
					// Perform the CPU's move
					Move m = Chess_Game_cpu_move(&game, cpu_color, false);
					printf("Opponent's move: %s\n", m.notation);
					Chess_Game_print(&game);
					
					// Handle game statuses
					if (game.checkmate){
						if (game.checkmated_color == player_color){
							printf("CHECKMATE PLAYER'S LOSS\n");
						}else{
							printf("CHECKMATE CPU'S LOSS\n");
						}
						game_started = false;
					}else if (game.check){
						if (game.checked_color == player_color){
							printf("CHECK\n");
						}else{
							printf("OK\n");
						}
					}else{
						printf("OK\n");
					}
					
					// Increment turn
					turn = (turn + 1) % 2;
				}else{
					printf("OOT\n");
				}
			}else{
				printf("NOGAME\n");
			}
		}

		// Handle a player request to make a move
		else if (streq(input_buff, "02 ", 0, 3)){
			if (game_started){
				if (turn == player_turn){
					// Attempt to render the move 
					Move m = Move_init2(input_buff + cmd_content_offset, NULL_GAIN);
					if (m.subject_color == NO_COLOR){
						printf("INVFMT\n");
						continue;
					}
					// Handle when the player uses wrong color to start with
					else if (input_buff[3] != player_color){
						printf("ILLMOVE\n");
						continue;
					}
					m = Chess_Game_render_move(&game, m, no_self_check, player_color, true);
					Chess_Game_print(&game);

					// Handle the result of the move e.g. ILLMOVE errors or post-move game status
					if (m.subject_color == NO_COLOR){
						printf("ILLMOVE\n");
						continue;
					}else if (game.checkmate){
						if (game.checkmated_color == cpu_color){
							printf("CHECKMATE CPU'S LOSS\n");
						}else{
							printf("CHECKMATE PLAYER'S LOSS\n");
						}
						game_started = false;
					}else if (game.check){
						if (game.checked_color == cpu_color){
							printf("CHECK\n");
						}else{
							printf("OK\n");
						}
					}else{
						printf("OK\n");
					}

					// Increment turn
					if (!debug){
						turn = (turn + 1) % 2;
					}
				}else{
					printf("OOT\n");
				}
			}else{
				printf("NOGAME\n");
			}
		}

		// Handle a request to show the board
		else if (streq(input_buff, "01", 0, 2)){
			if (game_started){
				Chess_Game_print(&game);
			}else{
				printf("NOGAME\n");
			}
		}

		// Handle starting a game
		else if (streq(input_buff, "00 ", 0, 3)){
			if (game_started){
				printf("ILLMOVE\n");
			}else{
				if (is_valid_color(input_buff[3])){
					game_started = true;
					player_color = input_buff[3];
					cpu_color = other_color(input_buff[3]);
					turn = 0;
					game = Chess_Game_init2(player_color, cpu_color);
					{// Determine if we're playing with no self checks
						printf("Prohibit self-checks(y/n)?: ");
						if (!fgets(input_buff, 2, stdin)){
							perror("Could not receive input");
							continue;
						}
						no_self_check = (input_buff[0] == 'y');
					}
					Chess_Game_print(&game);
					printf("OK\n");
				}else{
					printf("UNKCMD\n");
				}
			}
		}

		// Unrecognized cmds
		else{
			printf("UNKCMD\n");
		}
	}
	
	printf("Stopping...\n");
	
	return 0;
}
