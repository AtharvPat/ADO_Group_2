#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer_mgr.h"
#include "storage_mgr.h"

/* ***************************************************************************************** */

/*
bm = bufferPool
pageFileName = File_Name
numPages = pageFrameCount
strategy = replacementPolicy
stratData = strategyData
pageCache = cache
fhandle = File_Handle

hash = hashTable
frames = frames
page = Page_Handle


*/

RC CHECK_BUFFERPOOL(BM_BufferPool *const bufferPool)
{
    if (bufferPool == NULL)
    {
        return RC_ERROR;
    }
    return RC_OK;
}

RC CHECK_CACHE(PageCache *cache)
{
    if (cache == NULL)
    {
        return RC_ERROR;
    }
    return RC_OK;
}

RC CHECK_FRAME(Frame *frame)
{
    if (frame == NULL)
    {
        return RC_ERROR;
    }
    return RC_OK;
}

RC CHECK_BUFFERPOOL_PAGE_HANDLE(BM_BufferPool *const bufferPool, BM_PageHandle *const page)
{
    if (bufferPool == NULL || page == NULL)
    {
        return RC_ERROR;
    }
    return RC_OK;
}

RC CHECK_EMPTY_CACHE(PageCache *cache)
{
    if (isEmpty(cache))
    {
        return RC_ERROR;
    }
    return RC_OK;
}

RC CHECK_FRAME_INDEX(int frameIndex)
{
    if (frameIndex == -1)
        {
            return RC_ERROR;
        }
        return RC_OK;
} 

RC CHECK_AVAILABLE_FRAMES(int availableFrames)
{
    if (availableFrames == 0)
    {
        return RC_ERROR;
    }
    return RC_OK;
}

RC writePageToDisk(Frame *frame, SM_FileHandle *fileHandle)
{
    // Attempt to write the page data to disk
    return (writeBlock(frame->pageNum, fileHandle, frame->data) == RC_OK) ? RC_OK : RC_WRITE_FAILED;
}

RC validatePageExistence(int pageNum, SM_FileHandle *File_Handle)
{
    if (ensureCapacity(pageNum + 1, File_Handle) != RC_OK)
    {
        return RC_READ_NON_EXISTING_PAGE; // Return error if the page doesn't exist on disk
    }
    return RC_OK;
}

RC readPageData(int pageNum, SM_FileHandle *File_Handle, char *data)
{
    if (readBlock(pageNum, File_Handle, data) != RC_OK)
    {
        return RC_ERROR; // Return error if the read operation fails
    }
    return RC_OK;
}

PageCache *getPageCache(BM_BufferPool *bufferPool) 
{
    return (PageCache *)bufferPool->mgmtData;
}

void updateFrameAndTail(PageCache *cache, Frame **frame) 
{
    // Assign the frame at the current head position
    *frame = cache->frames[cache->head];

    // Update the tail to point to the previous frame, with circular indexing
    cache->tail = (cache->head == 0) ? cache->capacity - 1 : cache->head - 1;
}

/* ****************************************** Changed *********************************************** */

RC initBufferPool(BM_BufferPool *const bufferPool, const char *const File_Name,
                  const int pageFrameCount, ReplacementStrategy replacementPolicy,
                  void *strategyData)
{
    // Validate the input arguments
    if (!bufferPool || !File_Name)
    {
        return RC_FILE_NOT_FOUND;
    }

    // Ensure a valid number of page frames
    if (pageFrameCount < 1)
    {
        return RC_ERROR;
    }

    // Attempt to open the file to check if it exists
    FILE *filePtr = fopen(File_Name, "r+");
    if (!filePtr)
    {
        return RC_FILE_NOT_FOUND;
    }

    // Assign buffer pool properties
    bufferPool->pageFile = (char *)File_Name;
    bufferPool->pageFrameCount = pageFrameCount;
    bufferPool->replacementPolicy = replacementPolicy;

    // Allocate and initialize page cache
    bufferPool->mgmtData = createPageCache(bufferPool, pageFrameCount);

    // Close the file after verification
    fclose(filePtr);

    return RC_OK;
}

/* ***************************************** Changed ************************************************ */

