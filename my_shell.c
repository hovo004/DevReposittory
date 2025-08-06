#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MAX_ARGUMENTS 10
#define MAX_VARS 15
#define MAX_HISTORY_LINE 500
#define BUF_SIZE 1024
#define SMALL_BUF_SIZE 64

void handler_SIGINT (int);
void generate_pront();
char* Fgets(char* buf, size_t size, FILE* stream);
int tokenizing(char* pront_input, char** tokens);
void errExit(const char* msg);
bool is_builtin(const char* command);
bool is_external(char* command, char** args);
enum CommandType get_command_type(char* command, char** args);
enum BuiltinType get_builtin_type(char* command);
void handle_cd(char* pront, char** command);
char* handle_pwd();
void handle_set(char** names, char** values, char** tokens);
char* get_name_set(char** commands);
char* get_value_set(char** commands);
char** create();
void destroy(char**);
bool find_name(char** names, char* command, size_t size, int* name_index);
void handle_echo(char** names, char** values, char** commands);
void handle_help(char** commands);
void handle_history(char** commands, FILE*);
int line_count(FILE*);

enum CommandType
{
	CMD_BUILTIN,
	CMD_EXTERNAL,
	CMD_NOT_FOUND
}type;

enum BuiltinType
{
	CMD_PWD,
	CMD_CD,
	CMD_EXIT,
	CMD_SET,
	CMD_UNSET,
	CMD_ECHO,
	CMD_HELP,
	CMD_HISTORY
} builtin_type;

int name_index = 0;
int flag = false;

int main()
{
	char** names = create();          //   create array for keys
	char** values = create();		  //   create array for values
	FILE* history = fopen("history.txt", "a+");
	if(history == NULL)
		errExit("fopen()");
	int line = line_count(history);

	while(1)
	{
		generate_pront();

		char pront_input[100];
		Fgets(pront_input, sizeof(pront_input), stdin);

		fprintf(history, "%d %s\n", line++ + 1, pront_input);   //create history

		char* tokens[MAX_ARGUMENTS];
		int token_count = tokenizing(pront_input, tokens);

		type = get_command_type(tokens[0], tokens);

		switch(type)
		{
			case CMD_BUILTIN:
				builtin_type = get_builtin_type(tokens[0]);

				switch(builtin_type)
				{
					case CMD_PWD:
						char *cur_dir = handle_pwd();
						if(cur_dir == NULL)
						{
							perror("pwd");
						}

						printf("%s\n", cur_dir);
						free(cur_dir);
						break;
					case CMD_CD:

						handle_cd(pront_input, tokens);

						break;
					case CMD_EXIT:
						printf("\n");
						printf("     This small Shell created by Hovhannes Hovhannisyan\n");
						printf("     08.07.2025\n");
						printf("\n");
						return 0;

					case CMD_SET:

						handle_set(names, values, tokens);

						break;
					case CMD_UNSET:
							char* new_name = tokens[1];
							char* new_value = tokens[1];


						if(find_name(names, new_name, MAX_VARS, &name_index))
						{
							memset(names[name_index], 0, 64);
							memset(values[name_index], 0, 64);
						}else
							printf("%s: Invalid variable\n", new_name);
						break;
					case CMD_ECHO:

						handle_echo(names, values, tokens);
						break;
					case CMD_HELP:
						handle_help(tokens);
						break;
					case CMD_HISTORY:
						handle_history(tokens, history);
						break;
				}
				break;

			case CMD_EXTERNAL:
				break;

			case CMD_NOT_FOUND:
				printf("%s: command not found\n", tokens[0]);

				break;
		}

		if(flag)
		{	
			fflush (history);
			flag = 0;
			break;
		}
	}

	fclose(history);
	destroy(names);
	destroy(values);
}

void handler_SIGINT (int signum)
{
	flag = true;
}

void generate_pront()
{
	char cwd[256];
	char* home = getenv("HOME");

	if(getcwd(cwd, sizeof(cwd)) == NULL)
	{
		perror("getcwd");
		printf("mysh:$ ");
		return;
	}

	char pront[256] = "";
	strcat(pront, "mysh:");

	if(strcmp(cwd, "/") == 0)
		strcat(pront, "/");
	else if(home != NULL && strncmp(cwd, home, strlen(home)) == 0)
	{
		strcat(pront, "~");
		strcat(pront, cwd + strlen(home));
	}else if(strcmp(cwd, "/home") == 0)
	{
		strcat(pront, "/home");
	}

	strcat(pront, "$ ");
	printf("%s", pront);
}

int tokenizing(char* pront_input, char** tokens)
{
	char* buf = strtok(pront_input, " ");
	int count = 0;

	while(buf != NULL && count < MAX_ARGUMENTS)
	{
		tokens[count++] = buf;
		buf = strtok(NULL, " ");
	}
	tokens[count] = NULL;

	return count;
}

bool is_builtin(const char* command)
{
	const char* builtin_commands[] = {"pwd", "cd", "exit", "set", "unset", "echo", "help", "history", NULL};

	for(int i = 0; builtin_commands[i] != NULL; i++)
	{
		if(strcmp(command, builtin_commands[i]) == 0)
		{
			return true;
		}
	}
	return false;
}

