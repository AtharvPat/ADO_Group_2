#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_mgr.h"
#include "dberror.h"



/* ***************************************************************************************** */

void CHECK_FILE(char *File_Name)
{
    // Checking if the file Exists
    if (File_Name== NULL) 
    {
    return RC_FILE_NOT_FOUND; // Returing Error Code
    }
}

void CHECK_FILE_HANDLE(SM_FileHandle *File_Handel)
{
    if (File_Handle == NULL) {
    return RC_FILE_HANDLE_NOT_INIT; // Returing Error Code
  }
}

void CHECK_MEMORY_PAGE(SM_PageHandle Memory_Page)
{
      f (Memeory_Page == NULL) {
    return RC_WRITE_FAILED;
  }
}

/* ***************************************************************************************** */


// Initialization Function of the Storage Manager 
void initStorageMgr(void)
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
RC openPageFile(char *File_Name, SM_FileHandle *File_Handel){

// Checking if the file exists
CHECK_FILE(File_Name);

// Checking if the file handel is NULL
CHECK_FILE_HANDLE(File_Handel);


// Open File in read and write mode
FILE *file_position = fopen(File_Name,"r+");
CHECK_FILE(file_position);

// Finiding out end of the file and moving the file pointer there to calculate the size of the file 
if (fseek(file_position, 0, SEEK_END) != 0) 
{
    return RC_READ_NON_EXISTING_PAGE; // retrun error if the file_pointer cannot move 
}

// Store file pointer in file handel for future use
File_Handel->mgmtInfo = file_position;
File_Handel->fileName = File_Name;
File_Handel->curPagePos = 0;

  // Count the number of Pages
long fileSize = ftell(file_position);
File_Handel->totalNumPages = (int) (fileSize / PAGE_SIZE);
return RC_OK;
}

/* ***************************************************************************************** */

RC readBlock(int PG_Num, SM_FileHandle *File_Handle, SM_PageHandle Memory_Page) {

// Checking if the file handel is Vaid 
CHECK_FILE_HANDLE(File_Handle);

// Check if the Memory pointer is not NUll 
CHECK_MEMORY_PAGE(Memory_Page);

// Check if the requested file is in the bounds
 if (PG_Num >= File_Handle->totalNumPages || PG_Num < 0 ) {
    return RC_READ_NON_EXISTING_PAGE;
  }

// Get file pointer from the file handel
FILE *file_position = File_Handle->mgmtInfo;
CHECK_FILE(file_position);

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

RC readCurrentBlock(SM_FileHandle *File_Handle, SM_PageHandle Memory_Page) {

// get the current page position using the getBlockPos() function
  int PG_Num = getBlockPos(fHandle);

  //If the the retrived page number is -1 then return an error
  if (PG_Num== -1) {
    return RC_FILE_HANDLE_NOT_INIT; // Return an error code
  }

  // Read the current page into the memory using the readBlock() function
  return readBlock(PG_Num, File_Handle, Memory_Page);

}

/* ***************************************************************************************** */


RC writeBlock(int PG_Num, SM_FileHandle *File_Handle, SM_PageHandle Memeory_Page) {

// Checking if the file handel is Vaid 
CHECK_FileHandel(File_Handle);

// Check if the Memory pointer is not NUll 
CHECK_MEMORY_PAGE(Memeory_Page);

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
  if (fwrite(Memory_Page, sizeof(char), strlen(memPage), file_position) < PAGE_SIZE) {
    
    return RC_WRITE_FAILED; // Return Faile Code if Failed 
  }
  File_Handle->curPagePos = Page_Num;
  return RC_OK;  // Return Sucess Code

}

/* ***************************************************************************************** */