RC shutdownBufferPool(BM_BufferPool *const bufferPool)
{
    // Validate the buffer pool pointer
    if (!bufferPool)
    {
        return RC_ERROR;
    }

    // Retrieve the page cache
    PageCache *cache = bufferPool->mgmtData;

    // If no cache exists, return success
    CHECK_CACHE(cache);

    // Ensure all pages in the buffer pool are flushed
    if (forceFlushPool(bufferPool) != RC_OK)
    {
        return RC_ERROR;
    }

    // Deallocate resources associated with the page cache
    freePageCache(cache);
    bufferPool->mgmtData = NULL;

    return RC_OK;
}

/* ************************************* Changed ++ **************************************************** */

// forceFlushPool is to cause all dirty pages from the buffer pool to be written to disk
// -- check whether there are dirty pages as well as the pin counts is equal to 0
RC forceFlushPool(BM_BufferPool *const bufferPool)
{
    CHECK_BUFFERPOOL(bufferPool);

    // Retrieve the page cache
    PageCache *cache = bufferPool->mgmtData;

    CHECK_CACHE(cache);

    // Iterate through all frames in the cache
    int i = 0;
    while (i < cache->capacity)
    {
        Frame *frame = cache->frames[i];

        // Skip frames without assigned pages
        if (frame->pageNum == NO_PAGE)
        {
            i++;  // Increment index to avoid infinite loop
            continue;
        }

        // Your additional logic goes here...

        i++;  // Increment index to move to the next frame


        // Write dirty pages (unpinned) back to disk
        if (frame->pinCount == 0)
        {
            if (frame->dirtyBit)
            {
                // Get the file handle
                SM_FileHandle *fileHandle = cache->File_Handle;
        
                // Write the dirty page to disk
                RC status = writePageToDisk(frame, fileHandle);
                if (status != RC_OK)
                {
                    // Log the error or handle the failure
                    return RC_WRITE_FAILED; // Return a specific error code for write failure
                }
                frame->dirtyBit = 0;
        
                // Increment write count and reset dirty bit
                cache->numWrite++;
            }
        }
    }

    return RC_OK;
}

/* ********************************** Chnanged ++ ******************************************************* */

RC pinPage(BM_BufferPool *const bufferPool, BM_PageHandle *const pageHandle,
           const PageNumber pageNum)
{
    // Validate input parameters
    if (!bufferPool || !pageHandle || pageNum < 0)
    {
        return RC_ERROR;
    }

    // Retrieve and validate the page cache
    PageCache *cache = bufferPool->mgmtData;
    if (!cache)
    {
        return RC_ERROR;
    }

    // Check if the requested page is already in the cache
    Frame *frame = isHitPageCache(cache, pageNum);

    if (frame)
    {
        // Assign page number and data to page handle
        pageHandle->pageNum = pageNum;
        pageHandle->data = frame->data;

        // Increase the pin count of the frame
        ++frame->pinCount;

        // Update LRU order if using LRU replacement policy
        if (bufferPool->replacementPolicy == RS_LRU)
        {
            updateLRUOrder(cache, pageNum);
        }

        return RC_OK;
    }


    // Handle page replacement according to the selected policy
    return (bufferPool->replacementPolicy == RS_FIFO)
               ? addPageToPageCacheWithFIFO(bufferPool, pageHandle, pageNum)
               : addPageToPageCacheWithLRU(bufferPool, pageHandle, pageNum);
}

/* *********************************** Changed ****************************************************** */

RC forcePage(BM_BufferPool *const bufferPool, BM_PageHandle *const pageHandle)
{
    // Validate input parameters
    if (!bufferPool || !pageHandle)
    {
        return RC_ERROR;
    }

    // Retrieve and validate the page cache
    PageCache *cache = bufferPool->mgmtData;
    CHECK_CACHE(cache);

    // Locate the frame in the cache
    Frame *frame = searchPageFromCache(cache, pageHandle->pageNum);

    // Return error if the page is not found in the cache
    CHECK_FRAME(frame);

    // Retrieve the file handle
    SM_FileHandle *fileHandle = cache->File_Handle;

    // Write the page to disk
    return (writeBlock(frame->pageNum, fileHandle, frame->data) == RC_OK) ? RC_OK : RC_WRITE_FAILED;
}

/* ************************************ Changed ***************************************************** */

