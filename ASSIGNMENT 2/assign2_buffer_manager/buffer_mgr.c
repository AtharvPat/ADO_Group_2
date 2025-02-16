#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer_mgr.h"
#include "storage_mgr.h"


/* ***************************************************************************************** */



/*
bm = bufferPool
pageFileName = File_Name
numPages = PageFrameCount
stratrgy = replacementPolicy
stratData = strategyData
pageCache = cache
fhandle = File_Handle


*/


RC CHECK_BUFFERPOOL(BM_BufferPool *const bufferPool)
{
    if(bufferPool == NULL) 
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


RC shutdownBufferPool(BM_BufferPool *const bufferPool)
{
    // check validation of bufferPool
    if(bufferPool == NULL) {
        return RC_ERROR;
    }

    // get current page cacha
    PageCache* cache = bufferPool->mgmtData;

    if(cache == NULL) {
        return RC_OK;
    }

    // force to flush all pages in buffer pool
    if(forceFlushPool(bufferPool) != RC_OK) {
        return RC_ERROR;
    }

    // release all resources assigned to page cache
    freePageCache(cache);

    bufferPool->mgmtData = NULL;

    return RC_OK;

}

/* ***************************************************************************************** */

// forceFlushPool is to cause all dirty pages from the buffer pool to be written to disk
// -- check whether there are dirty pages as well as the pin counts is equal to 0
RC forceFlushPool(BM_BufferPool *const bufferPool)
{
    CHECK_BUFFERPOOL(bufferPool);

    // get the store the page cache
    PageCache* cache = bufferPool->mgmtData;

    if(cache == NULL) {
        return RC_OK;
    }
    // iterate to check all frames
    int i;
    for(i = 0; i < cache->capacity; i++) {
        Frame* frame = cache->arr[i];
        // this frame has no page file
        if(frame->pageNumber == NO_PAGE) {
            continue;
        }
        // force all drity pages from the buffer pool to be written to disk
        if (frame->dirtyBit == 1 && frame->pinCount == 0) {
            // get the disk page handle pointer
            SM_FileHandle *File_Handle = cache->File_Handle;

            // write this dirty page to the disk
            if(writeBlock(frame->pageNumber, File_Handle, frame->data) != RC_OK) {
                return RC_WRITE_FAILED;
            }
            cache->numWrite++;

            // after flush all dirth pages in buffer pool
            frame->dirtyBit = 0;
        }
    }

    return RC_OK;
}

/* ***************************************************************************************** */
RC pinPage (BM_BufferPool *const bufferPool, BM_PageHandle *const pageHandle,
    const PageNumber pageNumber)
{
// check validations of parameters
if(bufferPool == NULL || pageHandle == NULL || pageNumber < 0) {
    return RC_ERROR;
}

// get the cache in this buffer
PageCache* cache = bufferPool->mgmtData;

// check whether we can get right cache
if(cache == NULL) {
    return RC_ERROR;
}

// check whether this pageNumber hit the cache
Frame* frame = isHitPageCache(cache, pageNumber);

// if yes, hit page cache
if(frame != NULL) {
    pageHandle->pageNumber = pageNumber;
    pageHandle->data = frame->data;
    frame->pinCount++;

    if(bufferPool->strategy == RS_LRU) {
        updateLRUOrder(cache, pageNumber);
    }
    return RC_OK;
}

// if no execute different pin page processes based on replacement strategy
if(bufferPool->strategy == RS_FIFO) {
    return addPageToPageCacheWithFIFO(bufferPool, pageHandle, pageNumber);
} 
else if(bufferPool->strategy == RS_LRU) {
    return addPageToPageCacheWithLRU(bufferPool, pageHandle, pageNumber);
}
return RC_OK;
}

/* ***************************************************************************************** */

RC forcePage (BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle)
{
    // check the validation of parameters
    if(bufferPool == NULL || page == NULL) {
        return RC_ERROR;
    }

    // get page cache
    PageCache* cache = bufferPool->mgmtData;

    if(cache == NULL) {
        return RC_OK;
    }

    // search a frame from page cache
    Frame* frame = searchPageFromCache(cache, Page_Handle->pageNum);

    // if this frame doesn't exist
    if(frame == NULL) {
        return RC_ERROR;
    }

    // get the disk page handle pointer
    SM_FileHandle* File_Handle = cache->File_Handle;

    // write this dirty page to the disk
    if(writeBlock(frame->pageNum, File_Handle, frame->data) != RC_OK) {
        return RC_WRITE_FAILED;
    }

    return RC_OK;
}

/* ***************************************************************************************** */

Frame* createFrameNode()
{
    // allocate memory for this frame
    Frame* frame = (Frame*)calloc(1, sizeof(Frame));

    // allocate memory for storing the content of the page
    char* data = (char *) calloc(PAGE_SIZE, sizeof(char));

    // initialize values for every attributes
    frame->pageNum = NO_PAGE;
    frame->pinCount = 0;
    frame->dirtyBit = 0;
    frame->data = data;
    return frame;
}

/* ***************************************************************************************** */

//reset this new frame node when remove this frame from buffer pool.
void* resetFrameNode(Frame* frame) {
    frame->pageNum = NO_PAGE;
    frame->pinCount = 0;
    frame->dirtyBit = 0;
}

/* ***************************************************************************************** */


// create a map to record the utility of every frames, used for LRU
int* createHash(int capacity )
{
    int* hash =(int*)malloc(capacity * sizeof(int));
    for(int i = 0; i < capacity; i++) {
        hash[i] = -1;
    }
    return hash;
}

// create a cache area for pages
PageCache* createPageCache(BM_BufferPool *const bufferPool, int numPages) {
    // allocate memory for this page cache
    PageCache* cache = (PageCache* ) malloc(sizeof(PageCache));

    // initialize values for every attribute
    pageCache->front = 0;
    pageCache->rear = -1;
    pageCache->frameCnt = 0;
    pageCache->capacity = numPages;
    pageCache->numRead=0;
    pageCache->numWrite=0;

    // store a page data
    cache->frames = (Frame**) malloc(numPages * sizeof(Frame*));
    
    for(i = 0; i < cache->capacity; ++i ) {
        Frame* frame = createFrameNode();
        cache->frames[i] = frame;
    }

    // store file handle data
    SM_FileHandle* File_Handle = (SM_FileHandle*)calloc(1, sizeof(SM_FileHandle));

    openPageFile(bufferPool->pageFile, File_Handle);

    cache->File_Handle = File_Handle;

    // initialize hash map
    if(bufferPool->strategy == RS_LRU) {
        pageCache->hashTable = createHash(numPages);
    } else if(bufferPool->strategy == RS_FIFO) {
        cache->hashTable = NULL;
    }
    return pageCache;
}

/* ***************************************************************************************** */

int isFull(PageCache* cache)
{
    return (cache->frameCnt == cache->capacity);
}
/* ***************************************************************************************** */

int isEmpty(PageCache* cache)
{
    return (cache->frameCnt == 0);
}

/* ***************************************************************************************** */
