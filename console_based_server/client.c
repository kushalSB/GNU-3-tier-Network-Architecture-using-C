#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

int main()
{
    struct sockaddr_in serv_addr;

    while (1)
    {
        char buffer[1024] = {0};
        char table_name[1024]; // Increased buffer size to match server
        char trigger[5];
        int table_choice;
        int sock = 0, valread;

        // Create socket file descriptor
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        // Set server address and port
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
        {
            perror("inet_pton");
            exit(EXIT_FAILURE);
        }

        // Connect to server
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("connect");
            exit(EXIT_FAILURE);
        }

        int valid_input = 0;
        do
        {
            printf("Press: G1, GA, V, or A\n");
            scanf("%s", trigger);

            // Check if the trigger is valid
            if (strcmp(trigger, "G1") == 0 || strcmp(trigger, "GA") == 0 || strcmp(trigger, "V") == 0 || strcmp(trigger, "A") == 0)
            {
                send(sock, trigger, sizeof(trigger), 0);
                valid_input = 1; // Set flag to indicate valid input
            }
            else
            {
                printf("Invalid input. Please enter G1, GA, V, or A.\n");
            }
        } while (!valid_input); // Continue looping until valid input is received

        // Recieved Command response from server
        valread = read(sock, buffer, 1024);
        printf("%s\n", buffer);

        int valid_input_table = 0;

        do
        {
            // Prompt user for table choice
            printf("Choose table name:\n");
            printf("\t1: User");
            printf("\t2: Educator");
            printf("\t3: Student");
            printf("\t4: Subject");
            printf("\t5: Class");
            printf("\t6: Department\n");
            printf("Your choice: ");
            scanf("%d", &table_choice);

            while (getchar() != '\n')
                ;

            switch (table_choice)
            {
            case 1:
                strcpy(table_name, "User");
                valid_input_table = 1; // Set flag to indicate valid input
                break;
            case 2:
                strcpy(table_name, "Educator");
                valid_input_table = 1; // Set flag to indicate valid input
                break;
            case 3:
                strcpy(table_name, "Student");
                valid_input_table = 1; // Set flag to indicate valid input
                break;
            case 4:
                strcpy(table_name, "Subject");
                valid_input_table = 1; // Set flag to indicate valid input
                break;
            case 5:
                strcpy(table_name, "Class");
                valid_input_table = 1; // Set flag to indicate valid input
                break;
            case 6:
                strcpy(table_name, "Department");
                valid_input_table = 1; // Set flag to indicate valid input
                break;
            default:
                printf("Invalid choice. Please enter a number between 1 and 6.\n");
            }
        } while (!valid_input_table); // Continue looping until valid input is received

        // Send table name to server
        send(sock, table_name, strlen(table_name) + 1, 0);

        // Recieved Table name Response
        valread = read(sock, buffer, 1024);
        printf("%s\n", buffer);

        if (strcmp(trigger, "G1") == 0)
        {
            int id_choice;
            printf("Please input id of %s", table_name);
            scanf("%d", &id_choice);

            printf("Client: Id entered is'%d' with address '%p'\n", id_choice, &id_choice);

            send(sock, &id_choice, sizeof(int), 0);
        }

        if (strcmp(trigger, "A") == 0)
        {
            // Additional data for the ADDONE command
            char additional_data[1024];

            // Prompt the user for additional data
            printf("Enter additional data: ");
            fgets(additional_data, sizeof(additional_data), stdin);
            // Remove newline character from input if present
            additional_data[strcspn(additional_data, "\n")] = '\0';
            printf("%s", additional_data);
            send(sock, additional_data, strlen(additional_data) + 1, 0); // Include null terminator

            // Read response from the server
            valread = read(sock, buffer, 1024);
            printf("%s\n", buffer);
        }

        // Read response from server
        valread = read(sock, buffer, 1024);
        printf("%s\n", buffer);

        close(sock); // Close the socket after communication

        printf("Press 'q' to quit, any other key to continue: ");
        char choice;
        scanf(" %c", &choice);
        if (choice == 'q' || choice == 'Q')
            break;
    }

    return 0;
}