Frame *createFrameNode()
{
    // Allocate memory for the frame and its data
    Frame *frame = (Frame *)calloc(1, sizeof(Frame));
    if (!frame)
    {
        return NULL; // Return NULL if memory allocation fails
    }

    frame->data = (char *)calloc(PAGE_SIZE, sizeof(char));
    if (!frame->data)
    {
        free(frame); // Free allocated frame memory if data allocation fails
        return NULL;
    }

    // Initialize frame attributes
    frame->pageNum = NO_PAGE;
    frame->pinCount = 0;
    frame->dirtyBit = 0;

    return frame;
}

/* *************************************** Changed ************************************************** */

// reset this new frame node when remove this frame from buffer pool.
void resetFrameNode(Frame *frame)
{
    if (frame == NULL)
    {
        return; // Prevents dereferencing a NULL pointer
    }
    frame->dirtyBit = 0;

    frame->pinCount = 0;

    frame->pageNum = NO_PAGE;
}

/* ************************************* Changed ++ **************************************************** */

// create a map to record the utility of every frames, used for LRU
int *createHash(int capacity)
{
    if (capacity <= 0)
    {
        return NULL; // Prevents invalid allocations
    }

    int *hash = (int *)malloc(capacity * sizeof(int));
    if (hash == NULL)
    {
        return NULL; // Handles memory allocation failure
    }

    // Initialize hash table with -1
    int i = 0;
    while (i < capacity)
    {
        hash[i] = -1;
        i++;
    }

    return hash;
}

/* *************************************** chaged  ++************************************************** */

// Create a cache area for pages
PageCache *createPageCache(BM_BufferPool *const bufferPool, int pageFrameCount)
{
    // Validate input parameters
    if (!bufferPool || pageFrameCount <= 0)
    {
        return NULL;
    }

    // Allocate memory for page cache
    PageCache *cache = (PageCache *)malloc(sizeof(PageCache));
    CHECK_CACHE(cache);

    // Initialize cache attributes
    cache->numRead = 0;

    cache->head = 0;
    cache->numWrite = 0;
    cache->capacity = pageFrameCount;

    cache->tail = -1;
    cache->frameCnt = 0;

    // Allocate memory for frames array
    cache->frames = (Frame **)malloc(pageFrameCount * sizeof(Frame *));
    if (!cache->frames)
    {
        free(cache);
        return NULL;
    }

    // Initialize each frame
    for (int i = 0; i < cache->capacity; ++i)
    {
        cache->frames[i] = createFrameNode();
        if (!cache->frames[i])
        {
            // Cleanup previously allocated frames before returning
            for (int j = 0; j < i; ++j)
            {
                free(cache->frames[j]->data);
                free(cache->frames[j]);
            }
            free(cache->frames);
            free(cache);
            return NULL;
        }
    }

    // Allocate memory for file handle
    cache->File_Handle = (SM_FileHandle *)calloc(1, sizeof(SM_FileHandle));
    if (!cache->File_Handle)
    {
        // Cleanup memory before returning
        for (int i = 0; i < cache->capacity; ++i)
        {
            free(cache->frames[i]->data);
            free(cache->frames[i]);
        }
        free(cache->frames);
        free(cache);
        return NULL;
    }

    // Open the page file
    if (openPageFile(bufferPool->pageFile, cache->File_Handle) != RC_OK)
    {
        // Cleanup on failure
        free(cache->File_Handle);
        for (int i = 0; i < cache->capacity; ++i)
        {
            free(cache->frames[i]->data);
            free(cache->frames[i]);
        }
        free(cache->frames);
        free(cache);
        return NULL;
    }

    // Initialize hash table if needed
    if (bufferPool->replacementPolicy == RS_LRU)
    {
        cache->hashTable = createHash(pageFrameCount);
        if (!cache->hashTable)
        {
            // Cleanup if hash table allocation fails
            closePageFile(cache->File_Handle);
            free(cache->File_Handle);
            for (int i = 0; i < cache->capacity; ++i)
            {
                free(cache->frames[i]->data);
                free(cache->frames[i]);
            }
            free(cache->frames);
            free(cache);
            return NULL;
        }
    }
    else
    {
        cache->hashTable = NULL;
    }

    return cache;
}

/* ********************************* changed ******************************************************** */

