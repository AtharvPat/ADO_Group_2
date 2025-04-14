#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "storage_mgr.h"

//This metadata is to store the number of pages in the file :  Extra Feature
#define METADATAOFFSET sizeof(int)

/* Helper Function */
int checkFileExistance(const char *fileName);

typedef struct mgmtInformation {
    FILE *fp;
} mgmtInformation;

void initStorageManager(void) {
    printf("Storage Manager has been initialized.\n");
}


RC createPageFile (char *fileName)
{
    FILE* filePointer = fopen(fileName,"w");

    switch (filePointer != NULL)
    {
        case 1: {
            int numPages = 1;
            int *val = &numPages;

            // Write the page count to the beginning of the file
            fwrite(val, sizeof(int), 1, filePointer);

            // Move to the end of the first page to extend the file with a null byte
            long offset = PAGE_SIZE + METADATAOFFSET - 1;
            fseek(filePointer, offset, SEEK_SET);
            fputc('\0', filePointer);
            fclose(filePointer);
            return RC_OK;
        }
        case 0:
        default:
            printf("Creation Failed");
            return RC_NOT_OK;
    }


}



RC openPageFile (char *fileName, SM_FileHandle *fHandle)
{

    mgmtInformation *mgmtInfo;
    mgmtInfo = malloc(sizeof(struct mgmtInformation));

    int fileExists = checkFileExistance(fileName);

    switch (fileExists)
    {
        case 1: {
            FILE *openedFile = fopen(fileName, "r+");
            mgmtInfo->fp = openedFile;

            fHandle->fileName = fileName;
            fHandle->mgmtInfo = mgmtInfo;

            // Read the total number of pages stored at the beginning of the file
            int *pageCountBuffer = (int *)malloc(sizeof(int));
            fread(pageCountBuffer, sizeof(int), 1, openedFile);

            fHandle->totalNumPages = *pageCountBuffer;
            fHandle->curPagePos = 0;

            return RC_OK;
        }
        case 0:
        default:
            return RC_FILE_NOT_FOUND;
    }

}

RC closePageFile (SM_FileHandle *fHandle)
{

    mgmtInformation *mgmt = (mgmtInformation *)fHandle->mgmtInfo;
    FILE *fp = mgmt->fp;

    int numPages = fHandle->totalNumPages;
    int *numPagesPointer = &numPages;


    fwrite(numPagesPointer,sizeof(int),1,fp);

    switch (fclose(fp))
    {
        case 0:
            return RC_OK;
        default:
            return RC_NOT_OK;
    }

}

RC destroyPageFile (char *fileName)
{
    switch (remove(fileName))
    {
        case 0:
            return RC_OK;
        default:
            return RC_FILE_NOT_FOUND;
    }

}


//pageNum starts from zero
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    switch (pageNum < 0 || pageNum > fHandle->totalNumPages - 1)
    {
        case 1:
            return RC_READ_NON_EXISTING_PAGE;
        case 0:
        default:
            break;
    }


    FILE *fp = ((mgmtInformation*)fHandle->mgmtInfo)->fp;

    /*
        Move file pointer to the location of pageNumth starting location using fseek
        Here pageNum starts from 0
    */

    fseek(fp, PAGE_SIZE * pageNum + METADATAOFFSET, SEEK_SET);
    
    fHandle->curPagePos = pageNum;
    
    switch (fread(memPage, sizeof(char), PAGE_SIZE, fp))
    {
        case 0:
            return RC_NOT_OK;
        default:
            return RC_OK;
    }

}


RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    mgmtInformation *mgmt = (mgmtInformation *)fHandle->mgmtInfo;
    FILE *fp = mgmt->fp;


    fseek(fp, METADATAOFFSET, SEEK_SET);

    int startPage = 0;
    fHandle->curPagePos = startPage;



    switch (fread(memPage, sizeof(char), PAGE_SIZE, fp))
    {
        case 0:
            return RC_NOT_OK;
        default:
            return RC_OK;
    }

};

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    FILE* fp = ((mgmtInformation*)fHandle->mgmtInfo)->fp;

    int pageNum = fHandle->curPagePos;

    switch ((pageNum + 1) >= fHandle->totalNumPages)
    {
        case 1:
            return RC_READ_NON_EXISTING_PAGE;
        case 0:
        default:
            break;
    }


    fseek(fp, (pageNum + 1) * PAGE_SIZE + METADATAOFFSET, SEEK_SET);

    fHandle->curPagePos++;

    switch (fread(memPage, PAGE_SIZE, 1, fp))
    {
        case 0:
            return RC_NOT_OK;
        default:
            return RC_OK;
    }


    return RC_NOT_OK;
}



RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    mgmtInformation *mgmtData = (mgmtInformation *)fHandle->mgmtInfo;
    FILE *fp = mgmtData->fp;


    fseek(fp, -PAGE_SIZE, SEEK_END);

    int lastPageIndex = fHandle->totalNumPages - 1;
    fHandle->curPagePos = lastPageIndex;

    switch (fread(memPage, sizeof(char), PAGE_SIZE, fp))
    {
        case 0:
            return RC_NOT_OK;
        default:
            return RC_OK;
    }

}

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    int pageNum = fHandle->curPagePos;

    mgmtInformation *info = (mgmtInformation *)fHandle->mgmtInfo;
    FILE *fp = info->fp;


    fseek(fp, PAGE_SIZE * pageNum + METADATAOFFSET, SEEK_SET);

    switch (fread(memPage, sizeof(char), PAGE_SIZE, fp))
    {
        case 0:
            return RC_NOT_OK;
        default:
            return RC_OK;
    }

}

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    FILE* fp = ((mgmtInformation*)fHandle->mgmtInfo)->fp;

    int pageNum = fHandle->curPagePos;

    switch ((pageNum - 1) < 0)
    {
        case 1:
            return RC_READ_NON_EXISTING_PAGE;
        case 0:
        default:
            break;
    }


    /*
        Go to the previous page's first byte
     */
    fseek(fp, (pageNum - 1) * PAGE_SIZE + METADATAOFFSET, SEEK_SET);

    fHandle->curPagePos--;

    switch (fread(memPage, sizeof(char), PAGE_SIZE, fp))
    {
        case 0:
            return RC_NOT_OK;
        default:
            return RC_OK;
    }

}

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
 
    switch (pageNum < 0 || pageNum > fHandle->totalNumPages - 1)
    {
        case 1:
            return RC_READ_NON_EXISTING_PAGE;
        case 0:
        default:
            break;
    }


    FILE *filePointer = ((mgmtInformation *)fHandle->mgmtInfo)->fp;
    long offset = METADATAOFFSET + (pageNum * PAGE_SIZE);

    fseek(filePointer, offset, SEEK_SET);
    fwrite(memPage, 1, PAGE_SIZE, filePointer);


    fHandle->curPagePos = pageNum;

    return RC_OK;
}

int getBlockPos (SM_FileHandle *fHandle)
{
    int pos = fHandle->curPagePos;
    return pos;
}



RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    int pageNum = fHandle->curPagePos;

    mgmtInformation *mgmt = (mgmtInformation *)fHandle->mgmtInfo;
    FILE *file = mgmt->fp;
    long position = METADATAOFFSET + (pageNum * PAGE_SIZE);
    fseek(file, position, SEEK_SET);


    fHandle->curPagePos = pageNum;

    switch (fwrite(memPage, 1, PAGE_SIZE, ((mgmtInformation*)fHandle->mgmtInfo)->fp))
    {
        case 0:
            return RC_NOT_OK;
        default:
            return RC_OK;
    }
}

RC appendEmptyBlock (SM_FileHandle *fHandle)
{

    mgmtInformation *mgmtInfo = (mgmtInformation *)fHandle->mgmtInfo;
    FILE *filePtr = mgmtInfo->fp;

    // Calculate the byte offset for the new page (after existing pages + metadata)
    long newPageOffset = METADATAOFFSET + PAGE_SIZE * (fHandle->totalNumPages + 1);
    fseek(filePtr, newPageOffset, SEEK_SET);

    // Mark the new page with a null byte
    fputc('\0', filePtr);

    // Update file handle information
    fHandle->curPagePos = fHandle->totalNumPages;
    fHandle->totalNumPages += 1;

    return RC_OK;

}



RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)
{
    int existingNumOfPages = ceil(fHandle->totalNumPages);

    switch (existingNumOfPages < numberOfPages)
    {
        case 1: {
            mgmtInformation *info = (mgmtInformation *)fHandle->mgmtInfo;
            FILE *file = info->fp;

            // Extend the file by seeking to the end of the new last page and writing a null byte
            long fileExtensionOffset = METADATAOFFSET + PAGE_SIZE * numberOfPages - 1;
            fseek(file, fileExtensionOffset, SEEK_SET);
            fputc('\0', file);

            // Write the updated page count at the start of the file
            fseek(file, 0, SEEK_SET);
            fwrite(&numberOfPages, sizeof(int), 1, file);

            // Reflect changes in the file handle
            fHandle->totalNumPages = numberOfPages;

            break;
        }
        case 0:
        default:
            break;
    }

    return RC_OK;
}

/*
    Helper functions definitions
 */

int checkFileExistance(const char * fileName)
{
    FILE *fP;
    switch ((fP = fopen(fileName, "r")) != NULL)
    {
        case 1:
            fclose(fP);
            return 1;
        case 0:
        default:
            return 0;
    }
}