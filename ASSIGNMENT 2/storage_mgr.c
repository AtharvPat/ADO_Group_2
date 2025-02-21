#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_mgr.h"
#include "dberror.h"


RC CHECK_FILE(char *File_Name)
{
    // Checking if the file Exists
    if (File_Name== NULL) 
    {
      return RC_FILE_NOT_FOUND; 
    } else {
      return RC_OK;
    }

}

RC CHECK_FILE_HANDLE(SM_FileHandle *File_Handle)
{
  if (File_Handle == NULL) {
    return RC_FILE_HANDLE_NOT_INIT; 
  } else {
    return RC_OK;
  }
}

RC CHECK_MEMORY_PAGE(SM_PageHandle Memory_Page)
{
  if (Memory_Page == NULL) {
    return RC_WRITE_FAILED;
  }
  else {
    return RC_OK;
  }
}


// The readBlock method reads the block specified by pageNum from a file and stores its contents in the memory location pointed to by the memPage page handle.
//
// If the file has less than pageNum pages, the method should return RC_READ_NON_EXISTING_PAGE.

RC closePageFile(SM_FileHandle *fHandle) {
  // check file handle
  CHECK_FILE_HANDLE(fHandle);

  // retrieve file pointer
  FILE *fp = fHandle->mgmtInfo;
  fclose(fp);
  fp = NULL;
  return RC_OK;
}

int getBlockPos(SM_FileHandle *fHandle) {
  // check parameters
  return (fHandle == NULL) ? -1 : fHandle->curPagePos;
}

RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
  return readBlock(0, fHandle, memPage);
} 


RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
  // errors happen if pageNum is not positive
  int page_number = getBlockPos(fHandle);
  if (page_number == -1) {
    return RC_FILE_HANDLE_NOT_INIT;
  }

  // the next block should be pageNum + 1
  return readBlock(page_number + 1, fHandle, memPage);
}

RC appendEmptyBlock(SM_FileHandle *fHandle) {
  // check file handle
  CHECK_FILE_HANDLE(fHandle);

  // this is to get non-empty file
  FILE *fp = fHandle->mgmtInfo;
  if (fp == NULL) {
    return RC_FILE_NOT_FOUND;
  }

  // this will move pointer to the end
  if (fseek(fp, 0, SEEK_END) != 0) {
    return RC_READ_NON_EXISTING_PAGE;
  }

  // allocate page size and assign value to string, return rc write failed
  char str[PAGE_SIZE] = {0}; // Allocate on the stack and initialize to zero
  if (fwrite(str, sizeof(char), PAGE_SIZE, fp) < PAGE_SIZE) {
    return RC_WRITE_FAILED;
  }

  // add 1 to total pages and free the memory
  fHandle->totalNumPages = fHandle->totalNumPages + 1;
  free(str);
  return RC_OK;
}