int isFull(PageCache *cache)
{
    if (!cache)
    {
        return 0; // Treat NULL as not full (or handle it differently if needed)
    }
    return (cache->frameCnt == cache->capacity);
}
/* *************************************** changed ************************************************** */

int isEmpty(PageCache *cache)
{
    if (!cache)
    {
        return 1; // Treat NULL as empty
    }
    return (cache->frameCnt == 0);
}
/* **************************************** changed ************************************************* */


RC addPageToPageCacheWithLRU(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle,
    const PageNumber pageNum)
{
// Retrieve the page cache from the buffer pool
PageCache *cache = bufferPool->mgmtData;

int frameIndex = -1;
Frame *frame = NULL;

int *hashTable = cache->hashTable;

// Flag indicating whether the cache was full
int fullFlag = 0;

// Check if the cache is full
if (isFull(cache))
{
// The least recently used page is located at the start of the hashTable
int leastUsedPageNum = hashTable[0];

// Remove the least recently used page from the cache
frame = removePageWithLRU(bufferPool, Page_Handle, leastUsedPageNum);

fullFlag = 1; // Mark the cache as full
}
else
{
// Search for an available empty frame in the cache
int i = 0;
while (i < bufferPool->pageFrameCount)
{
if (hashTable[i] == -1) // If an empty frame is found
{
frameIndex = i;
break;
}
i++;
}

// If no empty frame is found, return an error
CHECK_FRAME_INDEX(frameIndex);

// Retrieve the frame from the cache at the found index
frame = cache->frames[frameIndex];
}

// If no frame is available or allocated, return an error
CHECK_FRAME(frame);

// Access the file handle for reading the page content
SM_FileHandle *File_Handle = cache->File_Handle;

// Ensure that the requested page exists on disk
RC ensure_result = validatePageExistence(pageNum, File_Handle);
if (ensure_result != RC_OK)
{
return ensure_result;
}

// Read the page's data from the disk into the frame's data buffer
RC read_result = readPageData(pageNum, File_Handle, frame->data);
if (read_result != RC_OK)
{
return read_result;
}

frame->pinCount = 1; // Pin the frame since it's now in use
// Increment the number of read operations
cache->numRead++;

// Update the frame's metadata (page number, pin count, and dirty bit)
frame->dirtyBit = 0; 
frame->pageNum = pageNum;// Page is clean


// Set the page number and data in the provided page handle
Page_Handle->pageNum = pageNum;


// Increment the frame count in the cache
cache->frameCnt++;

Page_Handle->data = frame->data;

// If the cache was full, remove the least recently used page and update hashTable
if (fullFlag != 1)
{ 
hashTable[frameIndex] = pageNum;   
}
else
{
// Store the page number in the available spot in the hashTable
// Shift the entries in the hashTable to make space for the new page
int i = 0;
while (i < cache->capacity - 1)
{
hashTable[i] = hashTable[i + 1];
i++;
}

// Place the new page at the end of the hashTable
hashTable[cache->capacity - 1] = pageNum;
}

return RC_OK; // Return success after the page is added to the cache
}

/* ************************************** chnaged *************************************************** */

Frame *isHitPageCache(PageCache *cache, const PageNumber pageNum)
{
    // Check if cache is valid
    if (!cache || cache->frames == NULL)
    {
        return NULL;
    }

    // Iterate through all frames in the cache to find the page
    int i = 0;
    while (i < cache->capacity)
    {
        Frame *frame = cache->frames[i];
    
        // If the frame contains the page, return the frame
        if (frame->pageNum == pageNum)
        {
            return frame;
        }
    
        i++;
    }
    
    return NULL; // Return NULL if the page is not found
}
/* *********************************** chnaged ****************************************************** */

RC updateLRUOrder(PageCache *cache, int pageNum)
{
    // Check if cache or hashTable is valid
    if (!cache || !cache->hashTable)
    {
        return RC_ERROR;
    }

    int *hashTable = cache->hashTable;

    // Find the index of the page in the hash table
    int index = -1;
    for (int i = 0; i < cache->capacity; ++i)
    {
        if (hashTable[i] == pageNum)
        {
            index = i;
            break;
        }
    }

    // If page is not found, return error
    if (index == -1)
    {
        return RC_ERROR;
    }

    // Move the found page to the end of the hash table to maintain LRU order
    int updatePageNum = hashTable[index];

    // Shift elements to the left to make space at the end
    int i = index;
    while (i < cache->capacity - 1)
    {
        hashTable[i] = hashTable[i + 1];
        ++i;
    }

    // Place the updated page at the end (most recently used)
    hashTable[cache->capacity - 1] = updatePageNum;

    return RC_OK;
}

