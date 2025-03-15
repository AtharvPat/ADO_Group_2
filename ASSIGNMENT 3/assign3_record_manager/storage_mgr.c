#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 4096  

/****************************************************************************************************/

RC CHECK_FILE(char *File_Name)
{
  // Checking if the file Exists
  if (File_Name == NULL)
  {
    return RC_FILE_NOT_FOUND; // Returing Error Code
  }
  return RC_OK;
}
RC CHECK_FILE_PTR(FILE *filePtr ){
	if (filePtr == NULL) {
	// Return error if the file cannot be opened
	return RC_FILE_NOT_FOUND;

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




/****************************************************************************************************/


FILE *filePointer;

// Initialize storage manager by setting filePointer to NULL
void initStorageManager(void) {
    filePointer = NULL;
}

/***************************************** ***********************************************************/

// Create a new page file with one page of PAGE_SIZE bytes
RC createPageFile(char *fileName) {
    FILE *file_position = fopen(fileName, "w+");
    if (file_position == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Allocate memory for a blank page and initialize it with zeros
    SM_PageHandle blankPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    if (blankPage == NULL) {
        fclose(file_position);
        return RC_FILE_NOT_FOUND;
    }

    // Write the blank page to the file
    size_t writtenBytes = fwrite(blankPage, sizeof(char), PAGE_SIZE, file_position);
    if (writtenBytes < PAGE_SIZE) {
        printf("Failed to write blank page\n");
        fclose(file_position);
        free(blankPage);
        return RC_WRITE_FAILED;
    } else {
        printf("Page file created successfully\n");
    }

    // Clean up and close the file
    fclose(file_position);
    free(blankPage);
    return RC_OK;
}

/********************************** *********** *******************************************************/
RC destroyPageFile(char *fileName) {
    FILE *file_position = fopen(fileName, "r");
    if (file_position == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Close the file before deleting
    fclose(file_position);

    // Delete the file
    if (remove(fileName) != 0) {
        return RC_FILE_NOT_FOUND;  // In case the file couldn't be deleted
    }

    return RC_OK;
}



/************************** **************************************************************************/
RC closePageFile(SM_FileHandle *fHandle) {
    FILE *file_position = fopen(fHandle->fileName, "r+");
    if (file_position == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Close the file after checking it opened successfully
    if (fclose(file_position) != 0) {
        return RC_FILE_NOT_FOUND;
    }

    return RC_OK;
}

/********************************************** ******************************************************/

RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    // Attempt to open the file in read-write mode
    FILE *filePtr = fopen(fileName, "r+");
	CHECK_FILE_PTR(filePtr);



    // Seek to the end of the file to determine its total size
    if (fseek(filePtr, 0, SEEK_END) != 0) {
        fclose(filePtr);  // Close the file if seeking fails
        return RC_FILE_NOT_FOUND;
    }

    // Calculate the total file size
    long fileSize = ftell(filePtr);
    if (fileSize == -1) {
        fclose(filePtr);  // Close the file if ftell fails
        return RC_FILE_NOT_FOUND;
    }
    // Assign file name and initialize the current page position
    fHandle->fileName = fileName;
    fHandle->curPagePos = 0;
    // Calculate the total number of pages in the file
    fHandle->totalNumPages = (fileSize / PAGE_SIZE);
    
    // If the file is not empty but doesn't fit an exact number of pages, ensure at least one page
    if (fHandle->totalNumPages == 0 && fileSize > 0) {
        fHandle->totalNumPages = 1;
    }

    // Close the file after determining the file properties
    fclose(filePtr);

    return RC_OK;  // Successfully opened the page file and set file handle properties
}
/******************************************* *********************************************************/
int getBlockPos(SM_FileHandle *fHandle) {
    // Check if the file handle is NULL, which is an invalid state
    if (fHandle == NULL) {
        printf("Error: File handle is NULL.\n");
        return -1;  // Return a special error code to indicate invalid file handle
    }

    // Ensure the current page position is valid (non-negative)
    if (fHandle->curPagePos < 0) {
        printf("Error: Current page position is negative. Invalid state.\n");
        return -2;  // Return a special error code indicating invalid page position
    }

    // Log the current position for debugging purposes
    printf("The current block position is: %d\n", fHandle->curPagePos);

    // Return the current block position stored in the file handle
    return fHandle->curPagePos;
}

/********************************************** ******************************************************/

RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Check if the pageNum is valid
    if (pageNum >= fHandle->totalNumPages || pageNum < 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    FILE *file_position = fopen(fHandle->fileName, "r");
    if (file_position == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Seek to the correct page position
    if (fseek(file_position, pageNum * PAGE_SIZE, SEEK_SET) != 0) {
        fclose(file_position);
        return RC_READ_NON_EXISTING_PAGE;
    }

    // Read the page into memory
    if (fread(memPage, sizeof(char), PAGE_SIZE, file_position) < PAGE_SIZE) {
        fclose(file_position);
        return RC_FILE_NOT_FOUND;
    }

    // Update the current page position in the file handle
    fHandle->curPagePos = ftell(file_position);

    // Close the file
    fclose(file_position);

    return RC_OK;
}


/******************************************* *********************************************************/

RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Check if the file handle is NULL or if the file has no pages
    if (fHandle == NULL) {
        printf("Error: File handle is NULL.\n");
        return RC_FILE_NOT_FOUND;  // Return specific error if file handle is NULL
    }
    
    if (fHandle->totalNumPages <= 0) {
        printf("Error: The file does not contain any pages.\n");
        return RC_READ_NON_EXISTING_PAGE;  // Return error if the file is empty
    }

    // Open the file for reading
    filePointer = fopen(fHandle->fileName, "r");
    if (filePointer == NULL) {
        printf("Error: Failed to open the file %s.\n", fHandle->fileName);
        return RC_FILE_NOT_FOUND;  // Return error if the file can't be opened
    }

    // Move to the start of the file and attempt to read the first block
    if (fseek(filePointer, 0, SEEK_SET) != 0) {
        printf("Error: Failed to seek to the start of the file.\n");
        fclose(filePointer);
        return RC_READ_NON_EXISTING_PAGE;  // Return error if seeking fails
    }

    // Read the first page into the provided memory handle
    if (fread(memPage, sizeof(char), PAGE_SIZE, filePointer) < PAGE_SIZE) {
        printf("Error: Failed to read the first page from the file.\n");
        fclose(filePointer);
        return RC_READ_NON_EXISTING_PAGE;  // Return error if reading fails
    }

    // Successfully read the first block, update the file handle's position
    fHandle->curPagePos = 0;

    // Close the file after reading
    fclose(filePointer);

    printf("Successfully read the first block from %s.\n", fHandle->fileName);  // Logging success

    return RC_OK;  // Return success code
}

/****************************************************************************************************/

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Check if the file handle or memory page is NULL
    if (fHandle == NULL) {
        printf("Error: File handle is NULL.\n");
        return RC_FILE_NOT_FOUND;  // Return error if the file handle is NULL
    }

    if (memPage == NULL) {
        printf("Error: Memory page is NULL.\n");
        return RC_FILE_NOT_FOUND;  // Return error if the memory page is NULL
    }

    // Check if the current page position is at the first page or out of bounds
    if (fHandle->curPagePos <= 0) {
        printf("Error: No previous block available, already at the first page.\n");
        return RC_READ_NON_EXISTING_PAGE;  // Return error if there's no previous block
    }

    // Attempt to read the previous block
    RC rc = readBlock(fHandle->curPagePos - 1, fHandle, memPage);
    if (rc != RC_OK) {
        printf("Error: Failed to read the previous block.\n");
    } else {
        printf("Successfully read the previous block from page %d.\n", fHandle->curPagePos - 1);  // Log success
    }

    return rc;  // Return the result of the readBlock function
}

/***************************************** ***********************************************************/

// Read the current block from the file
// Read the current block from the file
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Check if the file handle is NULL
    if (fHandle == NULL) {
        printf("Error: File handle is NULL.\n");
        return RC_FILE_NOT_FOUND;  // Return error if the file handle is NULL
    }

    // Check if the memory page is NULL
    if (memPage == NULL) {
        printf("Error: Memory page is NULL.\n");
        return RC_FILE_NOT_FOUND;  // Return error if the memory page is NULL
    }

    // Check if the current page position is valid
    if (fHandle->curPagePos < 0 || fHandle->curPagePos >= fHandle->totalNumPages) {
        printf("Error: Current page position is out of bounds.\n");
        return RC_READ_NON_EXISTING_PAGE;  // Return error if the page position is invalid
    }

    // Attempt to read the current block
    RC rc = readBlock(fHandle->curPagePos, fHandle, memPage);
    if (rc != RC_OK) {
        printf("Error: Failed to read the current block at position %d.\n", fHandle->curPagePos);
    } else {
        printf("Successfully read the current block from page %d.\n", fHandle->curPagePos);  // Log success
    }

    return rc;  // Return the result of the readBlock function
}
/************************************ ****************************************************************/
 
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Check if the file handle or memory page is NULL
    if (fHandle == NULL) {
        printf("Error: File handle is NULL.\n");
        return RC_FILE_NOT_FOUND;  // Return error if the file handle is NULL
    }

    if (memPage == NULL) {
        printf("Error: Memory page is NULL.\n");
        return RC_FILE_NOT_FOUND;  // Return error if the memory page is NULL
    }

    // Check if the current page position is at the last page or out of bounds
    if (fHandle->curPagePos + 1 >= fHandle->totalNumPages) {
        printf("Error: No next block available, already at the last page.\n");
        return RC_READ_NON_EXISTING_PAGE;  // Return error if there's no next block
    }

    // Attempt to read the next block
    RC rc = readBlock(fHandle->curPagePos + 1, fHandle, memPage);
    if (rc != RC_OK) {
        printf("Error: Failed to read the next block.\n");
    } else {
        printf("Successfully read the next block from page %d.\n", fHandle->curPagePos + 1);  // Log success
    }

    return rc;  // Return the result of the readBlock function
}

/***************************************** ***********************************************************/

// Read the last block from the file
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Check if the file handle is NULL
    if (fHandle == NULL) {
        printf("Error: File handle is NULL.\n");
        return RC_FILE_NOT_FOUND;  // Return error if the file handle is NULL
    }

    // Check if the memory page is NULL
    CHECK_MEMORY_PAGE(memPage);

    // Check if there are any pages in the file
    if (fHandle->totalNumPages <= 0) {
        printf("Error: No pages available in the file.\n");
        return RC_READ_NON_EXISTING_PAGE;  // Return error if there are no pages
    }

    // Attempt to read the last block
    RC rc = readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
    if (rc != RC_OK) {
        printf("Error: Failed to read the last block at page %d.\n", fHandle->totalNumPages - 1);
    } else {
        printf("Successfully read the last block from page %d.\n", fHandle->totalNumPages - 1);  // Log success
    }

    return rc;  // Return the result of the readBlock function
}

