#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 8080

// Global variable to store the response data
static char response_data[1024] = "";

void sanitize_string(const char *str)
{
    // Since you can't modify the string directly, you need to copy it to a new buffer
    char sanitized_str[strlen(str) + 1];
    strcpy(sanitized_str, str);

    // Iterate through the string and remove unwanted characters
    int len = strlen(sanitized_str);
    for (int i = 0; i < len; i++)
    {
        // Check if the character is a newline or a control character
        if (isspace(sanitized_str[i]) || iscntrl(sanitized_str[i]))
        {
            // Replace with a \0
            sanitized_str[i] = '\0';
        }
        // You can add more conditions to remove other unwanted characters if needed
    }

    printf("Sanitized string: %s\n", sanitized_str);
}
char *send_data_to_server_base(const char *trigger, const char *table_name)
{
    // Clear the previous response data
    bzero(response_data, sizeof(response_data));
    for (size_t i = 0; i < sizeof(response_data); ++i)
    {
        response_data[i] = 0;
    }

    int sock = 0;
    struct sockaddr_in serv_addr;

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

    if (send(sock, trigger, strlen(trigger) + 1, 0) == -1)
    {
        perror("send trigger");
        exit(EXIT_FAILURE);
    }

    // Send newline character as delimiter
    if (send(sock, "\n", 1, 0) == -1)
    {
        perror("send newline1");
        exit(EXIT_FAILURE);
    }
    printf("SEnding tablename %s\n ", table_name);
    sanitize_string(table_name);
    // Send table name to server
    while (1)
    {
        int sent_bytes = send(sock, table_name, strlen(table_name) + 1, 0);
        if (sent_bytes == -1)
        {
            perror("send table name");
            exit(EXIT_FAILURE);
        }
        else if (sent_bytes < strlen(table_name) + 1)
        {
            fprintf(stderr, "Incomplete send\n");
        }

        int response_code;
        int bytes_received = recv(sock, &response_code, sizeof(response_code), 0);
        if (bytes_received == -1)
        {
            perror("recv response");
            exit(EXIT_FAILURE);
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by server\n");
            exit(EXIT_FAILURE);
        }

        if (response_code == 1)
        {
            printf("Valid table name received\n");
            break;
        }
    }

    // Send newline character as delimiter
    if (send(sock, "\n", 1, 0) == -1)
    {
        perror("send newline2");
        exit(EXIT_FAILURE);
    } // Send a newline character as a delimiter

    // Send additional data to server if provided

    // Receive response from server
    read(sock, response_data, sizeof(response_data));

    close(sock); // Close the socket after communication

    printf("%s", response_data);

    return response_data;
}
char *send_data_to_server_get_one(const char *trigger, const char *table_name, const char *id)
{
    // Clear the previous response data
    bzero(response_data, sizeof(response_data));

    int sock = 0;
    struct sockaddr_in serv_addr;

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

    // Send trigger to server
    if (send(sock, trigger, strlen(trigger) + 1, 0) == -1)
    {
        perror("send trigger");
        exit(EXIT_FAILURE);
    }

    // Send newline character as delimiter
    if (send(sock, "\n", 1, 0) == -1)
    {
        perror("send newline1");
        exit(EXIT_FAILURE);
    }

    // Sanitize and send table name to server
    sanitize_string(table_name);
    int sent_bytes = send(sock, table_name, strlen(table_name) + 1, 0);
    if (sent_bytes == -1)
    {
        perror("send table name");
        exit(EXIT_FAILURE);
    }

    // Send newline character as delimiter
    // if (send(sock, "\n", 1, 0) == -1)
    // {
    //     perror("send newline2");
    //     exit(EXIT_FAILURE);
    // }

    // Send ID to server
    sanitize_string(id);
    sent_bytes = send(sock, id, strlen(id), 0);
    if (sent_bytes == -1)
    {
        perror("send id");
        exit(EXIT_FAILURE);
    }

    // Send newline character as delimiter
    if (send(sock, "\n", 1, 0) == -1)
    {
        perror("send newline3");
        exit(EXIT_FAILURE);
    }

    // Receive response from server
    int total_bytes_received = 0;
    int bytes_received;

    while (total_bytes_received < sizeof(response_data) - 1)
    {
        bytes_received = recv(sock, response_data + total_bytes_received, sizeof(response_data) - 1 - total_bytes_received, 0);
        if (bytes_received == -1)
        {
            perror("recv response");
            exit(EXIT_FAILURE);
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by server\n");
            exit(EXIT_FAILURE);
        }

        total_bytes_received += bytes_received;

        // Check if the message is complete
        if (response_data[total_bytes_received - 1] == '\0')
        {
            break; // Message complete
        }
    }

    response_data[total_bytes_received] = '\0';
    close(sock); // Close the socket after communication

    printf("%s", response_data);

    return response_data;
}

