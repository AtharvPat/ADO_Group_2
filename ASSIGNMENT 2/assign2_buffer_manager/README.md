# Group - 2  Buffer Pool Manager Assignment

In this Repository, we worked with a Buffer Pool Manager. The Buffer Pool Manager is responsible for managing pages in memory using caching mechanisms like FIFO and LRU. It handles reading and writing pages from and to disk while ensuring efficient memory usage.


## Implementation Video link 

Basic walkthrough of our Buffer Manager implementation for Assignment 2. We’ve looked at the code, the file structure, built the project, and ran the tests.   

https://drive.google.com/file/d/1HJOY1WxQtkWdD2q-S0VvMUmQxMgWrodI/view?usp=sharing
## Contributions

`CWID` | `Name` | `Contribution` | `Contribution Percentage`
-----|------|--------------|----------------------
A20580674 |[Atharv Patil](https://github.com/AtharvPat) | Worked on Buffer Pool Operations and Utility Functions, Made Implementation Video| `33.33%`
A25078342 |[Avneet Singh](https://github.com/asingh180) | Worked on Cache Management, Made File Hierarchy Diagram | `33.33%`
A20588887 |[Manthan Surjuse](https://github.com/Manthan0120) | Worked on Page and Frame Management and Performance Tracking, Made README.md file | `33.33%`

## Files

# ![image](https://github.com/AtharvPat/ADO_Group_2/blob/main/ASSIGNMENT%202/assign2_buffer_manager/Images/Files.png)


File | Description
-----|------------- 
`buffer_mgr.*` | This file contains the functions for managing the Buffer Pool Manager.
`dberror.*` | This file contains all possible errors and Keeps tracks of error.
`test_assign2_1.c` | This file contains the main function for testing the Buffer Manager.
`test_helper.h` | This file contains helper functions and assertion tools for testing.

## Function Hierarchy

# ![image](https://github.com/AtharvPat/ADO_Group_2/blob/main/ASSIGNMENT%202/assign2_buffer_manager/Images/diagram.png)

# ![image](https://github.com/AtharvPat/ADO_Group_2/blob/main/ASSIGNMENT%202/assign2_buffer_manager/Images/Buffer_mgr.png)


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
Step -5 : Run the command `./test_assign2_1` to run the test.  

## Implementation of the Storage Manager

Description for all the Functions in `buffer_mgr.h` are as follows:

### 1. Buffer Pool Operations

* #### 1.1 bufferPool: Initializes the buffer pool with a specified number of frames.

* #### 1.2 shutdownBufferPool: Shuts down the buffer pool, writing all dirty pages to disk.

* #### 1.3 forceFlushPool: Forces all dirty pages to be written to disk.

* #### 1.4 pinPage: Pins a page in the buffer pool.

* #### 1.5 forcePage: Writes a page in the buffer pool to disk.

### 2. Cache Management

* #### 2.1 isHitPageCache: Checks if a page is in the cache.

* #### 2.2 updateLRUOrder: Updates the page order using LRU policy.

* #### 2.3 addPageToPageCacheWithFIFO: Adds a page using FIFO policy.

* #### 2.4 addPageToPageCacheWithLRU: Adds a page using LRU policy.

* #### 2.5 searchPageFromCache: Searches and returns a page's frame number.

* #### 2.6 removePageWithFIFO: Removes a page using FIFO policy.

* #### 2.7 removePageWithLRU: Removes a page using LRU policy.

* #### 2.8 isFull: Checks if the cache is full.

* #### 2.9 isFree: Checks if free frames are available.

### 3. Page and Frame Management

* #### 3.1 createFrameNode: Creates a new frame node.

* #### 3.2 resetFrameNode: Resets frame node contents.

* #### 3.3 createPageCache: Initializes the page cache.

* #### 3.4 freeFrame: Releases a frame.

* #### 3.5 freeFileHandle: Releases the file handle.

* #### 3.6 freeHash: Frees the hash table.

* #### 3.7 freePageCache: Releases page cache memory.

### 4. Metadata and Performance Tracking

* #### 4.1 getFrameContents: Returns all frame contents.

* #### 4.2 getDirtyFlags: Returns flags indicating modified pages.

* #### 4.3 getFixCounts: Returns counts of pinned pages.

* #### 4.4 getNumReadIO: Returns total read I/O operations.

* #### 4.5 getNumWriteIO: Returns total write I/O operations.

### 5. Utility Functions

* #### 5.1 markDirty: Marks a page as modified.

* #### 5.2 uniPage: Ensures a page is unique.

* #### 5.3 createhash: Creates a hash table for page lookup.

## Testing Results

1.  We executed the `test_assign_2_1.c` and all testcase passed. (I just put
    part of result from screenshot)

    ```
    [test_assign2_1.c-Testing FIFO page replacement-L194-13:45:45] OK: expected <
    [test_assign2_1.c-Testing LRU page replacement-L279-13:45:45] OK: expected <[0 0],[1 0],[2 0],[3 0],[4 0]> and was <[0 0],[1 0],[2 0],[3 0],[4 0]>: check pool content using pages
    [test_assign2_1.c-Testing LRU page replacement-L279-13:45:45] OK: expected <[0 0],[1 0],[2 0],[3 0],[4 0]> and was <[0 0],[1 0],[2 0],[3 0],[4 0]>: check pool content using pages
    [test_assign2_1.c-Testing LRU page replacement-L279-13:45:45] OK: expected <[0 0],[1 0],[2 0],[3 0],[4 0]> and was <[0 0],[1 0],[2 0],[3 0],[4 0]>: check pool content using pages
    [test_assign2_1.c-Testing LRU page replacement-L288-13:45:45] OK: expected <[0 0],[1 0],[2 0],[5 0],[4 0]> and was <[0 0],[1 0],[2 0],[5 0],[4 0]>: check pool content using pages
    [test_assign2_1.c-Testing LRU page replacement-L288-13:45:45] OK: expected <[0 0],[1 0],[2 0],[5 0],[6 0]> and was <[0 0],[1 0],[2 0],[5 0],[6 0]>: check pool content using pages
    [test_assign2_1.c-Testing LRU page replacement-L288-13:45:45] OK: expected <[7 0],[1 0],[2 0],[5 0],[6 0]> and was <[7 0],[1 0],[2 0],[5 0],[6 0]>: check pool content using pages
    [test_assign2_1.c-Testing LRU page replacement-L288-13:45:45] OK: expected <[7 0],[1 0],[8 0],[5 0],[6 0]> and was <[7 0],[1 0],[8 0],[5 0],[6 0]>: check pool content using pages
    [test_assign2_1.c-Testing LRU page replacement-L288-13:45:45] OK: expected <[7 0],[9 0],[8 0],[5 0],[6 0]> and was <[7 0],[9 0],[8 0],[5 0],[6 0]>: check pool content using pages
    [test_assign2_1.c-Testing LRU page replacement-L293-13:45:45] OK: expected <0> and was <0>: check number of write I/Os
    [test_assign2_1.c-Testing LRU page replacement-L294-13:45:45] OK: expected <10> and was <10>: check number of read I/Os
    [test_assign2_1.c-Testing LRU page replacement-L301-13:45:45] OK: finished test
 
    ```
