# Group - 2 Record Manager Assignment

In this Repository, we worked on a Record Manager, which is responsible for managing records in a database. It includes functionalities such as table creation, insertion, deletion, updating, and querying records.

## Contributions

`CWID` | `Name` | `Contribution` | `Contribution Percentage`
-----|------|--------------|----------------------
A20580674 |[Atharv Patil](https://github.com/AtharvPat) | initRecordManager, shutdownRecordManager, openTable, getNumTuples, getRecords, insertRecord, closeScan, freeSchema, freeRecord, getStringAttr, createPageDirectoryNode, Made Implementation Video | `33.33%`
A25078342 |[Avneet Singh](https://github.com/asingh180) | createTable, FlushDataToPage, updateRecord, getRecord, getRecordSize, *createSchema, getNumAttr, getAttr, Made File Hierarchy Diagram | `33.33%`
A20588887 |[Manthan Surjuse](https://github.com/Manthan0120) | closeTable, deleteTable, deleteRecord, startScan, next, createRecord, setAttr, intToString, Made README.md file | `33.33%`

## Files

 # ![image](https://github.com/AtharvPat/ADO_Group_2/blob/main/ASSIGNMENT%203/assign3_record_manager/Images/Screenshot%202025-03-14%20at%209.30.42%E2%80%AFPM.png)

File | Description
--- | ---
buffer_mgr.* | Manages memory page frames and page files.
buffer_mgr_stat.* | Statistic interfaces of Buffer Manager.
dt.h | Boolean constants.
__expr.*__ | Parse condition expression in the scan.
__record_mgr.*__ | Responsible for managing tables in this database.
store_mgr.* | Responsible for managing database in files and memory.
dberror.* | Keeps track and report different types of error.
__rm_serializer.*__ | Responsible for serialize and deserialize data stored in files.
__tables.h__ | Define useful data structures and functions to implement the record manager. |
__test_assign3_1.c__ | Base test cases.
__test_helper.h__ | Testing and assertion tools.
__test_expr.h__ | Testing the expression functions.


## Function Hierarchy

# ![image](https://github.com/AtharvPat/ADO_Group_2/blob/main/ASSIGNMENT%203/assign3_record_manager/Images/harierchy.png)
# ![image](https://github.com/AtharvPat/ADO_Group_2/blob/main/ASSIGNMENT%203/assign3_record_manager/Images/h.png)


## Environment versions 

### Example for one of our environment versions

```shell 
make --version
```
GNU Make 3.81
built for i386-apple-darwin11.3.0  
Copyright (C) 2006  Free Software Foundation, Inc.  
This is free software; see the source for copying conditions.  
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  


```shell 
% gcc ---version 
```
Apple clang version 16.0.  
Target: arm64-apple-darwin24.1.0  
Thread model: posix  
InstalledDir: /Library/Developer/CommandLineTools/usr/bin
atharvpat@Atharvs-MacBook-Pro ~ %   

##  How to Run the Code 

Step -1 : Clone the repository.  
Step -2 : Navigate to the Folder with all the files.  
Step -3 : Run the command `make clean` to clear old make file.   
Step -4 : Run the command `make` to compile the code.  
Step -5 : Run the command `./test_assign3_1` to run the test.  

## Implementation of the Record Manager  

Description for all the functions in `record_mgr.h` are as follows:  

### 1. Record Manager Operations  

* #### 1.1 initRecordManager: Initializes the record manager by setting up necessary data structures.  

* #### 1.2 shutdownRecordManager: Shuts down the record manager and frees allocated memory.  

### 2. Table Management  

* #### 2.1 createTable: Creates a new table in the database with a given schema.  

* #### 2.2 openTable: Opens an existing table and loads metadata.  

* #### 2.3 closeTable: Closes the table and releases associated memory.  

* #### 2.4 deleteTable: Deletes a table from the database.  

### 3. Record Management  

* #### 3.1 insertRecord: Inserts a new record into the table.  

* #### 3.2 deleteRecord: Deletes a record from the table based on a given record ID.  

* #### 3.3 updateRecord: Updates an existing record in the table.  

* #### 3.4 getRecord: Retrieves a specific record from the table using its record ID.  

* #### 3.5 getRecords: Fetches multiple records from the table.  

### 4. Scanning Records  

* #### 4.1 startScan: Initializes a scan operation over the table for record retrieval.  

* #### 4.2 next: Retrieves the next record that matches a scan condition.  

* #### 4.3 closeScan: Ends a scan operation and releases resources.  

### 5. Schema Management  

* #### 5.1 *createSchema: Creates a schema structure for a table.  

* #### 5.2 freeSchema: Frees the memory allocated for a schema.  

* #### 5.3 getNumAttr: Returns the number of attributes in the schema.  

* #### 5.4 getAttr: Retrieves an attribute value from a record based on the schema.  

* #### 5.5 setAttr: Updates an attribute value in a record.  

* #### 5.6 getStringAttr: Retrieves an attribute value as a string.  

### 6. Record Memory Management  

* #### 6.1 createRecord: Allocates and initializes a new record.  

* #### 6.2 freeRecord: Frees the memory allocated for a record.  

* #### 6.3 getRecordSize: Returns the size of a record based on its schema.  

### 7. Utility Functions  

* #### 7.1 FlushDataToPage: Writes modified records back to disk.  

* #### 7.2 createPageDirectoryNode: Creates a new node in the page directory for record management.  

* #### 7.3 intToString: Converts an integer to a string representation.  
