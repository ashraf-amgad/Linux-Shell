/*-
 * DO NOT MODIFY ANYTHING OVER THIS LINE
 * THIS FILE IS TO BE MODIFIED
 */

#include <stddef.h> /* NULL */
#include <stdio.h>	/* setbuf, printf */
#include <stdlib.h> /* strtol */
#include <string.h>
#include <ctype.h>	   /* tolower */
#include <unistd.h>	   /* execvp, dup2 */
#include <sys/types.h> /* fork */
#include <sys/wait.h>  /* wait */
#include <fcntl.h>
#include <sys/stat.h> /* umask */
#include <pwd.h>	  /* getpwnam */
#include <time.h>	  /* time */

extern int obtain_order(); /* See parser.y for description */

#define Builtin_Command_NotBuiltin_ID -1
#define Builtin_Command_cd_ID 0
#define Builtin_Command_umask_ID 1
#define Builtin_Command_time_ID 2
#define Builtin_Command_readvar_ID 3

int Check_BuitInCommands(char *argv)
{

	if (strcmp(argv, "cd") == 0)
	{
		return Builtin_Command_cd_ID;
	}

	else if (strcmp(argv, "umask") == 0)
	{
		return Builtin_Command_umask_ID;
	}

	else if (strcmp(argv, "time") == 0)
	{
		return Builtin_Command_time_ID;
	}

	else if (strcmp(argv, "read") == 0)
	{
		return Builtin_Command_readvar_ID;
	}

	else
	{
		return Builtin_Command_NotBuiltin_ID;
	}
}

int msh_cd(char *args)
{
	char home_dir[3] = "..";

	if (args == NULL)
	{
		if (chdir(home_dir) != 0)
		{
			perror("msh error change directory...\n");
			return -1;
		}

		return 0;
	}

	else
	{
		if (chdir(args) != 0)
		{
			perror("msh error change directory...\n");
			return -1;
		}

		return 0;
	}
}

void msh_exit(void)
{
	exit(0);
}

void msh_umask(int mask)
{
	umask(mask);
}

time_t msh_time(void)
{
	return time(NULL);
}

void handle_sigint(int signum)
{
	printf("\nCaptured SIGINT....\n");
}

void handle_sigabrt(int signum)
{
	printf("\nCaptured SIGABRT....\n");
}

void handle_sigquit(int signum)
{
	printf("\nCaptured SIGAUIT....\n");
}

void handle_sigtstp(int signum)
{
	printf("\nCaptured SIGSTOP....\n");
}

int Exec_BuitInCommand(char **argv)
{
	int BuitInCommand_number;
	char **strtol_endptr = NULL;
	int umask_value;
	char *user_name;
	struct passwd *passwd_ptr;
	char home_envname[5] = "HOME";
	char *home_envvar;
	char *envvar_name;
	char *envvar_val;
	int readvar_count=0;

	if (argv[0][0] == '~')
	{
		user_name = argv[0] + 1;
		//printf("-->  %s\n", user_name);

		passwd_ptr = getpwnam(user_name);
		if (passwd_ptr == NULL)
		{
			perror("user name is not found....\n");
			home_envvar = getenv(home_envname);
			printf("HOME environment variable: %s\n", home_envvar);
		}

		else
		{
			printf("user name home directory: %s\n", passwd_ptr->pw_dir);
		}

		return 1;
	}

	else if (argv[0][0] == '$')
	{
		envvar_name = argv[0] + 1;
		envvar_val = getenv(envvar_name);

		if (envvar_val == NULL)
		{
			perror("Envirnment variable isn't found...\n");
		}
		else
		{
			printf("%s = %s \n", envvar_name, envvar_val);
		}

		return 1;
	}

	else
	{
		BuitInCommand_number = Check_BuitInCommands(argv[0]);

		switch (BuitInCommand_number)
		{
		case Builtin_Command_NotBuiltin_ID:
			return -1;
			break;

		case Builtin_Command_cd_ID:
			msh_cd(argv[1]);
			return 1;
			break;

		case Builtin_Command_umask_ID:
			umask_value = strtoll(argv[1], strtol_endptr, 8);
			msh_umask(umask_value);
			return 1;
			break;

		case Builtin_Command_time_ID:
			
			return 1;
			break;

		case Builtin_Command_readvar_ID:
			readvar_count = 0;
			while (argv[readvar_count] != NULL)
			{
				setenv(argv[readvar_count+1], argv[readvar_count+2], 1);
				readvar_count += 2;
			}

			return 1;
			break;

		default:
			return -1;
			break;
		}
	}
}

