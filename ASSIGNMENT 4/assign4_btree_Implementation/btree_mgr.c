#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "btree_mgr.h"
#include "buffer_mgr.h"
#include "buffer_mgr_stat.h"
#include "dberror.h"
#include "storage_mgr.h"
#include "btree_helper.h"

//Global Btree Manager
BTreeHandle* treeHandle; 
treeMgmtData* btreeMgmtData;
BT_ScanHandle* sHandle;
scanMgmtData* sMgmtData;
static int count = 0;

RC initIndexManager(void *mgmtData) {
    const char *msg = "\n========== Index Manager Initialized ==========\n";
    printf(msg);  
    return RC_OK;
}


RC shutdownIndexManager() {
    // Index manager shutdown triggered
    puts("Shutting down index manager...");
    return RC_OK;
}


RC createBtree(char *idxId, DataType keyType, int n) {
    // --- Initialization ---
    treeHandle = malloc(sizeof(BTreeHandle));
    sHandle = malloc(sizeof(BT_ScanHandle));
    btreeMgmtData = malloc(sizeof(treeMgmtData));
    sMgmtData = malloc(sizeof(scanMgmtData));

    treeHandle->mgmtData = btreeMgmtData;
    sHandle->mgmtData = sMgmtData;

    btreeMgmtData->bm = MAKE_POOL();
    btreeMgmtData->ph = MAKE_PAGE_HANDLE();

    // --- File Setup ---
    if (createPageFile(idxId) != RC_OK) return RC_ERROR;
    if (openPageFile(idxId, &btreeMgmtData->fh) != RC_OK) return RC_ERROR;

    // --- Metadata Preparation ---
    fileMetaData *meta = &btreeMgmtData->fmd;
    meta->rootPageNumber = 1;
    meta->maxEntriesPerPage = n;
    meta->numEntries = 0;
    meta->numNodes = 1;

    // --- Initialize Buffer ---
    if (initBufferPool(btreeMgmtData->bm, idxId, 10, RS_FIFO, NULL) != RC_OK) return RC_ERROR;
    ensureCapacity(2, &btreeMgmtData->fh);

    // --- Write Meta to Disk ---
    char *metaPage = NULL;
    allocateSpaceForData(&metaPage);
    prepareWritableMetaData(meta, metaPage);
    writePageData(btreeMgmtData->bm, btreeMgmtData->ph, metaPage, 0);
    deallocateSpace(&metaPage);

    // --- Setup & Write Root Page ---
    pageData rootNode;
    rootNode.pageNumber = meta->rootPageNumber;
    rootNode.leaf = 1;
    rootNode.numEntries = 0;
    rootNode.parentNode = -1;

    char *rootPage = NULL;
    allocateSpaceForData(&rootPage);
    prepareWritablePageData(&rootNode, rootPage);
    writePageData(btreeMgmtData->bm, btreeMgmtData->ph, rootPage, rootNode.pageNumber);
    deallocateSpace(&rootPage);

    // --- Cleanup ---
    shutdownBufferPool(btreeMgmtData->bm);

    return RC_OK;
}

RC openBtree(BTreeHandle **tree, char *idxId) {
    // Step 1: Open page file and initialize necessary handles
    if (openPageFile(idxId, &btreeMgmtData->fh) != RC_OK) {
        return RC_FILE_NOT_FOUND;
    }

    btreeMgmtData->bm = MAKE_POOL();
    btreeMgmtData->ph = MAKE_PAGE_HANDLE();

    RC initStatus = initBufferPool(btreeMgmtData->bm, idxId, 10, RS_FIFO, NULL);
    if (initStatus != RC_OK) {
        return initStatus;
    }

    // Step 2: Read metadata from page 0
    fileMetaData tempMeta;
    readFileMetaData(btreeMgmtData->bm, btreeMgmtData->ph, &tempMeta, 0);

    // Step 3: Copy metadata manually to avoid structural copying
    btreeMgmtData->fmd.rootPageNumber     = tempMeta.rootPageNumber;
    btreeMgmtData->fmd.maxEntriesPerPage  = tempMeta.maxEntriesPerPage;
    btreeMgmtData->fmd.numEntries         = tempMeta.numEntries;
    btreeMgmtData->fmd.numNodes           = tempMeta.numNodes;
    btreeMgmtData->fmd.keyType            = tempMeta.keyType;

    // Step 4: Set up tree handle with provided index ID
    treeHandle->idxId    = idxId;
    // Assign key type from metadata to the tree handle
    (*treeHandle).keyType = tempMeta.keyType;

    // Link management data to the handle
    (*treeHandle).mgmtData = btreeMgmtData;

    // Return the fully initialized tree handle
    *tree = treeHandle;

    // Indicate success
    return RC_OK;

}



