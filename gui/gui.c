#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

// Global variable to store the response data
static char response_data[1024] = "";

// Function to handle sending data to the server and receiving response
char *send_data_to_server(const char *trigger, const char *table_name, const char *additional_data)
{
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

    send(sock, trigger, strlen(trigger), 0);
    send(sock, "\n", 1, 0); // Send a newline character as a delimiter

    // Send table name to server
    send(sock, table_name, strlen(table_name) + 1, 0); // Include null terminator
    send(sock, "\n", 1, 0);                            // Send a newline character as a delimiter

    // Send additional data to server if provided
    if (additional_data != NULL)
    {
        send(sock, additional_data, strlen(additional_data) + 1, 0); // Include null terminator
        send(sock, "\n", 1, 0);                                      // Send a newline character as a delimiter
    }

    // Receive response from server
    read(sock, response_data, sizeof(response_data));

    close(sock); // Close the socket after communication

    return response_data;
}

// Helper function to find the additional data entry widget
gboolean find_additional_data_entry(GtkWidget *widget, gpointer data);

void on_send_button_clicked(GtkWidget *widget, gpointer data)
{
    // Clear the previous response data
    memset(response_data, 0, sizeof(response_data));

    // Get the top-level window
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(widget));

    // Check if the top-level window is valid
    if (!GTK_IS_WINDOW(window))
    {
        fprintf(stderr, "Error: Invalid top-level window.\n");
        return;
    }

    // Get the combo box widgets from the data passed as gpointer
    GtkWidget *vbox = GTK_WIDGET(data);
    GtkWidget *trigger_combo_box = NULL;
    GtkWidget *table_name_combo_box = NULL;

    // Get the children of the vertical box container
    GList *children = gtk_container_get_children(GTK_CONTAINER(vbox));

    // Iterate through the children to find the combo box widgets
    GList *child_iter = children;
    while (child_iter != NULL)
    {
        GtkWidget *child = GTK_WIDGET(child_iter->data);
        if (GTK_IS_BOX(child))
        {
            GList *box_children = gtk_container_get_children(GTK_CONTAINER(child));
            GList *box_child_iter = box_children;
            while (box_child_iter != NULL)
            {
                GtkWidget *box_child = GTK_WIDGET(box_child_iter->data);
                const gchar *label_text = gtk_label_get_text(GTK_LABEL(box_child));
                if (g_strcmp0(label_text, "Trigger") == 0)
                {
                    trigger_combo_box = box_child;
                }
                else if (g_strcmp0(label_text, "Table Name") == 0)
                {
                    table_name_combo_box = box_child;
                }
                box_child_iter = g_list_next(box_child_iter);
            }
            g_list_free(box_children);
        }
        child_iter = g_list_next(child_iter);
    }
    g_list_free(children);

    // Check if the combo box widgets were found
    if (!trigger_combo_box || !table_name_combo_box)
    {
        fprintf(stderr, "Error: Combo box widgets not found.\n");
        return;
    }

    // Get the selected values from the combo boxes
    const char *trigger = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(trigger_combo_box));
    const char *table_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(table_name_combo_box));

    // Check if the combo box texts are valid
    if (!trigger || !table_name)
    {
        fprintf(stderr, "Error: Invalid combo box texts.\n");
        return;
    }

    // Get additional data from the entry widget
    GtkWidget *additional_data_entry;
    gtk_container_foreach(GTK_CONTAINER(vbox), (GtkCallback)find_additional_data_entry, &additional_data_entry);

    const char *additional_data = gtk_entry_get_text(GTK_ENTRY(additional_data_entry));

    // Send data to server
    send_data_to_server(trigger, table_name, additional_data);

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

// Helper function to find the additional data entry widget
gboolean find_additional_data_entry(GtkWidget *widget, gpointer data)
{
    if (GTK_IS_ENTRY(widget) && gtk_widget_get_parent(widget) == data)
    {
        *((GtkWidget **)data) = widget;
        return TRUE;
    }
    return FALSE;
}

// Callback function for the 'Quit' button click event
void on_quit_button_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

// Create a combo box with specified options and add it to the given container
GtkWidget *create_combo_box_with_options(GtkWidget *container, const char *label_text, int num_options, ...)
{
    // Create a combo box text
    GtkWidget *combo_box = gtk_combo_box_text_new();

    // Create a label for the combo box
    GtkWidget *label = gtk_label_new(label_text);

    // Create a horizontal box to hold the label and combo box
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), combo_box, FALSE, FALSE, 0);

    // Add the horizontal box to the container
    gtk_box_pack_start(GTK_BOX(container), hbox, FALSE, FALSE, 0);

    // Initialize variable arguments
    va_list args;
    va_start(args, num_options);

    // Populate the combo box with options
    for (int i = 0; i < num_options; ++i)
    {
        const char *option = va_arg(args, const char *);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), option);
    }

    // End variable arguments
    va_end(args);

    // Show the widgets
    gtk_widget_show_all(hbox);

    return combo_box;
}

int main(int argc, char *argv[])
{
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Client");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a vertical box container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create a combo box for trigger options
    GtkWidget *trigger_combo_box = create_combo_box_with_options(vbox, "Trigger", 4, "GA", "G1", "V", "A");

    // Create a combo box for table name options
    GtkWidget *table_name_combo_box = create_combo_box_with_options(vbox, "Table Name", 7, "User", "Educator", "Student", "Subject", "Class", "Department");

    // Create an entry for additional data
    GtkWidget *additional_data_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), additional_data_entry, FALSE, FALSE, 0);

    // Create 'Send' button
    GtkWidget *send_button = gtk_button_new_with_label("Send");
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked), vbox);
    gtk_box_pack_start(GTK_BOX(vbox), send_button, FALSE, FALSE, 0);

    // Create 'Quit' button
    GtkWidget *quit_button = gtk_button_new_with_label("Quit");
    g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quit_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), quit_button, FALSE, FALSE, 0);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}