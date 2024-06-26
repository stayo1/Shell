#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
    char command[1024];
    char *token;
    char *outfile = NULL;
    char *errfile = NULL;
    int i, fd, append, amper, redirect_out, redirect_err, retid, status = 0;
    char *argv[10];
    pid_t child_pid;
    char prompt[256] = "hello";  // Default prompt

    while (1)
    {
        printf("%s: ", prompt);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            perror("fgets");
            continue;
        }
        command[strlen(command) - 1] = '\0';  // Remove newline character

        if (strcmp(command, "quit") == 0)
            exit(0);

        /* parse command line */
        i = 0;
        token = strtok(command, " ");
        while (token != NULL)
        {
            argv[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        argv[i] = NULL;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;

        /* Handle changing the prompt */
        if (i >= 3 && strcmp(argv[0], "prompt") == 0 && strcmp(argv[1], "=") == 0)
        {
            strncpy(prompt, argv[2], sizeof(prompt) - 1);
            prompt[sizeof(prompt) - 1] = '\0';  // Ensure null-termination
            continue;
        }

        /* Handle echo $? */
        if (strcmp(argv[0], "echo") == 0 && argv[1] && strcmp(argv[1], "$?") == 0)
        {
            printf("%d\n", WEXITSTATUS(status)); // Print the status of the last command
            continue;
        }

        /* Handle cd command */
        if (strcmp(argv[0], "cd") == 0)
        {
            if (argv[1] == NULL)
            {
                fprintf(stderr, "cd: expected argument to \"cd\"\n");
            }
            else
            {
                if (chdir(argv[1]) != 0)
                {
                    perror("cd");
                }
            }
            continue;
        }

        /* Does command line end with & */
        if (i > 0 && !strcmp(argv[i - 1], "&"))
        {
            amper = 1;
            argv[i - 1] = NULL;
            i--;
        }
        else
        {
            amper = 0;
        }

        /* Check for output redirection */
        if (i > 2 && !strcmp(argv[i - 2], ">"))
        {
            redirect_out = 1;
            append = 0;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
            i -= 2;
        }
        else if (i > 2 && !strcmp(argv[i - 2], ">>"))
        {
            redirect_out = 1;
            append = 1;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
            i -= 2;
        }
        else
        {
            redirect_out = 0;
        }

        /* Check for error redirection */
        if (i > 2 && !strcmp(argv[i - 2], "2>"))
        {
            redirect_err = 1;
            argv[i - 2] = NULL;
            errfile = argv[i - 1];
            i -= 2;
        }
        else
        {
            redirect_err = 0;
        }

        /* for commands not part of the shell command language */
        child_pid = fork();
        if (child_pid == 0)
        {
            /* Child process */
            if (redirect_out)
            {
                if (append)
                {
                    fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0660);
                }
                else
                {
                    fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0660);
                }
                if (fd < 0)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (redirect_err)
            {
                fd = open(errfile, O_WRONLY | O_CREAT | O_TRUNC, 0660);
                if (fd < 0)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
            }

            execvp(argv[0], argv);
            perror("execvp"); // execvp returns only on error
            exit(EXIT_FAILURE);
        }
        else if (child_pid < 0)
        {
            /* Fork failed */
            perror("fork");
        }
        else
        {
            /* Parent process */
            if (amper == 0)
                retid = waitpid(child_pid, &status, 0); // Store the status of the last command
        }
    }

    return 0;
}
