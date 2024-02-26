#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mysql/mysql.h>

typedef struct
{
    int status;
    char *result;
} QueryResult;

char *get_all_data_from_table(MYSQL *conn, const char *table_name)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    char *result = malloc(1000);
    result[0] = '\0'; // initialize buffer

    // Execute the SELECT query
    char query[100];
    snprintf(query, sizeof(query), "SELECT * FROM %s", table_name);
    if (mysql_query(conn, query) != 0)
    {
        fprintf(stderr, "mysql_query failed: %s\n", mysql_error(conn));
        return NULL;
    }

    // Get the result set
    res = mysql_use_result(conn);
    if (res == NULL)
    {
        fprintf(stderr, "mysql_use_result failed\n");
        return NULL;
    }

    // Fetch and store the rows
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        for (unsigned int i = 0; i < mysql_num_fields(res); i++)
        {
            strcat(result, row[i] ? row[i] : "NULL");
            strcat(result, " ");
        }
        strcat(result, "\n");
    }

    // Free the result set
    mysql_free_result(res);
    return result;
}

char *get_one_data_from_table(MYSQL *conn, const char *table_name, int id)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    char *result = malloc(1000); // Allocate memory for result buffer
    result[0] = '\0';            // Initialize buffer

    // Execute the SELECT query
    char query[100];
    snprintf(query, sizeof(query), "SELECT * FROM %s WHERE id=%d", table_name, id);
    if (mysql_query(conn, query) != 0)
    {
        fprintf(stderr, "mysql_query failed: %s\n", mysql_error(conn));
        return NULL;
    }

    // Get the result set
    res = mysql_use_result(conn);
    if (res == NULL)
    {
        fprintf(stderr, "mysql_use_result failed\n");
        return NULL;
    }

    // Fetch and store the row
    if ((row = mysql_fetch_row(res)) != NULL)
    {
        for (unsigned int i = 0; i < mysql_num_fields(res); i++)
        {
            strcat(result, row[i] ? row[i] : "NULL");
            strcat(result, " ");
        }
        strcat(result, "\n");
    }
    else
    {
        snprintf(result, 1000, "Record with ID %d not found in table %s\n", id, table_name);
    }

    // Free the result set
    mysql_free_result(res);

    return result;
}

int add_one_data_to_table(MYSQL *conn, const char *table_name, const char *column_details[], int num_columns)
{

    // Construct the column values string
    char column_values[500] = "";
    for (int i = 0; i < num_columns; i++)
    {
        if (i > 0)
        {
            strcat(column_values, ", ");
        }
        // Check if the column value needs to be enclosed in single quotes

        if (strchr(column_details[i], '-'))
        {
            // Enclose string values in single quotes
            strcat(column_values, "'");
            strcat(column_values, column_details[i]);
            strcat(column_values, "'");
        }
        else if (isdigit(column_details[i][0]) || column_details[i][0] == '\'')
        {
            // No need to enclose numeric values, values already enclosed in single quotes, or date values
            strcat(column_values, column_details[i]);
        }
        else
        {
            // Enclose string values in single quotes
            strcat(column_values, "'");
            strcat(column_values, column_details[i]);
            strcat(column_values, "'");
        }
    }

    printf("Executing insert query: INSERT INTO %s VALUES(NULL, %s) \n", table_name, column_values);

    // Execute the INSERT query
    char query[1000];
    snprintf(query, sizeof(query), "INSERT INTO %s VALUES(NULL, %s)", table_name, column_values);

    if (mysql_query(conn, query) != 0)
    {
        fprintf(stderr, "Error executing insert query for table %s: %s\n", table_name, mysql_error(conn));
        return -1;
    }

    // Retrieve the last inserted ID
    if (mysql_query(conn, "SELECT LAST_INSERT_ID()"))
    {
        fprintf(stderr, "Error retrieving last inserted ID from table %s: %s\n", table_name, mysql_error(conn));
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL)
    {
        fprintf(stderr, "Error storing result from query for table %s: %s\n", table_name, mysql_error(conn));
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL)
    {
        fprintf(stderr, "Error fetching row from result for table %s: %s\n", table_name, mysql_error(conn));
        mysql_free_result(result);
        return -1;
    }

    printf("Last ID inserted into table %s: %s\n", table_name, row[0]);

    mysql_free_result(result);
    printf("Record added to table %s\n", table_name);
    return 1;
}

