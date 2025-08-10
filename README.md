Based on the C code you've provided, here is a README that describes the project, how to use it, and its features.

mysh: A Simple Shell
mysh is a custom-built, simple command-line shell written in C. It provides a basic interactive environment for executing commands, managing environment variables, and viewing command history.

Features
Prompt: Displays a user-friendly prompt (mysh:$  or mysh:~/path$ ) showing the current working directory, with the home directory abbreviated as ~.

Built-in Commands: Includes several core shell commands implemented directly within the program:

cd [dir]: Changes the current working directory. If dir is not specified, it navigates to the user's home directory.

pwd: Prints the absolute path of the current working directory.

exit: Exits the shell.

set VAR=VALUE: Sets a new custom shell variable or updates an existing one.

unset VAR: Deletes a custom shell variable.

echo [args]: Prints arguments to the console, expanding custom variables prefixed with $.

history [N]: Displays the last N commands from the command history. If N is not specified, it shows all history.

help [command]: Provides information about supported built-in commands.

External Commands: Can execute external programs and commands by forking a new process.

Command History: All commands entered are saved to a history.txt file, which is used to display history when requested.

Signal Handling: The shell handles SIGINT (Ctrl+C) to gracefully exit without terminating the program immediately.

How to Compile and Run
To compile the mysh shell, use a standard C compiler like gcc.

Clone the repository or save the code to a file named mysh.c.

Open a terminal and navigate to the directory containing the file.

Compile the code:

Bash

gcc -o mysh mysh.c
Run the shell:

Bash

./mysh
Usage
Once you run ./mysh, you will see a prompt like mysh:$. You can then enter commands.

Examples
Changing directories:

Bash

mysh:$ cd /tmp
mysh:/tmp$ pwd
/tmp
Setting and using variables:

Bash

mysh:$ set GREETING="Hello, world!"
mysh:$ echo $GREETING
Hello, world!
Viewing command history:

Bash

mysh:$ history
1 pwd
2 cd /tmp
3 set GREETING="Hello, world!"
4 echo $GREETING
5 history


This shell was created by Hovhannes Hovhannisyan on 08.07.2025.
