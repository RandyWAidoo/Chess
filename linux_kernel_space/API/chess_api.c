#ifndef CHESS_API
#define CHESS_API

#include "chess.c"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/errno.h>

// File operation declarations
#define DEVICE_NAME "chess_driver"

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static const struct proc_ops proc_fops = {
	 .proc_read = device_read,
	 .proc_write = device_write,
	 .proc_open = device_open,
	 .proc_release = device_release
};

// Module-wide variables and constatns
static int device_open_count = 0;

static Chess_Game game;
static bool game_started = false; 
static bool turn = 0;
static bool player_turn = 0;
static bool cpu_turn = 1;
static char player_color;
static char cpu_color;
#define CMD_ARG_OFFSET 3
#define RESPONSE_BUFF_SIZE 1000
static bool no_self_check = true;

static char input_buff[CMD_ARG_OFFSET + MOVE_NOTATION_LENGTH + 1] = {0};
static byte response_buff[RESPONSE_BUFF_SIZE + 1] = {0};

// init and exit methods which create and remove the proc entry for this module
static struct proc_dir_entry* proc_entry;

static int __init chess_init(void) {
	 proc_entry = proc_create(DEVICE_NAME, 0666, NULL, &proc_fops);
	 printk(KERN_INFO "Chess driver loaded");
	 return 0;
}
static void __exit chess_exit(void) {
	 proc_remove(proc_entry);
	 printk(KERN_INFO "Chess driver terminated");
}

// Indicate the init and exit functions and other module metadata to the kernel
module_init(chess_init);
module_exit(chess_exit);
MODULE_AUTHOR("Randy Wiredu-Aidoo");
MODULE_DESCRIPTION(
	"An api for a chess game which handles and responds" 
	"to player requests relating to the game"
);
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");

// Define file operation functions
static ssize_t device_read(
	struct file *file, char __user* user_buff, 
	size_t len, loff_t *offset)
{
	int response_len, copied;
	
	for (response_len=0; response_len<RESPONSE_BUFF_SIZE; ++response_len){
		if (!response_buff[response_len]){
			break;
		}
	}
	len = (response_len < len) ? response_len:len;
	copied = len - copy_to_user(user_buff, response_buff, len);
	if (copied < len){
		return -EFAULT;
	}
	printk(KERN_INFO "Response: %s\n", response_buff);
	return len;
}

