#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>


char incmd[1024];
char *arg[64];
char envName[10];
char envValue[50];
void * on_child_exit();
void shell();
void evaluate_expression();
void execute_shell_bultin();
void execute_command();
void setup_environment();
void cd();
void echo();
void export();
int main()
{
    FILE *f = fopen("log.txt", "w");
    fprintf(f, "%s", "");
    fclose(f);

    signal(SIGCHLD, on_child_exit);
    setup_environment();
    shell();
    return 0;
}
///////////////////////////////////////////////////////////////////////
void * on_child_exit()
{
FILE *PF = fopen("log.txt", "a");
int status;
wait(&status);
fprintf(PF, "%s", "Child Process is Terminated \n");
fclose(PF);
}
void setup_environment()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    chdir(cwd);
}
//////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void shell()
{
    while (strcmp(incmd, "exit") != 0)
    {

       // printf("toka@toka:~$ ");
        char *username = getenv("USER");
        printf("@%s", username);
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("~%s", cwd);
        printf("$ ");
        fgets(incmd,1024, stdin);
        // deleting the newline
        if ((strlen(incmd) > 0) && (incmd[strlen (incmd) - 1] == '\n'))
        { incmd[strlen (incmd) - 1] = '\0';}
        // check if command is empty
        if(!strcmp(incmd,"")) continue;
        // check if command is exit
        if(!strcmp(incmd,"exit")) break;
        // Check if command is executed in background
        int checkbg = 0;
        if (incmd[strlen(incmd) - 1] == '&')
        {
            checkbg = 1;
            incmd[strlen(incmd) - 1] = '\0';
        }
        evaluate_expression();
        int input_type;
        if ((strcmp(arg[0],"cd")==0 )|| (strcmp(arg[0],"echo")==0 ) || (strcmp(arg[0],"export")==0)|| (strcmp(arg[0],"exit")==0))
        {
            input_type =1;
        }
        else
        {
            input_type = 2;
        }
        switch (input_type)
        {
            case 1:
                execute_shell_bultin(arg);
                break;
            case 2:
                execute_command(arg,checkbg);
                break;
            default:
                break;
        }

    }
}
//////////////////////////////////////////////////////////////////////
void evaluate_expression()
{
    char *token = strtok(incmd, " ");
    int i = 0;
    while (token != NULL)
    {
        arg[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    arg[i] = NULL;
}

///////////////////////////////////////////////////////////////////////
void execute_command(char **args, int checkbg)
{
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror(args[0]);
            exit(EXIT_FAILURE);
        }
        else
        {
            execvp(arg[0], arg);
        }
    }
    else if (pid == -1)
    {
        // Fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        if (!checkbg)
        {
            // Wait for the child process to complete
             waitpid(pid, NULL, 0);
        }
    }
}
//////////////////////////////////////////////////////////////
void execute_shell_bultin(char **arg)
{

    int i, num = 0;
    char* Cmds[4]={"cd","echo","export","exit"};
    char* username;


    for (i = 0; i < 4; i++) {
        if (strcmp(arg[0], Cmds[i]) == 0) {
            num = i + 1;
            break;
        }
    }

    switch (num) {
        case 1:
            cd(arg[1]);
            break;
        case 2:
            echo();
            break;
        case 3:
            export();
            break;
        case 4:
            exit(0);
        default:
            break;
    }
}
//////////////////////////////////////////////////
void cd()
{
      if (!strcmp(arg[1],"~"))
      {
        chdir(getenv("HOME"));
      }
      else
      {
        chdir(arg[1]);
      }
}

void echo()
{
    int sum=0;int i=1;int j=0;
    char temp[10];
    int ind; //indicator for environment variable or not
    while (arg[i] != NULL)
    {
        while (arg[i][j]!='\0')
        {
            //printf("i=  %d,j=  %d ,let=   %c  ",i,j,arg[i][j]);
            if (arg[i][j] == '$')
            {
             //  printf("founddd at i=%d",i);
                ind = 1;
                j++;
                int j2 = 0;
                while (arg[i][j] != '"')
                {
                    temp[j2] = arg[i][j];
                    j++;
                    j2++;
                }
             //  printf("temp is %s",temp);
                break;
            }
            else
                j++;
        }
        i++;
        j=0;
    }

    if (ind==1) {
        char envName[100];
        strncpy(envName, temp, 99);
        char *envValue = getenv(envName); // envir. variable's value
        if (envValue != NULL) {
            if (strcmp(arg[i-2],"echo")) {
                int z = 1;

                while (arg[i - 2][z] != '\0') {
                    printf("%c", arg[i - 2][z]);
                    z++;
                }
                printf(" %s\n", envValue);
            } else
            {
                printf(" %s\n", envValue);
            }

        } else
            printf(" ");
    }
    else
        {
            int i = 1;
            while (arg[1][i] != '"') {
                printf("%c", arg[1][i]);
                i++;
            }
            printf("\n");
        }

}


void export()
{
    int i=0;
        while (arg[1][i]!='=')
        {
            envName[i]=arg[1][i];
            i++;
        }
        i++;
    int j=0;
        while (arg[1][i]!='\0')
       {
           envValue[j]=arg[1][i];
           i++; j++;
       }
    setenv(envName, envValue, 1);

}