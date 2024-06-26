#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 10
#define MAX_VARS 10
#define MAX_COMMANDS 10

pid_t child_pid = 0;
char *history[100];
int history_length = 0;
int current_index = 0;
int history_index = 0;
int history_flag = 0;

typedef struct
{
    char name[50];
    char value[100];
} variable;

variable vars[MAX_VARS];
int var_count = 0;

void set_variable(char *name, char *value)
{
    // Remove leading $
    if (name[0] == '$')
    {
        name++;
    }
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {
            strcpy(vars[i].value, value);
            return;
        }
    }
    if (var_count < MAX_VARS)
    {
        strcpy(vars[var_count].name, name);
        strcpy(vars[var_count].value, value);
        var_count++;
    }
    else
    {
        printf("Error: Maximum number of variables reached.\n");
    }
}
int is_variable_exist(char *name)
{
    if (name[0] == '$')
    {
        name++;
    }
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {
            return 0;
        }
    }
    return 1;
}
// Function to update a variable's value
void update_variable(char *name, char *new_value)
{
    if (name[0] == '$')
    {
        name++;
    }
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {
            strcpy(vars[i].value, new_value);
            return;
        }
    }
    // If the variable does not exist, add it as a new variable
    set_variable(name, new_value);
}
char *get_variable(char *name)
{
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {
            return vars[i].value;
        }
    }
    return NULL;
}

void replace_variables(char *command)
{
    char temp[1024];
    char *pos = command;
    char *end = command + strlen(command);
    char *temp_ptr = temp;

    while (pos < end)
    {
        if (*pos == '$')
        {
            pos++;
            char var_name[50];
            char *var_ptr = var_name;
            while (isalnum(*pos) || *pos == '_')
            {
                *var_ptr++ = *pos++;
            }
            *var_ptr = '\0';
            char *value = get_variable(var_name);
            if (value != NULL)
            {
                while (*value)
                {
                    *temp_ptr++ = *value++;
                }
            }
        }
        else
        {
            *temp_ptr++ = *pos++;
        }
    }
    *temp_ptr = '\0';
    strcpy(command, temp);
}

