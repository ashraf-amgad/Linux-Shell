#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define max_word 25
#define Line_Length 512
#define max_commands 100
#define max_args 100
#define History_Length 20

#define Pipe_flag 1
#define Redirection_Input_flag 2
#define Redirection_Output_flag 3

int commands_counter = 0;
int Piping_Counter = 1;
int Piping_Redirection_Array[10];
char *Redirections_FileNames[10];
int There_Is_Piping_Flag = 0;

char myhistory[History_Length][Line_Length];
int myhistory_index = 0;

char shell_name[50] = "minishell:~] ";

typedef struct
{
    char alias_name[10];
    char alias_command[10][max_word];
} alias_type;

alias_type alias_struct[25];
int alias_number = 0;

void check_alias(char **args)
{
    int i = 0;
    int j = 0;

    for (i = 0; i < alias_number; i++)
    {
        if (strcmp(args[0], alias_struct[i].alias_name) == 0)
        {
            j = 0;
            while (alias_struct[i].alias_command[j][0] != 0)
            {
                args[j] = alias_struct[i].alias_command[j];
                j++;
            }

            args[j] = NULL;
        }
    }
}

/*...............................................................................*/

void remove_EndOfLine(char *line)
{
    int i = 0;

    while (line[i] != '\n')
    {
        i++;
    }
    line[i] = '\0';
}

/*...............................................................................*/

void read_line(char *line)
{

    printf("%s", shell_name);
    fgets(line, Line_Length, stdin);
    remove_EndOfLine(line);
    strcpy(myhistory[myhistory_index], line);

    if (myhistory_index == 25)
        myhistory_index = 0;
    else
        myhistory_index++;
}

/*...............................................................................*/

void process_line(char *line, char commands[max_commands][Line_Length])
{
    int i = 0;
    int j = 0;
    int k = 1;

    commands_counter = 0;
    while (line[j] != 0)
    {
        commands[commands_counter][k - 1] = line[j];
        if (line[j] == ';')
        {
            commands[commands_counter][k - 1] = 0;
            k = 0;
            commands_counter++;
        }
        j++;
        k++;
    }
    commands[commands_counter][k - 1] = 0;

    //printf("Commands counter is %d..\n",commands_counter);
    /*for (j = 0; j <= commands_counter; j++)
    {
        printf("command %d is %s...\n", j, commands[j]);
    }*/
}

/*...............................................................................*/

void process_args(char *args[max_word], char *command)
{
    int i, j;

    i = 0;
    args[i] = strtok(command, " ");

    if (args[0] == NULL)
    {
        printf("No Command...\n");
    }

    while (args[i] != NULL)
    {
        i++;
        args[i] = strtok(NULL, " ");
    }
}

/*...............................................................................*/

int check_redirection(char *args[max_word], char ***Piping_args)
{

    int i = 0;
    int j = 0;
    int k = 0;
    //Piping_Counter = 1;
    Piping_args[0] = &args[0];
    Piping_Counter = 1;
    /*while (Piping_args[0][i] != NULL)
    {
        printf("Piping_args[%d] : %s \n", i, Piping_args[0][i]);
        i++;
    }*/

    i = 0;
    k = 0;
    There_Is_Piping_Flag = 0;
    while (args[i] != NULL)
    {
        if (strcmp(args[i], ">") == 0)
        {
            There_Is_Piping_Flag = 1;
            args[i] = NULL;
            Piping_Redirection_Array[Piping_Counter - 1] = Redirection_Output_flag;
            Redirections_FileNames[k] = args[i + 1];
            //printf("Redirections_output[%d] %s \n", k, Redirections_FileNames[k]);
            Piping_Counter++;
            k++;
        }

        else if (strcmp(args[i], "<") == 0)
        {
            There_Is_Piping_Flag = 1;
            args[i] = NULL;
            Piping_Redirection_Array[Piping_Counter - 1] = Redirection_Input_flag;
            Redirections_FileNames[k] = args[i + 1];
            //printf("Redirections_input[%d] %s \n", k, Redirections_FileNames[k]);
            Piping_Counter++;
            k++;
        }

        else if (strcmp(args[i], "|") == 0)
        {
            There_Is_Piping_Flag = 1;
            args[i] = NULL;
            Piping_Redirection_Array[Piping_Counter - 1] = Pipe_flag;
            Piping_args[Piping_Counter] = &args[i + 1];
            Piping_Counter++;
        }
        else
        {
        }
        i++;
    }

    /*printf("Hi from check_redirection process....\n");
    printf("Piping_Counter : %d\n", Piping_Counter);*/
    /*
    int m = 0;
    int z = 0;
    for (m=0; m<5 ; m++)
    {
        for(z=0 ; z<5 ; z++)
        {
            printf("Piping_args[%d][%d] : %s \n", m , z, Piping_args[m][z]);
        }
    }*/
}

