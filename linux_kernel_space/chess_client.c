#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

bool streq(const char* s, const char* s2, int start, int end){
	int i = start;
	for (; i<end && s[i] && s2[i] && s[i] == s2[i]; ++i){}
	return (i == end || (!s[i] && !s2[i]));
}

#define INPUT_BUFF_SIZE 100
#define RESPONSE_BUFF_SIZE 1000

int main(){
	// Setup
	unsigned char input_buff[INPUT_BUFF_SIZE] = {0};
	unsigned char response_buff[RESPONSE_BUFF_SIZE] = {0};
    const char proc_file[] = "/proc/chess_driver";
    
    //  Open proc file for the chess API, 
    //   thereby opening a communication channel to it
    int fd = open(proc_file, O_RDWR);
	if (fd < 0) {
	    perror("Failed to open channel to chess module file");
	    return 1;
	}else{
		printf("Connected to chess API\n\n");
	}
	
	// Main loop
	while (true){
		// Get the player's move
		printf("Command: ");
		if (!fgets(input_buff, sizeof(input_buff), stdin)){
			perror("Could not receive input");
			continue;
		}
		
		// Find the cmd len
		int cmd_len = 0;
		{
			int i;
			for (i=0; i<sizeof(input_buff); ++i){
				if (input_buff[i] == '\n'){
					break;
				}else{++cmd_len;}
			}
			input_buff[cmd_len] = 0;
		}
		
		// Handle quiting
		if (cmd_len > 0 && streq(input_buff, "quit", 0, cmd_len)){
			break;
		}
		
		// Send the command off to the API and recieve the reponse
		//  Send the command/request by writing data to the /proc file
		ssize_t n_written = write(fd, input_buff, cmd_len);
		if (n_written < cmd_len) {
		    perror("Error occurred during write");
		    continue;
		}else{
			printf(
				"Request, '%s'(char [%d]), sent to API, "
				"awaiting response...\n", input_buff, cmd_len
			);
		}
		//  Read the response into the response buffer
		ssize_t response_size = read(fd, response_buff, RESPONSE_BUFF_SIZE - 1);
		if (response_size < 0) {
		    perror("Failed to recieve response: ");
		    printf("\n");
		    continue;
		}else{
			printf("Response: ");
		}
		//  Terminate the response
		response_buff[response_size] = 0;
		
		// Print the response 
		printf("%s\n", response_buff);

		// Reset the response buff
		{
			int i;
			for (i=0; i<sizeof(response_buff); ++i){
				response_buff[i] = 0;
			}
		} 
	}
	
	// Close the proc file/channel to the chess api
	close(fd);
	printf("Connection closed\nProgram terminated\n\n");
	
	return 0;
}
