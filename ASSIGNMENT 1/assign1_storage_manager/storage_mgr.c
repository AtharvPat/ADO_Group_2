#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"

/* ***************************************************************************************** */

RC CHECK_FILE(char *File_Name)
{
  // Checking if the file Exists
  if (File_Name == NULL)
  {
    return RC_FILE_NOT_FOUND; // Returing Error Code
  }
  return RC_OK;
}

RC CHECK_FILE_HANDLE(SM_FileHandle *File_Handle)
{
  if (File_Handle == NULL)
  {
    return RC_FILE_HANDLE_NOT_INIT; // Returing Error Code
  }
  return RC_OK;
}

RC CHECK_MEMORY_PAGE(SM_PageHandle Memory_Page)
{
  if (Memory_Page == NULL)
  {
    return RC_WRITE_FAILED;
  }
  return RC_OK;
}

/* ***************************************************************************************** changed  */

// Initialization Function of the Storage Manager
void initStorageManager(void)
{
  // Message indicating the starting of the storage manager
  printf("Storage Manager Starting...\n");
}


/* ***************************************************************************************** Changed  */

// Creation of a new file Function
RC createPageFile(char *File_Name)
{
  // Checking if the file already exists
  CHECK_FILE(File_Name);

  // Creating a new file
  int file_postiton = open(File_Name, O_RDWR | O_CREAT| O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR) ;
  if (file_postiton < 0)
  {
    return RC_FILE_NOT_FOUND; // Returning Error Code
  }
  // making a new empty page and initializing it with zeros
  char Empty_Page[PAGE_SIZE] ={0};

   // writing the Emty pqge to file 
  if (write(file_postiton, Empty_Page, PAGE_SIZE)!= PAGE_SIZE)
  {
    close(file_postiton);
    return RC_WRITE_FAILED; // Returning Error Code
  }
  //closing the file
  close(file_postiton);
  return RC_OK;
}

/* ***************************************************************************************** */
RC destroyPageFile(char *File_Name)
{
  // Validate the file name using the CHECK_FILE helper function
  CHECK_FILE(File_Name);

  // Attempt to remove the file
  if (remove(File_Name) != 0)
  {
    return RC_FILE_NOT_FOUND; // Return error code if file removal fails
  }

  return RC_OK; // Return success code if file removal is successful
}



/* ***************************************************************************************** Chnaged */
// Function to Open a file
RC openPageFile(char *File_Name, SM_FileHandle *File_Handle)
{

  // Checking if the file exists
  CHECK_FILE(File_Name);

  // Checking if the file handle is NULL
  CHECK_FILE_HANDLE(File_Handle);

  // Check if the file exist
  FILE *file_position = NULL; 
  file_position = fopen(File_Name, "r+");

  if (file_position == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }

  // get file size by traversing the file_pointer to the end of the file
  int Seek_res = fseek(file_position, 0 , SEEK_END);
  if (Seek_res != 0)
  {
    fclose(file_position);
    return RC_READ_NON_EXISTING_PAGE;
  }

  // Get the file size
  long fileSize = ftell(file_position);
  if (fileSize == -1)
  {
    fclose(file_position);
    return RC_READ_NON_EXISTING_PAGE;
  }

  // Store file pointer in the file handle
  File_Handle->curPagePos = 0;

  File_Handle->File_Name = File_Name;
  File_Handle->mgmtInfo = file_position;

  // get total number of pages
  File_Handle->totalNumPages = (fileSize > 0) ? (int)(fileSize / PAGE_SIZE) : 0;

  return RC_OK; // Return success code
}

/* ***************************************************************************************** */

RC closePageFile(SM_FileHandle *File_Handle)
{
  // check file handle
  CHECK_FILE_HANDLE(File_Handle);

  // retrieve file pointer
  FILE *file_position = File_Handle->mgmtInfo;
  fclose(file_position);
  file_position = NULL;
  return RC_OK;
}

/* *****************************************************************************************  Changed */