// Struct for User Table Widgets
typedef struct
{
    GtkWidget *label_id;
    GtkWidget *entry_id;
    GtkWidget *label_username;
    GtkWidget *entry_username;
    GtkWidget *label_password;
    GtkWidget *entry_password;
    GtkWidget *label_is_admin;
    GtkWidget *entry_is_admin;
} UserTableWidgets;

// Struct for Student Table Widgets
typedef struct
{
    GtkWidget *label_id;
    GtkWidget *entry_id;
    GtkWidget *label_user_id;
    GtkWidget *entry_user_id;
    GtkWidget *label_department_id;
    GtkWidget *entry_department_id;
    // Add more properties for other columns in the Student table
} StudentTableWidgets;

// Struct for Educator Table Widgets
typedef struct
{
    GtkWidget *label_id;
    GtkWidget *entry_id;
    GtkWidget *label_user_id;
    GtkWidget *entry_user_id;
    // Add more properties for other columns in the Educator table
} EducatorTableWidgets;

// Struct for Department Table Widgets
typedef struct
{
    GtkWidget *label_id;
    GtkWidget *entry_id;
    GtkWidget *label_name;
    GtkWidget *entry_name;
    GtkWidget *label_head_educator_id;
    GtkWidget *entry_head_educator_id;
    // Add more properties for other columns in the Department table
} DepartmentTableWidgets;

// Struct for Semester Table Widgets
typedef struct
{
    GtkWidget *label_id;
    GtkWidget *entry_id;
    GtkWidget *label_name;
    GtkWidget *entry_name;
    GtkWidget *label_department_id;
    GtkWidget *entry_department_id;
    // Add more properties for other columns in the Semester table
} SemesterTableWidgets;

typedef struct
{
    GtkWidget *label_id;
    GtkWidget *entry_id;
} GetOneIdWidgets;

UserTableWidgets create_user_table_widgets(GtkWidget *box)
{
    UserTableWidgets widgets;

    // Label and Entry for User ID
    widgets.label_id = gtk_label_new("User ID:");
    widgets.entry_id = gtk_entry_new();
    gtk_widget_set_size_request(widgets.entry_id, 200, 30);
    gtk_box_pack_start(GTK_BOX(box), widgets.label_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_id, FALSE, FALSE, 0);

    // Label and Entry for Username
    widgets.label_username = gtk_label_new("Username:");
    widgets.entry_username = gtk_entry_new();
    gtk_widget_set_size_request(widgets.entry_username, 200, 30);
    gtk_box_pack_start(GTK_BOX(box), widgets.label_username, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_username, FALSE, FALSE, 0);

    // Label and Entry for Password
    widgets.label_password = gtk_label_new("Password:");
    widgets.entry_password = gtk_entry_new();
    gtk_widget_set_size_request(widgets.entry_password, 200, 30);
    gtk_box_pack_start(GTK_BOX(box), widgets.label_password, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_password, FALSE, FALSE, 0);

    // Label and Entry for isAdmin
    widgets.label_is_admin = gtk_label_new("isAdmin:");
    widgets.entry_is_admin = gtk_entry_new();
    gtk_widget_set_size_request(widgets.entry_is_admin, 200, 30);
    gtk_box_pack_start(GTK_BOX(box), widgets.label_is_admin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_is_admin, FALSE, FALSE, 0);

    // Hide all entries initially
    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.entry_username);
    gtk_widget_hide(widgets.entry_password);
    gtk_widget_hide(widgets.entry_is_admin);

    return widgets;
}

