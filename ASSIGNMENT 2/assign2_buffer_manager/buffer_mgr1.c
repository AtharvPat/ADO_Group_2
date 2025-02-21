// This file implements interfaces related to Pool Handling and Access Page
//  defined in buffer_mgr.h header.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer_mgr.h"
#include "storage_mgr.h"







// initBufferPool creats a new buffer pool with pageFrameCount Page_Handle frames using the Page_Handle replacement replacementPolicy.
// The pool is used to cache pages from the Page_Handle file with name File_Name.
// -- Initially, all Page_Handle frames should be empty.
// -- The Page_Handle file should already exist.
RC initBufferPool(BM_BufferPool *const bufferPool, const char *const File_Name,
                    const int pageFrameCount, ReplacementStrategy replacementPolicy,
		            void *strategyData)
{
    // check the validation of parameters
    if(bufferPool == NULL || File_Name == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // the number of Page_Handle frames in this buffer pool
    if (pageFrameCount <= 0) {
        return RC_ERROR;
    }

    // check if the file specified by the filename exisits
    FILE *fp = fopen(File_Name, "r+");
    if(fp == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // initialzie values of a new buffer pool
    bufferPool->pageFile = (char *) File_Name;
    bufferPool->pageFrameCount = pageFrameCount;
    bufferPool->replacementPolicy= replacementPolicy;

    // initialize Page_Handle cache
    PageCache* cache = createPageCache(bufferPool, pageFrameCount);

    bufferPool->mgmtData = cache;

    fclose(fp);

    return RC_OK;

}

// shutdownBufferPool is to destory a buffer pool.
// The method frees up all resources associated with buffer pool.
// -- Free the memory allocated for Page_Handle frames.
// -- If the buffer pool contains any dirty pages, then these pages should be written back to disk before destroying.
// -- Raise an errot if a buffer pool has pinned pages.
RC shutdownBufferPool(BM_BufferPool *const bufferPool)
{
    // check validation of bufferPool
    if(bufferPool == NULL) {
        return RC_ERROR;
    }

    // get current Page_Handle cacha
    PageCache* cache = bufferPool->mgmtData;

    if(cache == NULL) {
        return RC_OK;
    }

    // force to flush all pages in buffer pool
    if(forceFlushPool(bufferPool) != RC_OK) {
        return RC_ERROR;
    }

    // release all resources assigned to Page_Handle cache
    freePageCache(cache);

    bufferPool->mgmtData = NULL;

    return RC_OK;

}


// forceFlushPool is to cause all dirty pages from the buffer pool to be written to disk
// -- check whether there are dirty pages as well as the pin counts is equal to 0
RC forceFlushPool(BM_BufferPool *const bufferPool)
{
    // check validation of bufferPool
    if(bufferPool == NULL) {
        return RC_ERROR;
    }

    // get the store the Page_Handle cache
    PageCache* cache = bufferPool->mgmtData;

    if(cache == NULL) {
        return RC_OK;
    }
    // iterate to check all frames
    int i;
    for(i = 0; i < cache->capacity; i++) {
        Frame* frame = cache->frames[i];
        // this frame has no Page_Handle file
        if(frame->pageNum == NO_PAGE) {
            continue;
        }
        // force all drity pages from the buffer pool to be written to disk
        if (frame->dirtyBit == 1 && frame->pinCount == 0) {
            // get the disk Page_Handle handle pointer
            SM_FileHandle *fHandle = cache->fHandle;

            // write this dirty Page_Handle to the disk
            if(writeBlock(frame->pageNum, fHandle, frame->data) != RC_OK) {
                return RC_WRITE_FAILED;
            }
            cache->numWrite++;

            // after flush all dirth pages in buffer pool
            frame->dirtyBit = 0;
        }
    }

    return RC_OK;
}


// Buffer Manager Interface Access Pages

// pinPage is to pin the Page_Handle with Page_Handle number pageNum.
// pinning a Page_Handle means that clients of the buffer mananger can request this Page_Handle number.
RC pinPage (BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle,
		const PageNumber pageNum)
{
    // check validations of parameters
    if(bufferPool == NULL || Page_Handle == NULL || pageNum < 0) {
        return RC_ERROR;
    }

    // get the cache in this buffer
    PageCache* cache = bufferPool->mgmtData;

    // checke whether we can get right cache
    if(cache == NULL) {
        return RC_ERROR;
    }

    // check whether this pageNum hit the cache
    Frame* frame = isHitPageCache(cache, pageNum);

    // if yes, hit Page_Handle cache
    if(frame != NULL) {
        Page_Handle->pageNum = pageNum;
        Page_Handle->data = frame->data;
        frame->pinCount++;
        if(bufferPool->replacementPolicy== RS_LRU) {
            updateLRUOrder(cache, pageNum);
        }
        return RC_OK;
    }

    // if no execute different pin Page_Handle processes based on replacement replacementPolicy
    if(bufferPool->replacementPolicy== RS_FIFO) {
        return addPageToPageCacheWithFIFO(bufferPool, Page_Handle, pageNum);
    } else if(bufferPool->replacementPolicy== RS_LRU) {
        return addPageToPageCacheWithLRU(bufferPool, Page_Handle, pageNum);
    }
    return RC_OK;
}


// make a Page_Handle as dirty
RC markDirty (BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle)
{
    // check validation of parameters
    if(bufferPool == NULL || Page_Handle == NULL) {
        return RC_ERROR;
    }

    // get Page_Handle cache
    PageCache* cache = bufferPool->mgmtData;

    if(cache == NULL) {
        return RC_OK;
    }

    // search a frame from Page_Handle cache
    Frame* frame = searchPageFromCache(cache, Page_Handle->pageNum);

    // if this frame doesn't exist
    if(frame == NULL) {
        return RC_ERROR;
    }

    frame->dirtyBit = 1;

    return RC_OK;
}

// unpins the Page_Handle.
// The pageNum field of Page_Handle is used to figure out which Page_Handle to pin.
RC unpinPage (BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle)
{
    // check the validation of parameters
    if(bufferPool == NULL || Page_Handle == NULL) {
        return RC_ERROR;
    }
    // get Page_Handle cache
    PageCache* cache = bufferPool->mgmtData;

    if(cache == NULL) {
        return RC_OK;
    }

    // search a frame from Page_Handle cache
    Frame* frame = searchPageFromCache(cache, Page_Handle->pageNum);

    // if this frame doesn't exist
    if(frame == NULL) {
        return RC_ERROR;
    }

    frame->pinCount--;

    if(frame->pinCount == 0 && frame->dirtyBit == 1) {
        forcePage(bufferPool, Page_Handle);
    }
    return RC_OK;

}

// forcePage is to write the current content of Page_Handle back to the Page_Handle file on disk.
RC forcePage (BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle)
{
    // check the validation of parameters
    if(bufferPool == NULL || Page_Handle == NULL) {
        return RC_ERROR;
    }

    // get Page_Handle cache
    PageCache* cache = bufferPool->mgmtData;

    if(cache == NULL) {
        return RC_OK;
    }

    // search a frame from Page_Handle cache
    Frame* frame = searchPageFromCache(cache, Page_Handle->pageNum);

    // if this frame doesn't exist
    if(frame == NULL) {
        return RC_ERROR;
    }

    // get the disk Page_Handle handle pointer
    SM_FileHandle* fHandle = cache->fHandle;

    // write this dirty Page_Handle to the disk
    if(writeBlock(frame->pageNum, fHandle, frame->data) != RC_OK) {
        return RC_WRITE_FAILED;
    }

    return RC_OK;
}


// initialize a new frame node in buffer pool
Frame* createFrameNode()
{
    // allocate memory for this frame
    Frame* frame = (Frame*)calloc(1, sizeof(Frame));

    // allocate memory for storing the content of the Page_Handle
    char* data = (char *) calloc(PAGE_SIZE, sizeof(char));

    // initialize values for every attributes
    frame->pageNum = NO_PAGE;
    frame->pinCount = 0;
    frame->dirtyBit = 0;
    frame->data = data;
    return frame;
}

//reset this new frame node when remove this frame from buffer pool.
void* resetFrameNode(Frame* frame) {
    frame->pageNum = NO_PAGE;
    frame->pinCount = 0;
    frame->dirtyBit = 0;
}

// create a map to record the utility of every frames, used for LRU
int* createHash(int capacity )
{
    int* hashTable =(int*)malloc(capacity * sizeof(int));
    for(int i = 0; i < capacity; i++) {
        hashTable[i] = -1;
    }
    return hashTable;
}

// create a cache area for pages
PageCache* createPageCache(BM_BufferPool *const bufferPool, int pageFrameCount) {
    // allocate memory for this Page_Handle cache
    PageCache* cache = (PageCache* ) malloc(sizeof(PageCache));

    // initialize values for every attribute
    cache->front = 0;
    cache->rear = -1;
    cache->frameCnt = 0;
    cache->capacity = pageFrameCount;
    cache->numRead=0;
    cache->numWrite=0;

    // store a Page_Handle data
    cache->frames = (Frame**) malloc(pageFrameCount * sizeof(Frame*));
    int i;
    for(i = 0; i < cache->capacity; ++i ) {
        Frame* frame = createFrameNode();
        cache->frames[i] = frame;
    }

    // store file handle data
    SM_FileHandle* fHandle = (SM_FileHandle*)calloc(1, sizeof(SM_FileHandle));

    openPageFile(bufferPool->pageFile, fHandle);

    cache->fHandle = fHandle;

    // initialize hashTable map
    if(bufferPool->replacementPolicy== RS_LRU) {
        cache->hashTable = createHash(pageFrameCount);
    } else if(bufferPool->replacementPolicy== RS_FIFO) {
        cache->hashTable = NULL;
    }
    return cache;
}

// release all resources assigned to frames
void freeFrame(PageCache* cache) {
    if(cache->frames) {
        for(int i = 0; i < cache->capacity; i++) {
            Frame* frame = cache->frames[i];
            // release the resources assigned to store the content of the Page_Handle
            if(frame->data != NULL) {
                free(frame->data);
            }
            free(frame);
            cache->frames[i] = NULL;
        }
        free(cache->frames);
    }
}

// release the resources assigned to the storage file handle.
void freeFileHandle(PageCache* cache) {
    if(cache->fHandle) {
        free(cache->fHandle);
    }
}

// release map resources assigned to frames created by LRU replacementPolicy
void freeHash(PageCache* cache) {
    if(cache->hashTable) {
        free(cache->hashTable);
    }
}
void freePageCache(PageCache* cache) {
    if(cache != NULL) {
        freeFileHandle(cache);
        freeFrame(cache);
        freeHash(cache);
        free(cache);
    }
}


// Page_Handle cache is full when the frameCnt becomes equal to size
int isFull(PageCache* cache)
{
    return (cache->frameCnt == cache->capacity);
}

// Page_Handle cache is empty when frameCnt is 0
int isEmpty(PageCache* cache)
{
    return (cache->frameCnt == 0);
}

// check whether the required pageNum hits the cache
Frame* isHitPageCache(PageCache* cache, const PageNumber pageNum) {
    // iterate all frames stored in this Page_Handle cache
    int i;
    for(int i = 0; i < cache->capacity; i++) {
        Frame* frame = cache->frames[i];
        if(frame->pageNum == pageNum) {
            return frame;
        }
    }
    // the Page_Handle cache didn't contain the current Page_Handle number data, return NULL
    return NULL;
}

RC updateLRUOrder(PageCache* cache, int pageNum)
{
    int* hashTable = cache->hashTable;
    // update hashTable
    int index = -1;
    int i;
    for(i = 0; i < cache->capacity; i++) {
        if(hashTable[i] == pageNum) {
            index = i;
            break;
        }
    }
    if(index == -1) {
        return RC_ERROR;
    }
    int updatePageNum = hashTable[index];
    for(i = index; i < cache->capacity - 1; i++) {
        hashTable[i] = hashTable[i + 1];
    }
    hashTable[cache->capacity - 1] = updatePageNum;
    return RC_OK;
}

// add a new frame to cache
RC addPageToPageCacheWithFIFO(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle, int pageNum)
{
    // get current Page_Handle cache
    PageCache* cache = bufferPool->mgmtData;

    // The following process is to add this new Page_Handle to Page_Handle cache

    // if current Page_Handle cache is full
    if (isFull(cache)) {
        removePageWithFIFO(bufferPool, Page_Handle);
    }
    // get the frame to store this Page_Handle content
    cache->rear = (cache->rear + 1) % cache->capacity;

    Frame* frame = cache->frames[cache->rear];

    // copy the file content from disk to memory
    SM_FileHandle *fHandle = cache->fHandle;

    if(ensureCapacity(pageNum + 1, fHandle) != RC_OK) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    if(readBlock(pageNum, fHandle, frame->data) != RC_OK) {
        return RC_ERROR;
    }

    cache->numRead++;

    // update this frame information Page_Handle
    frame->pageNum = pageNum;
    frame->pinCount = 1;
    frame->dirtyBit = 0;

    // store Page_Handle number info to Page_Handle
    Page_Handle->pageNum = pageNum;
    Page_Handle->data = frame->data;

    // store this Page_Handle in the cache
    // cache->frames[cache->rear] = frame;
    cache->frameCnt = cache->frameCnt + 1;

    return RC_OK;
}

// add new Page_Handle to Page_Handle cache based on LRU replacementPolicy
RC addPageToPageCacheWithLRU(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle,
		const PageNumber pageNum)
{
    // get current Page_Handle cache
    PageCache* cache = bufferPool->mgmtData;

    int* hashTable = cache->hashTable;

    int frameIndex = -1;

    Frame* frame = NULL;
    // mark whether the current cache is full
    int fullFlag = 0;
    if(isFull(cache)) {
        int leastUsedPageNum = hashTable[0];
        frame = removePageWithLRU(bufferPool, Page_Handle, leastUsedPageNum);
        fullFlag = 1;
        // frame = cache->frames[leastUsedPageNum];
    } else {
        for(int i = 0; i < bufferPool->pageFrameCount; i++) {
            if(hashTable[i] == -1) {
                frameIndex = i;
                break;
            }
        }
        if(frameIndex == -1) {
            return RC_ERROR;
        }
        frame = cache->frames[frameIndex];
    }

    if(frame == NULL) {
        return RC_ERROR;
    }

    // copy the Page_Handle content
    SM_FileHandle *fHandle = cache->fHandle;

    // ensure the file Page_Handle exists
    if(ensureCapacity(pageNum + 1, fHandle) != RC_OK) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    // copy the file content from disk to memory
    if(readBlock(pageNum, fHandle, frame->data) != RC_OK) {
        return RC_ERROR;
    }

    // print frame Page_Handle
    // printf("current fram index = %d\n", frameIndex);

    cache->numRead++;

    // update this frame information Page_Handle
    frame->pageNum = pageNum;
    frame->pinCount = 1;
    frame->dirtyBit = 0;

    // store Page_Handle number info to Page_Handle
    Page_Handle->pageNum = pageNum;
    Page_Handle->data = frame->data;

    cache->frameCnt = cache->frameCnt + 1;

    // store this Page_Handle in the cache
    if(fullFlag == 1) {
        // cache->frames[hashTable[0]] = frame;
        for(int i = 0; i < cache->capacity - 1; i++) {
            hashTable[i] = hashTable[i+1];
        }
        hashTable[cache->capacity - 1] = pageNum;
    } else {
        // cache->frames[frameIndex] = frame;
        hashTable[frameIndex] = pageNum;
    }

    return RC_OK;
}

// Remove a frame from queue based on FIFO. It changes front and frameCnt
RC removePageWithFIFO(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle)
{
    PageCache* cache = bufferPool->mgmtData;
    // check whether this Page_Handle cache is empty
    if (isEmpty(cache))
        return RC_ERROR;

    // check whether there exisit frame with pinCount = 0
    int cnt = 0;
    Frame** frames = cache->frames;
    for(int i = 0; i < cache->capacity; i++) {
        if(frames[i]->pinCount == 0) {
            cnt++;
        }
    }
    if(cnt == 0) {
        return RC_ERROR;
    }

    // get the first frame in the Page_Handle cache
    Frame* frame = cache->frames[cache->front];

    // fix test case :201
    if(frame->pinCount > 0) {
        while(cache->frames[cache->front]->pinCount > 0) {
            cache->front = (cache->front + 1) % cache->capacity;
        }
        frame = cache->frames[cache->front];

        // set the tail to the current frame
        cache->rear = cache->front - 1;
    }
    if(frame->pinCount == 0 && frame->dirtyBit == 1) {
        forcePage(bufferPool, Page_Handle);
        cache->numWrite++;
    }
    // remove the first frame
    cache->front = (cache->front + 1) % cache->capacity;

    // update the number of used frame in Page_Handle cache
    cache->frameCnt = cache->frameCnt - 1;

    // reset this frame node
    resetFrameNode(frame);

    return RC_OK;
}

Frame* removePageWithLRU(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle, int leastUsedPage)
{
    PageCache* cache = bufferPool->mgmtData;
    // check whether this Page_Handle cache is empty
    if (isEmpty(cache))
        return NULL;

    // check whether there exisit frame with pinCount = 0
    int cnt = 0;
    Frame** frames = cache->frames;
    for(int i = 0; i < cache->capacity; i++) {
        if(frames[i]->pinCount == 0) {
            cnt++;
        }
    }
    if(cnt == 0) {
        return NULL;
    }

    // get the least Page_Handle in the Page_Handle cache
    Frame* frame = searchPageFromCache(cache, leastUsedPage);

    if(frame == NULL) {
        return NULL;
    }

    if(frame->pinCount == 0 && frame->dirtyBit == 1) {
        forcePage(bufferPool, Page_Handle);
        cache->numWrite++;
    }

    // remove the least Page_Handle
    resetFrameNode(frame);

    cache->frameCnt = cache->frameCnt - 1;

    return frame;
}

// get the frame from the Page_Handle cache
Frame* searchPageFromCache(PageCache *const cache, int pageNum) {
    // get a frame based on Page_Handle number
    int i;
    for(int i = 0; i < cache->capacity; i++) {
        Frame* frame = cache->frames[i];
        if(frame->pageNum == pageNum) {
            return frame;
        }
    }
    return NULL;
}

/* Statistics Interface */

// The getFrameContents function returns an array of PageNumbers (of size
// pageFrameCount). where the ith element is the number of the Page_Handle stored in the ith
// Page_Handle frame. An empty Page_Handle frame is represented using the constant NO PAGE.
PageNumber *getFrameContents(BM_BufferPool *const bufferPool) {
  if (bufferPool == NULL) {
    return NULL;
  }

  int totalNumPages = bufferPool->pageFrameCount;

  PageCache *cache = bufferPool->mgmtData;

  PageNumber *frames = (PageNumber *) malloc(bufferPool->pageFrameCount * sizeof(PageNumber));
  for (int i = 0; i < bufferPool->pageFrameCount; i++) {
    frames[i] = cache->frames[i]->pageNum;
  }
  return frames;

}

// The getDirtyFlags function returns an array of bools (of size pageFrameCount) where
// the ith element. is TRUE if the Page_Handle stored in the ith Page_Handle frame is dirty.
// Empty Page_Handle frames are considered as clean.
int *getDirtyFlags(BM_BufferPool *const bufferPool) {
  if (bufferPool == NULL) {
    return NULL;
  }

  PageCache *cache = bufferPool->mgmtData;
  int pageFrameCount = bufferPool->pageFrameCount;
  int *frames = (PageNumber *) malloc(pageFrameCount * sizeof(int));

  for (int i = 0; i < pageFrameCount; i++) {
    frames[i] = cache->frames[i]->dirtyBit;
  }
  return frames;
}

// The getFixCounts function returns an array of ints (of size pageFrameCount) where
// the ith element is the fix count of the Page_Handle stored in the ith Page_Handle frame.
// Return 0 for empty Page_Handle frames.
int *getFixCounts(BM_BufferPool *const bufferPool) {
  if (bufferPool == NULL) {
    return NULL;
  }

  PageCache *cache = bufferPool->mgmtData;
  int pageFrameCount = bufferPool->pageFrameCount;
  int *frames = (PageNumber *) malloc(pageFrameCount * sizeof(int));

  for (int i = 0; i < pageFrameCount; i++) {
    frames[i] = cache->frames[i]->pinCount;
  }
  return frames;
}

//The function returns the number of pages that have been read from the disk since the buffer pool was initialized
int getNumReadIO(BM_BufferPool *const bufferPool) {
  if (bufferPool == NULL) {
    return -1;
  }
  // get Page_Handle cache
  PageCache *cache = bufferPool->mgmtData;
  return cache->numRead;
}

//The function returns the number of pages that have been written to the Page_Handle file since the buffer pool was initialized
int getNumWriteIO(BM_BufferPool *const bufferPool) {
  if (bufferPool == NULL) {
    return -1;
  }
  // get Page_Handle cache
  PageCache *cache = bufferPool->mgmtData;
  return cache->numWrite;
}