RC closeBtree(BTreeHandle *tree) {
    
    treeMgmtData *mgmt = (treeMgmtData *) tree->mgmtData;
    BM_BufferPool *bm = mgmt->bm;
    BM_PageHandle *ph = mgmt->ph;

    // Step 1: Write current metadata to page 0
    char *dataString = NULL;
    allocateSpaceForData(&dataString);
    prepareWritableMetaData(&(btreeMgmtData->fmd), dataString);
    writePageData(bm, ph, dataString, 0);
    deallocateSpace(&dataString);

    // Step 2: Properly shut down and clean up
    shutdownBufferPool(bm);
    closePageFile(&(mgmt->fh));

    // Ensure all file buffers are flushed
    fflush(NULL);
    _fcloseall();

    // Step 3: Free memory associated with BTree and management data
    if (bm) free(bm);
    if (ph) free(ph);
    free(tree->mgmtData);
    free(tree);

    // Reset global reference
    btreeMgmtData = NULL;

    return RC_OK;
}


RC deleteBtree (char *idxId){
    int result = remove(idxId);
    if(result == 0){
        printf("✅ File '%s' deleted successfully.\n", idxId);
        return RC_OK;
    } else {
        perror("❌ Error deleting file");
        printf("⚠️  File '%s' could not be deleted. Errno = %d\n", idxId, errno);
        return RC_FILE_NOT_FOUND;
    }
}


// access information about a b-tree
RC getNumNodes(BTreeHandle *tree, int *result) {
    treeMgmtData *mgmt = (treeMgmtData *)tree->mgmtData;
    if (result != NULL && mgmt != NULL) {
        *result = mgmt->fmd.numNodes;
        return RC_OK;
    }
    return RC_ERROR;
}


RC getNumEntries(BTreeHandle *tree, int *result) {
    if (tree && tree->mgmtData && result) {
        treeMgmtData *btreeData = (treeMgmtData *)tree->mgmtData;
        *result = btreeData->fmd.numEntries;
        return RC_OK;
    }
    return RC_ERROR;
}

RC getKeyType(BTreeHandle *tree, DataType *result) {
    if (tree && result) {
        *result = ((treeMgmtData *)tree->mgmtData)->fmd.keyType;
        return RC_OK;
    }
    return RC_ERROR;
}


RC findKey(BTreeHandle *tree, Value *key, RID *result) {
    // Extract necessary management structures
    
    treeMgmtData *mgmt = (treeMgmtData *)tree->mgmtData;
    BM_BufferPool *bm = mgmt->bm;
    BM_PageHandle *ph = mgmt->ph;
    
    int rootPageNumber;
    rootPageNumber = mgmt->fmd.rootPageNumber;

    // Load the root page into local memory
    pageData rootPage;
    readPageData(bm, ph, &rootPage, rootPageNumber);

    pageData leafPage = findPageToInsertNewEntry(bm, ph, rootPage, key->v.intV);


    int keyVal = key->v.intV;
    for (int i = 0; i < leafPage.numEntries; i++) {
        if (leafPage.keys[i] == keyVal) {
            // Decode pointer to retrieve page and slot
            float encodedPtr = leafPage.pointers[i];
            int encodedInt = (int)(encodedPtr * 10 + 0.5);  // Safer rounding

            result->page = encodedInt / 10;
            result->slot = encodedInt % 10;

            return RC_OK;
        }
    }

    // Key not found in the tree
    return RC_IM_KEY_NOT_FOUND;
}

RC deleteKeyAndPointerFromLeaf(pageData *page, int key) {
    // Temporary containers for key and pointer updates
    int tempKeys[5];
    float tempPointers[5];

// Initialize arrays to zero
    for (int i = 0; i < 5; i++) {
        tempKeys[i] = 0;
        tempPointers[i] = 0.0f;
    }

// Flags and counters for deletion logic
    int found = 0;
    int newIndex = 0;

    for (int i = 0; i < page->numEntries; i++) {
        if (page->keys[i] == key && !found) {
            found = 1; // Skip this key and pointer
            continue;
        }
        tempKeys[newIndex] = page->keys[i];
        tempPointers[newIndex] = page->pointers[i];
        newIndex++;
    }

    if (!found)
        return RC_IM_KEY_NOT_FOUND;

    tempPointers[newIndex] = -1;

    // Update the page with the filtered keys and pointers
    page->numEntries -= 1;
    int j = 0;
    while (j < newIndex) {
    page->keys[j] = tempKeys[j];
    page->pointers[j] = tempPointers[j];
    j++;
    }

    page->pointers[newIndex] = tempPointers[newIndex];

    return RC_OK;
}