bool is_external(char* command, char** args)
{
		pid_t pid = fork();
		if(pid < 0)
			errExit("fork");
		else if (pid == 0)
		{
			if(execvp(command, args) == -1)
			{
				_exit(1);
			}
		}else
		{
			int status;
			wait(&status);

			if(WIFEXITED(status) && WEXITSTATUS(status) == 0)
				return true;
			else
				return false;
		}
}

void errExit(const char* msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

char* Fgets(char* buf, size_t size, FILE* stream)  //fgets removed \n
{
	char *res = fgets(buf, size, stream);
	if(res != NULL && strlen(buf) > 0)
	{
		buf[strlen(buf) - 1] = '\0';
	}

	return res;
}

enum CommandType get_command_type(char* command, char** args)
{
		if (is_builtin(command))
		{
			return CMD_BUILTIN;
		}else if(is_external(command, args))
		{
			return CMD_EXTERNAL;
		}else
			return CMD_NOT_FOUND;
}

enum BuiltinType get_builtin_type(char* command)
{
	const char* builtin_commands[] = {"pwd", "cd", "exit", "set", "unset", "echo", "help", "history", NULL};

	if(!strcmp(command, "pwd"))
		return CMD_PWD;
	else if(!strcmp(command, "cd"))
		return CMD_CD;
	else if(!strcmp(command, "exit"))
		return CMD_EXIT;
	else if(!strcmp(command, "set"))
		return CMD_SET;
	else if(!strcmp(command, "unset"))
		return CMD_UNSET;
	else if(!strcmp(command, "echo"))
		return CMD_ECHO;
	else if(!strcmp(command, "help"))
		return CMD_HELP;
	else if(!strcmp(command, "history"))
		return CMD_HISTORY;
}

char* handle_pwd()
{
	char* cur_dir = (char*)malloc(100 * sizeof(char));
	if(cur_dir == NULL)
	{
		perror("malloc");
	}
	if(getcwd(cur_dir, 100) == NULL)
	{
		perror("getcwd");
		free(cur_dir);
		return NULL;
	}

	return cur_dir;
}

void handle_cd(char* pront, char** command)
{
	if(command[1] != NULL)
	{
		if(chdir(command[1]) != 0)
			{
				fprintf(stderr, "cd: %s: ", command[1]);
				perror("");
			}
	}else
	{
		char* home = getenv(("HOME"));
		if(home == NULL)
		{
			fprintf(stderr, "cd: HOME not set\n");
		}else if(chdir(home) != 0)
		{
			perror("cd");
		}
	}
}

void handle_set(char** names, char** values, char** tokens)
{
	char* new_name = get_name_set(tokens);
	if(new_name == NULL)
	{
		return;
	}

	char* new_value = get_value_set(tokens);
	if(new_value == NULL)
	{
		free(new_name);
		return;
	}

	if(find_name(names, new_name, MAX_VARS, &name_index))
	{
		strcpy(values[name_index], new_value);
	}else
	{

		bool inserted = false;

		for(int i = 0; i < MAX_VARS; i++)
		{
			if(names[i][0] == '\0')
			{
				strcpy(names[i], new_name);
				strcpy(values[i], new_value);
				inserted = true;
				break;
			}
		}
		if(!inserted)
		{
			printf("Variable limits reached (Max limits is %d)\n", MAX_VARS);
		}

	}
	free(new_name);
	free(new_value);

}

char* get_name_set(char** commands)     //give token for name
{
	if(strchr(commands[1], '='))
	{
		char* key = (char*)calloc(SMALL_BUF_SIZE, sizeof(char));
		if(key == NULL)
		errExit("calloc(key)");

		char value[SMALL_BUF_SIZE] = "";
		char input[BUF_SIZE] = "";
		char* p = input;

		for(int i = 1; commands[i] != NULL; i++)
		{
			strcat(input, commands[i]);
			strcat(input, " ");
		}

		int k = 0;
		while(*p != '=')
		{
			key[k++] = *p++;
		}
		key[k] = '\0';

		if(*p != '=')
		{
			printf("Invalid format, use VAR=VALUE\n");
			return NULL;
		}
		p++;

		int v = 0;
		while(*p != '\0')
		{
			if(*p == '"')
				p++;
			else
				value[v++] = *p++;
		}
		return key;

	}else
	{
		printf("Invalid format, use VAR=VALUE\n");
		return NULL;
	}

}

char* get_value_set(char** commands)     //  give token for value
{
	if(strchr(commands[1], '='))
	{
		char* value = (char*)calloc(SMALL_BUF_SIZE, sizeof(char));
		if(value == NULL)
		errExit("calloc(key)");

		char key[SMALL_BUF_SIZE] = "";
		char input[BUF_SIZE] = "";
		char* p = input;

		for(int i = 1; commands[i] != NULL; i++)
		{
			strcat(input, commands[i]);
			strcat(input, " ");
		}

		int k = 0;
		while(*p != '=')
		{
			key[k++] = *p++;
		}
		key[k] = '\0';

		if(*p != '=')
		{
			printf("Invalid format, use VAR=VALUE\n");
			return NULL;
	}
		p++;

		int v = 0;
		while(*p != '\0')
		{
			if(*p == '"')
				p++;
			else
				value[v++] = *p++;
		}
		return value;

	}else
	{
		printf("Invalid format, use VAR=VALUE\n");
		return NULL;
	}
}

char** create(void)                          //create array (char[])
{
	char** mat = (char**)calloc(MAX_VARS, sizeof(char*));
	if(mat == NULL)
		errExit("calloc(key)");

	for(int i = 0; i < MAX_VARS; i++)
	{
		 mat[i] = (char*)calloc(SMALL_BUF_SIZE, sizeof(char));
	}
	return mat;
}

void destroy(char** mat)                //destroy created arrays
{
	for(int i = 0; i < MAX_VARS; i++)
	{
		free(mat[i]);
	}
	free(mat);
}

bool find_name(char** names, char* command, size_t size, int* name_index)  //whether a variable with this name exists or not
{
	for(int i = 0; i < size; i++)
	{
		if(strcmp(names[i],command) == 0)
		{
			*name_index = i;
			return true;
		}
	}
	return false;
}

void handle_echo(char** names, char** values, char** commands)
{
	char input[BUF_SIZE] = "";

	for(int i = 1; commands[i] != NULL; i++)
	{
		if(commands[i][0] == '$')
		{
			char* temp = commands[i] + 1;

			if(find_name(names, temp, MAX_VARS, &name_index))
			{
				strcat(input, values[name_index]);
			}
			else
			{
				strcat(input, "");
			}
		}else
		{
			strcat(input, commands[i]);
			strcat(input, " ");
		}
	}
	printf("%s\n", input);
}

void handle_help(char** commands)
{
	if(commands[1] == NULL)
	{	printf("\n");
		printf("     mysh: supported built-in commands:\n");
        printf("       cd [dir]        Change the current directory to [dir]. If no dir, goes to HOME.\n");
        printf("       pwd             Print the current working directory.\n");
        printf("       exit            Exit the shell.\n");
        printf("       set VAR=VALUE   Set a variable VAR to VALUE.\n");
        printf("       unset VAR       Unset a variable VAR.\n");
        printf("       echo [args]     Print arguments, substituting $VAR with its value.\n");
        printf("       help [command]  Display help for a specific command.\n");
        printf("       history [N]     Display command history.\n");
		printf("\n");
	}else
	{
		if(is_builtin(commands[1]))
		{
			switch(get_builtin_type(commands[1]))
			{
				case CMD_PWD:
					printf("\n");
					printf("     pwd: Print the name of current working directory.\n");
					printf("\n");
					break;
				case CMD_CD:
					printf("\n");
					printf("     cd: cd [dir]\n");
					printf("         change the shell working directory\n");
					printf("\n");
					break;
				case CMD_EXIT:
					printf("\n");
					printf("     exit: Exit the shell.\n");
					printf("\n");
					break;
				case CMD_SET:
					printf("\n");
					printf("     set: set [VAR]=[VALUE]\n");
					printf("          Create variable\n");
					printf("\n");
					break;
				case CMD_UNSET:
					printf("\n");
					printf("     unset: unset [VAR]\n");
					printf("            Delete variable\n");
					printf("\n");
					break;
				case CMD_ECHO:
					printf("\n");
					printf("     echo: echo [arg ...]\n");
					printf("           Write arguments to the standard output.\n");
					printf("\n");
					break;
				case CMD_HELP:
					printf("\n");
					printf("     help: help [-dms] [pattern ...]\n");
					printf("           Display information about builtin commands.\n");
					printf("\n");
					break;
				case CMD_HISTORY:
					printf("\n");
					printf("     history: history [N]\n");
					printf("              Print the history list.\n");
					printf("\n");
					break;

			}
		}else
			printf("Invalid argument for --help\n");
	}
	return;
}

void handle_history(char** commands, FILE* file)
{
	fseek(file, 0, SEEK_SET);
	char line[BUF_SIZE] = "";
	int count_line = line_count(file);

	if(commands[1] == NULL)
	{
		while(fgets(line, sizeof(line), file) != NULL)
		{
			printf("%s", line);
		}
		return;
	}

	int num = atoi(commands[1]);   // string --> integer;

	if(num < 0 || num > MAX_HISTORY_LINE)
	{
		printf("Invalid argument for history (MAX lines: %d)\n", MAX_HISTORY_LINE);
		return;
	}

	int skip = count_line - num;

	for(int i = 0; i < skip; i++)          //skip first lines
	{
		fgets(line, sizeof(line), file);
	}

	for(int i = 0; i < num; ++i)    //print N lines (history N)
	{
		fgets(line, sizeof(line), file);
		printf("%s", line);
	}
	return;
}

int line_count(FILE* file)
{
	int lines = 0;
	char ch;

	while((ch = fgetc(file)) != EOF)
	{
		if(ch == '\n')
			lines++;
	}

	fseek(file, 0, SEEK_SET);

	return lines;
}