/* ************************************** changed *************************************************** */


/* ************************************* chsanged **************************************************** */



PageNumber *getFrameContents(BM_BufferPool *const bufferPool)
{
    CHECK_BUFFERPOOL(bufferPool);

    // Ensure the page cache is properly initialized
    PageCache *cache = bufferPool->mgmtData;

    int totalNumPages = bufferPool->pageFrameCount;

    PageNumber *frames = (PageNumber *)malloc(bufferPool->pageFrameCount * sizeof(PageNumber));
    if (frames == NULL)
    {
        return NULL; // Return NULL if memory allocation fails
    }

    // Iterate over the frames and store the page numbers
    for (int i = 0; i < bufferPool->pageFrameCount; i++)
    {
        // Check if the frame exists and is valid
        if (cache->frames[i] != NULL)
        {
            frames[i] = cache->frames[i]->pageNum;
        }
        else
        {
            // Mark empty or uninitialized frames with a special sentinel value
            frames[i] = -1; // Can be replaced with a different sentinel value if needed
        }
    }

    return frames; // Return the array containing the page numbers
}

/* ************************************ ok ***************************************************** */

int *getDirtyFlags(BM_BufferPool *const bufferPool)
{
    CHECK_BUFFERPOOL(bufferPool);

    PageCache *cache = bufferPool->mgmtData;
    int pageFrameCount = bufferPool->pageFrameCount;
    int *frames = (PageNumber *)malloc(pageFrameCount * sizeof(int));

    int i = 0;
    while (i < pageFrameCount)
    {
        frames[i] = cache->frames[i]->dirtyBit;
        i++;
    }
    return frames;
}

/* *************************************** changed ************************************************** */

RC addPageToPageCacheWithFIFO(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle, int pageNum)
{
    PageCache *cache = getPageCache(bufferPool);

    // If the cache is full, we need to remove the least recently used page (FIFO replacement)
    if (isFull(cache)) 
    {
        // Attempt to remove a page using FIFO replacement policy
        RC removeResult = removePageWithFIFO(bufferPool, Page_Handle);
        
        if (removeResult != RC_OK) 
        {
            fprintf(stderr, "Error: Failed to remove page using FIFO policy.\n");
            return removeResult; // Return the actual error code instead of a generic error
        }
    }

    // Determine the next available frame (FIFO: Circular buffer behavior)
    cache->tail = (cache->tail + 1) % cache->capacity;

    // Access the file handle from the cache
    SM_FileHandle *File_Handle = cache->File_Handle;
    Frame *frame = cache->frames[cache->tail];

    // Ensure that there's enough space to store the page on disk
    RC ensure_result = validatePageExistence(pageNum, File_Handle);
    if (ensure_result != RC_OK)
    {
        return ensure_result;
    }

    // Read the page's data from the disk into the frame's data buffer
    RC read_result = readPageData(pageNum, File_Handle, frame->data);
    if (read_result != RC_OK)
    {
        return read_result;
    }

    // Update cache statistics
    cache->numRead++;

    frame->dirtyBit = 0;

    // Update the frame's metadata
    frame->pageNum = pageNum;
    // Mark the page as clean initially

    // Set the page information in the page handle for the caller to access
    Page_Handle->pageNum = pageNum;

    frame->pinCount = 1; // Pin the frame (indicates it's in use)

    // Increment the number of frames currently in the cache
    Page_Handle->data = frame->data;
    cache->frameCnt++;

    return RC_OK; // Return success after adding the page
}
/* ************************************** changed *************************************************** */


int *getFixCounts(BM_BufferPool *const bufferPool)
{

    CHECK_BUFFERPOOL(bufferPool);

    PageCache *cache = bufferPool->mgmtData;
    int pageFrameCount = bufferPool->pageFrameCount;
    int *frames = (PageNumber *)malloc(pageFrameCount * sizeof(int));

    if (!frames)
    {
        return NULL; // Return NULL if memory allocation fails
    }

    // Use a while loop to iterate over frames in the cache
    int i = 0;
    while (i < bufferPool->pageFrameCount)
    {
        // Store the pin count of each frame into the array
        frames[i] = cache->frames[i]->pinCount;
        i++; // Increment the loop counter
    }

    // Return the array of pin counts
    return frames;
}

