# Group - 2 B+ tree Implementation 

In this repository, we implemented a **B+ tree Implementation**, which provides index-based access to records using B+ tree data structures. It includes functionalities such as index creation, insertion, deletion, searching keys, scanning, and retrieving metadata.

---

## 📽️ Implementation Video Link

Walkthrough of our B+ tree Implementation for Assignment 4. This includes code structure, logic explanation, test execution, and design decisions.

**[Watch the video here](https://drive.google.com/file/d/1cM_-Injc8wC7fRV5gSzqUyJTqte2ZvMv/view?usp=sharing)**

---

## 👥 Contributions

| CWID       | Name                                                                 | Contribution                                                                                  | Percentage |
|------------|----------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|------------|
| A20580674  | [Atharv Patil](https://github.com/AtharvPat)                         | `initIndexManager`, `shutdownIndexManager`, `findKey`, `insertKey`, `closeTreeScan`, `printTree`, Made Implementation Video | 33.33%     |
| A25078342  | [Avneet Singh](https://github.com/asingh180)                         | `createBtree`, `getNumEntries`, `deleteKey`, `getKeyType`, `openTreeScan`, `nextEntry`, Made Index Structure Diagram | 33.33%     |
| A20588887  | [Manthan Surjuse](https://github.com/Manthan0120)                    | `openBtree`, `deleteBtree`, `closeBtree`, `getNumNodes`, `openTreeScan`, Made README.md file | 33.33%     |

---

## 📂 Files Overview

| File                 | Description                                                                 |
|----------------------|-----------------------------------------------------------------------------|
| `btree_mgr.c/h`      | Core B+ Tree logic and API declarations.                                    |
| `btree_helper.h`     | Helper functions and internal structures for B+ Tree operations.            |
| `buffer_mgr.c/h`     | Buffer Manager to manage pages in memory.                                   |
| `buffer_mgr_stat.c/h`| Provides statistics and debugging support for the Buffer Manager.           |
| `record_mgr.h`       | Interface for Record Manager integration.                                   |
| `dberror.c/h`        | Error codes and logging functionality.                                      |
| `expr.c/h`           | Expression parsing used during scans and conditions.                        |x
| `storage_mgr.c/h`    | Manages low-level file I/O for storage.                                     |
| `test_assign4_1.c`   | Main test file for validating B+ Tree functionality.                        |
| `extra_tests.c`      | Additional test cases (optional/bonus).                                     |
| `test_helper.h`      | Helper macros for assertions and test messages.                             |

---

## Function Hierarchy


## Environment versions 

### Example for one of our environment versions

```shell 
make --version
```
GNU Make 3.81  
built for i386-apple-darwin11.3.0  
Copyright (C) 2006  Free Software Foundation, Inc.  
This is free software; see the source for copying conditions.  
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  


```shell 
% gcc ---version 
```
Apple clang version 16.0.  
Target: arm64-apple-darwin24.1.0  
Thread model: posix  
InstalledDir: /Library/Developer/CommandLineTools/usr/bin
atharvpat@Atharvs-MacBook-Pro ~ %  


## ⚙️ How to Run the Code

```bash
# Step 1: Clone the repository
git clone <repo_url>

# Step 2: Navigate to the project directory
cd assign4_btree_index_manager

# Step 3: Clean old build files
make clean

# Step 4: Compile the code
make

# Step 5: Run the test file
./test_assign4_1

```




## Implementation of the Record Manager  

Description for all the functions in `btree_mgr.h` are as follows:  


### 🔧 Index Manager Operations

- `initIndexManager`: Initializes the B+ Tree Index Manager.
- `shutdownIndexManager`: Cleans up and shuts down the Index Manager.

### 🌲 B+ Tree Management

- `createBtree`: Creates a B+ tree index.
- `openBtree`: Opens an existing index.
- `closeBtree`: Closes the index and deallocates memory.
- `deleteBtree`: Deletes the B+ tree index from disk.

### 📊 Metadata and Stats

- `getNumNodes`: Gets the total number of nodes in the B+ tree.
- `getNumEntries`: Gets the total number of entries.
- `getKeyType`: Returns the data type of the B+ tree’s keys.

### 🔍 Index Operations

- `findKey`: Finds a key and returns the corresponding `RID`.
- `insertKey`: Inserts a key-RID pair.
- `deleteKey`: Deletes a key.

### 🔁 Index Scanning

- `openTreeScan`: Starts an index scan.
- `nextEntry`: Retrieves the next entry in the scan.
- `closeTreeScan`: Ends the scan.

### 🧪 Debug & Visualization

- `printTree`: Returns a string-based visual of the tree.

---

## ⚙️ How to Run the Code

```bash
# Step 1: Clone the repository
git clone <repo_url>

# Step 2: Navigate to the project directory
cd assign4_btree_index_manager

# Step 3: Clean old build files
make clean

# Step 4: Compile the code
make

# Step 5: Run the test file
./test_assign4_1