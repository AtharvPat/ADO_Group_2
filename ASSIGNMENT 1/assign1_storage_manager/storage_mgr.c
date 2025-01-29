#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_mgr.h"
#include "dberror.h"



/* ***************************************************************************************** */

RC CHECK_FILE(char *File_Name)
{
    // Checking if the file Exists
    if (File_Name== NULL) 
    {
    return RC_FILE_NOT_FOUND; // Returing Error Code
    }
    return RC_OK;
}

RC CHECK_FILE_HANDLE(SM_FileHandle *File_Handle)
{
    if (File_Handle == NULL) {
    return RC_FILE_HANDLE_NOT_INIT; // Returing Error Code
  }
  return RC_OK;
}

RC CHECK_MEMORY_PAGE(SM_PageHandle Memory_Page)
{
      if (Memory_Page == NULL) {
    return RC_WRITE_FAILED;
  }
  return RC_OK;
}

/* ***************************************************************************************** */


// Initialization Function of the Storage Manager 
void initStorageManager(void)
{
    // Message indicating the starting of the storage manager 
    printf("Storage Manager Starting...\n");
}

// Creation of a new file Function
RC createPageFile( char *File_Name)
{
    // Checking if the file already exists
    CHECK_FILE(File_Name);
    
// Checking if the File already exists
FILE *file_position = fopen(File_Name,"w+");
if(file_position == NULL)
    {
        return RC_FILE_NOT_FOUND; // Returning Error Code
    }

// Allocate memory for the Empty Page of size = PAGE_SIZE and initialize it with zeros (0)
char *EmptyPage = (char *) calloc(PAGE_SIZE, sizeof(char));
fwrite(EmptyPage, sizeof(char), PAGE_SIZE, file_position); 

//Close file and Free allocated Memory 
fclose(file_position);
free(EmptyPage);
return RC_OK; // Return success Code
}


/* ***************************************************************************************** */
// Function to Open a file
RC openPageFile(char *File_Name, SM_FileHandle *File_Handle){

// Checking if the file exists
CHECK_FILE(File_Name);

// Checking if the file handle is NULL
CHECK_FILE_HANDLE(File_Handle);


// Open File in read and write mode
FILE *file_position = fopen(File_Name,"r+");
if (file_position == NULL)
{
  return RC_FILE_NOT_FOUND; // Return Error Code
}

// Finiding out end of the file and moving the file pointer there to calculate the size of the file 
if (fseek(file_position, 0, SEEK_END) != 0) 
{
    return RC_READ_NON_EXISTING_PAGE; // retrun error if the file_pointer cannot move 
}

// Store file pointer in file handle for future use
File_Handle->mgmtInfo = file_position;
File_Handle->File_Name = File_Name;
File_Handle->curPagePos = 0;

  // Count the number of Pages
long fileSize = ftell(file_position);
File_Handle->totalNumPages = (int) (fileSize / PAGE_SIZE);
return RC_OK;
}

/* ***************************************************************************************** */

RC closePageFile(SM_FileHandle *File_Handle) {
  // check file handle
  CHECK_FILE_HANDLE(File_Handle);

  // retrieve file pointer
  FILE *file_position = File_Handle->mgmtInfo;
  fclose(file_position);
  file_position = NULL;
  return RC_OK;
}

/* ***************************************************************************************** */
RC destroyPageFile(char *File_Name) {
    // Validate the file name using the CHECK_FILE helper function
    CHECK_FILE(File_Name);

    // Attempt to remove the file
    if (remove(File_Name) != 0) {
        return RC_FILE_NOT_FOUND; // Return error code if file removal fails
    }

    return RC_OK; // Return success code if file removal is successful
}

/* ***************************************************************************************** */

RC readBlock(int PG_Num, SM_FileHandle *File_Handle, SM_PageHandle Memory_Page) {

// Checking if the file handle is Vaid 
CHECK_FILE_HANDLE(File_Handle);

// Check if the Memory pointer is not NUll 
CHECK_MEMORY_PAGE(Memory_Page);

// Check if the requested file is in the bounds
 if (PG_Num >= File_Handle->totalNumPages || PG_Num < 0 ) {
    return RC_READ_NON_EXISTING_PAGE;
  }

// Get file pointer from the file handle
FILE *file_position = File_Handle->mgmtInfo;
if (file_position == NULL)
{
  return RC_FILE_NOT_FOUND; // Return Error Code
}
// Calculate byte offset of the Page
 int byteOffset = PG_Num * PAGE_SIZE;
 if (fseek(file_position, byteOffset, SEEK_SET) != 0) {
 return RC_READ_NON_EXISTING_PAGE;
}

// Read the Pages in buffer memory
  if (fread(Memory_Page, sizeof(char), PAGE_SIZE, file_position) < PAGE_SIZE) {
    return RC_READ_NON_EXISTING_PAGE;
  }
  File_Handle->curPagePos = PG_Num;
  return RC_OK;   // Return Sucess code 

}

/* ***************************************************************************************** */

int getBlockPos(SM_FileHandle *File_Handle) {
  // check parameters
  return (File_Handle == NULL) ? -1 : File_Handle->curPagePos;
}

/* ***************************************************************************************** */

RC readFirstBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page) {
  return readBlock(0, File_Handle, Memory_Page);
} 

/* ***************************************************************************************** */