RC readFileMetaData(BM_BufferPool *bm, BM_PageHandle *ph, fileMetaData *fmd, int pageNumber) {
    // Load metadata page into buffer
    if (pinPage(bm, ph, pageNumber) != RC_OK) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    char *cursor = ph->data;

    // Sequentially extract each metadata field after skipping initial delimiters
    if (*cursor == '$') cursor++;
    fmd->rootPageNumber = getDataBeforeSeparatorForInt(&cursor, '$');

    if (*cursor == '$') cursor++;
    fmd->numNodes = getDataBeforeSeparatorForInt(&cursor, '$');

    if (*cursor == '$') cursor++;
    fmd->numEntries = getDataBeforeSeparatorForInt(&cursor, '$');

    if (*cursor == '$') cursor++;
    fmd->maxEntriesPerPage = getDataBeforeSeparatorForInt(&cursor, '$');

    if (*cursor == '$') {
        cursor++;
    }
    
    // Parse key type from cursor
    fmd->keyType = getDataBeforeSeparatorForInt(&cursor, '$');
    
    // Unpin the current page from the buffer
    unpinPage(bm, ph);
}

RC insertKey(BTreeHandle *tree, Value *key, RID rid) {
    treeMgmtData *mgmt = (treeMgmtData *)tree->mgmtData;
    int rootPageNumber = mgmt->fmd.rootPageNumber;
    int maxEntries = mgmt->fmd.maxEntriesPerPage;
    int currentNumberOfNodes = mgmt->fmd.numNodes;

    BM_BufferPool *bm = mgmt->bm;
    BM_PageHandle *ph = mgmt->ph;
    SM_FileHandle fh = mgmt->fh;

    // Load root and locate insertion page
    
    pageData rootPageData;
    readPageData(bm, ph, &rootPageData, rootPageNumber);

    pageData pageToInsert = findPageToInsertNewEntry(bm, ph, rootPageData, key->v.intV);

    int success = addNewKeyAndPointerToLeaf(&pageToInsert, key->v.intV, rid);

    switch (success) {
        case RC_IM_KEY_ALREADY_EXISTS:
            return RC_IM_KEY_ALREADY_EXISTS;
        default:
            break;
    }
    
    // Use switch-case instead of if-else
    int caseType = (pageToInsert.numEntries > maxEntries) ? 1 : 0;

    switch (caseType) {
        case 1: {
            // SPLIT required
        
            int *keysForNewNode = (int *)malloc(10 * sizeof(int));
            float *childrenForNewNode = (float *)malloc(10 * sizeof(float));
            int count = 0;
        
            size_t i = (int)ceil((pageToInsert.numEntries) / 2) + 1;
            while (i < pageToInsert.numEntries) {
                keysForNewNode[count] = pageToInsert.keys[i];
                childrenForNewNode[count] = pageToInsert.pointers[i];
                count++;
                i++;
            }
            childrenForNewNode[count] = -1;


            int *keysForOldNode       = malloc(10 * sizeof(int));
            float *childrenForOldNode = malloc(10 * sizeof(float));

            // Reset counter for re-use
            count = 0;
        
            i = 0;
            int limit = (int)ceil((pageToInsert.numEntries) / 2);
            while (i <= limit) {
                keysForOldNode[count] = pageToInsert.keys[i];
                childrenForOldNode[count] = pageToInsert.pointers[i];
                count++;
                i++;
            }
            // Terminate pointer array for old (left) child
            childrenForOldNode[count] = -1;

// Ensure space for two new pages and update tree metadata
            ensureCapacity(currentNumberOfNodes + 2, &fh);
            currentNumberOfNodes++;
            // Increment the node count in file metadata
            treeMgmtData *meta = (treeMgmtData *)tree->mgmtData;
            meta->fmd.numNodes = meta->fmd.numNodes + 1;

// Prepare the new right child node
            pageData pRightChild;
            memset(&pRightChild, 0, sizeof(pageData));  // Optional safety initialization

            pRightChild.pageNumber   = currentNumberOfNodes;
            pRightChild.leaf         = 1;
            pRightChild.numEntries   = (int)floor((maxEntries + 1) / 2.0);
            pRightChild.keys         = keysForNewNode;
            pRightChild.pointers     = childrenForNewNode;

            pRightChild.parentNode = (pageToInsert.parentNode == -1) ? 3 : pageToInsert.parentNode;

           // Finalize and write right child to disk
            char *dataString;
            allocateSpaceForData(&dataString);
            prepareWritablePageData(&pRightChild, dataString);
            writePageData(bm, ph, dataString, pRightChild.pageNumber);
            deallocateSpace(&dataString);

            // Prepare left child metadata
            pageData pLeftChild;
            pLeftChild.keys = keysForOldNode;
            pLeftChild.pointers = childrenForOldNode;
            pLeftChild.pageNumber = pageToInsert.pageNumber;
            pLeftChild.leaf = 1;
            pLeftChild.numEntries = (int)ceil((maxEntries + 1) / 2) + 1;

// Assign parentNode conditionally
            pLeftChild.parentNode = pageToInsert.parentNode;
            if (pLeftChild.parentNode == -1) {
                pLeftChild.parentNode = 3;
            }

// Write left child to disk
            allocateSpaceForData(&dataString);
            prepareWritablePageData(&pLeftChild, dataString);
            writePageData(bm, ph, dataString, pLeftChild.pageNumber);
            deallocateSpace(&dataString);

// Record page numbers
            float left = pLeftChild.pageNumber;
            float right = pRightChild.pageNumber;
            int pagenumber = pageToInsert.parentNode;

            keyData kd = {
                .key  = pRightChild.keys[0],
                .left = pLeftChild.pageNumber,
                .right = pRightChild.pageNumber
            };
            
            insertPropagateUp(tree, pageToInsert.parentNode, kd);
            
            break;
        }

        case 0: {
            // NO SPLIT
            char *dataString;
            allocateSpaceForData(&dataString);
        
            prepareWritablePageData(&pageToInsert, dataString);
            writePageData(bm, ph, dataString, pageToInsert.pageNumber);
        
            deallocateSpace(&dataString);
            break;
        }
    }

    ((treeMgmtData*)tree->mgmtData)->fmd.numEntries++;
    forceFlushPool(bm);

    return RC_OK;
}


