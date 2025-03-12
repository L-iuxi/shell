#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>


char prev_dir[1024]; // 用于保存上一个目录

void handle_sigint(int sig)
{
    printf("crtl c");
   // exit(0);
}

void jiexi(char* input, char **args, char **infile, char **outfile, char **append)
{
    int i = 0;
    *infile = *outfile = *append = NULL;
   // int i = 0;

    char *command = strtok(input, " \t\n\r");
    while (command != NULL)
    {
 //  if (command[0] == '"') {
    //     // 去除引号
    //     command++; 
    //     char *test = strchr(command, '"');
    //     if (test != NULL) {
    //         *test = '\0'; 
    //     }
    // }
        if (strcmp(command, ">") == 0) 
        {
            *outfile = strtok(NULL, " \t\n\r");
        }
        else if (strcmp(command, ">>") == 0) 
        {
            *append = strtok(NULL, " \t\n\r");
        }
        else if (strcmp(command, "<") == 0)
        {
            *infile = strtok(NULL, " \t\n\r");
        }
        else
        {
            args[i++] = command;
        }
        command = strtok(NULL, " \t\n\r");
    }
    args[i] = NULL;
}

void zhixing(char **args, char *infile, char *outfile, char *append)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        // printf("now Command is(000): %s\n", args[0]);//现在读入的命令
        // if(execvp(args[0], args) == -1) 
        // {
        // perror("exec failed");
        // exit(1);
        // }

        // 输入重定向
        if (infile != NULL)
        {
            int in_fd = open(infile, O_RDONLY);
            if (in_fd == -1)
            {
                perror("Input file open failed");
                exit(1);
            }
            if (dup2(in_fd, STDIN_FILENO) == -1)
            {
                perror("dup2 input failed");
                exit(1);
            }
            close(in_fd);
        }

        // 输出重定向
        if (outfile != NULL)
        {
            int out_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd == -1)
            {
                perror("Output file open failed");
                exit(1);
            }
            if (dup2(out_fd, STDOUT_FILENO) == -1)
            {
                perror("dup2 output failed");
                exit(1);
            }
            close(out_fd);
        }

        // 追加输出重定向
        if (append != NULL)
        {
            int append_fd = open(append, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (append_fd == -1)
            {
                perror("Append file open failed");
                exit(1);
            }
            if (dup2(append_fd, STDOUT_FILENO) == -1)
            {
                perror("dup2 append failed");
                exit(1);
            }
            close(append_fd);
        }

        if (execvp(args[0], args) == -1)
        {
            perror("exec failed");
            exit(1);
        }
    }
    else
    {
        wait(NULL);
    }
}
// void easy_zhixing()
// {
//     if(execvp(args[0],args) == -1)
//         {
//              perror("exec failed");
//         exit(1);
//         }
// }
void cd_command(char *dir)
{
    char current_dir[1024];

    if (getcwd(current_dir, sizeof(current_dir)) != NULL)
    {
    
        if (dir == NULL || strcmp(dir, "") == 0)//cd
        {
             const char *home_dir = getenv("HOME");
            if (home_dir == NULL)
            {
                fprintf(stderr, "HOME environment variable is not set\n");
                return;
            }

           
            if (chdir(home_dir) == -1)
            {
                perror("cd failed");
            }
            else
            {
                
                strcpy(prev_dir, current_dir);
                printf("%s\n", home_dir);
            }
            
           // printf("当前目录为: %s\n", current_dir);
        }else if(strcmp(dir,"-") == 0)
        {
             if (prev_dir[0] == '\0')  
            {
                fprintf(stderr, "No previous directory to return to\n");
                return;
            }

            if (chdir(prev_dir) == -1)
            {
                perror("cd failed");
            }
            else
            {
               
                getcwd(current_dir, sizeof(current_dir));
                printf("返回上一层目录: %s\n", current_dir);
                
               
                strcpy(prev_dir, current_dir);
            }
        }
    
       
        
        // else if (strcmp(dir, "-") == 0) // cd -
        // {
           
        //     if (chdir(prev_dir) == -1)
        //     {
        //         perror("cd failed");
        //     }
        //     else
        //     {
        //         printf("当前目录为: %s\n", current_dir);
        //         strcpy(prev_dir, current_dir); 
        //     }
        // }
        else
        {
           
            if (chdir(dir) == -1)
            {
                perror("cd failed");
            }
            else
            {
                strcpy(prev_dir, current_dir); 
                printf("Current directory is: %s\n", dir);
            }
        } 
    }else
    {
        perror("getcwd failed");
    }
}