// Function to create Student Table Widgets
StudentTableWidgets create_student_table_widgets(GtkWidget *box)
{
    StudentTableWidgets widgets;

    // Label and Entry for Student ID
    widgets.label_id = gtk_label_new("Student ID:");
    widgets.entry_id = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_id, FALSE, FALSE, 0);

    // Label and Entry for User ID
    widgets.label_user_id = gtk_label_new("User ID:");
    widgets.entry_user_id = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_user_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_user_id, FALSE, FALSE, 0);

    // Label and Entry for Department ID
    widgets.label_department_id = gtk_label_new("Department ID:");
    widgets.entry_department_id = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_department_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_department_id, FALSE, FALSE, 0);

    // Hide all entries initially
    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.entry_user_id);
    gtk_widget_hide(widgets.entry_department_id);

    return widgets;
}

// Function to create Educator Table Widgets
EducatorTableWidgets create_educator_table_widgets(GtkWidget *box)
{
    EducatorTableWidgets widgets;

    // Label and Entry for Educator ID
    widgets.label_id = gtk_label_new("Educator ID:");
    widgets.entry_id = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_id, FALSE, FALSE, 0);

    // Label and Entry for User ID
    widgets.label_user_id = gtk_label_new("User ID:");
    widgets.entry_user_id = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_user_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_user_id, FALSE, FALSE, 0);

    // Hide all entries initially
    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.entry_user_id);

    return widgets;
}

// Function to create Department Table Widgets
DepartmentTableWidgets create_department_table_widgets(GtkWidget *box)
{
    DepartmentTableWidgets widgets;

    // Label and Entry for Department ID
    widgets.label_id = gtk_label_new("Department ID:");
    widgets.entry_id = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_id, FALSE, FALSE, 0);

    // Label and Entry for Department Name
    widgets.label_name = gtk_label_new("Department Name:");
    widgets.entry_name = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_name, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_name, FALSE, FALSE, 0);

    // Label and Entry for Head Educator ID
    widgets.label_head_educator_id = gtk_label_new("Head Educator ID:");
    widgets.entry_head_educator_id = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_head_educator_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_head_educator_id, FALSE, FALSE, 0);

    // Hide all entries initially
    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.entry_name);
    gtk_widget_hide(widgets.entry_head_educator_id);

    return widgets;
}

// Function to create Semester Table Widgets
SemesterTableWidgets create_semester_table_widgets(GtkWidget *box)
{
    SemesterTableWidgets widgets;

    // Label and Entry for Semester ID
    widgets.label_id = gtk_label_new("Semester ID:");
    widgets.entry_id = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_id, FALSE, FALSE, 0);

    // Label and Entry for Semester Name
    widgets.label_name = gtk_label_new("Semester Name:");
    widgets.entry_name = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_name, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_name, FALSE, FALSE, 0);

    // Label and Entry for Department ID
    widgets.label_department_id = gtk_label_new("Department ID:");
    widgets.entry_department_id = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_department_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_department_id, FALSE, FALSE, 0);

    // Hide all entries initially
    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.entry_name);
    gtk_widget_hide(widgets.entry_department_id);

    return widgets;
}
GetOneIdWidgets create_get_one_id_widgets(GtkWidget *box)
{
    GetOneIdWidgets widgets;
    widgets.label_id = gtk_label_new("Enter Id:");
    widgets.entry_id = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), widgets.label_id, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets.entry_id, FALSE, FALSE, 0);

    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.label_id);
    return widgets;
}

