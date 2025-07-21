//built-in libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
//custom libraries
#include "string_parser.h"
#include "command.h"
int main(int argc, char** argv)
{
	/*We're going to need to use the command_line struct and reset it after every command, this is probably
	 * the best way to do it for now. */
	//buffer size for getline. Just make it really big.
	size_t buff_size = 1024;
	command_line large_token_buffer; //for each command seperated by a ';'
	command_line small_token_buffer; //for each command seperated by a ' '
	//flag for the getopt function
	//./pseudo_shell -f <filename> is the usage. So only one flag.
	int opt;
	//input file pointer for -f mode
	char *input_file = NULL;
	char* output_file = "output.txt"; //output file name.
	FILE *fp; //file pointer 
	//kind of silly to do it this way, but one can add more later if you want to build on it
	while((opt = getopt(argc, argv, "f:")) != -1)
	{
		switch(opt)
		{
			case 'f':
				input_file = optarg;
				break;

			default:
				printf("default case, no args passed\n");
				break;
		}
	}
	//printf("input file: %s\n", input_file);
	//open file for reading/writing for non-interactive mode
	if(input_file != NULL)
	{
		//open file for stdin
		fp = fopen(input_file, "r");
		//open file for stdout
		freopen(output_file, "w", stdout);
	}

    while(1)
	{	
		size_t characters; //for getline. can also be unsigned long if you don't like size_t.
		//malloc buffer
		char* buffer;
		buffer = (char*)malloc(sizeof(char) * buff_size);
		if(input_file == NULL)
		{
			write(STDOUT_FILENO, ">>> ", 4);
			characters = getline(&buffer, &buff_size, stdin);
		}
		//write(STDOUT_FILENO, ">>> ", 4);
		//Be aware, getline() appends a '\n' character to the end of every string when used this way.
		//characters = getline(&buffer, &buff_size, stdin);
		//
		else
		{
			characters = getline(&buffer, &buff_size, fp);
		}
		//specialized exit for when we get to the end of a line in file mode
		if((characters == -1) && (input_file != NULL))
		{
			//write(STDOUT_FILENO, "EOF reached\n", 12);
			fclose(fp);
			free(buffer);
			//return 0;
			exit(0);
		}
		//tokenize input into command segments
		large_token_buffer = str_filler(buffer, ";");
		//let's print this shit out to make sure it worked
		for(int i = 0; large_token_buffer.command_list[i] != NULL; i++)
		{
			//check if any large tokens were the exit command
			/* there is some weirdness with the tokenization; if you have a space after a ';' character then the large_token will
			 * retain this. However, it will not retain this delimiter for the small tokens becuase we strip it. Therefore ";exit" 
			 * will terminate the program but "; exit" will not. These are 2 different strings, and as we haven't stripped out
			 * " " delimiters yet, we need to make a very dirty manuel check for this special command. We could make it a bit
			 * more clean by doing a second check once we tokenize, but this is one stupid way of doing it.*/
			if(strcmp(large_token_buffer.command_list[i], "exit") == 0 || strcmp(large_token_buffer.command_list[i], " exit") == 0 || strcmp(large_token_buffer.command_list[i], "exit ") == 0)
			{
				//debug print statements
				//printf("terminating shell\n");
				free_command_line(&large_token_buffer);
				memset(&large_token_buffer, 0, 0);
				free(buffer);
				//break out of main loop
				exit(0);
			}
			//tokenize each word in each larger token. This is basically checking for inputs to a command.
			small_token_buffer = str_filler(large_token_buffer.command_list[i], " ");
			//iterate each token, and check if a command is entered
			for(int j = 0; small_token_buffer.command_list[j] != NULL; j++)
			{
				//printf("\t\tToken %d:%s\n", j + 1, small_token_buffer.command_list[j]);
				//the ugliest fucking if-elseif-else hell I have ever done seen
				//ls
				if(strcmp(small_token_buffer.command_list[j], "ls") == 0)
				{
					if(small_token_buffer.num_token == 1)
					{
						listDir();
					}
					else
					{
						write(STDOUT_FILENO, "Error! Unsupported parameters for command: ls\n", 47);
						//consume the rest of the input
						j += (small_token_buffer.num_token - 1);
					}
				}
				//pwd
				else if(strcmp(small_token_buffer.command_list[j], "pwd") == 0)
				{
					if(small_token_buffer.num_token == 1)
					{
						showCurrentDir();
					}
					else
					{
						write(STDOUT_FILENO, "Error! Unsupported parameters for command: pwd\n", 48);
						//consume the rest of the input
						j += (small_token_buffer.num_token - 1);
					}

				}
				//mkdir
				else if(strcmp(small_token_buffer.command_list[j], "mkdir") == 0)
				{
					//check if input was provided
					if((small_token_buffer.num_token == 2) && (small_token_buffer.command_list[j + 1] != NULL))
					{
						makeDir(small_token_buffer.command_list[j + 1]);
						//force the token counter to move, essentially 'eating' the input.
						j++;
					}
					else
					{
						write(STDOUT_FILENO, "Error! Unsupported parameters for command: mkdir\n", 50);
						//consume the rest of the input
						j += (small_token_buffer.num_token - 1);
					}
				}
				//cd
				else if(strcmp(small_token_buffer.command_list[j], "cd") == 0)
				{
					if((small_token_buffer.num_token == 2) && (small_token_buffer.command_list[j + 1] != NULL))
					{
						changeDir(small_token_buffer.command_list[j + 1]);
						j++; //consume input
					}
					else
					{
						write(STDOUT_FILENO, "Error! Unsupported parameters for command: cd\n", 47);
						//consume the rest of the input
						j += (small_token_buffer.num_token - 1);
					}
				}
				//cp
				else if(strcmp(small_token_buffer.command_list[j], "cp") == 0)
				{
					//check for correct number of args, and that both aren't null
					if((small_token_buffer.num_token == 3) && (small_token_buffer.command_list[j + 1] != NULL) && (small_token_buffer.command_list[j + 2] != NULL))
					{
						copyFile(small_token_buffer.command_list[j + 1], small_token_buffer.command_list[j + 2]);
						//force counter to move
						j++;
						j++;
					}
					else
					{
						write(STDOUT_FILENO, "Error! Unsupported parameters for command: cp\n", 47);
						//consume the rest of the input
						j += (small_token_buffer.num_token - 1);
					}
				}
				//mv
				else if(strcmp(small_token_buffer.command_list[j], "mv") == 0)
				{
					if((small_token_buffer.num_token == 3) && (small_token_buffer.command_list[j + 1] != NULL) && (small_token_buffer.command_list[j + 2] != NULL))
					{
						moveFile(small_token_buffer.command_list[j + 1], small_token_buffer.command_list[j + 2]);
						//force counter to move
						j++;
						j++;
					}
					else
					{
						write(STDOUT_FILENO, "Error! Unsupported parameters for command: mv\n", 47);
						//consume the rest of the input
						j += (small_token_buffer.num_token - 1);
					}
				}
				//rm
				else if(strcmp(small_token_buffer.command_list[j], "rm") == 0)
				{
					if((small_token_buffer.num_token == 2) && (small_token_buffer.command_list != NULL))
					{
						deleteFile(small_token_buffer.command_list[j + 1]);
						j++;
					}
					else
					{
						write(STDOUT_FILENO, "Error! Unsupported parameters for command: rm\n", 47);
						//consume the rest of the input
						j += (small_token_buffer.num_token - 1);
					}
				}
				//cat
				else if(strcmp(small_token_buffer.command_list[j], "cat") == 0)
				{
					//should be the next item in the token is the file name. There is no validation for this command.
					if((small_token_buffer.num_token == 2) && (small_token_buffer.command_list != NULL))
					{
						displayFile(small_token_buffer.command_list[j +1]);
						//force the token counter to move.
						j++;
					}
					else
					{
						write(STDOUT_FILENO, "Error! Unsupported parameters for command: cat\n", 48);
						//consume the rest of the input
						j += (small_token_buffer.num_token - 1);
					}
				}
				//unrecognized command
				else
				{
					printf("Error! Unrecognized command: %s\n", small_token_buffer.command_list[j]);
				}
			}
			//free the token buffers and reset them after use
			free_command_line(&small_token_buffer);
			memset(&small_token_buffer, 0, 0);
		}
		//free the larger token buffer, and buffer
		free_command_line(&large_token_buffer);
		memset(&large_token_buffer, 0, 0);
		free(buffer);
	}	
	//idk what to return on termination for now.
	return 0;
}