void pipe_zhixing(char* input)
{
    char *commands[100];
    char *command = strtok(input, "|");
    int i = 0;

    //char *args[10086];
    while (command != NULL)
    {
        commands[i++] = command;
        command = strtok(NULL, "|");
    }
    //pid = fork();
    int pipefd[2];
    pid_t pid;
    int prev = -1;

    //char buf[10086];
    for (int k = 0; k < i; k++)
    {
        if (pipe(pipefd) == -1)
        {
            perror("pipe failed");
            exit(1);
        }

        pid = fork();
        if (pid == -1)
        {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0)
        {
            
            if (prev != -1)
            {
                if (dup2(prev, STDIN_FILENO) == -1)
                {
                    perror("dup2 stdin failed");
                    exit(1);
                }
                close(prev); 
            }

            
            if (k < i - 1)
            {
                if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                {
                    perror("dup2 stdout failed");
                    exit(1);
                }
            }

            
            close(pipefd[0]);
            close(pipefd[1]);

            
            char *args[100];
            char *infile = NULL, *outfile = NULL, *append = NULL;
            jiexi(commands[k], args, &infile, &outfile, &append);

            
            zhixing(args, infile, outfile, append);
            exit(0);
        }
        else
        {
            
            close(pipefd[1]);

            
            if (prev != -1)
            {
                close(prev);
            }

            
            prev = pipefd[0];
        }
    }

    
    while (wait(NULL) > 0); 
}


int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);
    char input[100];
    
    uid_t use = getuid();
    gid_t group = getgid();
    struct passwd *pw = getpwuid(use);
    struct group *gr = getgrgid(group);

//     char cwd[10086];
//     if(getcwd(cwd,sizeof(cwd)) == NULL)
//     {
//         perror("getcwd() error");
//         return 1;
//     }

//     char *home = getenv("HOME");
//     char new_cwd[10086];

//    //～
//     if (strncmp(cwd, home, strlen(home)) == 0) {
        
//         snprintf(new_cwd, sizeof(new_cwd), "~%s", cwd + strlen(home));
//     } else {
       
//         snprintf(new_cwd, sizeof(new_cwd), "%s", cwd);
//     }


    while (1)
    {
        char cwd[10086];
    if(getcwd(cwd,sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }

    char *home = getenv("HOME");
    char new_cwd[10086];

   //～
    if (strncmp(cwd, home, strlen(home)) == 0) {
        
        snprintf(new_cwd, sizeof(new_cwd), "~%s", cwd + strlen(home));
    } else {
       
        snprintf(new_cwd, sizeof(new_cwd), "%s", cwd);
    }
        //printf("\033[35");
        //printf("%s%s",use,group);
        //printf("\033[0m");
        if(pw != NULL && gr != NULL)
        {
            printf("\033[34m");
            printf("%s",pw->pw_name);
            printf("\033[0m");
            
            printf("@");
             printf("\033[34m");
            printf("%s",gr->gr_name);
            printf("\033[32m");
            printf(" %s ",new_cwd);
            printf("\033[0m");
            printf("$");
        }

        if (fgets(input, 100, stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0'; // 去掉换行符

        int background = 0;
        if (input[strlen(input) - 1] == '&')
        {
            background = 1;
            input[strlen(input) - 1] = '\0'; // 去掉 '&'
        }


        if (strcmp(input, "exit") == 0)
        {
            break;
        }

        if (strchr(input, '|'))
        {
            pipe_zhixing(input);
        }
        else
        {
            char *args[100];
            char *infile = NULL, *outfile = NULL, *append = NULL;
            jiexi(input, args, &infile, &outfile, &append);

            if (strcmp(args[0], "cd") == 0)
            {
                if (args[1] != NULL)
                {
                    cd_command(args[1]);
                }
                else
                {
                    cd_command(NULL);
                }
            }
            else 
            {
              if (background)
                {
                    // 后台执行
                    pid_t pid = fork();
                    if (pid == 0)
                    {
                        zhixing(args, infile, outfile, append);
                        exit(0);
                    }

            }
            else
            {
                zhixing(args, infile, outfile, append);
            }
        }
    }
    }
    return 0;
}