/*...............................................................................*/

int read_parse_line(char *args[max_word], char *line, char commands[5][512], int command_number)
{
    int i = 0;
    int j = 0;

    process_args(args, commands[command_number]);

    check_alias(args);

    /*int q = 0;
    while (args[q] != NULL)
    {
        printf("args[%d] : %s \n", q, args[q]);
        q++;
    }*/

    if (strcmp(args[0], "cd") == 0)
    {
        chdir(args[1]);
        return 1;
    }

    else if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
        return 1;
    }

    else if (args[0][0] == 'P')
    {
	if(args[0][1] == 'S')
	{
		if(args[0][2] == '1')
		{
			strcpy(shell_name, &args[0][4]);
			shell_name[strlen(shell_name)] = ' ';
			shell_name[strlen(shell_name)] = '\0';		
		}
	}
        return 1;
    }

    else if (strcmp(args[0], "alias") == 0)
    {
        if (args[1] == NULL)
        {
            // display a list of all existing aliases
            for (i = 0; i < alias_number; i++)
            {
                //printf("%s = %s", alias_struct[i].alias_name, &alias_struct[i].alias_command[0][0]);
                printf("%s=", alias_struct[i].alias_name);
                j = 0;
                while (alias_struct[i].alias_command[j][0] != 0)
                {
                    printf("%s ", alias_struct[i].alias_command[j]);
                    j++;
                }

                printf("\n");
            }
        }

        else if (strcmp(args[1], "-r") == 0)
        {
            // remove a single alias
        }

        else if (strcmp(args[1], "-c") == 0)
        {
            // remove all defined aliases
            alias_number = 0;
            // remove aliases from file if you saved it in file.
        }

        else
        {
            j = 0;
            while (args[1][j] != '=')
                j++;

            args[1][j] = 0;

            strcpy(alias_struct[alias_number].alias_name, &args[1][0]);
            // first command arg
            strcpy(alias_struct[alias_number].alias_command[0], &args[1][j + 1]);

            j = 2;
            if (args[j] != NULL)
            {
                while (args[j] != NULL)
                {
                    strcpy(alias_struct[alias_number].alias_command[j - 1], args[j]);
                    j++;
                }

                alias_struct[alias_number].alias_command[j][0] = 0;
            }

            else
            {
                alias_struct[alias_number].alias_command[1][0] = 0;
            }

            alias_number++;
        }

        return 1;
    }

    else if (strcmp(args[0], "myhistory") == 0)
    {
        int i = 0;
        if (args[1] == NULL)
        {
            while (i < myhistory_index)
            {
                printf("%d --- %s \n", i, myhistory[i]);
                i++;
            }
            return 1;
        }

        else if (strcmp(args[1], "-c") == 0)
        {
            // clear history
            myhistory_index = 0;
            return 1;
        }

        else if (strcmp(args[1], "-e") == 0)
        {
            // execute history command
            int command_index = atoi(args[2]);
            strcpy(line, myhistory[command_index]);

            process_line(line, commands);
            read_parse_line(args, line, commands, 0);
            return -1;
        }
    }

    else
    {
        return -1;
    }
}


/*...............................................................................*/