RC readPageData(BM_BufferPool *bm, BM_PageHandle *ph, pageData *pd, int pageNumber) {
    // Pin the required page
    if (pinPage(bm, ph, pageNumber) != RC_OK) {
        return RC_ERROR;
    }

    char *cursor = ph->data;

    if (*cursor == '$') cursor++;
    pd->leaf = getDataBeforeSeparatorForInt(&cursor, '$');

    if (*cursor == '$') cursor++;
    pd->numEntries = getDataBeforeSeparatorForInt(&cursor, '$');

    if (*cursor == '$') cursor++;
    pd->parentNode = getDataBeforeSeparatorForInt(&cursor, '$');

    if (*cursor == '$') cursor++;
    pd->pageNumber = getDataBeforeSeparatorForInt(&cursor, '$');

    if (*cursor == '$') cursor++;

    int total = pd->numEntries;
    int *keysArray = malloc(total * sizeof(int));
    float *pointersArray = malloc((total + 1) * sizeof(float));

    if (total > 0) {
        for (int i = 0; i < total; i++) {
            pointersArray[i] = getDataBeforeSeparatorForFloat(&cursor, '$');
            if (*cursor == '$') cursor++;
            keysArray[i] = getDataBeforeSeparatorForInt(&cursor, '$');
            if (*cursor == '$') cursor++;
        }
        // Final child pointer
        *(pointersArray + total) = getDataBeforeSeparatorForFloat(&cursor, '$');
    }

    // Assign decoded arrays to the page descriptor
    pd->keys     = keysArray;
    pd->pointers = pointersArray;

    // Finish reading by unpinning the current page
    if (bm && ph) {
        unpinPage(bm, ph);
    }

    // Return status to indicate success
    return RC_OK;

}

RC writePageData(BM_BufferPool *bm, BM_PageHandle *ph, char *content, int pageNumber) {
    // Load the target page into memory
    if (pinPage(bm, ph, pageNumber) != RC_OK) {
        return RC_ERROR;
    }

    // Clear previous data and write new content
    for (int i = 0; i < 100; i++) {
        ph->data[i] = '\0';
    }

    snprintf(ph->data, 100, "%s", content);

    // Mark the page as modified and release it
    markDirty(bm, ph);
    unpinPage(bm, ph);

    return RC_OK;
}

RC deleteKey(BTreeHandle *tree, Value *key) {
    // Step 1: Retrieve B-tree components
    treeMgmtData *mgmt = (treeMgmtData *)tree->mgmtData;
    int rootPageNumber = mgmt->fmd.rootPageNumber;
    BM_BufferPool *bm = mgmt->bm;
    BM_PageHandle *ph = mgmt->ph;

    // Step 2: Determine root page and load it
    
    pageData rootNode;
    readPageData(bm, ph, &rootNode, rootPageNumber);

    // Step 3: Locate page containing the key to delete
    pageData targetPage = findPageToInsertNewEntry(bm, ph, rootNode, key->v.intV);

    // Step 4: Remove the key from target leaf page
    int status = deleteKeyAndPointerFromLeaf(&targetPage, key->v.intV);

    switch (status) {
        case RC_IM_KEY_NOT_FOUND:
            return RC_IM_KEY_NOT_FOUND;
        default:
            break;
    }
    
    // Step 5: Persist updated page content
    char *buffer;
    buffer = NULL;

    allocateSpaceForData(&buffer);
    if (buffer) {
        prepareWritablePageData(&targetPage, buffer);
        writePageData(bm, ph, buffer, targetPage.pageNumber);
        deallocateSpace(&buffer);
    }

    // Step 6: Confirm success
    return RC_OK;
}


