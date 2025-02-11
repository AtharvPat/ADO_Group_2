#ifndef STORAGE_MGR_H
#define STORAGE_MGR_H

#include "dberror.h"

/************************************************************
 *                    handle data structures                *
 ************************************************************/
typedef struct SM_FileHandle {
	char *File_Name;
	int totalNumPages;
	int curPagePos;
	void *mgmtInfo;
} SM_FileHandle;

typedef char* SM_PageHandle;

/************************************************************
 *                    interface                             *
 ************************************************************/
/* manipulating page files */
extern void initStorageManager (void);
extern RC createPageFile (char *fileName);
extern RC openPageFile (char *fileName, SM_FileHandle *File_Handle);
extern RC closePageFile (SM_FileHandle *File_Handle);
extern RC destroyPageFile (char *fileName);

/* reading blocks from disc */
extern RC readBlock (int PG_Num, SM_FileHandle *File_Handle, SM_PageHandle Memory_Page);
extern int getBlockPos (SM_FileHandle *File_Handle);
extern RC readFirstBlock (SM_FileHandle *File_Handle, SM_PageHandle Memory_Page);
extern RC readPreviousBlock (SM_FileHandle *File_Handle, SM_PageHandle Memory_Page);
extern RC readCurrentBlock (SM_FileHandle *File_Handle, SM_PageHandle Memory_Page);
extern RC readNextBlock (SM_FileHandle *File_Handle, SM_PageHandle Memory_Page);
extern RC readLastBlock (SM_FileHandle *File_Handle, SM_PageHandle Memory_Page);

/* writing blocks to a page file */
extern RC writeBlock (int PG_Num, SM_FileHandle *File_Handle, SM_PageHandle Memory_Page);
extern RC writeCurrentBlock (SM_FileHandle *File_Handle, SM_PageHandle Memory_Page);
extern RC appendEmptyBlock (SM_FileHandle *File_Handle);
extern RC ensureCapacity (int numberOfPages, SM_FileHandle *File_Handle);

#endif
