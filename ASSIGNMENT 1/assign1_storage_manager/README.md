# Group #2 - Storage Manager Assignment_1

In this Repository we wroked with a Storage Manager. The Storage Manager is responsible for Reading blocks form a file into the memory and writing blocks from the momory to the file. The Storage Manager is also responsible for managing the free space in the file. All the functions are stated in `storage_mgr.h`

## Files 

### File | Description
-----|-------------
`storage_mgr.*` | This file contains the functions for managing the Storage Manager.
`dberror.*` | This file contains all possible errors and Keeps tracks of error.
`test_assign1.c` | This file contains the main function for testing the Storage Manager.
`test_helper.h` | This file contains helper functions for testing the Storage Manager.

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
Step -5 : Run the command `./test_assign1` to run the test.  

## Implementation of the Storage Manager

Description for all the Functions in `storage_mgr.h` are as follows:

### 1. Manipulating Page File 

#### 1.1 `Create Page file`
- This function creates a new page file with the given name.
  
#### 1.2 `Destroy Page file`
- This function destroys the page file with the given name.

#### 1.3 `Open Page`
- This function opens the page file with the given name.

#### 1.4 `Close Page`
- The `closePageFile` function is used to close a file that was previously opened using the `openPageFile` function.

### 2. Reading Blocks from the Disk 

#### 2.1 `Read Block`
- The `readBlock` function is used to read a block from the disk.

#### 2.2 `Read First Block`
- The `readFirstBlock` function is used to read the first block from the disk.

#### 2.3 `Read Previous Block`
- The `readPreviousBlock` function is used to read the previous block from the disk.

#### 2.4 `Read Current Block`
- The `readCurrentBlock` function is used to read the current block from the disk.

#### 2.5 `Read Next Block`
- The `readNextBlock` function is used to read the next block from the disk.