RC findLeafPages(pageData root, BM_BufferPool *bm, BM_PageHandle *ph, int *leafPages) {
    switch (root.leaf ? 1 : 0) {
        case 1:
            leafPages[count++] = root.pageNumber;
            return RC_OK;
        case 0:
            // Do nothing here; continue to recursion if needed
            break;
    }
    

    // Recursively explore all child nodes
    for (int i = 0; i <= root.numEntries; i++) {
        int childPageNum = (int)root.pointers[i];
        pageData childNode;
        RC status = readPageData(bm, ph, &childNode, childPageNum);

        if (status != RC_OK)
            return status;

        findLeafPages(childNode, bm, ph, leafPages);
    }

    return RC_OK;
}

RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle) {
    // Step 1: Allocate memory for scan handle and management data
    sHandle = (BT_ScanHandle *)malloc(sizeof(BT_ScanHandle));
    sMgmtData = (scanMgmtData *)malloc(sizeof(scanMgmtData));

    // Step 2: Access B-tree metadata
    treeMgmtData *mgmt = (treeMgmtData *)tree->mgmtData;
    int rootPageNumber = mgmt->fmd.rootPageNumber;

    BM_BufferPool *bm = mgmt->bm;
    BM_PageHandle *ph = mgmt->ph;

    // Step 3: Load root page data
    pageData root;
    readPageData(bm, ph, &root, rootPageNumber);

    // Step 4: Allocate array and reset count
    int *leafPageNumbers = (int *)malloc(sizeof(int) * 100);
    count = 0;

    // Step 5: Perform DFS to gather all leaf pages
    findLeafPages(root, bm, ph, leafPageNumbers);

    // Step 6: Setup scanning information
    sMgmtData->leafPages = leafPageNumbers;
    sMgmtData->numOfLeafPages = count;
    sMgmtData->currentPage = leafPageNumbers[0];
    sMgmtData->nextPagePosInLeafPages = 1;
    sMgmtData->currentPosInPage = 0;
    sMgmtData->isCurrentPageLoaded = 1;

    // Step 7: Load data of first leaf page
    pageData firstLeaf;
    readPageData(bm, ph, &firstLeaf, sMgmtData->currentPage);
    sMgmtData->currentPageData = firstLeaf;

    // Step 8: Link structures and return handle
    sHandle->tree = tree;
    sHandle->mgmtData = sMgmtData;
    *handle = sHandle;

    return RC_OK;
}


RC nextEntry(BT_ScanHandle *handle, RID *result) {
    treeMgmtData *mgmt = (treeMgmtData *)handle->tree->mgmtData;
    BM_BufferPool *bm = mgmt->bm;
    BM_PageHandle *ph = mgmt->ph;
    scanMgmtData *smdata = handle->mgmtData;

    int entries = smdata->currentPageData.numEntries;
    int pos = smdata->currentPosInPage;

    // If we’ve reached the end of the current leaf
    if (!(pos < entries)) {
        // No more leaf pages to move to
        if (smdata->nextPagePosInLeafPages != -1) {
            smdata->currentPage = smdata->leafPages[smdata->nextPagePosInLeafPages];
            smdata->isCurrentPageLoaded = 0;

            if (++smdata->nextPagePosInLeafPages >= smdata->numOfLeafPages) {
                smdata->nextPagePosInLeafPages = -1;
            }
        } else {
            return RC_IM_NO_MORE_ENTRIES;
        }
    }

    
    switch (smdata->isCurrentPageLoaded) {
        case 0: {
            // Load the leaf page data into the scan management structure
            pageData leaf;
            readPageData(bm, ph, &leaf, smdata->currentPage);
    
            // Initialize scan state for the new page
            smdata->currentPageData      = leaf;
            smdata->currentPosInPage     = 0;
            smdata->isCurrentPageLoaded  = 1;
        }
        break;
    
        case 1:
        default:
            // No operation required if the page is already loaded
            break;
    }
    
    
    // Decode the pointer value into RID format
    float rawEncoded = smdata->currentPageData.pointers[smdata->currentPosInPage];
    int decoded = (int)(rawEncoded * 10 + 0.5);
    
    result->page = decoded / 10;
    result->slot = decoded % 10;

    smdata->currentPosInPage++;

    return RC_OK;
}


RC closeTreeScan(BT_ScanHandle *handle) {
    if (handle != NULL) {
        if (handle->mgmtData != NULL) {
            free(handle->mgmtData);
        }
        free(handle);
        handle = NULL;
    }
    return RC_OK;
}


