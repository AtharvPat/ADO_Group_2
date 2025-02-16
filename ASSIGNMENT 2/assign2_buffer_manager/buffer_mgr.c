#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer_mgr.h"
#include "storage_mgr.h"


/* ***************************************************************************************** */



/*
bm = bufferpool
pageFileName = File_Name
numPages = PageFrameCount
stratrgy = replacementPolicy
stratData = strategyData
pageCache = cache


*/


RC CHECK_BUFFERPOOL(BM_BufferPool *const bufferPool)
{
    if(bufferpool == NULL) 
    {
        return RC_ERROR;
    }
    return RC_OK;
}










/* ***************************************************************************************** */

RC initBufferPool(BM_BufferPool *const bufferPool, const char *const File_Name,
    const int pageFrameCount, ReplacementStrategy replacementPolicy,
    void *strategyData)
{
// check the validation of parameters
if(bufferPool == NULL || File_Name == NULL) {
return RC_FILE_NOT_FOUND;
}

// the number of page frames in this buffer pool
if (pageFrameCount <= 0) {
return RC_ERROR;
}

// check if the file specified by the filename exisits
FILE *file_postiton = fopen(pageFileName, "r+");
if(file_postiton == NULL) {
return RC_FILE_NOT_FOUND;
}

// initialzie values of a new buffer pool
bufferPool->pageFile = (char *) File_Name;
bufferPool->numPages = pageFrameCount;
bufferPool->strategy = replacementPolicy;

// initialize page cache
PageCache* cache = createPageCache(bufferPool, pageFrameCount);

bufferPool->mgmtData = cache;

fclose(file_postiton);

return RC_OK;

}

/* ***************************************************************************************** */


RC shutdownBufferPool(BM_BufferPool *const bufferpool)
{
    // check validation of bm
    if(bufferpool == NULL) {
        return RC_ERROR;
    }

    // get current page cacha
    PageCache* cache = bufferpool->mgmtData;

    if(cache == NULL) {
        return RC_OK;
    }

    // force to flush all pages in buffer pool
    if(forceFlushPool(bufferpool) != RC_OK) {
        return RC_ERROR;
    }

    // release all resources assigned to page cache
    freePageCache(cache);

    bufferpool->mgmtData = NULL;

    return RC_OK;

}

/* ***************************************************************************************** */

