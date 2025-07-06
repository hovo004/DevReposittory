#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>
#define MAX_ARGUMENTS 10
#define MAX_VARS 3


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

int main()
{
	char** names = create();
	char** values = create();

	while(1)					 
	{
		generate_pront();

		char pront_input[100];
		Fgets(pront_input, sizeof(pront_input), stdin);

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
							// for(int i = 0; i < MAX_VARS; i++)
							// 		printf("%s -->  %s\n", names[i], values[i]);
						break;
					case CMD_ECHO:

						handle_echo(names, values, tokens);
						break;
					case CMD_HELP:
						break;
					case CMD_HISTORY:
						break;
				}
				break;

			case CMD_EXTERNAL:
				break;

			case CMD_NOT_FOUND:
				printf("%s: command not found\n", tokens[0]);

				break;
		}
	}

	
	destroy(names);
	destroy(values);
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

char* Fgets(char* buf, size_t size, FILE* stream)
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
	char cur_dir[100];

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
		    // for(int i = 0; i < MAX_VARS; i++)
		 	// printf("%s -->  %s\n", names[i], values[i]);
	free(new_name);
	free(new_value);
	
}

char* get_name_set(char** commands)
{
	if(strchr(commands[1], '='))
	{
		char* key = (char*)calloc(64, sizeof(char));
		if(key == NULL)
		errExit("calloc(key)");

		char value[64] = "";
		char input[128] = "";
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

char* get_value_set(char** commands)
{
	if(strchr(commands[1], '='))
	{
		char* value = (char*)calloc(64, sizeof(char));
		if(value == NULL)
		errExit("calloc(key)");

		char key[64] = "";
		char input[128] = "";
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

char** create()
{
	char** mat = (char**)calloc(MAX_VARS, sizeof(char*));
	if(mat == NULL)
		errExit("calloc(key)");

	for(int i = 0; i < MAX_VARS; i++)
	{
		 mat[i] = (char*)calloc(64, sizeof(char)); // 64 for key or value
	}
	return mat; 
}

void destroy(char** mat)
{
	for(int i = 0; i < MAX_VARS; i++)
	{
		free(mat[i]);
	}
	free(mat);
}

bool find_name(char** names, char* command, size_t size, int* name_index)
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
	char input[128] = "";

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
				// bool inserted = false;

				// for(int j = 0; j < MAX_VARS; j++)
				// {
				// 	if(names[j][0] == '\0')
				// 	{
				// 		strcpy(names[j], temp);
				// 		strcat(input, " ");
				// 		inserted = true;

				// 		break;
				// 	}
				// }

				// if(!inserted)
				// {
				// 	printf("Variable limits reached (Max limits is %d)\n", MAX_VARS);
				// 	return;
				// }

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