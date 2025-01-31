# Group - 2  Storage Manager Assignment_1

In this Repository we wroked with a Storage Manager. The Storage Manager is responsible for Reading blocks form a file into the memory and writing blocks from the momory to the file. The Storage Manager is also responsible for managing the free space in the file. All the functions are stated in `storage_mgr.h`


## Contributions

`CWID` | `Name` | `Contribution` | `Contribution Percentage`
-----|------|--------------|----------------------
A20580674 |[Atharv Patil](https://github.com/AtharvPat) | Worked on Manipulating Page File Functions | `33.33%`
A25078342 |[Avneet Singh](https://github.com/asingh180) | Worked on Reading Blocks from the Disk Functions | `33.33%`
A20588887 |[Manthan Surjuse](https://github.com/Manthan0120) | Worked on Writing Blocks to the Disk and Memory Managemnet Functions | `33.33%`


## Files 

 # ![image](https://github.com/AtharvPat/ADO_Group_2/blob/main/ASSIGNMENT%201/assign1_storage_manager/images/image.png)



File | Description
-----|------------- 
`storage_mgr.*` | This file contains the functions for managing the Storage Manager.
`dberror.*` | This file contains all possible errors and Keeps tracks of error.
`test_assign1.c` | This file contains the main function for testing the Storage Manager.
`test_helper.h` | This file contains helper functions for testing the Storage Manager.

## Function Hierarchy

# ![image](https://github.com/AtharvPat/ADO_Group_2/blob/main/ASSIGNMENT%201/assign1_storage_manager/images/image2.png)

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

### 1. Manipulating Page File Functions
    
* #### 1.1 `Create Page file`
     - This function creates a new page file with the given name.
  
- #### 1.2 `Destroy Page file`
    - This function destroys the page file with the given name.

- #### 1.3 `Open Page`
    - This function opens the page file with the given name.

- #### 1.4 `Close Page`
    - This function is used to close a file that was previously opened using the `openPageFile` function.

### 2. Reading Blocks from the Disk Functions

- #### 2.1 `Read Block`
    - This function is used to read a block from the disk.

- #### 2.2 `Read First Block`
    - This function is used to read the first block from the disk using `ReadBlock` Function.

- #### 2.3 `Read Previous Block`
    - This function is used to read the previous block from the disk using `ReadBlock` Function.

- #### 2.4 `Read Current Block`
    - This function is used to read the current block from the disk using `ReadBlock` Function.

- #### 2.5 `Read Next Block`
    - This function is used to read the next block from the disk using `ReadBlock` Function.

- #### 2.6 `Read Last Block`
    - This function is used to read the last block from the disk using `ReadBlock` Function

### 3. Writing Blocks to the Disk Functions

- #### 3.1 `Write Block`
    - This function is used to write a block to the disk.

- ####  3.2 `Write Current Block`
    - This function is used to write the current block to the disk using `WriteBlock` Function

### 4. Memory Managemnet Functions 

- #### 4.1 `ensureCapacity`
    - This function is used to ensure that the memory has enough capacity to store the given number of blocks.

- #### 4.2 `appendEmptyBlock`
    - This function is used to append an empty block to the memory.

## Extra Test Cases

We have Added 2 more New test Cases `static void testMultiplePage(void)` to test how storage Manager works when there are multiple Pages and `static void testCapacityExpansion(void)` to Test how storage Manager works when there is need of increase in the  capacity of the storage.

- ### `testMultiplePage()` 
```c
void testMultiplePage(void) {
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test multiple page";

  ph = (SM_PageHandle) calloc(PAGE_SIZE, sizeof(char));

  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));
  printf("created and opened  a file\n");

  // add new page
  TEST_CHECK(appendEmptyBlock(&fh));
  printf("Add new page with zero bytes in it \n");

  // read new page into handle
  TEST_CHECK(readNextBlock(&fh, ph));
  // the page should be empty (zero bytes)
  for (i = 0; i < PAGE_SIZE; i++) {
    ASSERT_TRUE(ph[i] == 0,
                "zero byte expected in new page of freshly initialized page");
  }
  printf("\n new block was empty\n");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));
  TEST_DONE();
}
```

 - ### `testCapacityExpansion()`
 ```c
void testCapacityExpansion(void) {
  SM_FileHandle fh;

  testName = "test capacity expansion";

  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(openPageFile(TESTPF, &fh));

  ASSERT_TRUE(fh.totalNumPages == 1, "Capacity not  expanded yet");
  TEST_CHECK(ensureCapacity(5, &fh));
  ASSERT_TRUE(fh.totalNumPages == 5, "Capacity Expanded to 5");
  TEST_CHECK(ensureCapacity(10, &fh));
  ASSERT_TRUE(fh.totalNumPages == 10, "Capacity Again Expanded to 10");

  TEST_CHECK(closePageFile(&fh));
  TEST_CHECK(destroyPageFile(TESTPF));
  TEST_DONE();
}
 ```