RC readPreviousBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page) {
  // if the value of position is -1, errors happened in this process
  int PG_Num = getBlockPos(File_Handle);
  if (PG_Num == -1) {
    return RC_FILE_HANDLE_NOT_INIT;
  }

  // the previous block should be PG_Num - 1
  return readBlock(PG_Num - 1, File_Handle, Memory_Page);
}

/* ***************************************************************************************** */

RC readCurrentBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page) {

// get the current page position using the getBlockPos() function
  int PG_Num = getBlockPos(File_Handle);

  //If the the retrived page number is -1 then return an error
  if (PG_Num== -1) {
    return RC_FILE_HANDLE_NOT_INIT; // Return an error code
  }

  // Read the current page into the memory using the readBlock() function
  return readBlock(PG_Num, File_Handle, Memory_Page);

}

/* ***************************************************************************************** */

RC readNextBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page) {
  // errors happen if PG_Num is not positive
  int PG_Num = getBlockPos(File_Handle);
  if (PG_Num == -1) {
    return RC_FILE_HANDLE_NOT_INIT;
  }

  // the next block should be PG_Num + 1
  return readBlock(PG_Num + 1, File_Handle, Memory_Page);
}

/* ***************************************************************************************** */

RC readLastBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page) {
    // Validate the file handle using the CHECK_FILE_HANDLE helper function
    CHECK_FILE_HANDLE(File_Handle);

    // Validate the memory page using the CHECK_MEMORY_PAGE helper function
    CHECK_MEMORY_PAGE(Memory_Page);

    // Calculate the last block page number
    int lastBlockPG_Num = File_Handle->totalNumPages - 1;

    // Read the last block
    return readBlock(lastBlockPG_Num, File_Handle, Memory_Page);
}

/* ***************************************************************************************** */

RC writeBlock(int PG_Num, SM_FileHandle *File_Handle, SM_PageHandle Memory_Page) {

// Checking if the file handle is Vaid 
CHECK_FILE_HANDLE(File_Handle);

// Check if the Memory pointer is not NUll 
CHECK_MEMORY_PAGE(Memory_Page);

// Check if the requested file is in the bounds
if (PG_Num < 0 || PG_Num >= File_Handle->totalNumPages) {
    return RC_READ_NON_EXISTING_PAGE;
}

// Get the file pointer from the File Handle 
  FILE *file_position = File_Handle->mgmtInfo;
  if (file_position == NULL) {
    return RC_FILE_NOT_FOUND;
  }

// Calculate byte offset of the Page
int ByteOffSet = PG_Num * PAGE_SIZE;

// Move the file pointer to the desired location
  if (fseek(file_position, ByteOffSet, SEEK_SET) != 0) {
    return RC_READ_NON_EXISTING_PAGE;
  }

// Write the data from the memory into the file
  if (fwrite(Memory_Page, sizeof(char), strlen(Memory_Page), file_position) < PAGE_SIZE) {

    return RC_WRITE_FAILED; // Return Faile Code if Failed 
  }
  File_Handle->curPagePos = PG_Num;
  return RC_OK;  // Return Sucess Code

}

/* ***************************************************************************************** */

RC writeCurrentBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page) {
    // Validate the file handle using the CHECK_FILE_HANDLE helper function
    CHECK_FILE_HANDLE(File_Handle);

    // Validate the memory page using the CHECK_MEMORY_PAGE helper function
    CHECK_MEMORY_PAGE(Memory_Page);

    // Write the block using the current page number
    int curPagePos = File_Handle->curPagePos;
    return writeBlock(curPagePos, File_Handle, Memory_Page);
}

/* ***************************************************************************************** */


RC appendEmptyBlock(SM_FileHandle *File_Handle) {
  // check file handle
  CHECK_FILE_HANDLE(File_Handle);

  // this is to get non-empty file
  FILE *file_position = File_Handle->mgmtInfo;
  if (file_position == NULL) {
    return RC_FILE_NOT_FOUND;
  }

  // this will move pointer to the end
  if (fseek(file_position, 0, SEEK_END) != 0) {
    return RC_READ_NON_EXISTING_PAGE;
  }

  // allocate page size and assign value to string, return rc write failed
  char *temp= (char *) calloc(PAGE_SIZE, sizeof(char)); // Allocate on the stack and initialize to zero
  if (fwrite(temp, sizeof(char), PAGE_SIZE, file_position) < PAGE_SIZE) {
    return RC_WRITE_FAILED;
  }

  // add 1 to total pages and free the memory
  File_Handle->totalNumPages = File_Handle->totalNumPages + 1;
  free(temp);
  return RC_OK;
}

/* ***************************************************************************************** */

RC ensureCapacity(int numOf_PG, SM_FileHandle *File_Handle) {
    // Validate the file handle using the CHECK_FILE_HANDLE helper function
    CHECK_FILE_HANDLE(File_Handle);

    // Validate the number of pages (ensure it is at least 1)
    if (numOf_PG < 1) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    // Append remaining blocks if the current number of pages is less than required
    int current_NumberOf_PG = File_Handle->totalNumPages;
    int count = numOf_PG - current_NumberOf_PG;

    int i = 0; // Initialize the counter
    while (i < count) {
    appendEmptyBlock(File_Handle);
    i++; // Increment the counter
    }

    // Verify if the total number of pages matches the required number
    if (File_Handle->totalNumPages != numOf_PG) {
        return RC_WRITE_FAILED;
    }

    return RC_OK;
}