bool streq(const char* s, const char* s2, int start, int end){
	int i = start;
	for (; i<end && s[i] && s2[i] && s[i] == s2[i]; ++i){}
	return (i == end || (!s[i] && !s2[i]));
}
void respond(byte* dest, const byte* src, int len){
	int i;
	for (i=0; i<len; ++i){
		dest[i] = src[i];
	}
	dest[len] = 0;
}
static ssize_t device_write(
	struct file* file, const char __user* request, 
	size_t len, loff_t* offset)
{
	int i;
	Move m;
	int bytes_not_copied;
	int cmd_len;
	int j;
	
	// Create a reference to the response buffer 
	//  that can be safely manipulated without 
	//  affecting the actual buffer variable
	byte* response_iterator = response_buff;
	
	// Setup and copy to the kernel space input buffer
	//  Overwrite the input buffer to set it to a default state
	Move move_template = Move_template();
	for (i=0; i<CMD_ARG_OFFSET; ++i){
		input_buff[i] = 0;
	}
	for (i=0; i<CMD_ARG_OFFSET + MOVE_NOTATION_LENGTH; ++i){
		input_buff[i] = move_template.notation[i - CMD_ARG_OFFSET];
	}
	input_buff[CMD_ARG_OFFSET + MOVE_NOTATION_LENGTH] = 0;

	// Error on 0 length cmds or cmds that are too long
	if (!len){
		printk(KERN_INFO "Request: <None>\n");
		respond(response_iterator, "UNKCMD\n", 7);
		return len;
	}else if (len > CMD_ARG_OFFSET + MOVE_NOTATION_LENGTH){
		printk(KERN_INFO "Request too long\n");
		respond(response_iterator, "UNKCMD\n", 7);
		return len;
	}
	
	// Copy the user's request into the input buffer
	bytes_not_copied = copy_from_user(input_buff, request, len);
	if (bytes_not_copied){
		return len - bytes_not_copied;
	}
	input_buff[len] = 0;
	printk(KERN_INFO "Request: %s\n", input_buff);
	
	// Handle the request
	{
		// Fill out the rest of the input_buff with default notation values
		//  from the move template
		cmd_len = len;
		for (i=(cmd_len<CMD_ARG_OFFSET) ? CMD_ARG_OFFSET:cmd_len; 
			 i<CMD_ARG_OFFSET + MOVE_NOTATION_LENGTH; 
			 ++i)
		{
			input_buff[i] = move_template.notation[i - CMD_ARG_OFFSET];
		}
		input_buff[CMD_ARG_OFFSET + MOVE_NOTATION_LENGTH] = 0;
		
		// Handle resignation/quiting
		if (streq(input_buff, "04", 0, cmd_len)){
			if (game_started){
				game_started = false;
				respond(response_iterator, "OK\n", 3);
			}else{
				respond(response_iterator, "NOGAME\n", 7);
			}
		}

		// Handle a request for the CPU to make a move
		else if (streq(input_buff, "03", 0, cmd_len)){
			if (game_started){
				if (turn == cpu_turn){
					// Perform the CPU's move
					m = Chess_Game_cpu_move(&game, cpu_color, false);
					respond(response_iterator, m.notation, MOVE_NOTATION_LENGTH); 
					response_iterator += MOVE_NOTATION_LENGTH;
					respond(response_iterator, "\n", 1); response_iterator += 1;
					
					// Handle game statuses
					if (game.checkmate){
						if (game.checkmated_color == player_color){
							respond(response_iterator, "MATE\n", 5);
							game_started = false;
						}else{
							respond(response_iterator, "OK\n", 3);
						}
					}else if (game.check){
						if (game.checked_color == player_color){
							respond(response_iterator, "CHECK\n", 6);
						}else{
							respond(response_iterator, "OK\n", 3);
						}
					}else{
						respond(response_iterator, "OK\n", 3);
					}
					
					// Increment turn
					turn = (turn + 1) % 2;
				}else{
					respond(response_iterator, "OOT\n", 4);
				}
			}else{
				respond(response_iterator, "NOGAME\n", 7);
			}
		}

		// Handle a player request to make a move
		else if (streq(input_buff, "02 ", 0, 3)){
			if (game_started){
				if (turn == player_turn){
					// Attempt to render the move 
					m = Move_init1(input_buff + CMD_ARG_OFFSET);
					if (m.subject_color == NO_COLOR){
						respond(response_iterator, "INVFMT\n", 7);
						return len;
					}
					// Handle when the player uses wrong color to start with
					else if (input_buff[3] != player_color){
						respond(response_iterator, "ILLMOVE\n", 8);
						return len;
					}
					m = Chess_Game_render_move(&game, m, no_self_check, player_color, true);

					// Handle the result of the move e.g. ILLMOVE errors or post-move game status
					if (m.subject_color == NO_COLOR){
						respond(response_iterator, "ILLMOVE\n", 8);
						return len;
					}else if (game.checkmate){
						if (game.checkmated_color == cpu_color){
							respond(response_iterator, "MATE\n", 5);
							game_started = false;
						}else{
							respond(response_iterator, "OK\n", 3);
						}
					}else if (game.check){
						if (game.checked_color == cpu_color){
							respond(response_iterator, "CHECK\n", 6);
						}else{
							respond(response_iterator, "OK\n", 3);
						}
					}else{
						respond(response_iterator, "OK\n", 3);
					}

					// Increment turn
					turn = (turn + 1) % 2;
				}else{
					respond(response_iterator, "OOT\n", 4);
				}
			}else{
				respond(response_iterator, "NOGAME\n", 7);
			}
		}

		// Handle a request to show the board
		else if (streq(input_buff, "01", 0, cmd_len)){
			if (game_started){
				{
					respond(response_iterator, "\n", 1); response_iterator += 1;
					
					respond(response_iterator, "   ", 3); response_iterator += 3;
					for (i=0; i<BOARD_SIZE; ++i){
						char repr[] = " c   ";
						repr[1] = get_colc(i);
						respond(response_iterator, repr, 5); response_iterator += 5;
					}
					respond(response_iterator, "\n", 1); response_iterator += 1;
					for (i=0; i<BOARD_SIZE; ++i){
						char repr[] = "c |";
						repr[0] = get_rowc(i);
						respond(response_iterator, repr, 3); response_iterator += 3;
						for (j=0; j<BOARD_SIZE; ++j){
							char repr[] = " cc |";
							repr[1] = game.board[i][j].color;
							repr[2] = game.board[i][j].rank;
							respond(response_iterator, repr, 5); response_iterator += 5;
						}
						respond(response_iterator, "\n", 1); response_iterator += 1;
					}
				}
			}else{
				respond(response_iterator, "NOGAME\n", 7);
			}
		}

		// Handle starting a game
		else if (streq(input_buff, "00 ", 0, 3) && cmd_len == 4){
			if (is_valid_color(input_buff[3])){
				game_started = true;
				player_color = input_buff[3];
				cpu_color = other_color(input_buff[3]);
				turn = 0;
				game = Chess_Game_init2(player_color, cpu_color);
				respond(response_iterator, "OK\n", 3);
			}else{
				respond(response_iterator, "UNKCMD\n", 7);
			}
		}

		// Unrecognized cmds
		else{
			respond(response_iterator, "UNKCMD\n", 7);
		}
	}
	
	return len;
}

static int device_open(struct inode *inode, struct file *file) {
	 if (device_open_count) {
	 	return -EBUSY;
	 }
	 device_open_count++;
	 try_module_get(THIS_MODULE);
	 return 0;
}

static int device_release(struct inode *inode, struct file *file) {
	 device_open_count--;
	 module_put(THIS_MODULE);
	 return 0;
}

#endif //CHESS_API