// Helper function to hide all widgets of a table
void hide_user_table_widgets(UserTableWidgets widgets)
{
    gtk_widget_hide(widgets.label_id);
    gtk_widget_hide(widgets.label_username);
    gtk_widget_hide(widgets.label_password);
    gtk_widget_hide(widgets.label_is_admin);
    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.entry_username);
    gtk_widget_hide(widgets.entry_password);
    gtk_widget_hide(widgets.entry_is_admin);
    // Hide other widgets for the table as well
}

void hide_student_table_widgets(StudentTableWidgets widgets)
{
    gtk_widget_hide(widgets.label_id);
    gtk_widget_hide(widgets.label_user_id);
    gtk_widget_hide(widgets.label_department_id);
    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.entry_user_id);
    gtk_widget_hide(widgets.entry_department_id);
    // Hide other widgets for the table as well
}

void hide_educator_table_widgets(EducatorTableWidgets widgets)
{
    gtk_widget_hide(widgets.label_id);
    gtk_widget_hide(widgets.label_user_id);
    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.entry_user_id);
    // Hide other widgets for the table as well
}

void hide_department_table_widgets(DepartmentTableWidgets widgets)
{
    gtk_widget_hide(widgets.label_id);
    gtk_widget_hide(widgets.label_name);
    gtk_widget_hide(widgets.label_head_educator_id);
    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.entry_name);
    gtk_widget_hide(widgets.entry_head_educator_id);
    // Hide other widgets for the table as well
}

void hide_semester_table_widgets(SemesterTableWidgets widgets)
{
    gtk_widget_hide(widgets.label_id);
    gtk_widget_hide(widgets.label_name);
    gtk_widget_hide(widgets.label_department_id);
    gtk_widget_hide(widgets.entry_id);
    gtk_widget_hide(widgets.entry_name);
    gtk_widget_hide(widgets.entry_department_id);
    // Hide other widgets for the table as well
}
void hide_get_one_id_widgets(GetOneIdWidgets widgets)
{
    gtk_widget_hide(widgets.label_id);
    gtk_widget_hide(widgets.entry_id);
}

// Helper function to show all widgets of a table
void show_user_table_widgets(UserTableWidgets widgets)
{
    // gtk_widget_show(widgets.label_id);
    gtk_widget_show(widgets.label_username);
    gtk_widget_show(widgets.label_password);
    gtk_widget_show(widgets.label_is_admin);
    // gtk_widget_show(widgets.entry_id);
    gtk_widget_show(widgets.entry_username);
    gtk_widget_show(widgets.entry_password);
    gtk_widget_show(widgets.entry_is_admin);
    // Show other widgets for the table as well
}

void show_student_table_widgets(StudentTableWidgets widgets)
{
    // gtk_widget_show(widgets.label_id);
    gtk_widget_show(widgets.label_user_id);
    gtk_widget_show(widgets.label_department_id);
    // gtk_widget_show(widgets.entry_id);
    gtk_widget_show(widgets.entry_user_id);
    gtk_widget_show(widgets.entry_department_id);
    // Show other widgets for the table as well
}

void show_educator_table_widgets(EducatorTableWidgets widgets)
{
    // gtk_widget_show(widgets.label_id);
    gtk_widget_show(widgets.label_user_id);
    // gtk_widget_show(widgets.entry_id);
    gtk_widget_show(widgets.entry_user_id);
    // Show other widgets for the table as well
}

void show_department_table_widgets(DepartmentTableWidgets widgets)
{
    // gtk_widget_show(widgets.label_id);
    gtk_widget_show(widgets.label_name);
    gtk_widget_show(widgets.label_head_educator_id);
    // gtk_widget_show(widgets.entry_id);
    gtk_widget_show(widgets.entry_name);
    gtk_widget_show(widgets.entry_head_educator_id);
    // Show other widgets for the table as well
}