int execute_pipe_command(char **command, char *input_file, char *output_file, char *error_file, int append)
{

    if (input_file != NULL)
    {
        int fd = open(input_file, O_RDONLY);
        if (fd == -1)
        {
            perror("input file open failed");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    // Redirect output if necessary
    if (output_file != NULL)
    {
        int fd;
        if (append)
        {
            fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        else
        {
            fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        if (fd == -1)
        {
            perror("output file open failed");
            return 1;
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    // Redirect error if necessary
    if (error_file != NULL)
    {
        int fd;
        if (append)
        {
            fd = open(error_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        else
        {
            fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        if (fd == -1)
        {
            perror("error file open failed");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }

    // Execute command

    if (execvp(command[0], command) == -1)
    {
        perror("execvp failed");
        return 1;
        exit(EXIT_FAILURE);
    }
    return 0;
}

void sig_hendler(int signum)
{
    if (signum == SIGINT)
    {
        printf("You typed Control-C! \n");
       
    }
    if (signum == SIGUSR1)
    {
        printf("You typed Control-C! \n");
    }
}
void print_arry_arg(int arry_arg[], int num_commands)
{
    for (int i = 0; i < num_commands; i++)
    {
        printf("arry_arg[%d] = %d \n", i, arry_arg[i]);
    }
}

void print_commands(char *commands[10][10], int num_commands, int arry_arg[])
{
    for (int i = 0; i < num_commands; i++)
    {
        printf("commands[%d] = ", i);
        for (int j = 0; j < arry_arg[i]; j++)
        {
            printf("%s ", commands[i][j]);
        }
        printf("\n");
    }
}

void add_command_to_history(char *commands[10][10], int num_commands, int arry_arg[])
{
    if (commands[0][0] == NULL || commands[0] == NULL || strcmp(commands[0][0], "") == 0)
        return;

    char *command = (char *)calloc(1024, sizeof(char));
    for (int i = 0; i < num_commands; i++)
    {
        for (int j = 0; j < arry_arg[i]; j++)
        {
            strcat(command, commands[i][j]);
            strcat(command, " ");
        }
        if (i != num_commands - 1)
            strcat(command, "| ");
    }

    history[history_index] = (char *)malloc(strlen(command) + 1);
    strcpy(history[history_index], command);
    history_index = (history_index + 1) % 100;

    if (history_index == 100)
        history_flag = 1;

    if (history_flag == 1)
        history_length = 100;
    else
        history_length = history_index;

    free(command);
}
void free_history()
{
    HIST_ENTRY **hist_list = history_list();
    if (hist_list)
    {
        for (int i = 0; hist_list[i]; i++)
        {
            free(hist_list[i]->line);
            free(hist_list[i]->timestamp);
        }
    }
}

void print_history()
{
    for (int i = 0; i < history_index - 1; i++)
    {
        printf("%d: %s\n", i, history[i]);
    }
}

void print_output_file(char *input_files[], int num_commands)
{
    for (int i = 0; i < num_commands; i++)
    {
        printf("input_files[%d] = %s \n", i, input_files[i]);
    }
}

int handle_pipes_and_execution(char *commands[MAX_COMMANDS][MAX_ARGS], char *input_files[MAX_COMMANDS],
                               char *output_file[MAX_COMMANDS], char *error_file[MAX_COMMANDS],
                               int *append_flag, int num_commands, int arry_arg[], int amper)
{
    int num_pipe = num_commands - 1;
    int pipes[num_pipe][2];

    for (int i = 0; i < num_pipe; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            exit(1);
        }
    }

    int pid, status;
    for (int i = 0; i < num_commands; i++)
    {
        pid = fork();

        if (pid == -1)
        {
            perror("fork");
            exit(1);
        }
        else if (pid == 0)
        {
            // Child process

            // Set up signal handling for Ctrl+c
            signal(SIGINT, SIG_DFL);

            // Redirect input from the previous pipe, if this is not the first command
            if (i > 0)
            {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1)
                { // duplicate the read end of the previous pipe to the standard input
                    perror("dup2");
                    exit(1);
                }
            }

            // Redirect output to the next pipe, if this is not the last command
            if (i < num_pipe)
            {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    exit(1);
                }
            }

            // Close all pipe ends except the ones being used by this command
            for (int j = 0; j < num_pipe; j++)
            {
                if (i > 0 && j == i - 1)
                {
                    close(pipes[j][1]);
                }
                else if (i < num_pipe && j == i)
                {
                    close(pipes[j][0]);
                }
                else
                {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            execute_pipe_command(commands[i], input_files[i], output_file[i], error_file[i], append_flag[i]);
        }
    }
    for (int i = 0; i < num_pipe; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < num_commands; i++)
    {
        if (amper == 0)
        {
            wait(&status);
        }
    }
    // Add the executed command with its return status to history
    add_command_to_history(commands, num_commands, arry_arg);

    return WEXITSTATUS(status);
}
void reset_command_vars(char *commands[10][10], char *input_files[10], char *output_file[10], char *error_file[10], int append_flag[10], int arry_arg[10], int *num_commands, int *num_args, int *amper)
{

    // Reset variables for new command line input
    for (int i = 0; i < 10; i++)
    {
        arry_arg[i] = 0;
        input_files[i] = NULL;
        output_file[i] = NULL;
        error_file[i] = NULL;
        append_flag[i] = 0;
        for (int j = 0; j < 10; j++)
        {
            commands[i][j] = NULL; // Initialize commands array to NULL
        }
    }
    *num_commands = 0;
    *amper = 0;
}
void parse(char *input_stream, char *commands[10][10], char *input_files[10], char *output_file[10], char *error_file[10], int append_flag[10], int arry_arg[10], int *num_commands, int *amper)
{
    char *token;
    int num_args = 0;
    // Reset variables for new command line input
    for (int i = 0; i < 10; i++)
    {
        arry_arg[i] = 0;
        input_files[i] = NULL;
        output_file[i] = NULL;
        error_file[i] = NULL;
        append_flag[i] = 0;
        for (int j = 0; j < 10; j++)
        {
            commands[i][j] = NULL; // Initialize commands array to NULL
        }
    }
    *num_commands = 0;
    *amper = 0;

    token = strtok(input_stream, " ");

    while (token != NULL)
    {
        if (token[0] == '"')
        {
            // If the token starts with a quote, it is a string
            char string[1024];
            bzero(string, 1024);
            strcat(string, token);
            token = strtok(NULL, " ");
            while (token != NULL && token[strlen(token) - 1] != '"')
            {
                strcat(string, " ");
                strcat(string, token);
                token = strtok(NULL, " ");
            }
            if (token != NULL)
            {
                strcat(string, " ");
                strcat(string, token);
            }
            // Remove the quotes from the string
            string[strlen(string) - 1] = '\0';
            for (int i = 1; i < strlen(string); i++)
            {
                string[i - 1] = string[i];
            }
            string[strlen(string) - 1] = '\0';
            token = string;
        }
        else if (!strcmp(token, "&"))
        {
            *amper = 1;
            token = strtok(NULL, " ");
        }
        else if (strcmp(token, ">") == 0)
        {
            // Output redirection
            token = strtok(NULL, " ");
            output_file[*num_commands - 1] = token;
            token = strtok(NULL, " ");
            append_flag[*num_commands - 1] = 0;
        }
        else if (strcmp(token, ">>") == 0)
        {
            // Append output redirection
            token = strtok(NULL, " ");
            output_file[*num_commands - 1] = token;
            token = strtok(NULL, " ");
            append_flag[*num_commands - 1] = 1;
        }
        else if (strcmp(token, "<") == 0)
        {
            // Input redirection
            token = strtok(NULL, " ");
            input_files[*num_commands - 1] = token;
            token = strtok(NULL, " ");
        }
        else if (strcmp(token, "2>") == 0)
        {
            // Error output redirection
            token = strtok(NULL, " ");
            error_file[*num_commands - 1] = token;
            token = strtok(NULL, " ");
        }
        else if (strcmp(token, "|") == 0)
        {
            // Pipe
            arry_arg[*num_commands - 1] = num_args;
            commands[*num_commands - 1][num_args] = NULL; // NULL terminate the last command and argument
            num_args = 0;
            token = strtok(NULL, " ");
            continue;
        }
        else
        {
            // Command or argument
            if (num_args == 0)
            {
                // Command
                commands[*num_commands][num_args++] = token;
                (*num_commands)++;
            }
            else
            {
                // Argument
                commands[*num_commands - 1][num_args++] = token;
            }
        }
        token = strtok(NULL, " ");
    }
    if (*num_commands > 0)
    { // If there is a command
        arry_arg[*num_commands - 1] = num_args;
        commands[*num_commands - 1][num_args] = NULL; // NULL terminate the last command and argument
    }

    // Handle variable substitution
    for (int i = 0; i < *num_commands; i++)
    {
        for (int j = 0; j < arry_arg[i]; j++)
        {
            if (commands[i][j][0] == '$')
            {
                char *value = get_variable(commands[i][j] + 1);
                if (value != NULL)
                {
                    commands[i][j] = value;
                }
            }
        }
    }
}
int main()
{

    char *input_stream;
    char original_input_stream[1024];
    char *commands[10][10];
    char *token;
   
    int amper;
    // 0: overwrite, 1: append
    int num_commands = 0; // number of commands.
    int num_args = 0;     // number of arguments for the command.
    int status;
    int orig_stdin = dup(STDIN_FILENO);   // save original stdin file descriptor
    int orig_stdout = dup(STDOUT_FILENO); // save original stdout file descriptor
    int arry_arg[10];                     // number of arguments for each command
    char *input_files[10];                // input files for each command
    char *output_file[10];                // output file for each command
    char *error_file[10];                 // Array to store error files
    int append_flag[10];                  // append flag for each command

    // malloc the prompt
    char *prompt = (char *)malloc(1024);
    strcpy(prompt, "hello: ");

    // ignore ctrl+c
    if (signal(SIGINT, sig_hendler) == SIG_ERR)
    {
        printf("Error: %s\n", strerror(errno));
        exit(1);
    }

    using_history();

    while (1)
    {
        input_stream = readline(prompt);
       
        if (input_stream == NULL)
        {
            continue;
        }
        else
        {
            add_history(input_stream);
        }
        //printf("input stream = %s \n", input_stream);
       
        // Check for variable assignment
        if (input_stream[0] == '$' && strchr(input_stream, '=') != NULL)
        {
            char *name = strtok(input_stream, "=");
            char *value = strtok(NULL, "=");
            if (name && value && name[0] == '$')
            {
                // Remove leading $
                name++;
                // remove leading and trailing spaces from value
                value = strtok(value, " ");
                name = strtok(name, " ");
                if (is_variable_exist(name) == 0)
                {
                    update_variable(name, value);
                    free(input_stream);
                    continue;
                }
                else
                {
                    set_variable(name, value);
                    free(input_stream);
                    continue;
                }
            }
            else
            {
                printf("Invalid variable assignment\n");
                // reset name and value
                name = NULL;
                value = NULL;
                free(input_stream);
                continue;
            }
        }
        if (strcmp(input_stream, "!!") == 0)
        {
            if (history_length == 0)
            {
                printf("No commands in history \n");
                free(input_stream);
                continue;
            }
            strcpy(input_stream, history[history_index - 1]);
        }
       
        // Save the original input stream
        strcpy(original_input_stream, input_stream);
        // Get the first token
        token = strtok(input_stream, " ");

        // Check if the first token is "if"
        if (token != NULL && strcmp(token, "if") == 0)
        {
            //remove the if from the input stream
            input_stream = input_stream + strlen(token) + 1; // +1 for the space after "if"
            while (*input_stream == ' ') input_stream++; // Skip any additional spaces
            parse(input_stream, commands, input_files, output_file, error_file, append_flag, arry_arg, &num_commands, &amper);
            if (num_commands == 0)
            {
                printf("Invalid if statement\n");
               
                continue;
            }
            else
            {
               
                input_stream = readline("> ");
                if (strcmp(input_stream, "then") == 0)
                {
                    free(input_stream);
                    char *then_commend = readline("> ");
                    input_stream = readline("> ");
                    if (strcmp(input_stream, "else") == 0)
                    {
                        free(input_stream);
                        char *else_commend = readline("> ");
                        input_stream = readline("> ");
                        if (strcmp(input_stream, "fi") == 0)
                        {
                            free(input_stream);
                             int ans = handle_pipes_and_execution(commands, input_files, output_file, error_file, append_flag, num_commands, arry_arg, amper);
                                        reset_command_vars(commands, input_files, output_file, error_file, append_flag, arry_arg, &num_commands, &num_args, &amper);
                            if (ans == 0)
                            {
                                parse(then_commend, commands, input_files, output_file, error_file, append_flag, arry_arg, &num_commands, &amper);
                                if (num_commands == 0)
                                {
                                    printf("Invalid if statement\n");
                                    free(then_commend);
                                    continue;
                                }
                                else
                                {
                                    handle_pipes_and_execution(commands, input_files, output_file, error_file, append_flag, num_commands, arry_arg, amper);
                                    reset_command_vars(commands, input_files, output_file, error_file, append_flag, arry_arg, &num_commands, &num_args, &amper);
                                    free(then_commend);
                                    continue;
                                }
                            }
                            else
                            { // if the condition is false
                                parse(else_commend, commands, input_files, output_file, error_file, append_flag, arry_arg, &num_commands, &amper);
                                if (num_commands == 0)
                                {
                                    printf("Invalid if statement\n");
                                    free(else_commend);
                                    continue;
                                }
                                else
                                {
                                    handle_pipes_and_execution(commands, input_files, output_file, error_file, append_flag, num_commands, arry_arg, amper);
                                    reset_command_vars(commands, input_files, output_file, error_file, append_flag, arry_arg, &num_commands, &num_args, &amper);
                                    free(else_commend);
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            printf("Invalid "
                                   "fi"
                                   " statement\n");
                            continue;
                        }
                    }
                    else
                    {
                        printf("Invalid else statement\n");
                        continue;
                    }
                }
                else
                {
                    printf("Invalid then statement\n");
                    continue;
                }
            }
        }
       
        /* parse command line */
        parse(original_input_stream, commands, input_files, output_file, error_file, append_flag, arry_arg, &num_commands, &amper);
        // print_arry_arg(arry_arg, num_commands);
        //print_commands(commands, num_commands, arry_arg);
        // print_output_file(output_file, num_commands);

        /* Is command empty or ..*/
        if (commands[0] == NULL || commands[0][0] == NULL || strcmp(commands[0][0], "") == 0 || strcmp(commands[0][0], "\n") == 0)
        {
            free(input_stream);
            continue;
        }

        if (strcmp(commands[0][0], "quit") == 0)
        {

            free_history();
            free(prompt);
            free(input_stream);
            exit(0);
        }

        if (strcmp(commands[0][0], "read") == 0)
        {
            char *var_name = commands[0][1];
            if (var_name != NULL)
            {
                char *value = readline("Enter value: ");
                set_variable(var_name, value);
                free(input_stream);
                free(value);
                continue;
            }
            else
            {
                printf("Invalid variable name\n");
                free(input_stream);
                continue;
            }
        }

        if (strcmp(commands[0][0], "prompt") == 0 && strcmp(commands[0][1], "=") == 0)
        {
            // realloc the prompt
            prompt = (char *)realloc(prompt, strlen(commands[0][2]) + 1);
            strcpy(prompt, commands[0][2]);

            free(input_stream);
            continue;
        }

        if (strcmp(commands[0][0], "cd") == 0)
        {
            if (commands[0][1] == NULL)
            {
                fprintf(stderr, "cd: missing argument\n");
            }
            else if (strcmp(commands[0][1], "..") == 0)
            {
                // Change to parent directory
                if (chdir("..") != 0)
                {
                    perror("cd to parent directory failed");
                }
                else
                {
                    char cwd[1024];
                    if (getcwd(cwd, sizeof(cwd)) != NULL)
                    {
                        printf("Changed to parent directory: %s\n", cwd);
                    }
                    else
                    {
                        perror("getcwd() error");
                    }
                }
            }
            else
            {
                // Change to specified directory
                if (chdir(commands[0][1]) != 0)
                {
                    perror("cd failed");
                }
                else
                {
                    char cwd[1024];
                    if (getcwd(cwd, sizeof(cwd)) != NULL)
                    {
                        printf("Changed directory: %s\n", cwd);
                    }
                    else
                    {
                        perror("getcwd() error");
                    }
                }
            }
            // Add cd command to history with status 0 for success or 1 for failure
            add_command_to_history(commands, num_commands, arry_arg);
            // Reset the commands
            free(input_stream);
            continue;
        }

        if (strcmp(commands[0][0], "echo") == 0 && strcmp(commands[0][1], "$?") == 0)
        {
            // print the last command status
            if (history_length == 0)
            {
                printf("No commands in history \n");
            }
            else
            {
                printf("%d \n", status);
            }
            add_command_to_history(commands, num_commands, arry_arg); // Add echo command to history
            // reset the commands
            free(input_stream);
            continue;
        }
        // this opt is for me not a part of the assignment

        status = handle_pipes_and_execution(commands, input_files, output_file, error_file, append_flag, num_commands, arry_arg, amper);
        free(input_stream);
        //clear input stream


        dup2(orig_stdin, STDIN_FILENO);   // reset stdin to its original state
        dup2(orig_stdout, STDOUT_FILENO); // reset stdout to its original state
    }
}