/* ************************************** changed *************************************************** */

int getNumReadIO(BM_BufferPool *const bufferPool)
{
    // Check if the buffer pool is valid
    if (!bufferPool)
    {
        return -1; // Return -1 if the buffer pool is NULL
    }

    // Access the page cache stored in the buffer pool's management data
    PageCache *cache = bufferPool->mgmtData;

    // Directly return the number of read IOs
    return cache ? cache->numRead : -1;
}

/* ********************************** changed ******************************************************* */

RC markDirty(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle)
{
    // check validation of parameters
    CHECK_BUFFERPOOL_PAGE_HANDLE(bufferPool, Page_Handle);

    // get page cache
    PageCache *cache = bufferPool->mgmtData;

    CHECK_CACHE(cache);

    // search a frame from page cache
    Frame *frame = searchPageFromCache(cache, Page_Handle->pageNum);

    // if this frame doesn't exist
    CHECK_FRAME(frame);

    frame->dirtyBit = 1;

    return RC_OK;
}

/* ************************************* changed **************************************************** */

// Locate the frame containing the requested page
Frame *searchPageFromCache(PageCache *const cache, int pageNum)
{
    // Check if the cache is valid
    if (cache == NULL)
    {
        return NULL; // Return NULL if the cache is invalid
    }

    // Iterate through the frames in the cache to find the page
    for (int i = 0; i < cache->capacity; i++)
    {
        // If the frame contains the page number, return the frame
        if (cache->frames[i]->pageNum == pageNum)
        {
            return cache->frames[i];
        }
    }

    // Return NULL if the page is not found in any frame
    return NULL;
}

/* **************************************** changed ************************************************* */

RC unpinPage(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle)
{
    // Validate input parameters
    CHECK_BUFFERPOOL_PAGE_HANDLE(bufferPool, Page_Handle);

    // Access page cache from buffer pool
    PageCache *cache = bufferPool->mgmtData;
    CHECK_CACHE(cache);

    // Find the frame associated with the given page
    Frame *frame = searchPageFromCache(cache, Page_Handle->pageNum);
    CHECK_FRAME(frame);

    // Decrease pin count
    frame->pinCount--;

    // If unpinned and dirty, write it back
    if (frame->pinCount == 0 && frame->dirtyBit == 1)
    {
        forcePage(bufferPool, Page_Handle);
    }

    return RC_OK;
}

/* ********************************** ok ******************************************************* */

// Free allocated frames in the page cache
void freeFrames(PageCache *cache)
{
    if (cache->frames)
    {
        for (int i = 0; i < cache->capacity; i++)
        {
            if (cache->frames[i])
            {
                free(cache->frames[i]->data);
                free(cache->frames[i]);
                cache->frames[i] = NULL;
            }
        }
        free(cache->frames);
    }
}

/* **************************************** ok ************************************************* */

// Free allocated file handle
void freeFileHandle(PageCache *cache)
{
    free(cache->File_Handle);
}

/* ************************************* ok **************************************************** */

// Free allocated hash table
void freeHashTable(PageCache *cache)
{
    free(cache->hashTable);
}

/* ************************************** ok *************************************************** */

// Deallocate the entire page cache
void freePageCache(PageCache *cache)
{
    if (cache)
    {
        freeFileHandle(cache);
        freeFrames(cache);
        freeHashTable(cache);
        free(cache);
    }
}

/* ******************************************* chnaged ********************************************** */