void printNodeContent(BM_BufferPool *bm, BM_PageHandle *ph, pageData root) {
    
    switch (root.leaf) {
        case 1: {
            char *dataBuffer = NULL;
            allocateSpaceForData(&dataBuffer);
            prepareWritablePageData(&root, dataBuffer);
            printf("\n%s\n", dataBuffer);
            deallocateSpace(&dataBuffer);
            return;
        }
        default:
            break;
    }
    
    // Recursive case: internal node — traverse children
    for (int i = 0; i <= root.numEntries; i++) {
        if (root.pointers[i] != -1) {
            int childPageNum = (int)root.pointers[i];
            pageData childNode;
            childNode.pageNumber = childPageNum;
            readPageData(bm, ph, &childNode, childPageNum);
            printNodeContent(bm, ph, childNode);
        }
    }
}

// debug and test functions
char *printTree(BTreeHandle *tree) {
    treeMgmtData *mgmt = (treeMgmtData *)tree->mgmtData;

    int rootPageNumber = mgmt->fmd.rootPageNumber;
    int maxEntries = mgmt->fmd.maxEntriesPerPage;
    int currentNumberOfNodes = mgmt->fmd.numNodes;

    BM_BufferPool *bm = mgmt->bm;
    BM_PageHandle *ph = mgmt->ph;
    SM_FileHandle fh = mgmt->fh;

    // Load and print root page content
    pageData rootNode;
    readPageData(bm, ph, &rootNode, rootPageNumber);
    printNodeContent(bm, ph, rootNode);

    return "abc";
}

//type = 0 for int and 1 for float
int getDataBeforeSeparatorForInt(char **itr, char sep) {
    char buffer[100] = {0};  // Use stack memory instead of malloc for efficiency
    int index = 0;
    char *cursor = *itr;

    while (*cursor != sep && index < 99) {
        buffer[index++] = *cursor;
        cursor++;
    }

    buffer[index] = '\0';
    *itr = cursor; // move original iterator forward

    return atoi(buffer);
}


float getDataBeforeSeparatorForFloat(char **itr, char sep) {
    char buffer[100] = {0};
    int index = 0;
    char *cursor = *itr;

    while (*cursor != sep && index < 99) {
        buffer[index++] = *cursor;
        cursor++;
    }

    buffer[index] = '\0';
    *itr = cursor;

    return atof(buffer);
}



RC prepareWritableMetaData(fileMetaData *fmd, char *content) {
    // Serialize metadata fields with separators
    snprintf(content, 50, "$%d$%d$%d$%d$%d$",
             fmd->rootPageNumber,
             fmd->numNodes,
             fmd->numEntries,
             fmd->maxEntriesPerPage,
             fmd->keyType);
    return RC_OK;
}

RC prepareWritablePageData(pageData *pd, char *content) {
    snprintf(content, 50, "$%d$%d$%d$%d$", pd->leaf, pd->numEntries, pd->parentNode, pd->pageNumber);

    switch (pd->numEntries > 0) {
        case 1: {
            char *formatted = NULL;
            allocateSpaceForData(&formatted);
            keyPointerFormattedData(pd, formatted);

        // Append to existing content
            strncat(content, formatted, 49 - strlen(content));
            deallocateSpace(&formatted);
            break;
        }
        default:
            break;
    }

    return RC_OK;
}


RC keyPointerFormattedData(pageData *pd, char *data) {
    char *cursor = data;

    for (int i = 0; i < pd->numEntries; i++) {
        int encoded = (int)(pd->pointers[i] * 10 + 0.5);
        int page = encoded / 10;
        int slot = encoded % 10;

        cursor += sprintf(cursor, "%d.%d$", page, slot);
        cursor += sprintf(cursor, "%d$", pd->keys[i]);
    }

    // Write the final child pointer
    sprintf(cursor, "%0.1f$", pd->pointers[pd->numEntries]);

    return RC_OK;
}


RC allocateSpaceForData(char **data) {
    *data = (char *)malloc(50);
    if (*data == NULL) return RC_ERROR;
    memset(*data, 0, 50);
    return RC_OK;
}

RC deallocateSpace(char **data) {
    if (*data) {
        free(*data);
        *data = NULL;
    }
    return RC_OK;
}

