# smallsh
smallsh is a simple linux shell made in c that provides full functionality all linux commands using the exec() functions
smallsh provides the following functionalities
1. Provide a prompt for running commands
2. Handle blank lines and comments, which are lines beginning with the # character
3. Provide expansion for the variable $$. Which expands to the process id of the function
4. Execute 3 commands exit, cd, and status via code built into the shell
5. Execute other commands by creating new processes using a function from the exec family of functions
6. Support input and output redirection using < and >
7. Support running commands in foreground and background processes using &
8. Implement custom handlers for 2 signals, SIGINT and SIGTSTP

To run the file you must download it on a linux system and compile it with gcc.
After that simply run the executable
