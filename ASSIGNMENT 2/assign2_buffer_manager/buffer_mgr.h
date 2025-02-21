#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

// Include return codes and methods for logging errors
#include "dberror.h"
#include "storage_mgr.h"

// Include bool DT
#include "dt.h"

// Replacement Strategies
typedef enum ReplacementStrategy {
	RS_FIFO = 0,
	RS_LRU = 1,
	RS_CLOCK = 2,
	RS_LFU = 3,
	RS_LRU_K = 4
} ReplacementStrategy;

// Data Types and Structures
typedef int PageNumber;
#define NO_PAGE -1

typedef struct BM_BufferPool {
	char *pageFile;
	int pageFrameCount; // the number of page frames
	ReplacementStrategy replacementPolicy;
	void *mgmtData; // use this one to store the bookkeeping info your buffer
	// manager needs for a buffer pool
} BM_BufferPool;

typedef struct BM_PageHandle {
	PageNumber pageNum;
	char *data;
} BM_PageHandle;


// Page Frame: each array entry in buffer pool
typedef struct Frame {
	PageNumber pageNum; // which page is currently stored in the frame
	int pinCount; // how many processes are using this page
	int dirtyBit; // whether the page has been modified
	char* data; // points to the area in memory storing the content of the page
}Frame;

// used by LRU
typedef struct Hash
{
    int capacity;
	int pageCnt;
    Frame* *frames;
} Hash;

// The cached page information
typedef struct PageCache {
	int head;
	int tail;
	int frameCnt; // the number of used frames in this buffer pool
	int capacity; // the total number of frames the page cache can store
	Frame* *frames; // store frames information
	//add by Jessica
	int numRead; //stores number of pages that have been read
	int numWrite; //stores number of pages that been written
	// to solve segment default issue by store the file handle
	SM_FileHandle* File_Handle;
	// hash for LRU
	int* hashTable; // store the frames information
}PageCache;


// convenience macros
#define MAKE_POOL()					\
		((BM_BufferPool *) malloc (sizeof(BM_BufferPool)))

#define MAKE_PAGE_HANDLE()				\
		((BM_PageHandle *) malloc (sizeof(BM_PageHandle)))

// Helper Interface
// manamge resources in buffer pool
Frame* createFrameNode();
void resetFrameNode(Frame* frame);
int* createHash(int capacity);
PageCache* createPageCache(BM_BufferPool *const bufferPool, int pageFrameCount);
void freeFrame(PageCache* cache);
void freeFileHandle(PageCache* cache);
void freeHash(PageCache* cache);
void freePageCache(PageCache* cache);

// Manage PageCache in buffer pool
int isFull(PageCache* cache);
int isEmpty(PageCache* cache);
Frame* isHitPageCache(PageCache* cache, const PageNumber pageNum);
RC addPageToPageCacheWithFIFO(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle,
				int pageNum);
RC addPageToPageCacheWithLRU(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle,
		const PageNumber pageNum);
RC updateLRUOrder(PageCache* cache, int pageNum);
RC removePageWithFIFO(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle);
Frame* removePageWithLRU(BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle, int leastUsedPage);
Frame* searchPageFromCache(PageCache *const cache, int pageNum);

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bufferPool, const char *const File_Name, 
		const int pageFrameCount, ReplacementStrategy replacementPolicy,
		void *strategyData);
RC shutdownBufferPool(BM_BufferPool *const bufferPool);
RC forceFlushPool(BM_BufferPool *const bufferPool);

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle);
RC unpinPage (BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle);
RC forcePage (BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle);
RC pinPage (BM_BufferPool *const bufferPool, BM_PageHandle *const Page_Handle, 
		const PageNumber pageNum);

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bufferPool);
int *getDirtyFlags (BM_BufferPool *const bufferPool);
int *getFixCounts (BM_BufferPool *const bufferPool);
int getNumReadIO (BM_BufferPool *const bufferPool);
int getNumWriteIO (BM_BufferPool *const bufferPool);

#endif
