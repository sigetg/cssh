#define _POSIX_C_SOURCE 200809L // required for strdup() on cslab
#define _DEFAULT_SOURCE // required for strsep() on cslab
#define _BSD_SOURCE // required for strsep() on cslab

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "pid_list.h"

#define MAX_ARGS 32

char **get_next_command(size_t *num_args)
{
	// print the prompt
	printf("cssh$ ");

	// get the next line of input
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	if (ferror(stdin))
	{
		perror("getline");
		exit(1);
	}
	if (feof(stdin))
	{
		return NULL;
	}

	// turn the line into an array of words
	char **words = (char **)malloc(MAX_ARGS*sizeof(char *));
	int i=0;

	char *parse = line;
	while (parse != NULL)
	{
		char *word = strsep(&parse, " \t\r\f\n");
		if (strlen(word) != 0)
		{
			words[i++] = strdup(word);
		}
	}
	*num_args = i;
	for (; i<MAX_ARGS; ++i)
	{
		words[i] = NULL;
	}

	// all the words are in the array now, so free the original line
	free(line);

	return words;
}

void free_command(char **words)
{
	for (int i=0; i<MAX_ARGS; ++i)
	{
		if (words[i] == NULL)
		{
			break;
		}
		free(words[i]);
	}
	free(words);
}

void input_redirection(char *filename)
{
        // open the file test for reading
        int read_fd = open(filename, O_RDONLY);
        if (read_fd == -1)
        {
            perror("open");
            exit(1);
        }

        // dup read_fd onto stdin
        if (dup2(read_fd, STDIN_FILENO) == -1)
        {
            perror("dup2");
            exit(1);
        }
        close(read_fd);
}

void output_redirection(char *filename, int if_append)
{
        // open the file test.out for writing
	int write_fd;
	if (if_append == 0)
	{
        	write_fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
	}
	else
	{
		write_fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
	}
	if (write_fd == -1)
        {
            perror("open");
            exit(1);
        }

        // dup write_fd onto stdout
        if (dup2(write_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            exit(1);
        }
        close(write_fd);
}

node *run_command(char **command, node *pid_list)
{
	pid_t fork_rv = fork();
	int command_size = 0;
	while (command[command_size] != NULL)
	{
		command_size += 1;
	}
	if (fork_rv == -1)
	{
		perror("fork");
		exit(1);
	}

	if (fork_rv == 0)
	{
		// in the child process
                int i = 0;
                while (command[i] != NULL)
                {
			if ((strcmp(command[i], ">") == 0) || (strcmp(command[i], ">>") == 0))
                        {
				int if_append = 0;
				if (strcmp(command[i], ">>") == 0)
				{
					if_append = 1;
				}
                                char *filename = command[i+1];
                                output_redirection(filename, if_append);
                                command[i] = NULL;
                        }
                        if (command[i] == NULL)
                        {
                                i++;
                                continue;
                        }
                        else if (strcmp(command[i], "<") == 0)
                        {
                                char *filename = command[i+1];
                                input_redirection(filename);
                                command[i] = NULL;
                        }
                        i++;
                }
		// now, execute command
		if (command[0] != NULL)
		{
			if (strcmp(command[0], "exit") == 0)
			{       
				if (command[1] ==  NULL)
				{
					exit(0);
				}
			}
                        if (strcmp(command[i-1], "&") == 0)
                        {
				command[i-1] = NULL;
                        }
			execvp(command[0], command);
			// will never get here if things work correctly
			perror(command[0]);
		}       
		exit(1);
	}
	// in the parent
	// wait on the child to exit
        if (command[command_size - 1] == NULL)
        {
                if (waitpid(fork_rv, NULL, 0) == -1)
                {
                        perror("waitpid");
                        exit(1);
                }
        }	
	else if (strcmp(command[command_size - 1], "&") == 0)
	{	
		pid_t wpid = waitpid(fork_rv, NULL, WNOHANG);
		if (wpid == -1)
        	{
                	perror("waitpid");
                	exit(1);
        	}
		else if (wpid == 0)
		{
			add_node(pid_list, fork_rv);
		}
	}
	else
	{
		if (waitpid(fork_rv, NULL, 0) == -1)
		{
			perror("waitpid");
			exit(1);
		}
	}
	return pid_list;
}

int io_error_check(char **command)
{
	int i = 0;
	int return_val = 0;
	int output_count = 0;
	int input_count = 0;
	while (command[i] != NULL)
	{
		if ((strcmp(command[i], ">") == 0) || (strcmp(command[i], ">>") == 0))
		{       
			output_count += 1;
			if (output_count == 2)
			{       
				printf("Error! Can't have two >'s or >>'s!\n");
				return_val = 1;
			}
		}
		if (strcmp(command[i], "<") == 0)
		{
			input_count += 1;
			if (input_count == 2)
			{
				printf("Error! Can't have two <'s!\n");
				return_val = 1;
			}
		}
		i++;
	}
	return return_val;
}


int main()
{
	size_t num_args;
	node *head = new_list();

	// get the next command
	char **command_line_words = get_next_command(&num_args);
	while (command_line_words != NULL)
	{
		// check for io redirection errors
		int error_val = io_error_check(command_line_words);
		if (error_val == 1)
		{
                	free_command(command_line_words);
                        command_line_words = get_next_command(&num_args);
                        continue;
		}
		
		// don't forget to skip blank commands
		if (command_line_words[0] == NULL)
		{
			free_command(command_line_words);
			command_line_words = get_next_command(&num_args);
			continue;
		}

                head = run_command(command_line_words, head);
		
		// and add something to stop the loop if the user
		// runs "exit"
		if (strcmp(command_line_words[0], "exit") == 0)
		{
			if (command_line_words[1] ==  NULL)
			{
				free_command(command_line_words);
				exit(0);
			}
		}
		
		//remove any defunct processes from the list
	        if (head->next != head)
		{              
			node *node = head->next;
			while (node != head)
			{
				if ((waitpid(node->pid, NULL, WNOHANG) != 0) && (waitpid(node->pid, NULL, WNOHANG) != -1))
				{
					remove_node(head, node->pid);
				}
				node = node->next;
			}
		}
		// free the memory for this command
		free_command(command_line_words);

		// get the next command
		command_line_words = get_next_command(&num_args);
	}

	// free the memory for the last command
	free_command(command_line_words);

	return 0;
}