void Exe_Command(char **argv, char *filev[3], int bg)
{
	pid_t pid;
	int wstatus;
	int ifd, ofd, errfd;

	pid = fork();

	if (pid == 0)
	{
		if (filev[0] != NULL)
		{
			// dup2 input file discreptor
			ifd = open(filev[0], O_RDONLY);
			if(ifd < 0)
			{
				printf("file [%s] is not found... \n",filev[0]);
				exit(0);
			}
			else
			{
				dup2(ifd, 0);
			}
			
			// close unused input file descriptor
			close(ifd);
		}

		if (filev[1] != NULL)
		{
			// dup2 output file discreptor
			ofd = open(filev[1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			if(ofd < 0)
			{
				printf("file [%s] is not found... \n",filev[1]);
				exit(0);
			}

			else
			{
				dup2(ofd, 1);
			}

			// close unused output file descriptor
			close(ofd);
		}

		if (filev[2] != NULL)
		{
			// dup2 error file discreptor
			errfd = open(filev[2], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			if(errfd < 0)
			{
				printf("file [%s] is not found... \n",filev[2]);
				exit(0);
			}
			
			else
			{
				dup2(errfd, 2);
			}
			
			
			// close unused error file descriptor
			close(errfd);
		}

		if(Exec_BuitInCommand(argv) == -1)
		{
			if (execvp(argv[0], argv) < 0)
			{
				perror("Command not found...\n");
				exit(0);
			}	
		}
		else
		{
			exit(0);
		}
		
		
	}

	else if (pid > 0)
	{
		if (bg == 0)
		{
			waitpid(pid, &wstatus, 0);
		}

		else
		{
			signal(SIGINT, handle_sigint);
			signal(SIGABRT, handle_sigabrt);
			signal(SIGQUIT, handle_sigquit);
			signal(SIGTSTP, handle_sigtstp); // terminal stop ^Z
			return;
		}
	}

	else
	{
		perror("couldn't fork a child process.....\n");
	}
}

void Pipe_Process(char ***Piping_args, int number_of_pipes, char *filev[3])
{

	int fd[number_of_pipes][2];
	int status;
	int i, j, ofd;

	for (i = 0; i < number_of_pipes; i++)
	{
		pipe(fd[i]);
	}

	for (i = 0; i < number_of_pipes; i++)
	{
		if (i == 0)
		{
			if (fork() == 0)
			{

				dup2(fd[i][1], 1);

				for (j = 0; j < number_of_pipes; j++)
				{
					close(fd[j][0]);
					close(fd[j][1]);
				}

				// frist command
				if(Exec_BuitInCommand(&Piping_args[0][0]) == -1)
				{
					if (execvp(Piping_args[0][0], &Piping_args[0][0]) < 0)
						exit(0);
				}
				
			}
		}
		else if (i == number_of_pipes - 1)
		{
			if (fork() == 0)
			{
				dup2(fd[i - 1][0], 0);

				if (filev[1] != NULL)
				{
					// dup2 output file discreptor
					ofd = open(filev[1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
					if(ofd < 0)
					{
						printf("file [%s] is not found... \n",filev[1]);
						exit(0);
					}

					else
					{
						dup2(ofd, 1);
					}

					// close unused output file descriptor
					close(ofd);
				}

				for (j = 0; j < number_of_pipes; j++)
				{
					close(fd[j][0]);
					close(fd[j][1]);
				}

				if(Exec_BuitInCommand(&Piping_args[i][0]) == -1)
				{
					if (execvp(Piping_args[i][0], &Piping_args[i][0]) < 0)
						exit(0);
				}
				
			}
		}

		else
		{
			if (fork() == 0)
			{
				dup2(fd[i - 1][0], 0);
				dup2(fd[i][1], 1);

				for (j = 0; j < number_of_pipes; j++)
				{
					close(fd[j][0]);
					close(fd[j][1]);
				}
				if(Exec_BuitInCommand(&Piping_args[i][0]) == -1)
				{
					if (execvp(Piping_args[i][0], &Piping_args[i][0]) < 0)
						exit(0);
				}
				
			}
		}
	}

	for (i = 0; i < number_of_pipes; i++)
	{
		close(fd[i][0]);
		close(fd[i][1]);
		wait(&status);
	}
}



int main(void)
{
	char ***argvv = NULL;
	int argvc;
	char **argv = NULL;
	char *filev[3] = {NULL, NULL, NULL};
	int bg;
	int ret;

	

	setbuf(stdout, NULL); /* Unbuffered */
	setbuf(stdin, NULL);

	signal(SIGINT, handle_sigint);
	signal(SIGABRT, handle_sigabrt);
	signal(SIGQUIT, handle_sigquit);
	signal(SIGTSTP, handle_sigtstp); // terminal stop ^Z

	while (1)
	{
		fprintf(stderr, "%s", "msh> "); /* Prompt */
		ret = obtain_order(&argvv, filev, &bg);
		if (ret == 0)
			break; /* EOF */
		if (ret == -1)
			continue;	 /* Syntax error */
		argvc = ret - 1; /* Line */
		if (argvc == 0)
			continue; /* Empty line */

		/* for debugging */
		/*printf("n = %d \nfilev[0] : %s\nfilev[1] : %s\nfilev[2] : %s\nbg : %d\n", argvc, filev[0], filev[1], filev[2], bg);
		int i = 0;
		int j = 0;
		while (i < argvc)
		{
			while (argvv[i][j] != NULL)
			{
				printf("[%d][%d]--> %s\n", i, j, argvv[i][j]);
				j++;
			}
			j = 0;
			i++;
		}*/

		/* Code here */
		if(argvc == 1)
		{
			argv = argvv[0];

			if (strcmp(argv[0], "exit") == 0)
			{
				exit(0);
			}
			else
			{
				Exe_Command(argv, filev, bg);
			}
			
		}

		else
		{
			// there is piping...
			Pipe_Process(argvv, argvc, filev);
		}
	}

	exit(0);
	return 0;
}