void show_semester_table_widgets(SemesterTableWidgets widgets)
{
    // gtk_widget_show(widgets.label_id);
    gtk_widget_show(widgets.label_name);
    gtk_widget_show(widgets.label_department_id);
    // gtk_widget_show(widgets.entry_id);
    gtk_widget_show(widgets.entry_name);
    gtk_widget_show(widgets.entry_department_id);
    // Show other widgets for the table as well
}
void show_get_one_id_widgets(GetOneIdWidgets widgets)
{
    gtk_widget_show(widgets.label_id);
    gtk_widget_show(widgets.entry_id);
}

void on_combo2_changed(GtkWidget *combo2, gpointer data);
// Callback function for combo box 2 change eventvoid on_combo2_changed(GtkWidget * widget, gpointer data);
void on_combo1_changed(GtkWidget *widget, gpointer data)
{
    GtkWidget *combo2 = GTK_WIDGET(data);
    GetOneIdWidgets *get_one_widgets = (GetOneIdWidgets *)g_object_get_data(G_OBJECT(combo2), "get_one_id_widgets");
    gtk_widget_show(combo2);
    show_get_one_id_widgets(*get_one_widgets);

    // Clear previous selection in combo2
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), -1);

    on_combo2_changed(combo2, data);
}
void on_combo2_changed(GtkWidget *combo2, gpointer data)
{
    // Retrieve the value of combo1 from the data parameter
    GtkWidget *combo1 = GTK_WIDGET(data);

    // Determine the selected value of combo1
    gint active_combo1 = gtk_combo_box_get_active(GTK_COMBO_BOX(combo1));
    const gchar *selected_value_combo1 = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo1));

    // Now you have access to the value of combo1
    // printf("Selected value of combo1: %s\n", selected_value_combo1);

    // Set the flag indicating combo box 1 is selected

    // printf("It works\n");
    // Retrieve widgets for all tables
    UserTableWidgets *user_widgets = (UserTableWidgets *)g_object_get_data(G_OBJECT(combo2), "user_widgets");
    StudentTableWidgets *student_widgets = (StudentTableWidgets *)g_object_get_data(G_OBJECT(combo2), "student_widgets");
    EducatorTableWidgets *educator_widgets = (EducatorTableWidgets *)g_object_get_data(G_OBJECT(combo2), "educator_widgets");
    DepartmentTableWidgets *department_widgets = (DepartmentTableWidgets *)g_object_get_data(G_OBJECT(combo2), "department_widgets");
    SemesterTableWidgets *semester_widgets = (SemesterTableWidgets *)g_object_get_data(G_OBJECT(combo2), "semester_widgets");
    GetOneIdWidgets *get_one_widgets = (GetOneIdWidgets *)g_object_get_data(G_OBJECT(combo2), "get_one_id_widgets");

    hide_user_table_widgets(*user_widgets);
    hide_student_table_widgets(*student_widgets);
    hide_educator_table_widgets(*educator_widgets);
    hide_department_table_widgets(*department_widgets);
    hide_semester_table_widgets(*semester_widgets);
    hide_get_one_id_widgets(*get_one_widgets);

    // Determine which table is selected and display its widgets
    gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(combo2));
    if (active >= 0)
    {
        const gchar *selected_table = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo2));
        // printf("Selected value of combo2: %s\n", selected_table);

        if (strcmp(selected_value_combo1, "A") == 0)
        {

            if (g_strcmp0(selected_table, "User") == 0)
            {
                // Show widgets for User table
                show_user_table_widgets(*user_widgets);
            }
            else if (g_strcmp0(selected_table, "Student") == 0)
            {
                // Show widgets for Student table
                show_student_table_widgets(*student_widgets);
            }
            else if (g_strcmp0(selected_table, "Educator") == 0)
            {
                // Show widgets for Educator table
                show_educator_table_widgets(*educator_widgets);
            }
            else if (g_strcmp0(selected_table, "Department") == 0)
            {
                // Show widgets for Department table
                show_department_table_widgets(*department_widgets);
            }
            else if (g_strcmp0(selected_table, "Semester") == 0)
            {
                // Show widgets for Semester table
                show_semester_table_widgets(*semester_widgets);
            }
            gtk_widget_set_size_request(combo2, 300, 30);
        }
        else if (strcmp(selected_value_combo1, "G1") == 0)
        {
            show_get_one_id_widgets(*get_one_widgets);
        }
        g_free((gchar *)selected_table);
    }
    g_free((gchar *)selected_value_combo1);
}

