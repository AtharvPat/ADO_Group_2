// This file implements interfaces related to Pool Handling and Access Page
//  defined in buffer_mgr.h header.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer_mgr.h"
#include "storage_mgr.h"


Frame* isHitPageCache(PageCache* pageCache, const PageNumber pageNum) {
    // iterate all frames stored in this page cache
    int x = 0;
    while (x < pageCache->cap) {
        Frame* frame = pageCache->array[x];
        if (frame->pageNum == pageNum) {
            return frame;
        }
        x++;
    }

    // the page cache didn't contain the current page number data, return NULL
    return NULL;
}

RC updateLRUOrder(PageCache* pageCache, int pageNum) {
    int ind = -1;
    for (int i = 0; i < pageCache->capacity; i++) {
        if (pageCache->page_hash[i] == pageNum) {
            ind = i;
            break;
        }
    }
    if (ind == -1) return RC_ERROR;
    
    int temp = pageCache->page_hash[ind];
    memmove(&pageCache->page_hash[ind], &pageCache->page_hash[ind + 1], 
            (pageCache->capacity - ind - 1) * sizeof(int));
    pageCache->page_hash[pageCache->capacity - 1] = temp;
    return RC_OK;
}

RC addPageToPageCacheWithFIFO(BM_BufferPool *const bm, BM_PageHandle *const page, int pageNum) {
    PageCache* pageCache = bm->mgmtData;
    if (isFull(pageCache)) removePageWithFIFO(bm, page);
    
    pageCache->store = (pageCache->store + 1) % pageCache->capacity;
    Frame* frame = pageCache->arr[pageCache->store];
    
    if (ensureCapacity(pageNum + 1, pageCache->fHandle) != RC_OK ||
        readBlock(pageNum, pageCache->fHandle, frame->data) != RC_OK) {
        return RC_ERROR;
    }
    
    pageCache->numRead++;
    *frame = (Frame){.pageNum = pageNum, .pinCount = 1, .dirtyBit = 0};
    *page = (BM_PageHandle){.pageNum = pageNum, .data = frame->data};
    pageCache->frameCnt++;
    return RC_OK;
}

RC addPageToPageCacheWithLRU(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    PageCache* cache = bm->mgmtData;
    int index = isFull(cache) ? 0 : -1;
    
    if (index == 0) removePageWithLRU(bm, page, cache->hash[0]);
    else {
        for (int i = 0; i < bm->numPages; i++) {
            if (cache->hash[i] == -1) {
                index = i;
                break;
            }
        }
    }
    
    Frame* frame = cache->arr[index];
    if (ensureCapacity(pageNum + 1, cache->fHandle) != RC_OK ||
        readBlock(pageNum, cache->fHandle, frame->data) != RC_OK) {
        return RC_ERROR;
    }
    
    cache->numRead++;
    *frame = (Frame){.pageNum = pageNum, .pinCount = 1, .dirtyBit = 0};
    *page = (BM_PageHandle){.pageNum = pageNum, .data = frame->data};
    
    if (index == 0) memmove(&cache->hash[0], &cache->hash[1], (cache->capacity - 1) * sizeof(int));
    cache->hash[index == 0 ? cache->capacity - 1 : index] = pageNum;
    
    cache->frameCnt++;
    return RC_OK;
}

Frame* searchPageFromCache(PageCache *const pageCache, int pageNum) {
    for (int i = 0; i < pageCache->capacity; i++) {
        if (pageCache->arr[i]->pageNum == pageNum) {
            return pageCache->arr[i];
        }
    }
    return NULL;
}

PageNumber *getFrameContents(BM_BufferPool *const bm) {
    if (!bm) return NULL;
    PageNumber *arr = malloc(bm->numPages * sizeof(PageNumber));
    int i = 0;
    while (i < bm->numPages) {
        arr[i] = bm->mgmtData->arr[i]->pageNum;
        i++;
    }

    return arr;
}

int *getDirtyFlags(BM_BufferPool *const bm) {
    if (!bm) return NULL;
    int *flags = malloc(bm->numPages * sizeof(int));
    for (int i = 0; i < bm->numPages; i++) {
        flags[i] = bm->mgmtData->arr[i]->dirtyBit;
    }
    return flags;
}

int *getFixCounts(BM_BufferPool *const bm) {
    if (!bm) return NULL;
    int *counts = malloc(bm->numPages * sizeof(int));
    for (int i = 0; i < bm->numPages; i++) {
        counts[i] = bm->mgmtData->arr[i]->pinCount;
    }
    return counts;
}

int getNumReadIO(BM_BufferPool *const bm) {
    return bm ? bm->mgmtData->numRead : -1;
}