/****************************************** **********************************************************/

// Write a block at a specific position
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Check if the page number is valid
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        printf("Error: Invalid page number %d.\n", pageNum);
        return RC_WRITE_FAILED;  // Return an error if pageNum is out of range
    }

    // Open the file for read and write operations
    filePointer = fopen(fHandle->fileName, "r+");
    if (filePointer == NULL) {
        printf("Error: File '%s' not found.\n", fHandle->fileName);
        return RC_FILE_NOT_FOUND;  // Return an error if the file cannot be opened
    }

    // Move to the position of the page to be written
    if (fseek(filePointer, pageNum * PAGE_SIZE, SEEK_SET) != 0) {
        printf("Error: Failed to seek to page number %d.\n", pageNum);
        fclose(filePointer);
        return RC_WRITE_FAILED;  // Return an error if fseek fails
    }

    // Write the content of the memory page to the file
    for (int i = 0; i < PAGE_SIZE; i++) {
        // If end of file is reached, append an empty block
        if (feof(filePointer)) {
            printf("Warning: Reached end of file. Appending an empty block.\n");
            appendEmptyBlock(fHandle);  // Append an empty block
        }
        
        // Write each byte from the memory page to the file
        if (fputc(memPage[i], filePointer) == EOF) {
            printf("Error: Failed to write byte %d to file.\n", i);
            fclose(filePointer);
            return RC_WRITE_FAILED;  // Return error if writing fails
        }
    }

    // Update the current page position in the file handle
    fHandle->curPagePos = ftell(filePointer);

    // Close the file after writing
    fclose(filePointer);

    printf("Successfully wrote page %d to the file '%s'.\n", pageNum, fHandle->fileName);
    return RC_OK;  // Return success
}

