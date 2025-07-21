/*
 * string_parser.c
 *
 *  Created on: Nov 25, 2020
 *      Author: gguan, Monil
 *	Edited on: Oct 7, 2024
 *		New Author: Scharpf, Lochlan
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"

#define _GUN_SOURCE

int count_token (char* buf, const char* delim)
{
	//TODO：
	/*
	*	#1.	Check for NULL string
	*	#2.	iterate through string counting tokens
	*		Cases to watchout for
	*			a.	string start with delimeter
	*			b. 	string end with delimeter
	*			c.	account NULL for the last token
	*	#3. return the number of token (note not number of delimeter)
	*/
	//init token count and pointer to buf acting as a copy for strtok_r.
	int count = 0;
	char *token_cpy = (char *)malloc(sizeof(char) * strlen(buf) + 1);
	strcpy(token_cpy, buf);
	//token_cpy = strdup(buf);
	char *token;
	//we also need to hold the address of the rest of the string for strtok.
	char *rest = token_cpy;
	//Get rid of newline character
	strtok_r(token_cpy, "\n", &rest);
	//check for null string.
	if(buf == NULL)
	{
		//assuming this is what we want to return for now.
		return 0;
	}
	//otherwise, proceed.
	else
	{
		//get length of delimeters list
		int delim_len = strlen(delim);
		//check the begining and end of the string for bad characters, then remove them.
		for(int i = 0; i < delim_len; i++)
		{
			//leading bad character case.
			if (token_cpy[0] == delim[i])
			{
				//remove the character by using strcpy to move the string over.
				//strcpy(&token_cpy[0], &token_cpy[1]); //this is bad for valgrind some reason.
				//char *tmp = (char *)malloc(sizeof(char) * strlen(token_cpy) + 1); //store copy of copy for manipulation
				//strcpy(tmp, token_cpy);
				memmove(token_cpy, token_cpy + 1, strlen(token_cpy)); //use memmove to remove first character.
			}	
			//trailing bad character case.
			if (token_cpy[strlen(token_cpy) - 1] == delim[i])
			{
				//remove the character in the same fashion as with a leading character.
				//strcpy(&token_cpy[strlen(token_cpy) - 1], &token_cpy[strlen(token_cpy) - 1 + 1]);
				//char *tmp = (char *)malloc(sizeof(char) * strlen(token_cpy) + 1); //store copy of copy for manipulation
				//strcpy(tmp, token_cpy);
				token_cpy[strlen(token_cpy) - 1] = '\0'; //set last character to null
				//token_cpy = tmp;
				//free(tmp);
			}
		}
		//get first token.
		token = strtok_r(token_cpy, delim, &rest);
		//iterate through the rest of the string
		while(token != NULL)
		{
			count++;
			//overwrite token
			token = strtok_r(NULL, delim, &rest);
		}
	}
	//free memory and return value
	free(token_cpy);
	return count;
}

command_line str_filler (char* buf, const char* delim)
{
	//TODO：
	/*
	*	#1.	create command_line variable to be filled and returned
	*	#2.	count the number of tokens with count_token function, set num_token. 
    *           one can use strtok_r to remove the \n at the end of the line.
	*	#3. malloc memory for token array inside command_line variable
	*			based on the number of tokens.
	*	#4.	use function strtok_r to find out the tokens 
    *   #5. malloc each index of the array with the length of tokens,
	*			fill command_list array with tokens, and fill last spot with NULL.
	*	#6. return the variable.
	*/

	//basically, this is the same as the funciton count_tokens, with some functions removed
	//init variables, similarly to count_tokens
	//below values are used in count_token() function
	int i = 0; //create index variable
	int tok_len; //needed for inner-malloc.
	char *token_cpy;
	token_cpy = strdup(buf);
	char *rest = token_cpy;
	char *token;
	//Get rid of newline character.
	strtok_r(token_cpy, "\n", &rest);
	command_line cmd; //init a command_line struct.
	//**
	//as of now we don't have to go through the list and remove leading and trailing characters. This block is reserved just in case we do.
	//**
	//get number of tokens.
	cmd.num_token = count_token(token_cpy, delim);
	//malloc memory for the comamnd list.
	cmd.command_list = (char **)malloc(sizeof(char*) * (cmd.num_token + 1) + 1);
	//get first token.
	token = strtok_r(token_cpy, delim, &rest);
	//go through all tokens.
	while(token != NULL)
	{
		cmd.command_list[i] = strdup(token);
		//tokenize, then use that data to get len, malloc space, and appended to the command list.
		token = strtok_r(NULL, delim, &rest);
		i++;
	}
	//set last token to NULL for afterwards free function.
	cmd.command_list[i] = NULL;
	//free memory and return value.
	free(token_cpy);
	return cmd;

}


void free_command_line(command_line* command)
{
	//TODO：
	/*
	*	#1.	free the array base num_token
	*/
	//this is a 2D array, so we need to free with a for loop.
	/*for int ()*/
	//quick n dirty way to free this. will be more exact later.
	for (int i = 0; (command->command_list[i] != NULL) ; i++)
	{
		free(command->command_list[i]);
	}
	free(command->command_list);
}