RC addNewKeyAndPointerToNonLeaf(pageData *page, keyData kd) {
    // Step 1: Allocate memory for new keys and child pointers
    int allocationSize = 10;
    int *newKeys = (int *)malloc(allocationSize * sizeof(int));
    float *newChildren = (float *)malloc(allocationSize * sizeof(float));

    int insertPos = 0;

    // Step 2: Copy existing keys and children until insertion point
    while (insertPos < page->numEntries && kd.key > page->keys[insertPos]) {
        newKeys[insertPos] = page->keys[insertPos];
        newChildren[insertPos] = page->pointers[insertPos];
        insertPos++;
    }

    // Step 3: Check for duplicate key using switch-case
    int isDuplicate = (insertPos < page->numEntries && kd.key == page->keys[insertPos]) ? 1 : 0;

    switch (isDuplicate) {
        case 1:
            return RC_IM_KEY_ALREADY_EXISTS;

        case 0: {
            // Insert the new key and its associated left child pointer
            newChildren[insertPos] = kd.left;
            newKeys[insertPos] = kd.key;

            // Move ahead and copy key from original page at current insertPos
            int backupKey = page->keys[insertPos];
            insertPos++;

            newChildren[insertPos] = kd.right;
            newKeys[insertPos] = backupKey;
            insertPos++;
            break;
        }
    }

    // Step 4: Copy remaining keys and child pointers
    for (int j = insertPos; j < page->numEntries + 2; j++) {
        newKeys[j] = page->keys[j - 1];
        newChildren[j] = page->pointers[j - 1];
    }

    // Step 5: Set final terminator for children array
    newChildren[page->numEntries + 2] = -1;

    // Step 6: Clean up old memory and update the page
    if (page->keys) free(page->keys);
    if (page->pointers) free(page->pointers);

    page->keys = newKeys;
    page->pointers = newChildren;
    page->numEntries += 1;

    return RC_OK;
}

pageData findPageToInsertNewEntry(BM_BufferPool *bm, BM_PageHandle *ph, pageData root, int key) {
    // Base case: if the current node is a leaf, return it directly
    if (root.leaf) {
        return root;
    }

    pageData childNode;
    int pageNum = -1;

    // Case 1: key is smaller than the smallest key
    if (key < root.keys[0]) {
        pageNum = (int)(root.pointers[0] * 10 + 0.5) / 10;
    }
    // Case 2: key falls between two internal keys
    else {
        int i = 0;
        while (i < root.numEntries - 1) {
            if (key >= root.keys[i] && key < root.keys[i + 1]) {
                pageNum = (int)(root.pointers[i + 1] * 10 + 0.5) / 10;
                break;
            }
            i++;
        }

        // Case 3: key is greater than or equal to all internal keys
        if (pageNum == -1) {
            pageNum = (int)(root.pointers[root.numEntries] * 10 + 0.5) / 10;
        }
    }

    readPageData(bm, ph, &childNode, pageNum);
    return findPageToInsertNewEntry(bm, ph, childNode, key);
}


RC addNewKeyAndPointerToLeaf(pageData *page, int key, RID rid) {
    int *updatedKeys;
    float *updatedPointers;

    updatedKeys = malloc(10 * sizeof(int));
    updatedPointers = malloc(11 * sizeof(float));
    
    float childPtr = rid.page + rid.slot * 0.1;
    int insertIndex = 0;

    // Find the correct position to insert the key
    while (insertIndex < page->numEntries && key > page->keys[insertIndex]) {
        updatedKeys[insertIndex] = page->keys[insertIndex];
        updatedPointers[insertIndex] = page->pointers[insertIndex];
        insertIndex++;
    }

    // Check for duplicate key
    if (insertIndex < page->numEntries && key == page->keys[insertIndex]) {
        free(updatedKeys);
        free(updatedPointers);
        return RC_IM_KEY_ALREADY_EXISTS;
    }

    // Insert the new key and pointer
    updatedKeys[insertIndex] = key;
    updatedPointers[insertIndex] = childPtr;

    // Shift remaining keys and pointers after insertion point
    for (int i = insertIndex; i < page->numEntries; i++) {
        updatedKeys[i + 1] = page->keys[i];
        updatedPointers[i + 1] = page->pointers[i];
    }

    // Mark the end of the child pointer list
    *(updatedPointers + (page->numEntries + 1)) = -1;

// Release memory previously used by the page
    if (page->keys != NULL) {
        free(page->keys);
    }
    if (page->pointers != NULL) {
        free(page->pointers);
    }

    // Assign updated arrays to the page
    page->pointers = updatedPointers;
    page->keys = updatedKeys;

    // Increment entry count
    page->numEntries = page->numEntries + 1;

    // Return operation status
    return RC_OK;
}