void send_command_and_display_response(const char *command, const char *table_name, const char *additional_data)
{
    // Send data to server
    if (additional_data == NULL)
    {
        send_data_to_server_base(command, table_name);
    }
    else if (additional_data != NULL)
    {
        send_data_to_server_get_one("G1", table_name, additional_data);
    }

    // Display the response data in a new window
    GtkWidget *response_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(response_window), "Response Data");
    gtk_window_set_default_size(GTK_WINDOW(response_window), 400, 300);

    // Create a label for the response data
    GtkWidget *response_label = gtk_label_new(NULL);

    // Set the response data to the label
    gtk_label_set_text(GTK_LABEL(response_label), response_data);

    // Add the response label to the response window
    gtk_container_add(GTK_CONTAINER(response_window), response_label);

    // Show all widgets
    gtk_widget_show_all(response_window);
}

typedef struct
{
    UserTableWidgets user_widgets;
    StudentTableWidgets student_widgets;
    EducatorTableWidgets educator_widgets;
    DepartmentTableWidgets department_widgets;
    SemesterTableWidgets semester_widgets;
    GetOneIdWidgets get_one_id_widgets;
    GtkWidget *combo1;
    GtkWidget *combo2;
} ButtonClickData;
void on_button_clicked(GtkWidget *widget, gpointer data)
{

    ButtonClickData *button_data = (ButtonClickData *)data;

    // Access the widgets as needed
    UserTableWidgets *user_widgets = &(button_data->user_widgets);
    StudentTableWidgets *student_widgets = &(button_data->student_widgets);
    EducatorTableWidgets *educator_widgets = &(button_data->educator_widgets);
    DepartmentTableWidgets *department_widgets = &(button_data->department_widgets);
    SemesterTableWidgets *semester_widgets = &(button_data->semester_widgets);
    GetOneIdWidgets *get_one_id_widgets = &(button_data->get_one_id_widgets);
    GtkWidget *combo1 = button_data->combo1;
    GtkWidget *combo2 = button_data->combo2;

    // Retrieve the selected values from combo1 and combo2
    const gchar *selected_value_combo1 = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(button_data->combo1));
    const gchar *selected_value_combo2 = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(button_data->combo2));

    // Check the selected value of combo1
    if (g_strcmp0(selected_value_combo1, "GA") == 0)
    {
        g_print("Selected Command: %s\n", selected_value_combo1);
        // Send data to server
        send_command_and_display_response(selected_value_combo1, selected_value_combo2, NULL);
    }
    else if (g_strcmp0(selected_value_combo1, "G1") == 0)
    {
        g_print("Selected Command: %s\n", selected_value_combo1);
        GtkWidget *get_one_id = get_one_id_widgets->entry_id;
        const gchar *get_one_id_text = gtk_entry_get_text(GTK_ENTRY(get_one_id));
        g_print("Id inputted: %s\n", get_one_id_text);
        send_command_and_display_response(selected_value_combo1, selected_value_combo2, get_one_id_text);
    }
    else if (g_strcmp0(selected_value_combo1, "V") == 0)
    {
        g_print("Selected Command: %s\n", selected_value_combo1);
        send_command_and_display_response("V", selected_value_combo2, NULL);
    }
    else if (g_strcmp0(selected_value_combo1, "A") == 0)
    {

        g_print("Selected Command: %s\n", selected_value_combo1);
        if (g_strcmp0(selected_value_combo2, "User") == 0)
        {
            GtkWidget *entry_username = user_widgets->entry_username;
            GtkWidget *entry_password = user_widgets->entry_password;
            GtkWidget *entry_is_admin = user_widgets->entry_is_admin;

            // Printing the values
            const gchar *username_text = gtk_entry_get_text(GTK_ENTRY(entry_username));
            const gchar *password_text = gtk_entry_get_text(GTK_ENTRY(entry_password));
            const gchar *is_admin_text = gtk_entry_get_text(GTK_ENTRY(entry_is_admin));

            g_print("Username: %s\n", username_text);
            g_print("Password: %s\n", password_text);
            g_print("isAdmin: %s\n", is_admin_text);
        }
        else if (g_strcmp0(selected_value_combo2, "Student") == 0)
        {
            GtkWidget *entry_user_id = student_widgets->entry_user_id;
            GtkWidget *entry_department_id = student_widgets->entry_department_id;

            // Printing the values
            const gchar *user_id_text = gtk_entry_get_text(GTK_ENTRY(entry_user_id));
            const gchar *department_id_text = gtk_entry_get_text(GTK_ENTRY(entry_department_id));

            g_print("User ID: %s\n", user_id_text);
            g_print("Department ID: %s\n", department_id_text);
        }
        else if (g_strcmp0(selected_value_combo2, "Educator") == 0)
        {
            GtkWidget *entry_user_id = educator_widgets->entry_user_id;

            // Printing the values
            const gchar *user_id_text = gtk_entry_get_text(GTK_ENTRY(entry_user_id));

            g_print("User ID: %s\n", user_id_text);
        }
        else if (g_strcmp0(selected_value_combo2, "Department") == 0)
        {
            GtkWidget *entry_name = department_widgets->entry_name;
            GtkWidget *entry_head_educator_id = department_widgets->entry_head_educator_id;

            // Printing the values
            const gchar *name_text = gtk_entry_get_text(GTK_ENTRY(entry_name));
            const gchar *head_educator_id_text = gtk_entry_get_text(GTK_ENTRY(entry_head_educator_id));

            g_print("Department Name: %s\n", name_text);
            g_print("Head Educator ID: %s\n", head_educator_id_text);
        }
        else if (g_strcmp0(selected_value_combo2, "Semester") == 0)
        {
            GtkWidget *entry_name = semester_widgets->entry_name;
            GtkWidget *entry_department_id = semester_widgets->entry_department_id;

            // Printing the values
            const gchar *name_text = gtk_entry_get_text(GTK_ENTRY(entry_name));
            const gchar *department_id_text = gtk_entry_get_text(GTK_ENTRY(entry_department_id));

            g_print("Semester Name: %s\n", name_text);
            g_print("Department ID: %s\n", department_id_text);
        }
    }

    // Check selected value of combo2

    // Check the selected value of combo2 and retrieve associated entry widgets if they are visible

    // g_free(button_data);

    g_print("\n");
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Client");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 1500, 1000);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a scrolled window to contain the combo boxes
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(window), scrolled_window);

    // Create a vertical box to hold the combo boxes
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(scrolled_window), box);

    GtkWidget *label1 = gtk_label_new("Select Command Getall, Getone, View Columns or Addone:");
    gtk_box_pack_start(GTK_BOX(box), label1, FALSE, FALSE, 0);

    GtkWidget *combo1 = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo1), "GA");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo1), "G1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo1), "V");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo1), "A");
    // gtk_widget_set_size_request(combo1, 300, 30);
    // gtk_widget_set_size_request(combo1, -1, 50);
    // gtk_fixed_put(GTK_FIXED(box), combo1, 10, 10);
    gtk_box_pack_start(GTK_BOX(box), combo1, FALSE, FALSE, 0);

    GtkWidget *label2 = gtk_label_new("Select Table");
    gtk_box_pack_start(GTK_BOX(box), label2, FALSE, FALSE, 0);
    // g_signal_connect(combo1, "changed", G_CALLBACK(on_combo1_changed), NULL);

    GtkWidget *combo2 = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo2), "User");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo2), "Student");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo2), "Educator");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo2), "Department");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo2), "Semester");
    gtk_box_pack_start(GTK_BOX(box), combo2, FALSE, FALSE, 0);

    gtk_widget_hide(combo2);
    // gtk_widget_set_size_request(combo2, 300, 30);
    // gtk_widget_set_size_request(combo1, -1, 50);
    // gtk_fixed_put(GTK_FIXED(box), combo2, 10, 70);

    gtk_box_pack_start(GTK_BOX(box), combo2, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(combo2), "combo1", combo1);
    g_signal_connect(combo2, "changed", G_CALLBACK(on_combo2_changed), combo1);

    g_object_set_data(G_OBJECT(combo1), "combo2", combo2);
    g_signal_connect(combo1, "changed", G_CALLBACK(on_combo1_changed), combo2);

    gtk_widget_set_size_request(combo2, 300, 30);
    // g_object_set_data(G_OBJECT(combo1), "combo2", combo2);
    // g_signal_connect(combo2, "changed", G_CALLBACK(on_combo2_changed), NULL);
    // g_signal_connect(combo1, "changed", G_CALLBACK(on_combo1_changed), NULL);
    // Create widgets for User table
    // GtkWidget *user_label = gtk_label_new("User Table:");
    // gtk_box_pack_start(GTK_BOX(box), user_label, FALSE, FALSE, 0);
    UserTableWidgets user_widgets = create_user_table_widgets(box);

    // Create widgets for Student table
    // GtkWidget *student_label = gtk_label_new("Student Table:");
    // gtk_box_pack_start(GTK_BOX(box), student_label, FALSE, FALSE, 0);
    StudentTableWidgets student_widgets = create_student_table_widgets(box);

    // Create widgets for Educator table
    // GtkWidget *educator_label = gtk_label_new("Educator Table:");
    // gtk_box_pack_start(GTK_BOX(box), educator_label, FALSE, FALSE, 0);
    EducatorTableWidgets educator_widgets = create_educator_table_widgets(box);

    // Create widgets for Department table
    // GtkWidget *department_label = gtk_label_new("Department Table:");
    // gtk_box_pack_start(GTK_BOX(box), department_label, FALSE, FALSE, 0);
    DepartmentTableWidgets department_widgets = create_department_table_widgets(box);

    // Create widgets for Semester table
    // GtkWidget *semester_label = gtk_label_new("Semester Table:");
    // gtk_box_pack_start(GTK_BOX(box), semester_label, FALSE, FALSE, 0);
    SemesterTableWidgets semester_widgets = create_semester_table_widgets(box);

    GetOneIdWidgets get_one_id_widgets = create_get_one_id_widgets(box);

    g_object_set_data(G_OBJECT(combo2), "user_widgets", &user_widgets);
    g_object_set_data(G_OBJECT(combo2), "student_widgets", &student_widgets);
    g_object_set_data(G_OBJECT(combo2), "educator_widgets", &educator_widgets);
    g_object_set_data(G_OBJECT(combo2), "department_widgets", &department_widgets);
    g_object_set_data(G_OBJECT(combo2), "semester_widgets", &semester_widgets);
    g_object_set_data(G_OBJECT(combo2), "get_one_id_widgets", &get_one_id_widgets);
    printf("Actual combo1: %p\n", (void *)combo1);
    printf("Actual combo2: %p\n", (void *)combo2);

    GtkWidget *button = gtk_button_new_with_label("SEND TO SERVER");
    ButtonClickData *button_data = g_new(ButtonClickData, 1);
    button_data->user_widgets = user_widgets;
    button_data->student_widgets = student_widgets;
    button_data->educator_widgets = educator_widgets;
    button_data->department_widgets = department_widgets;
    button_data->semester_widgets = semester_widgets;
    button_data->get_one_id_widgets = get_one_id_widgets;
    button_data->combo1 = combo1;
    button_data->combo2 = combo2;

    // Display the values stored in ComboData

    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), button_data);

    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