/************************************* ***************************************************************/

// Write the current block to the file
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Open the file in read-write mode
    filePointer = fopen(fHandle->fileName, "r+");
    if (filePointer == NULL) {
        printf("Error: File '%s' not found.\n", fHandle->fileName);
        return RC_FILE_NOT_FOUND;  // Return error if file cannot be opened
    }

    // Ensure there is space at the end of the file if needed
    if (ftell(filePointer) == fHandle->curPagePos + PAGE_SIZE) {
        printf("Warning: Reached the end of the file. Appending an empty block.\n");
        appendEmptyBlock(fHandle);  // Ensure the block is available in case of EOF
    }

    // Move the file pointer to the current page position
    if (fseek(filePointer, fHandle->curPagePos, SEEK_SET) != 0) {
        printf("Error: Failed to seek to the current page position %ld.\n", fHandle->curPagePos);
        fclose(filePointer);
        return RC_WRITE_FAILED;  // Return error if fseek fails
    }

    // Write the memory page to the file
    size_t bytesWritten = fwrite(memPage, sizeof(char), PAGE_SIZE, filePointer);
    if (bytesWritten < PAGE_SIZE) {
        printf("Error: Failed to write the entire page to the file. Only %zu bytes were written.\n", bytesWritten);
        fclose(filePointer);
        return RC_WRITE_FAILED;  // Return error if fwrite fails to write enough bytes
    }

    // Update the current page position in the file handle
    fHandle->curPagePos = ftell(filePointer);
    
    // Close the file after writing
    fclose(filePointer);

    printf("Successfully wrote the current block to the file '%s'.\n", fHandle->fileName);
    return RC_OK;  // Return success
}

