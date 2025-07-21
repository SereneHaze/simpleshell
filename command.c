//built-in libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h> //needed for basename
//custom libraries
#include "command.h"
#include "string_parser.h"
//MACROS
#define PATHLEN 1024
#define _GNU_SOURCE
//version of this was implimented in lab02. I guess this a command that only works in the current directory.
void listDir() /*for the ls command*/
{
	//set up character pointer to current directory
	char *cdir;
	//malloc space for the length of the directory
	cdir = (char*)malloc(sizeof(char) * PATHLEN);
	//get current working directory
	cdir = getcwd(cdir, PATHLEN);
	//set up a directory struct and pointer
	DIR *dir;
	struct dirent *entries;
	dir = opendir(cdir);
	//iterate through directory entries, setting the read directory to the entries struct
	while((entries = readdir(dir)) != NULL)
	{
		//write file names to stdout.
		//I'm seperating them out by single spaces. Not pretty, but it works.
		write(STDOUT_FILENO, entries->d_name, strlen(entries->d_name));
		write(STDOUT_FILENO, " ", 1);
	}
	write(STDOUT_FILENO, "\n", 1);
	//free memory.
	free(cdir);
	closedir(dir);	
}

//version of this was partially implimented in lab02
void showCurrentDir() /*for the pwd command*/
{
	//set up character pointer to current directory
	char *cdir;
	//malloc space for the length of the directory
	cdir = (char*)malloc(sizeof(char) * PATHLEN);
	//get current working directory
	cdir = getcwd(cdir, PATHLEN);
	//write to console with syscalls.
	write(STDOUT_FILENO, cdir, strlen(cdir));
	write(STDOUT_FILENO, "\n", 1);
	//free memory
	free(cdir);
}

void makeDir(char *dirName) /*for the mkdir command*/
{
	//used a stack overflow answer for this, CITE THIS
	//struct stat st;
	struct stat st = {0};
	if(stat(dirName, &st) == -1)
	{
		//make directory with proper permissions
		mkdir(dirName, 0700);
	}
	else
	{
		//throw error to stdout
		write(STDOUT_FILENO, "directory already exists\n", 25);
	}

}

void changeDir(char *dirName) /*for the cd command*/
{
	//check if directory already exists using stat
	//struct stat st;
	struct stat st = {0};
	if(stat(dirName, &st) == 0)
	{
		chdir(dirName);
	}
	else
	{
		//throw error to stdout
		write(STDOUT_FILENO, "directory does not exists\n", 26);
	}
}
/*There are some odd mallocs in this function with seemingly arbitrary sizes. This is true, they are compleatly arbitrary,
 * I just kept adding constant sizes to the buffer until valgrind stoped nagging me. Effiecent? no. Leak and error free? yes.*/
void copyFile(char *sourcePath, char *destinationPath) /*for the cp command*/
{
	//use basname on file path to get file without the filepath
	struct stat dest_stat;
	char *newPath = (char *)malloc(sizeof(char) * (strlen(sourcePath) + strlen(destinationPath) + 5)); //way more memory than needed
	strcpy(newPath, destinationPath);
	//if status == 0 (meaning its a directory), append the basename of the source file to the end of the dest file path.
	if(stat(destinationPath, &dest_stat) == 0)
	{
		if(dest_stat.st_mode & S_IFDIR)
		{
			char *sourceFile = (char *)malloc(sizeof(char) * strlen(sourcePath) + 5); //way more than enough.
			strcpy(sourceFile, basename(sourcePath));
			//I'm going to be naive and just use strcat.
			//re-add the '/' after basename, if it removes it.
			//strcpy(newPath, destinationPath);
			strcat(newPath, "/");
			strcat(newPath, sourceFile);
			free(sourceFile);
		}
	}
	//printf("new path: %s\n", destinationPath);
	char *buffer[1024];
	size_t bytes;
	int dst_file_id;
	int src_file_id;
	//check for file access
	if(access(sourcePath, F_OK) == 0)
	{
		//open file
		src_file_id = open(sourcePath, O_RDONLY);
		//check for errors
		if (src_file_id == -1)
		{
			write(STDOUT_FILENO, "error opening file\n", 19);
		}
		//set up destination file
		dst_file_id = open(newPath, O_WRONLY | O_CREAT | O_TRUNC, 0744);
		//begin writing
		do
		{
			bytes = read(src_file_id, buffer, sizeof(buffer));
			write(dst_file_id, buffer, bytes);
		} while(bytes > 0);
		//close out files
		close(src_file_id);
		close(dst_file_id);

	}
	//otherwise, throw error to stdout
	else
	{
		write(STDOUT_FILENO, "file does not exists\n", 21);
	}
	free(newPath);
}

void moveFile(char *sourcePath, char *destinationPath) /*for the mv command*/
{
	struct stat dest_stat;
	//char *sourceFile = basename(sourcePath);
	char *newPath = (char *)malloc(sizeof(char) * (strlen(sourcePath) + strlen(destinationPath) + 5)); //way more memory than needed
	strcpy(newPath, destinationPath);
	//check if path is literal; fix it if not.
	if(stat(destinationPath, &dest_stat) == 0)
	{
		if(dest_stat.st_mode & S_IFDIR)
		{
			//I'm going to be naive and just use strcat
			char *sourceFile = (char *)malloc(sizeof(char) * strlen(sourcePath) + 5); //way more than enough.
			strcpy(sourceFile, basename(sourcePath));
			//I'm going to be naive and just use strcat.
			//re-add the '/' after basename, if it removes it.
			//strcpy(newPath, destinationPath);
			strcat(newPath, "/");
			strcat(newPath, sourceFile);
			free(sourceFile);
		}
	}
	//printf("newPath: %s\n", newPath);
	//printf("source");
	//this only works if the files are in the same directory.
	if(rename(sourcePath, newPath) == -1)
	{
		write(STDOUT_FILENO, "MV error\n", 9);

	}
	free(newPath);
}

void deleteFile(char *filename) /*for the rm command*/
{
	//check if file exists
	if(access(filename, F_OK) == 0)
	{
		//remove file
		//unlink(filename);
		remove(filename);
	}
	//otherwise, throw error to stdout
	else
	{
		write(STDOUT_FILENO, "file does not exists\n", 21);
	}
}

//version of this was implimented in lab02
void displayFile(char *filename) /*for the cat command*/
{
	//variable to hold the number of bytes in a file
	size_t bytes;
	//arbitrary buffer; might need to use malloc and realloc here. For now, this works.
	char buffer[1024];
	//file exists
	if(access(filename, F_OK) == 0)
	{

		//get an integer table location of file
		int file_id = open(filename, O_RDONLY);
		//iterate throught the file and read bytes from it into a buffer.
		//this kind of implimentation with the do-while was done by our lab leader.
		do
		{
			bytes = read(file_id, buffer, sizeof(buffer));
			write(STDOUT_FILENO, buffer, bytes);
		} while(bytes > 0);
		//close file
		close(file_id);
	}
	//file does not exist
	else
	{
		write(STDOUT_FILENO, "file not found\n", 15);
	}
}
