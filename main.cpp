/*
	Author: Connor Lunsford
	Date: 10/20/20
	Desc: Assignment 3: Smallsh. A functional shell in C with a small selection of features */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>

using namespace std;

bool mode = 0;

struct command {
	char* command_text;
	char* arguments[512];
	char* input;
	char* output;
	int counter;
	bool background;
};

void child_terminate_sigint(int signo) {
	exit(1);
}

void foreground_mode(int signo) {
	// if foreground mode is off
	if (mode == 0) {
		char message1[] = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message1, 50);
		mode = 1;
	}
	// if foreground mode is on
	else {
		char message2[] = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, message2, 30);
		mode = 0;
	}
	return;
}

int change_directory(command cmd) {

	char* home;
	if (cmd.arguments[1] == NULL) {
		home = getenv("HOME");
		chdir(home);
	}
	else {
		chdir(cmd.arguments[1]);
	}
	return 0;
}

int main(void) {

	//set up the signal handler for sigint
	struct sigaction ignore = { 0 };
	ignore.sa_handler = SIG_IGN;
	sigaction(SIGINT, &ignore, NULL);

	//set up the signal handler for sigtstp
	struct sigaction foreground = { 0 };
	foreground.sa_handler = foreground_mode;
	sigaction(SIGTSTP, &foreground, NULL);

	int pid = getpid();
	int status = 0;
	int background_status = 0;
	int pidlist[101] = { 0 };

	while (1) {

		fflush(stdin);
		int j = 1;
		while (pidlist[j] != 0) {
			if (waitpid(pidlist[j], &background_status, WNOHANG) != 0) {
				if (background_status == 256) {
					background_status = 1;
				}
				printf("Background process %d has terminated with exit status: %d\n", pidlist[j], background_status);
				fflush(stdout);
				pidlist[j] = 0;
				pidlist[0]--;
			}
			j++;
		}

		char input[2048];
		char* store;
		char* store2;
		char* token;
		struct command cmd;
		cmd.command_text = NULL;
		int i;
		for (i = 1; i < cmd.counter; i++) {
			cmd.arguments[i] = NULL;
		}
		cmd.input = NULL;
		cmd.output = NULL;
		cmd.background = 0;
		cmd.counter = 1;

		//gets input
		printf(":");
		fflush(stdout);
		fflush(stdin);
		fgets(input, 2048, stdin);

		if (strncmp(input, "#", 1) == 0) {
			continue;
		}
		if (strcmp(input, "\n") == 0) {
			continue;
		}
		else {

			while (strstr(input, "$$") != NULL) {
				
				//printf("%s", input);
				//stores the first part of the string, up to the $
				token = strtok(input, "$$");
				store = (char*)malloc(sizeof(token));
				strcpy(store, token);
				// stores the last part of the string, up to the end
				token = strtok(NULL, "\0");
				store2 = (char*)malloc(sizeof(token));
				strcpy(store2, token);
				// joins the stored string, pid, and the rest of the string
				//printf("%s\n", store);
				//printf("%s\n", store2);
				store2 += sizeof(char);
				sprintf(input, "%s%d%s", store, pid, store2);
				fflush(stdout);
				//printf("%s\n", input);
			}

			//removes the new line character
			token = strtok(input, "\n");
			strcpy(input, token);
			//parses
			token = strtok(input, " ");
			cmd.command_text = (char*)malloc(sizeof(token));
			strcpy(cmd.command_text, token);
			while ((token = strtok(NULL, " ")) != NULL) {
				// if the next token will be the input
				if (strcmp(token, "<") == 0) {
					token = strtok(NULL, " ");
					cmd.input = (char*)malloc(sizeof(token));
					strcpy(cmd.input, token);
				}
				// if the next token will be the output
				else if (strcmp(token, ">") == 0) {
					token = strtok(NULL, " ");
					cmd.output = (char*)malloc(sizeof(token));
					strcpy(cmd.output, token);
				}
				// if the next token is an argument
				else {
					cmd.arguments[cmd.counter] = (char*)malloc(sizeof(token));
					strcpy(cmd.arguments[cmd.counter], token);
					cmd.counter++;
				}
			}

			strcpy(input, "\n");

			cmd.arguments[0] = cmd.command_text;
			// checks the last argument to see if it uses the & command, if it is, lowers the counter and sets the background bool
			if (mode == 0) {
				if (cmd.counter == 1) {
					cmd.arguments[1] == NULL;
				}
				else if (strncmp(cmd.arguments[cmd.counter - 1], "&", 1) == 0) {
					cmd.arguments[cmd.counter - 1] = NULL;
					cmd.counter--;
					cmd.background = 1;
				}
			}
			else if (mode == 1) {
				if (cmd.counter == 1) {
					cmd.arguments[1] == NULL;
				}
				else if (strncmp(cmd.arguments[cmd.counter - 1], "&", 1) == 0) {
					cmd.arguments[cmd.counter - 1] = NULL;
					cmd.counter--;
					cmd.background = 0;
				}
			}
		}

		// for testing purposes, delete these after you're done working on the project
		/*
		printf("%s\n", cmd.command_text);
		printf("%d\n", cmd.counter);
		for (i = 1; i < cmd.counter; i++) {
			printf("%s\n", cmd.arguments[i]);
		}
		if (cmd.input != NULL) {
			printf("input: %s\n", cmd.input);
		}
		if (cmd.output != NULL) {
			printf("output: %s\n", cmd.output);
		}
		printf("%d\n", cmd.background);
		*/
		// exit command
		if (strcmp(cmd.command_text, "exit") == 0) {
			j = 1;
			while (pidlist[j] != 0) {
				kill(pidlist[j],SIGKILL);
				j++;
			}
			exit(0);
		}
		// cd command
		else if (strcmp(cmd.command_text, "cd") == 0) {
			change_directory(cmd);
		}
		// status command
		else if (strcmp(cmd.command_text, "status") == 0) {
			printf("exit status: %d\n", status);
			fflush(stdout);
		}
		// non built in commands
		else {
			int childpid = -1;
			childpid = fork();
			switch (childpid) {
			case -1:
				printf("fork failed, try agin\n");
				fflush(stdout);
				break;
			case 0:

				//set up the signal handler for sigtstp
				sigaction(SIGTSTP, &ignore, NULL);

				// if the process runs in the background
				if (cmd.background == 1) {
					if (cmd.input == NULL) {
						// redirects standard input to /dev/null
						int nullinput = open("/dev/null", O_RDONLY, 0640);
						dup2(nullinput, 0);
					}
					if (cmd.output == NULL) {
						// redirects standard input to /dev/null
						int nulloutput = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0640);
						dup2(nulloutput, 1);
					}
				}
				else {
					//set up the signal handler for sigint
					struct sigaction respond = { 0 };
					respond.sa_handler = child_terminate_sigint;
					sigaction(SIGINT, &respond, NULL);
				}

				// checks for input
				if (cmd.input != NULL) {
					int fdinput = open(cmd.input, O_RDONLY, 0640);
					if (fdinput == -1) {
						printf("Could not open the specified input: %s\n", cmd.input);
						fflush(stdout);
						exit(1);
					}
					if (dup2(fdinput, 0) == -1) {
						printf("Could not open the specified input: %s\n", cmd.input);
						fflush(stdout);
						exit(1);
					}
					//closes the file when exec is called
					fcntl(fdinput, F_SETFD, FD_CLOEXEC);
				}

				// checks for output
				if (cmd.output != NULL) {
					int fdoutput = open(cmd.output, O_WRONLY | O_CREAT | O_TRUNC, 0640);
					if (fdoutput == -1) {
						printf("Could not open the specified output: %s\n", cmd.output);
						fflush(stdout);
						exit(1);
					}
					if (dup2(fdoutput, 1) == -1) {
						printf("Could not open the specified output: %s\n", cmd.output);
						fflush(stdout);
						exit(1);
					}
					//closes the file when exec is called
					fcntl(fdoutput, F_SETFD, FD_CLOEXEC);
				}

				// runs the command

				cmd.arguments[0] = cmd.command_text;
				cmd.arguments[cmd.counter] = NULL;
				if (execvp(cmd.command_text, cmd.arguments) == -1) {
					printf("Could not find command '%s'\n", cmd.command_text);
					fflush(stdout);
					exit(1);
				}
				exit(0);

			default:
				if (cmd.background == 0) {
					waitpid(childpid, &status, WUNTRACED);
					if (status == 256) {
						status = 1;
					}
				}
				else {
					printf("Background PID is: %d\n", childpid);
					fflush(stdout);
					pidlist[0]++;
					pidlist[pidlist[0]] = childpid;
					continue;
				}
			}
		}
		
	}
	return 0;
}