/************************************* ***************************************************************/


RC appendEmptyBlock(SM_FileHandle *fHandle) {
    // Allocate memory for an empty block, initialized to zero
    SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    if (emptyBlock == NULL) {
        printf("Error: Memory allocation failed for empty block.\n");
        return RC_WRITE_FAILED;  // Return error if memory allocation fails
    }

    // Open the file in append mode to ensure we're adding to the end
    FILE *filePointer = fopen(fHandle->fileName, "r+");
    if (filePointer == NULL) {
        printf("Error: Could not open file '%s'.\n", fHandle->fileName);
        free(emptyBlock);  // Free allocated memory before returning
        return RC_FILE_NOT_FOUND;  // Return error if file cannot be opened
    }

    // Move to the end of the file to append the block
    if (fseek(filePointer, 0, SEEK_END) != 0) {
        printf("Error: Failed to seek to the end of the file '%s'.\n", fHandle->fileName);
        fclose(filePointer);
        free(emptyBlock);  // Free allocated memory before returning
        return RC_WRITE_FAILED;  // Return error if seeking fails
    }

    // Write the empty block to the file
    size_t bytesWritten = fwrite(emptyBlock, sizeof(char), PAGE_SIZE, filePointer);
    if (bytesWritten < PAGE_SIZE) {
        printf("Error: Failed to write the empty block. Only %zu bytes written.\n", bytesWritten);
        fclose(filePointer);
        free(emptyBlock);  // Free allocated memory before returning
        return RC_WRITE_FAILED;  // Return error if write fails
    }

    // Update the total number of pages in the file handle
    fHandle->totalNumPages++;

    // Close the file after writing
    fclose(filePointer);

    // Free the allocated memory for the empty block
    free(emptyBlock);

    printf("Successfully appended an empty block to the file '%s'.\n", fHandle->fileName);
    return RC_OK;  // Return success
}

/********************************************** ******************************************************/

RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    // Check if the file already has enough pages
    if (fHandle->totalNumPages >= numberOfPages) {
        return RC_OK;  // No need to add more pages
    }

    // Loop to append empty blocks until the required number of pages is reached
    for (int i = fHandle->totalNumPages; i < numberOfPages; i++) {
        // Append an empty block to the file
        RC response = appendEmptyBlock(fHandle);
        if (response != RC_OK) {
            printf("Error: Failed to append empty block at page %d\n", i);
            return response;  // Return the error code if appending fails
        }
    }

    // Successfully ensured the required capacity
    return RC_OK;
}