void Pipe_Process(char ***Piping_args)
{

    //Piping_Counter--;
    //int number_of_pipes = 3;
    // file descriptors for 2 pipes: fd1 for cat-to-grep, fd2 for grep-to-cut
    int fd[Piping_Counter][2];
    int status;
    int i, j, k;
    int ifp, ofp;

    /*printf("Hi from Pipe process....\n");
    printf("Piping_Counter : %d\n", Piping_Counter);
    for(i=0 ;i<10 ;i++)
    {
        printf("Piping_Redirection_Array[%d] : %d\n",i ,Piping_Redirection_Array[i]);
    }*/

    // make pipe for cat to grep
    // fd1[0] = read  end of cat->grep pipe (read by grep)
    // fd1[1] = write end of cat->grep pipe (written by cat)
    k = 0;
    for (i = 0; i < Piping_Counter; i++)
    {
        pipe(fd[i]);
    }

    for (i = 0; i < Piping_Counter; i++)
    {
        if (i == 0)
        {
            if (fork() == 0)
            {
                if ((Piping_Redirection_Array[i] == Pipe_flag) || (Piping_Redirection_Array[i] == 0))
                {
                    dup2(fd[i][1], 1);
                }
                else if (Piping_Redirection_Array[i] == Redirection_Input_flag)
                {

                    ifp = open(Redirections_FileNames[k], O_RDONLY);
                    dup2(ifp, 0);
                    k++;
                }
                else if (Piping_Redirection_Array[i] == Redirection_Output_flag)
                {
                    dup2(fd[i][1], 1);
                    ofp = open(Redirections_FileNames[k], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                    dup2(ofp, 1);
                    k++;
                }
                for (j = 0; j < Piping_Counter; j++)
                {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }

                // frist command
                if (execvp(Piping_args[0][0], &Piping_args[0][0]) < 0)
                {
                    perror("Command not found.....\n");
                    exit(0);
                }
            }
        }
        else if (i == Piping_Counter - 1)
        {
            if (fork() == 0)
            {
                if ((Piping_Redirection_Array[i] == Pipe_flag) || (Piping_Redirection_Array[i] == 0))
                {
                    dup2(fd[i - 1][0], 0);
                }
                else if (Piping_Redirection_Array[i] == Redirection_Input_flag)
                {

                    ifp = open(Redirections_FileNames[k], O_RDONLY);
                    dup2(ifp, 0);
                    k++;
                }
                else if (Piping_Redirection_Array[i] == Redirection_Output_flag)
                {
                    dup2(fd[i - 1][0], 0);
                    ofp = open(Redirections_FileNames[k], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                    dup2(ofp, 1);
                    k++;
                }

                for (j = 0; j < Piping_Counter; j++)
                {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }
                if (execvp(Piping_args[i][0], &Piping_args[i][0]) < 0)
                {
                    perror("Command not found.....\n");
                    exit(0);
                }
            }
        }

        else
        {
            if (fork() == 0)
            {
                if ((Piping_Redirection_Array[i] == Pipe_flag) || (Piping_Redirection_Array[i] == 0))
                {
                    dup2(fd[i - 1][0], 0);
                    dup2(fd[i][1], 1);
                }

                else if (Piping_Redirection_Array[i] == Redirection_Input_flag)
                {

                    ifp = open(Redirections_FileNames[k], O_RDONLY);
                    dup2(ifp, 0);
                    k++;
                }

                else if (Piping_Redirection_Array[i] == Redirection_Output_flag)
                {
                    dup2(fd[i - 1][0], 0);
                    dup2(fd[i][1], 1);
                    ofp = open(Redirections_FileNames[k], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                    dup2(ofp, 1);
                    k++;
                }

                for (j = 0; j < Piping_Counter; j++)
                {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }
                if (execvp(Piping_args[i][0], &Piping_args[i][0]) < 0)
                {
                    perror("Command not found.....\n");
                    exit(0);
                }
            }
        }
    }

    for (i = 0; i < Piping_Counter; i++)
    {
        close(fd[i][0]);
        close(fd[i][1]);
        wait(&status);
    }
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

int main()
{

    char *args[max_word];
    char **Piping_args[max_word];
    char line[Line_Length];
    char commands[5][512];
    int ifp, ofp;
    int i = 0;

    signal(SIGINT, handle_sigint);
    signal(SIGABRT, handle_sigabrt);
    signal(SIGQUIT, handle_sigquit);
    signal(SIGTSTP, handle_sigtstp); // terminal stop ^Z

    i = 0;
    while (1)
    {
        read_line(line);
        process_line(line, commands);

        for (i = 0; i <= commands_counter; i++)
        {
            if (read_parse_line(args, line, commands, i) == 1)
                continue;

            check_redirection(args, Piping_args);

            if (args[0] != NULL)
            {
                int pid = fork();

                if (pid == 0)
                {

                    if (There_Is_Piping_Flag == 0)
                    {
                        if (execvp(args[0], &args[0]) < 0)
                        {
                            perror("Command not found.....\n");
                            exit(0);
                        }
                    }
                    else
                    {
                        Pipe_Process(Piping_args);
                        exit(0);
                    }
                }

                else
                {
                    waitpid(pid, 0, 0);
                }
            }

            bzero(Redirections_FileNames, sizeof(Redirections_FileNames));
            int k = 0;
            for (k = 0; k < 10; k++)
            {
                Piping_Redirection_Array[k] = 0;
            }
        }
    }

    free(args);
    return 0;
}
