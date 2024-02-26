#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8080
#define MAX_CLIENTS 5

char *execute_with_pipe(char *args[])
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
        close(pipefd[1]);

        // Read the output from the pipe and store it in a string
        char buffer[1024];
        ssize_t bytes_read;
        char *output_string = malloc(1); // Start with an empty string
        output_string[0] = '\0';         // Null-terminate the string
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        {
            buffer[bytes_read] = '\0'; // Null-terminate the buffer
            output_string = realloc(output_string, strlen(output_string) + bytes_read + 1);
            strcat(output_string, buffer); // Append buffer to output_string
        }

        // Close the read end of the pipe
        close(pipefd[0]);

        return output_string;
    }
}

int main()
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    char table_name[100];
    char trigger[5];
    char *id_chosen;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow multiple connections on the same port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept incoming connections and handle client requests
    while (1)
    {
        char table_name[100] = {0};
        char trigger[5] = {0};

        printf("Server listening on port %d\n", PORT);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Receive client request
        memset(trigger, 0, sizeof(trigger));

        // Receive the trigger from the client
        valread = read(new_socket, trigger, sizeof(trigger) - 1);
        if (valread <= 0)
        {
            // Handle error or connection closed
            break;
        }
        printf("Received request from client: %s\n", trigger);

        while (1)
        {
            // Clear the table name buffer before receiving a new table name
            memset(table_name, 0, sizeof(table_name));

            // Receive the table name from the client
            valread = read(new_socket, table_name, sizeof(table_name) - 1);
            if (valread <= 0)
            {
                // Handle error or connection closed
                return 1;
            }

            // Check if the received table name matches predefined values
            int response_code = 0; // Default response code
            if (strcmp(table_name, "User") == 0 ||
                strcmp(table_name, "Department") == 0 ||
                strcmp(table_name, "Semester") == 0 ||
                strcmp(table_name, "Educator") == 0 ||
                strcmp(table_name, "Student") == 0 ||
                strcmp(table_name, "Subject") == 0 ||
                strcmp(table_name, "Class") == 0)
            {
                // If the received table name matches, set response code to 1
                response_code = 1;
            }

            // Send the response code back to the client
            write(new_socket, &response_code, sizeof(response_code));

            // If response code is 0, listen for the table name again
            if (response_code == 0)
            {
                // Continue to the beginning of the loop to receive a new table name
                continue;
            }
            else
            {
                // If response code is 1, break out of the loop
                break;
            }
        }

        printf("Received request from client: tablename %s\n", table_name);

        // Handle client request
        if (strcmp(trigger, "GA") == 0)
        {
            printf("Received trigger: %s, Table name: %s\n", trigger, table_name);

            // Execute the GETALL command with the given table name
            char *args[] = {"./simpleMysql", "GETALL", table_name, NULL};
            char *output = execute_with_pipe(args);

            // Send the output back to the client
            send(new_socket, output, strlen(output), 0);

            // Free dynamically allocated memory
            free(output);
        }

        else if (strcmp(trigger, "G1") == 0)
        {
            // Read the integer ID from the client
            printf("Reached at triige g1");
            int id_chosen;
            // memset(id_chosen, 0, sizeof(id_chosen));
            valread = read(new_socket, &id_chosen, sizeof(int)); // Use address of id_chosen
            printf("Received id: %d\n", id_chosen);              // Print the received ID

            // Convert the integer ID to a string
            char id_string[20];                                      // Buffer to store string representation of ID
            snprintf(id_string, sizeof(id_string), "%d", id_chosen); // Convert ID to string

            // Execute the GETONE command with the given table name and ID
            char *args[] = {"./simpleMysql", "GETONE", table_name, id_string, NULL};
            char *output = execute_with_pipe(args);

            printf("Out put: %s \n", output);

            // Send the output back to the client
            send(new_socket, output, strlen(output), 0);

            // Free dynamically allocated memory
            free(output);
        }
        else if (strcmp(trigger, "V") == 0)
        {

            printf("Received trigger: %s, Table name: %s\n", trigger, table_name);

            // Execute the GETALL command with the given table name
            char *args[] = {"./simpleMysql", "VIEWCOLUMNS", table_name, NULL};
            printf("Reched point1");
            char *output = execute_with_pipe(args);
            printf("Output: %s", output);

            // Send the output back to the client
            send(new_socket, output, strlen(output), 0);

            // Free dynamically allocated memoryGETALL
            free(output);
        }
        else if (strcmp(trigger, "A") == 0)
        {
            // Handle the ADDONE command
            // You need to read additional data from the client to perform the command
            // Similar logic as GETALL
            valread = read(new_socket, table_name, 100);
            printf("Received table name: %s\n", table_name);

            // response to client
            char received_table_desc[1024];                              // Define a buffer to store the concatenated string
            strcpy(received_table_desc, "Server: Received Table name "); // Copy the initial string
            strcat(received_table_desc, table_name);
            send(new_socket, received_table_desc, strlen(received_table_desc), 0);

            // Read additional data from the client
            char additional_data[1024];
            valread = read(new_socket, additional_data, sizeof(additional_data));
            printf("Received additional data: %s\n", additional_data);
            send(new_socket, "Server: Received additional data", strlen("Server: Received additional data"), 0);

            char *args[20];
            char *token = strtok(additional_data, " ");
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

            // Execute the command
            char *output = execute_with_pipe(args);

            // Send the output back to the client
            send(new_socket, output, strlen(output), 0);

            // Free dynamically allocated memory
            free(output);
        }
        else
        {
            printf("Invalid command\n");
            char *invalid_msg = "Server: Invalid command";
            send(new_socket, invalid_msg, strlen(invalid_msg), 0);
        }

        printf("Request handled successfully\n");

        // Close the socket
        close(new_socket);
    }

    return 0;
}