RC insertPropagateUp(BTreeHandle *tree, int pageNumber, keyData kd) {
    treeMgmtData *mgmt = (treeMgmtData *)tree->mgmtData;
    int maxEntries = mgmt->fmd.maxEntriesPerPage;
    int currentNumberOfNodes = mgmt->fmd.numNodes;

    BM_BufferPool *bm = mgmt->bm;
    BM_PageHandle *ph = mgmt->ph;
    SM_FileHandle fh = mgmt->fh;

    // Case 1: No parent, need to create new root
    if (pageNumber == -1) {
        ensureCapacity(currentNumberOfNodes + 2, &fh);

        pageData newNode;
        newNode.pageNumber = currentNumberOfNodes + 1;
        newNode.numEntries = 1;
        newNode.leaf = 0;
        newNode.parentNode = -1;

        int *keys = malloc(sizeof(int) * 10);
        float *ptrs = malloc(sizeof(float) * 11);
        keys[0] = kd.key;

        // Set pointer values from keyData
        *(ptrs + 0) = kd.left;
        *(ptrs + 1) = kd.right;

        // Link allocated memory to the newNode structure
        newNode.pointers = ptrs;
        newNode.keys = keys;

        // Update metadata: total number of nodes and root page reference
        mgmt->fmd.numNodes = mgmt->fmd.numNodes + 1;
        mgmt->fmd.rootPageNumber = newNode.pageNumber;

        // Allocate and write page content
        char *buffer = NULL;
        allocateSpaceForData(&buffer);
        prepareWritablePageData(&newNode, buffer);
        writePageData(bm, ph, buffer, newNode.pageNumber);


        updateParentInDownChildNodes(tree, newNode);
        return RC_OK;
    }

    // Case 2: Parent exists
    pageData parentPage;
    readPageData(bm, ph, &parentPage, pageNumber);
    int insertStatus = addNewKeyAndPointerToNonLeaf(&parentPage, kd);

    // If no overflow, write and return
    if (parentPage.numEntries <= maxEntries) {
        char *buffer;
        allocateSpaceForData(&buffer);
        prepareWritablePageData(&parentPage, buffer);
        writePageData(bm, ph, buffer, parentPage.pageNumber);
        deallocateSpace(&buffer);
        return RC_OK;
    }

    // Case 3: Overflow → Split
    int splitPoint = (int)ceil(parentPage.numEntries / 2.0);
    int *leftKeys = malloc(sizeof(int) * 10);
    int *rightKeys = malloc(sizeof(int) * 10);
    float *leftPtrs = malloc(sizeof(float) * 11);
    float *rightPtrs = malloc(sizeof(float) * 11);

    int l = 0, r = 0;
    for (int i = 0; i < parentPage.numEntries + 1; i++) {
        if (i <= splitPoint) {
            leftKeys[l] = parentPage.keys[i];
            leftPtrs[l++] = parentPage.pointers[i];
        } else {
            rightKeys[r] = parentPage.keys[i];
            rightPtrs[r++] = parentPage.pointers[i];
        }
    }
    leftPtrs[l] = parentPage.pointers[splitPoint + 1];
    rightPtrs[r] = parentPage.pointers[parentPage.numEntries + 1];

    ensureCapacity(currentNumberOfNodes + 2, &fh);
    mgmt->fmd.numNodes++;

    // Build left child (overwrite original)
    pageData pLeftChild;
    pLeftChild.pageNumber = parentPage.pageNumber;
    pLeftChild.leaf = 0;
    pLeftChild.parentNode = parentPage.parentNode;
    pLeftChild.numEntries = l - 1;
    pLeftChild.keys = leftKeys;
    pLeftChild.pointers = leftPtrs;

    // Build right child (new page)
    pageData pRightChild;
    pRightChild.pageNumber = currentNumberOfNodes + 1;
    pRightChild.leaf = 0;
    pRightChild.parentNode = parentPage.parentNode;
    pRightChild.numEntries = r;
    pRightChild.keys = rightKeys;
    pRightChild.pointers = rightPtrs;

    // Write both children
    char *buffer;
    allocateSpaceForData(&buffer);
    prepareWritablePageData(&pLeftChild, buffer);
    writePageData(bm, ph, buffer, pLeftChild.pageNumber);
    deallocateSpace(&buffer);

    allocateSpaceForData(&buffer);
    prepareWritablePageData(&pRightChild, buffer);
    writePageData(bm, ph, buffer, pRightChild.pageNumber);
    deallocateSpace(&buffer);

    // Prepare propagation key for next level
    keyData nextKey;
    nextKey.key = parentPage.keys[splitPoint];
    nextKey.left = (float)pLeftChild.pageNumber;
    nextKey.right = (float)pRightChild.pageNumber;

    updateParentInDownChildNodes(tree, pRightChild);

    // Recursive propagation
    insertPropagateUp(tree, parentPage.parentNode, nextKey);

    return RC_OK;
}

RC updateParentInDownChildNodes(BTreeHandle *tree, pageData node) {
    treeMgmtData *mgmt = (treeMgmtData *)tree->mgmtData;
    BM_BufferPool *bm = mgmt->bm;
    BM_PageHandle *ph = mgmt->ph;
    SM_FileHandle fh = mgmt->fh;

    int totalChildren = node.numEntries + 1;

    for (int i = 0; i < totalChildren; i++) {
        int childPage = (int)node.pointers[i];
        pageData childNode;

        // Load child page and update its parent reference
        readPageData(bm, ph, &childNode, childPage);
        childNode.parentNode = node.pageNumber;

        // Serialize and write updated child node
        char *buffer = NULL;
        allocateSpaceForData(&buffer);
        prepareWritablePageData(&childNode, buffer);
        writePageData(bm, ph, buffer, childNode.pageNumber);
        deallocateSpace(&buffer);
    }

    return RC_OK;
}
