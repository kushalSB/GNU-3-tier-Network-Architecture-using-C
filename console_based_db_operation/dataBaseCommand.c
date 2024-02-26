#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void execute_with_pipe(char *args[])
{
    // Create a pipe
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create a child process
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    { // Child process
        // Close the read end of the pipe
        close(pipefd[0]);

        // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);

        // Close the write end of the pipe
        close(pipefd[1]);

        // Execute the command with given arguments
        if (execvp(args[0], args) == -1)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    else
    { // Parent process
        // Close the write end of the pipe
        wait(NULL);

        close(pipefd[1]);

        // Read the output from the pipe and print it
        char buffer[1024];
        ssize_t bytes_read;
        char *output_string = malloc(1); // Start with an empty string
        output_string[0] = '\0';         // Null-terminate the string
        printf("Output from command:\n");
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        {
            buffer[bytes_read] = '\0'; // Null-terminate the buffer
            output_string = realloc(output_string, strlen(output_string) + bytes_read + 1);
            strcat(output_string, buffer); // Append buffer to output_string
        }
        printf("%s", output_string);

        // Free the dynamically allocated string and close the read end of the pipe
        free(output_string);
        close(pipefd[0]);

        // Close the read end of the pipe
        close(pipefd[0]);
    }
}

int main()
{
    // Command to run the simpleMysql executable

    char table_name[100];

    // Prompt the user for the command
    printf("Enter command (GA: GETALL, G1: GETONE, V: VIEWCOLUMNS, A: ADDONE): ");
    char user_command[20];
    scanf("%s", user_command);

    // Switch statement to handle different commands
    if (strcmp(user_command, "GA") == 0) // GETALL
    {

        // Prompt the user for the table name
        printf("Enter table name: ");
        char table_name[100];
        scanf("%s", table_name);

        char *args[] = {"./simpleMysql", "GETALL", table_name, NULL};
        execute_with_pipe(args);
    }
    else if (strcmp(user_command, "G1") == 0) // GETONE
    {

        // Prompt the user for the table name
        printf("Enter table name: ");

        scanf("%s", table_name);

        // Prompt the user for the ID
        printf("Enter ID: ");
        char id[20];
        scanf("%s", id);

        char *args[] = {"./simpleMysql", "GETONE", table_name, id, NULL};
        execute_with_pipe(args);
    }
    else if (strcmp(user_command, "V") == 0) // VIEWCOLUMNS
    {

        // Prompt the user for the table name
        printf("Enter table name: ");

        scanf("%s", table_name);

        char *args[] = {"./simpleMysql", "VIEWCOLUMNS", table_name, NULL};
        execute_with_pipe(args);
    }
    else if (strcmp(user_command, "A") == 0) // ADDONE
    {

        // Prompt the user for the table name
        printf("Enter table name: ");

        scanf("%s", table_name);

        char temp_view[500] = "./simpleMysql VIEWCOLUMNS ";
        system(strcat(temp_view, table_name));

        // Prompt the user for additional details
        printf("Enter Column values: ");
        char additional_details[500];
        scanf(" %[^\n]", additional_details); // Read until newline

        // Tokenize the additional details into separate tokens
        char *args[20];
        char *token = strtok(additional_details, " ");
        int arg_count = 3;
        while (token != NULL && arg_count < 20)
        {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL; // Null-terminate the array
        args[0] = "./simpleMysql";
        args[1] = "ADDONE";
        args[2] = table_name;
        // For demonstration, print the tokens
        for (int i = 0; i < arg_count; ++i)
        {
            printf("Token[%d]: %s\n", i, args[i]);
        }

        execute_with_pipe(args);
    }
    else
    {
        printf("Invalid command\n");
        return 1;
    }

    return 0;
}