char *view_columns_of_table(MYSQL *conn, const char *table_name)
{
    // Execute the DESCRIBE query
    char query[100];
    snprintf(query, sizeof(query), "DESCRIBE %s", table_name);
    if (mysql_query(conn, query) != 0)
    {
        fprintf(stderr, "mysql_query failed: %s\n", mysql_error(conn));
        return NULL;
    }

    // Get the result set
    MYSQL_RES *res = mysql_use_result(conn);
    if (res == NULL)
    {
        fprintf(stderr, "mysql_use_result failed\n");
        return NULL;
    }

    // Fetch and store the column names
    char *result = malloc(1000); // Allocate memory for result buffer
    result[0] = '\0';
    strcat(result, "Columns of table ");
    strcat(result, table_name);
    strcat(result, ":\n");
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        strcat(result, row[0]);
        strcat(result, "\n");
    }

    // Free the result set
    mysql_free_result(res);
    return result;
}

QueryResult execute_query(MYSQL *conn, int argc, char *argv[])
{
    QueryResult result;
    result.status = 0;
    result.result = NULL;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <GETALL/GETONE/ADDONE/VIEWCOLUMNS> <table_name> <column_details>\n", argv[0]);
        result.status = -1;
        return result;
    }

    // Initialize MySQL connection
    conn = mysql_init(NULL);
    if (conn == NULL)
    {
        fprintf(stderr, "mysql_init failed\n");
        result.status = -1;
        return result;
    }

    // Connect to MySQL database
    if (mysql_real_connect(conn, "localhost", "root", "Gandapuri@123", "ncit", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "mysql_real_connect failed\n");
        mysql_close(conn);
        result.status = -1;
        return result;
    }

    printf("Connected to MySQL database 'ncit'\n");

    // Check command and execute corresponding function
    if (strcmp(argv[1], "GETALL") == 0)
    {
        result.result = get_all_data_from_table(conn, argv[2]);
        result.status = 1;
    }
    else if (strcmp(argv[1], "ADDONE") == 0)
    {
        // Extract table name and column details from command-line arguments
        const char *table_name = argv[2];
        const int num_columns = argc - 3;
        const char *column_details[num_columns];
        for (int i = 0; i < num_columns; i++)
        {
            column_details[i] = argv[i + 3];
        }

        // Example usage: Adding a record to the specified table
        int add_result = add_one_data_to_table(conn, table_name, column_details, num_columns);
        if (add_result == 1)
        {
            result.status = 1;
            result.result = strdup("Record added successfully");
        }
        else
        {
            result.status = -1;
            result.result = strdup("Failed to add record");
        }
    }
    else if (strcmp(argv[1], "VIEWCOLUMNS") == 0)
    {
        if (argc != 3)
        {
            fprintf(stderr, "Usage: %s VIEWCOLUMNS <table_name>\n", argv[0]);
            exit(1);
        }
        result.result = view_columns_of_table(conn, argv[2]);
        result.status = 1;
    }
    else if (strcmp(argv[1], "GETONE") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "Usage: %s GETONE <table_name> <id>\n", argv[0]);
            result.status = -1;
        }
        else
        {
            const char *table_name = argv[2];
            int id = atoi(argv[3]);
            result.result = get_one_data_from_table(conn, table_name, id);
            if (result.result != NULL)
            {
                result.status = 1;
            }
            else
            {
                result.status = -1;
                result.result = strdup("Failed to retrieve record");
            }
        }
    }
    else
    {
        fprintf(stderr, "Invalid command\n");
        result.status = -1;
    }

    // Close MySQL connection
    mysql_close(conn);

    return result;
}

QueryResult main(int argc, char *argv[])
{
    QueryResult queryResult = execute_query(NULL, argc, argv);

    if (queryResult.result != NULL)
    {
        printf("%s\n", queryResult.result);
        free(queryResult.result);
    }

    return queryResult;
}