Frame *removePageWithLRU(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle, int leastUsedPage)
{
    PageCache *cache = bufferPool->mgmtData;
    // check whether this page cache is empty
    CHECK_EMPTY_CACHE(cache);

    // check whether there exisit frame with pinCount = 0
    int cnt = 0;
    Frame **frames = cache->frames;
    int i = 0;
    while (i < cache->capacity)
    {
        if (frames[i]->pinCount == 0)
        {
            cnt++;
        }
        i++;
    }
    CHECK_AVAILABLE_FRAMES(cnt);

    // get the least page in the page cache
    Frame *frame = searchPageFromCache(cache, leastUsedPage);

    CHECK_FRAME(frame);

    if (frame->pinCount == 0)
    {
        // If the frame is dirty, write it back to the disk
        if (frame->dirtyBit == 1)
        {
            forcePage(bufferPool, Page_Handle);
            cache->numWrite++; // Increment the write count when a dirty page is written back
        }

        // Reset the frame to remove it from the cache
        resetFrameNode(frame);

        // Decrease the frame count after the page is removed
        cache->frameCnt--;
    }

    return frame;
}

/* ********************************************* changed ******************************************** */
// RC getFrameToEvict(BM_BufferPool *bufferPool, Frame **frame)
// {
//     // Ensure buffer pool is valid
//     if (!bufferPool || !frame)
//     {
//         return RC_INVALID_PARAMETER;
//     }

//     // Retrieve the cache from buffer pool
//     PageCache *cache = bufferPool->mgmtData;
    
//     // Loop through the frames to find an unpinned frame
//     if (cache->frames[cache->head]->pinCount > 0)
//     {
//         while (cache->frames[cache->head]->pinCount > 0)
//         {
//             // Move head pointer to the next frame
//             cache->head = (cache->head + 1) % cache->capacity;
//         }

//         // Retrieve the frame to evict
//         *frame = cache->frames[cache->head];

//         // If the frame is dirty, write it back to disk before removal
//         if ((*frame)->dirtyBit)
//         {
//             RC status = forcePage(bufferPool, *frame);
//             if (status != RC_OK)
//             {
//                 return status;
//             }
//             cache->numWrite++;
//         }

//         // Update the tail pointer to point to the previous frame
//         cache->tail = (cache->head == 0) ? cache->capacity - 1 : cache->head - 1;
//     }
//     else
//     {
//         *frame = cache->frames[cache->head];
//     }

//     return RC_OK;
// }

/* ********************************************* changed ******************************************** */


RC removePageWithFIFO(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle)
{
    PageCache *cache = bufferPool->mgmtData;
    CHECK_EMPTY_CACHE(cache);


    // Check if any unpinned frame exists
    int availableFrames = 0;

    Frame **frames = cache->frames;
    int i = 0;
    while (i < cache->capacity)
    {
        if (frames[i]->pinCount == 0)
        {
            availableFrames++;
        }
        i++;
    }
    Frame *frame = cache->frames[cache->head];
    
    CHECK_AVAILABLE_FRAMES(availableFrames);

    // Locate the first unpinned frame
    
// Check if the frame is pinned
if (frame->pinCount > 0) 
{
    // Use a for loop to find an unpinned frame
    for (;;)
    {
        // Check if the current frame is pinned
        if (cache->frames[cache->head]->pinCount == 0)
        {
            break;  // Found an unpinned frame, exit loop
        }
    
        // Move to the next frame in a circular manner
        cache->head = (cache->head + 1) % cache->capacity;
    }
    
    updateFrameAndTail(cache, &frame);
} 

// Check if the frame is dirty
if (frame->dirtyBit == 1)
{
    // Only force a page write if it's not pinned
    if (frame->pinCount == 0) 
    {
        forcePage(bufferPool, Page_Handle);  // Force the dirty page to disk
        cache->numWrite++;  // Increment the write count
    } 
    else 
    {
        // Handle case where the frame is still pinned
        printf("Page cannot be written to disk as it is currently pinned.\n");
    }
}

// Remove the first frame by updating the head
cache->head = (cache->head + 1) % cache->capacity;

// Update the number of used frames in the cache
cache->frameCnt = cache->frameCnt - 1;

// Reset the removed frame
resetFrameNode(frame);

return RC_OK;
}

/* ******************************************** changed ********************************************* */

int getNumWriteIO(BM_BufferPool *const bufferPool)
{
    // Validate input parameters
    if (bufferPool == NULL || bufferPool->mgmtData == NULL)
    {
        return RC_ERROR;
    }

    // Extract page cache from buffer pool
    PageCache *cache = bufferPool->mgmtData;

    // Check if numWrite is valid (non-negative)
    if (cache->numWrite < 0)
    {
        return RC_ERROR;
    }

    // Return the number of write operations
    return cache->numWrite;
}