RC readBlock(int PG_Num, SM_FileHandle *File_Handle, SM_PageHandle Memory_Page)
{

  // Checking if the file handle is Vaid
  CHECK_FILE_HANDLE(File_Handle);

  // Check if the Memory pointer is not NUll
  CHECK_MEMORY_PAGE(Memory_Page);

  // Check if the requested file is in the bounds
  if (PG_Num >= File_Handle->totalNumPages || PG_Num < 0)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }

  // Get file pointer from the file handle
  FILE* file_position = File_Handle->mgmtInfo;
  if(file_position < 0)
  {
    return RC_FILE_NOT_FOUND;
  }

  // get the ByteOffset of the page 
  long ByteOffset = PG_Num * PAGE_SIZE;

  // Read the page into the buffer memory using pread (thread safety)
  ssize_t BytesRead = pread(fileno(file_position), Memory_Page, PAGE_SIZE, ByteOffset);
  if(BytesRead < PAGE_SIZE)
  {
    return RC_READ_NON_EXISTING_PAGE; 
  }

  // update the current page in the file Handle
  File_Handle->curPagePos = PG_Num;

  return RC_OK; // Success Code

}


/* ***************************************************************************************** Changed */



RC writeBlock(int PG_Num, SM_FileHandle *File_Handle, SM_PageHandle Memory_Page)
{

  // Checking if the file handle is Vaid
  CHECK_FILE_HANDLE(File_Handle);

  // Check if the Memory pointer is not NUll
  CHECK_MEMORY_PAGE(Memory_Page);

  // Check if the requested file is in the bounds
  if (PG_Num < 0 || PG_Num >= File_Handle->totalNumPages)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }
  // assign File Position from File_Handle
  int File_position = fileno((FILE *)File_Handle->mgmtInfo);
  if (File_position < 0)
  {
    return RC_FILE_NOT_FOUND;
  }

  // Find Byte Offset of the Page
  off_t ByteOffset = PG_Num * PAGE_SIZE;

  // Write memory page to the file
  ssize_t bytesWritten = pwrite(File_position, Memory_Page, PAGE_SIZE, ByteOffset);
  if (bytesWritten < PAGE_SIZE)
  {
    return RC_WRITE_FAILED;
  }

  File_Handle->curPagePos = PG_Num;
  return RC_OK; // Return Success Code
}

/* ***************************************************************************************** change this */

RC appendEmptyBlock(SM_FileHandle *File_Handle)
{

      // Check if the file handle is valid
    if (File_Handle == NULL || File_Handle->mgmtInfo == NULL) {
        return RC_FILE_NOT_FOUND;  // Return error if the file handle or file pointer is invalid
    }

    // Get the file pointer from the file handle
    FILE *file_position = File_Handle->mgmtInfo;

    // Move the file pointer to the end of the file
    if (fseek(file_position, 0, SEEK_END) != 0) {
        return RC_READ_NON_EXISTING_PAGE;  // Return error if unable to seek to the end of the file
    }

    // Allocate memory for an empty page (initialized to zero)
    char *emptyPage = (char *)calloc(PAGE_SIZE, sizeof(char));  // Memory is zero-initialized

    if (emptyPage == NULL) {
        return RC_READ_NON_EXISTING_PAGE;  // Return error if memory allocation fails
    }

    // Write the empty page to the file
    if (fwrite(emptyPage, sizeof(char), PAGE_SIZE, file_position) < PAGE_SIZE) {
        free(emptyPage);  // Free memory before returning
        return RC_WRITE_FAILED;  // Return error if writing the empty page fails
    }

    // Update the total number of pages in the file
    File_Handle->totalNumPages++;

    // Free allocated memory
    free(emptyPage);

    return RC_OK; 
}

/* ***************************************************************************************** Changed */

