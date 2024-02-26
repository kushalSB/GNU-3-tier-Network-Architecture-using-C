# Two files are here

- For Modularity , two seperate files are created, change in implementation of simpleMysql file should not affect dataBaseCommand file
- simpleMysql can be replaced by other databases according to need, the underlying architecture must remain constant.

## dataBaseCommand

- dataBaseCommand (Run this for console based database operations)
- GETALL Returns all entry in table
- GETONE Returns one entry of the specified id
- VIEWCOLUMNS displays the column names of tables
- ADDONE adds one entry to the table (Do not supply id)

<p> It uses exec() to run simpleMysql as a child process and reads the STDOUT and stores it in an array. </p>

## simpleMysql

- Connection to MySQL database is established here.
- Query to MySQL is done here, results extracted and printed.
- The printed results are read by dataBaseCommand file
- Lifecycle if for one operation only.