RC readPreviousBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page)
{
  // check if the File_handle and Memory_Page are LIGIT
  if(File_Handle == NULL || File_Handle->curPagePos == -1)
  {
    return RC_FILE_HANDLE_NOT_INIT;
  }

  // get the current position of the page number 
  int PG_Num = File_Handle->curPagePos;

  // Ensure the page number is ligit and is in bounds
  if(PG_Num <= 0 )
  {
    return RC_READ_NON_EXISTING_PAGE;
  }

  // read the previous block
  return readBlock(PG_Num - 1,File_Handle,Memory_Page);

}

/* *****************************************************************************************  Changed */

RC readFirstBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page)
{
  // check if the File_handle and Memory_Page are LIGIT
  if (File_Handle == NULL || Memory_Page == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }
  // read 1st block (PAGE 0)
  return readBlock(0,File_Handle,Memory_Page);
}

/* ***************************************************************************************** Changed  */



int getBlockPos(SM_FileHandle *File_Handle)
{
 // return -1 id the File_Handle is NULL 
 if (File_Handle == NULL)
 {
  return -1; 
 }

// if the file_handle is ligit return current position
 return File_Handle->curPagePos;
}

/* ***************************************************************************************** Changed  */

RC readCurrentBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page)
{
  // Check is the File_handle is LIGIT
  if (File_Handle == NULL || File_Handle-> curPagePos == -1)
  {
    return RC_FILE_HANDLE_NOT_INIT; 
  }
  // Get the current position of the page number 
  int PG_Num = File_Handle->curPagePos;

  //read the current page 
  return readBlock(PG_Num, File_Handle, Memory_Page);
}

/* *****************************************************************************************  Changed*/

RC readNextBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page)
{
  // Check is the File_handle is LIGIT
  if(File_Handle == NULL || File_Handle->curPagePos == -1)
  {
    return RC_FILE_HANDLE_NOT_INIT;
  }

  //Get the current position of the page number
  int PG_Num = File_Handle->curPagePos;

  //Ensure that the next block is in valid bounds 
  if(PG_Num+1 >= File_Handle->totalNumPages)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }

  // read the next page (PG_Num + 1)
  return readBlock(PG_Num + 1, File_Handle, Memory_Page);
}

/* *****************************************************************************************   chanage this a little but */

RC readLastBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page)
{
  // check if the File_handle and Memory_Page are LIGIT
  if(File_Handle == NULL || Memory_Page== NULL)
  {
    return RC_FILE_HANDLE_NOT_INIT;
  }

  // check that file has atleat one page 
  if(File_Handle->totalNumPages <= 0){
    return RC_READ_NON_EXISTING_PAGE;
  }

  // Get the Last block page number 
  int last_PG_Num = File_Handle->totalNumPages-1;   // plag here 

  // read the the last block 
  return readBlock(last_PG_Num, File_Handle, Memory_Page);
}

/* ***************************************************************************************** */

RC writeCurrentBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page)
{
  // Validate the file handle using the CHECK_FILE_HANDLE helper function
  CHECK_FILE_HANDLE(File_Handle);

  // Validate the memory page using the CHECK_MEMORY_PAGE helper function
  CHECK_MEMORY_PAGE(Memory_Page);

  // Write the block using the current page number
  int curPagePos = File_Handle->curPagePos;
  return writeBlock(curPagePos, File_Handle, Memory_Page);
}

/* ***************************************************************************************** */

RC ensureCapacity(int numOf_PG, SM_FileHandle *File_Handle)
{
  // Validate the file handle using the CHECK_FILE_HANDLE helper function
  CHECK_FILE_HANDLE(File_Handle);

  // Validate the number of pages (ensure it is at least 1)
  if (numOf_PG < 1)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }

  // Append remaining blocks if the current number of pages is less than required
  int current_NumberOf_PG = File_Handle->totalNumPages;
  int count = numOf_PG - current_NumberOf_PG;

  int i = 0; // Initialize the counter
  while (i < count)
  {
    appendEmptyBlock(File_Handle);
    i++; // Increment the counter
  }

  // Verify if the total number of pages matches the required number
  if (File_Handle->totalNumPages != numOf_PG)
  {
    return RC_WRITE_FAILED;
  }

  return RC_OK;
}
