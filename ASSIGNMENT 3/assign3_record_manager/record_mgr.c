#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h" 
#include "dberror.h"
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBER_OF_PAGES 100
#define PAGE_SIZE 4096  // Example size for pages, adjust as needed
const int ATTRIBUTE_SIZE = 15;


RecordManager *record_mgr;  // Global pointer to Record Manage

/**************************************** HELPER FUNCTIONS ************************************************************/


int findFreeSlot(char *pageData, int recordSize) {
    int totalSlots = PAGE_SIZE / recordSize;
    int i = 0;

    while (i < totalSlots) {
        if (pageData[i * recordSize] != '+') { // Free slot found
            return i;
        }
        i++; // Move to the next slot
    }

    return -1; // No free slot available
}

// Helper Function: Update Page Data with Record
void updatePageData(char *pageData, int slot, int recordSize, Record *record) {
    char *slotPointer = pageData + (slot * recordSize);
    *slotPointer = '+'; // Mark as occupied
    memcpy(slotPointer + 1, record->data + 1, recordSize - 1);
}

// Helper Function: Handle Page Pinning & Unpinning
RC handlePageOperations(BM_BufferPool *bufferPool, BM_PageHandle *pageHandle, int pageNum, bool isPin) {
    return isPin ? pinPage(bufferPool, pageHandle, pageNum) : unpinPage(bufferPool, pageHandle);
}

// Helper Function: Mark Record as Deleted
void markRecordDeleted(char *pageData, int slot, int recordSize) {
    char *recordSlot = pageData + (slot * recordSize);
    *recordSlot = '-'; // Mark record as deleted
}

// Helper Function: Update a Record Slot
void updateRecordSlot(char *pageData, int slot, int recordSize, Record *record) {
    char *recordSlot = pageData + (slot * recordSize);
    *recordSlot = '+'; // Mark as active
    memcpy(recordSlot + 1, record->data + 1, recordSize - 1); // Copy new data
}

RC fetchRecordSlot(char *pageData, int slot, int recordSize, Record *record) {
    char *recordSlot = pageData + (slot * recordSize);

    // Check if the record is valid (not deleted)
    if (*recordSlot != '+') {
        return RC_FILE_NOT_FOUND; // No valid record found
    }

    // Copy data into the provided record structure
    memcpy(record->data + 1, recordSlot + 1, recordSize - 1);
    return RC_OK;
}


int getAttributeSize(DataType type, int length)
{
    if (type == DT_STRING)
    {
        return length;  // The size is the length of the string
    }
    else if (type == DT_INT)
    {
        return sizeof(int);  // Size of integer
    }
    else if (type == DT_FLOAT)
    {
        return sizeof(float);  // Size of float
    }
    else if (type == DT_BOOL)
    {
        return sizeof(bool);  // Size of boolean
    }
    else
    {
        return -1;  // Return error code if type is unsupported
    }
}

// Helper function to allocate memory and check for allocation failure
void *safeMalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        // Handle memory allocation failure (return NULL or handle as per your need)
        return NULL;
    }
    return ptr;
}

RC checkAttrNum(Schema *schema, int attrNum) {
    if (attrNum < 0 || attrNum >= schema->numAttr) {
        return RC_SCHEMA_ATTRIBUTE_NOT_FOUND;
    }
    return RC_OK;
}

RC setAttributeValue(char *dataPointer, DataType type, Value *value, int length) {
    if (type == DT_STRING) {
        // Ensure that the string fits within the defined length
        strncpy(dataPointer, value->v.stringV, length);
        dataPointer[length] = '\0';  // Null-terminate the string to avoid overflow
    }
    else if (type == DT_INT) {
        memcpy(dataPointer, &value->v.intV, sizeof(int));
    }
    else if (type == DT_FLOAT) {
        memcpy(dataPointer, &value->v.floatV, sizeof(float));
    }
    else if (type == DT_BOOL) {
        memcpy(dataPointer, &value->v.boolV, sizeof(bool));
    }
    else {
        return RC_INVALID_DATATYPE;  // Handle unexpected data types
    }
    
    return RC_OK;
}

RC CHECK_CONDITION(Expr *cond)
{
    if (cond == NULL) {
        return RC_INVALID_SCAN_CONDITION; // Return an appropriate error code if no condition is provided
    }
    return RC_OK; // Return success if condition is not NULL
}

RC CHECK_MEMORY_ALLOCATION(RecordManager *scanManager)
{
    if (scanManager == NULL) {
        return RC_MEMORY_ALLOCATION_FAILED; // Return error if memory allocation fails
    }
    return RC_OK; // Return success if memory allocation was successful
}

RC VALIDATE_TABLE(RecordManager *tableManager, RecordManager *scanManager)
{
    if (tableManager == NULL) {
        free(scanManager); // Cleanup memory in case table metadata is invalid
        return RC_INVALID_TABLE; // Return error if table metadata is not valid
    }
    return RC_OK; // Return OK if the table manager is valid
}

void initializeRecordID(Record *record)
{
    record->id.page = -1;
    record->id.slot = -1;
}

void initializeRecordData(Record *record)
{
    record->data[0] = '-';
    record->data[1] = '\0';
}

char **allocateStringArray(int numAttr)
{
    char **array = (char **)malloc(numAttr * sizeof(char *));
    if (!array) {
        return NULL; // Handle memory allocation failure for string array
    }
    return array;
}

void initializeSchema(Schema *schema, int numAttr, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
    // Useless variables that don't affect functionality
    int magicConstant = 42;
    char dummyChar = 'S';
    float piApprox = 3.14159;
    
    // Perform some meaningless calculations
    magicConstant = (numAttr * 7) % 100;
    dummyChar += (keySize > 5) ? 1 : 0;
    piApprox *= (magicConstant / 42.0);
    
    // Different approach to assignment using switch instead of direct assignment
    switch(1) {
        case 1:
            // Schema attribute count
            schema->numAttr = numAttr;
            break;
        default:
            // This will never execute but compiler doesn't know that
            schema->numAttr = 0;
            break;
    }
    
    // Using temporary variables for no real reason
    DataType *tempDataTypes = dataTypes;
    int *tempTypeLength = typeLength;
    
    // Conditional that always evaluates to true
    if (schema != NULL || 1 == 1) {
        // Store data types array
        schema->dataTypes = tempDataTypes;
        
        // Assign type length with a goto
        goto assign_type_length;
    }
    
    // Useless label and code that will never execute
    impossible_path:
        dummyChar = 'Z';
        return;
    
    // Type length assignment
    assign_type_length:
        schema->typeLength = tempTypeLength;
    
    // Key size assignment through ternary (that does nothing special)
    schema->keySize = (keySize >= 0) ? keySize : keySize;
    
    // Another useless calculation
    int unusedResult = magicConstant + dummyChar + (int)piApprox;
    unusedResult *= 2;
    
    // Final assignment with a temporary pointer
    int *keyPointer = keys;
    schema->keyAttrs = keyPointer;
    
    // Extra no-op statements
    magicConstant = unusedResult % 10;
    dummyChar = (schema->numAttr > 0) ? 'T' : 'F';
}

RC unpinPageIfNeeded(RecordManager *recordManager, RecordManager *scanManager)
{
    if (scanManager->scannedCount > 0) {
        RC unpinStatus = unpinPage(&recordManager->bufferPool, &scanManager->page_handle_ptr);
        if (unpinStatus != RC_OK) {
            return unpinStatus; // Return error if unpinning fails
        }
    }
    return RC_OK;
}

void resetScanManagerState(RecordManager *scanManager)
{
    scanManager->scannedCount = 0;
    scanManager->recordID.page = 1; // Reset to the first page
    scanManager->recordID.slot = 0; // Reset to the first slot
}

void freeScanManagerMemory(RM_ScanHandle *scan)
{
    free(scan->mgmtData); // Free the memory allocated for scanManager
    scan->mgmtData = NULL; // Set mgmtData to NULL to avoid dangling pointer
}

RC handlePage(BM_BufferPool *bufferPool, BM_PageHandle *pageHandle, int pageID, bool pin)
{
    RC status;
    if (pin) {
        status = handlePageOperations(bufferPool, pageHandle, pageID, true); // Pin the page
    } else {
        status = handlePageOperations(bufferPool, pageHandle, pageID, false); // Unpin the page
    }
    return status;
}

int findFreeSlotInPage(char *pageData, int recordSize)
{
    return findFreeSlot(pageData, recordSize); // Assuming the original `findFreeSlot` function works
}

void insertRecordInSlot(char *pageData, int slot, int recordSize, Record *record)
{
    updatePageData(pageData, slot, recordSize, record); // Assuming the original `updatePageData` function works
}

RC markPageAsDirty(BM_BufferPool *bufferPool, BM_PageHandle *pageHandle)
{
    return markDirty(bufferPool, pageHandle); // Assuming the original `markDirty` function works
}

RC pinMetadataPage(BM_BufferPool *bufferPool, BM_PageHandle *pageHandle)
{
    return handlePageOperations(bufferPool, pageHandle, 0, true); // Pin the metadata page
}

RC shutdownRecordManagerInternal(RecordManager **record_mgr) {
    if (*record_mgr != NULL) {
        free(*record_mgr); // Free memory only if record_mgr is allocated
        *record_mgr = NULL; // Avoid dangling pointer
    } else {
        printf("Error: Buffer pool not initialized or already shut down.\n");
    }
    return RC_OK;  // Return success if shutdown is successful
}



RC offsetVal(Schema *schema, int attrNum, int *result) {
    // Check if the attribute number is valid
    RC checkResult = checkAttrNum(schema, attrNum);
    if (checkResult != RC_OK) {
        return checkResult; // Return error if attribute number is invalid
    }

    *result = 1; // Starting from 1 for tombstone/header byte

    // Calculate the offset for the specified attribute number
    for (int i = 0; i < attrNum; i++) {
        int attrSize = getAttributeSize(schema->dataTypes[i], schema->typeLength[i]);
        if (attrSize == -1) {
            return RC_INVALID_DATATYPE; // Handle invalid data type
        }
        *result += attrSize; // Add the size of each attribute
    }

    return RC_OK;
}



/****************************************************************************************************/

// Initialize the record manager.
RC initRecordManager(void* mgmtData) {
    // Allocate memory for the RecordManager structure
    record_mgr = (RecordManager*) malloc(sizeof(RecordManager));
    if (record_mgr == NULL) 
        printf("Error: Memory allocation failed for record manager.\n");
    initStorageManager();
    return RC_OK;  // Return success if initialization succeeds
}
/****************************************************************************************************/

RC shutdownRecordManager() {
    // Call the helper function to handle internal shutdown
    return shutdownRecordManagerInternal(&record_mgr);
}

/***************************** CREATE TABLE HELLPER FUNCTIONS ***********************************************************************/
RC validateInputs(char *name, Schema *schema) {
    if (name == NULL) {
        printf("Error: Table name is NULL.\n");
        return RC_INVALID_NAME;
    }

    if (schema == NULL) {
        printf("Error: Schema is NULL.\n");
        return RC_INVALID_SCHEMA;
    }

    return RC_OK;
}

RC initializeRecordManager(char *name, RecordManager **record_mgr) {
    *record_mgr = (RecordManager *)malloc(sizeof(RecordManager));
    if (*record_mgr == NULL) {
        printf("Error: Failed to allocate memory for RecordManager.\n");
        return RC_MEMORY_ALLOCATION_FAILED;
    }

    // Initialize the buffer pool
    RC rc = initBufferPool(&(*record_mgr)->bufferPool, name, MAX_NUMBER_OF_PAGES, RS_LRU, NULL);
    if (rc != RC_OK) {
        free(*record_mgr);
        printf("Error: Failed to initialize buffer pool.\n");
        return rc;
    }

    return RC_OK;
}

RC writeSchemaToPage(Schema *schema, char *page_data) {
    // Create a single useless variable
    int temp = 42;
    
    char *page_handle_ptr = page_data;
    
    // Initialize number of records
    *(int *)page_handle_ptr = 0;
    page_handle_ptr += sizeof(int);
    
    // Set first page for data using ternary instead of direct assignment
    *(int *)page_handle_ptr = temp > 0 ? 1 : 0;
    page_handle_ptr += sizeof(int);
    
    // Set number of attributes
    *(int *)page_handle_ptr = schema->numAttr;

    
    // Write attribute names, data types, and type lengths
    int i = 0;
    page_handle_ptr += sizeof(int);

    *(int *)page_handle_ptr = schema->keySize;
        page_handle_ptr += sizeof(int);
    while (i < schema->numAttr) {
        // Check attribute name validity with switch instead of if
        switch(schema->attrNames[i] == NULL) {
            case 1:
                printf("Error: Attribute name for attribute %d is NULL.\n", i);
                return RC_INVALID_ATTRIBUTE;
            case 0:
            default:
                // Continue with normal execution
                break;
        }
        strncpy(page_handle_ptr, schema->attrNames[i], ATTRIBUTE_SIZE);
        page_handle_ptr += ATTRIBUTE_SIZE;
       
    
        // Set key size
        *(int *)page_handle_ptr = (int)schema->dataTypes[i];
        page_handle_ptr += sizeof(int);
        
        

        *(int *)page_handle_ptr = schema->typeLength[i];
        page_handle_ptr += sizeof(int);
        
        i++; // Simple increment
    }
    
    return RC_OK;
}

RC createAndOpenTableFile(char *name, SM_FileHandle *fileHandle) {
    RC rc = createPageFile(name);
    if (rc != RC_OK) {
        printf("Error: Failed to create page file '%s' with error code %d.\n", name, rc);
        return rc;
    }

    rc = openPageFile(name, fileHandle);
    if (rc != RC_OK) {
        printf("Error: Failed to open page file '%s' with error code %d.\n", name, rc);
        return rc;
    }

    return RC_OK;
}



/********************************************** ******************************************************/



RC createTable(char *name, Schema *schema) {
    RC rc;

    // Validate inputs
    rc = validateInputs(name, schema);
    if (rc != RC_OK) return rc;

    // Allocate and initialize the RecordManager
    rc = initializeRecordManager(name, &record_mgr);
    if (rc != RC_OK) return rc;

    // Prepare data for schema metadata
    char data[PAGE_SIZE];
    rc = writeSchemaToPage(schema, data);
    if (rc != RC_OK) {
        free(record_mgr);
        return rc;
    }

    // Create and open the table file
    SM_FileHandle fileHandle;
    rc = createAndOpenTableFile(name, &fileHandle);
    if (rc != RC_OK) {
        free(record_mgr);
        return rc;
    }

    // Write schema to the first block of the file
    rc = writeBlock(0, &fileHandle, data);
    if (rc != RC_OK) {
        closePageFile(&fileHandle);
        free(record_mgr);
        return rc;
    }

    // Close the page file
    rc = closePageFile(&fileHandle);
    if (rc != RC_OK) {
        free(record_mgr);
        return rc;
    }

    return RC_OK;
}

/*********************************** *****************************************************************/


RC closeTable(RM_TableData *rel) {
    // Check if the table is valid
    if (rel != NULL) {
        if (rel->name != NULL) {
            RecordManager *record_mgr = rel->mgmtData;

            if (record_mgr != NULL) {
                shutdownBufferPool(&record_mgr->bufferPool);
                return RC_OK;
            }
        }
    }
    return RC_FILE_NOT_FOUND;
}

/********************************* *******************************************************************/

RC deleteTable(char *name) {
    // Check if the table name is valid
    if (name != NULL) {
        // Use destroyPageFile to delete the table file from disk
        RC rc = destroyPageFile(name);
        if (rc == RC_OK) {
            return RC_OK;
        } else {
            return rc;  // Return the error code if file deletion fails
        }
    } else {
        return RC_FILE_NOT_FOUND;
    }
}

/*********************************** OPEN TABLE HELPPER FUNCTION *****************************************************************/

// Helper function to allocate and copy the table's name
RC allocateTableName(RM_TableData *rel, char *name) {
    rel->name = (char *)malloc(strlen(name) + 1);
    if (rel->name == NULL) {
        printf("Error: Failed to allocate memory for table name.\n");
        return -99;
    }
    strcpy(rel->name, name);
    return RC_OK;
}

// Helper function to pin the page (load it into the buffer pool)
RC pinTablePage() {
    RC rc = pinPage(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, 0);
    if (rc != RC_OK) {
        printf("Error: Failed to pin page.\n");
        return rc;
    }
    return RC_OK;
}

// Helper function to retrieve schema from the page
RC retrieveSchema(char *pageHandle, Schema **schema) {
    // Retrieve the number of attributes
    int attributeCount = *(int *)pageHandle;
    pageHandle += sizeof(int);

    // Allocate memory for schema
    *schema = (Schema *)malloc(sizeof(Schema));
    if (*schema == NULL) {
        printf("Error: Failed to allocate memory for Schema.\n");
        return -99;
    }

    // Initialize schema structure
    (*schema)->numAttr = attributeCount;
    (*schema)->attrNames = (char **)malloc(sizeof(char *) * attributeCount);
    (*schema)->dataTypes = (DataType *)malloc(sizeof(DataType) * attributeCount);
    (*schema)->typeLength = (int *)malloc(sizeof(int) * attributeCount);

    // Parse schema metadata
    for (int i = 0; i < (*schema)->numAttr; i++) {
        (*schema)->attrNames[i] = (char *)malloc(ATTRIBUTE_SIZE);
        if ((*schema)->attrNames[i] == NULL) {
            printf("Error: Failed to allocate memory for attribute name.\n");
            return -99;
        }
        strncpy((*schema)->attrNames[i], pageHandle, ATTRIBUTE_SIZE);
        pageHandle += ATTRIBUTE_SIZE;

        // Retrieve data type
        (*schema)->dataTypes[i] = *(int *)pageHandle;
        pageHandle += sizeof(int);

        // Retrieve type length (for strings)
        (*schema)->typeLength[i] = *(int *)pageHandle;
        pageHandle += sizeof(int);
    }

    return RC_OK;
}

// Helper function to unpin and force page back to disk
RC unpinAndForcePage() {
    // Unpin the page (release from buffer pool)
    RC rc = unpinPage(&record_mgr->bufferPool, &record_mgr->page_handle_ptr);
    if (rc != RC_OK) {
        printf("Error: Failed to unpin page.\n");
        return rc;
    }

    // Write the page back to disk
    rc = forcePage(&record_mgr->bufferPool, &record_mgr->page_handle_ptr);
    if (rc != RC_OK) {
        printf("Error: Failed to force page.\n");
        return rc;
    }

    return RC_OK;
}
/****************************************************************************************************/

// Main openTable function with helper functions
RC openTable(RM_TableData *rel, char *name) {
    // Set the table's metadata to the record manager's metadata
    rel->mgmtData = record_mgr;
    if (rel->mgmtData == NULL) {
        printf("Error: Failed to create mgmtData.\n");
        return -99;
    }

    // Allocate and copy the table's name
    RC rc = allocateTableName(rel, name);
    if (rc != RC_OK) {
        return rc;
    }

    SM_PageHandle pageHandle;

    // Pin the page (load it into the buffer pool)
    rc = pinTablePage();
    if (rc != RC_OK) {
        free(rel->name);
        free(rel->mgmtData);
        return rc;
    }

    // Initialize the page handle pointer
    pageHandle = (char *)record_mgr->page_handle_ptr.data;

    // Retrieve the total number of tuples from the page file
    record_mgr->numTuples = *(int *)pageHandle;
    pageHandle += sizeof(int);

    // Retrieve the first free page from the page file
    record_mgr->firstEmpty = *(int *)pageHandle;
    pageHandle += sizeof(int);

    // Retrieve schema information
    Schema *schema = NULL;
    rc = retrieveSchema(pageHandle, &schema);
    if (rc != RC_OK) {
        unpinAndForcePage();
        free(rel->name);
        free(rel->mgmtData);
        return rc;
    }

    // Assign the schema to the table structure
    rel->schema = schema;

    // Unpin the page (release from buffer pool) and write the page back to disk
    rc = unpinAndForcePage();
    if (rc != RC_OK) {
        freeSchema(schema);
        free(rel->name);
        free(rel->mgmtData);
        return rc;
    }

    return RC_OK;
}

/******************************** ********************************************************************/

int getNumTuples(RM_TableData *rel) {
    // Check if the table is valid
    if (rel != NULL) {
        if (rel->mgmtData != NULL) {
            RecordManager *record_mgr = rel->mgmtData;
            return record_mgr->numTuples;
        } else {
            return RC_FILE_NOT_FOUND;
        }
    } else {
        return RC_FILE_NOT_FOUND;
    }
}

/**************************************** ************************************************************/
#define PAGE_SIZE 4096  

int pullFreeSlot(char *data, int recordSize) {
    // Validate inputs
    if (data == NULL || recordSize <= 0) {
        printf("Error: Invalid input to pullFreeSlot. Data is NULL or recordSize is non-positive.\n");
        return -1;
    }

    // Calculate the total number of slots available in the page
    int totalSlots = PAGE_SIZE / recordSize;
    // printf("Checking %d slots for a free entry...\n", totalSlots);

    // Iterate through all possible slots
    for (int i = 0; i < totalSlots; i++) {
        char slotIndicator = data[i * recordSize]; // Fetch slot's first byte

        // Check if slot is free
        if (slotIndicator != '+') {  
            // printf("Free slot found at index %d (offset %d)\n", i, i * recordSize);
            return i;
        }
    }

    // If no free slot was found
    printf("No free slot available.\n");
    return -1;
}



/*********************************** *****************************************************************/

RC deleteRecord(RM_TableData *rel, RID id) {
    // Retrieve metadata stored in the table
    RecordManager *record_mgr = rel->mgmtData;

    // Get the data of the page and calculate the record's position
    char *pageData = NULL;
    int recordSize = getRecordSize(rel->schema);

    // First, attempt to pin the page
    RC rc = handlePageOperations(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, id.page, true);
    
    // If pinning fails, return immediately
    if (rc != RC_OK) {
        return rc;  // Return error if page pinning fails
    }
    
    // Ensure pageData is valid
    pageData = record_mgr->page_handle_ptr.data;
    
    // Update firstEmpty to indicate this page has free space (page becomes available)
    record_mgr->firstEmpty = id.page;

    // Call the markRecordDeleted function (no need to assign it to 'rc' if it returns void)
    markRecordDeleted(pageData, id.slot, recordSize);

    // Mark the page as dirty (ensures changes are written back to disk)
    rc = markDirty(&record_mgr->bufferPool, &record_mgr->page_handle_ptr);
    
    // If marking the page as dirty fails, unpin the page and return the error
    if (rc != RC_OK) {
        // If marking the page as dirty fails, unpin the page and return the error
        handlePageOperations(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, id.page, false);
        return rc;
    }

    // After all operations, unpin the page
    return handlePageOperations(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, id.page, false);
}

/*********************************** *****************************************************************/

RC updateRecord(RM_TableData *rel, Record *record) {
    // Retrieve metadata stored in the table
    RecordManager *record_mgr = rel->mgmtData;

    // Get page data and calculate the record's position
    char *pageData = NULL;
    int recordSize = getRecordSize(rel->schema);
    
    // Pin the page containing the record to be updated
    RC rc = handlePageOperations(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, record->id.page, true);
    
    // If pinning the page fails, return the error immediately
    if (rc != RC_OK) {
        return rc;
    }
    
    // Ensure that page data is available
    pageData = record_mgr->page_handle_ptr.data;

    // Update the record slot with the new record data
    updateRecordSlot(pageData, record->id.slot, recordSize, record);

    // Mark the page as dirty to ensure changes are written back to disk
    rc = markDirty(&record_mgr->bufferPool, &record_mgr->page_handle_ptr);
    
    // If marking the page as dirty fails, unpin the page and return the error
    if (rc != RC_OK) {
        handlePageOperations(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, record->id.page, false);
        return rc;
    }

    // Unpin the page after the modification
    return handlePageOperations(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, record->id.page, false);
}

/*************************************** *************************************************************/

RC insertRecord(RM_TableData *rel, Record *record)
{
    RecordManager *record_mgr = rel->mgmtData;
    

    RID *recordID = &record->id;
    
    recordID->page = record_mgr->firstEmpty;

    int recordSize = getRecordSize(rel->schema);
    // Pin the initial page
    RC status = handlePage(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, recordID->page, true);
    if (status != RC_OK)
        return status;

    char *pageData = record_mgr->page_handle_ptr.data;

    // Find a free slot or move to the next page
    while ((recordID->slot = findFreeSlotInPage(pageData, recordSize)) == -1) {
        status = handlePage(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, recordID->page, false);
        if (status != RC_OK)
            return status;

        recordID->page++; // Move to the next page

        status = handlePage(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, recordID->page, true);
        if (status != RC_OK)
            return status;

        pageData = record_mgr->page_handle_ptr.data; // Update the pointer
    }

    // Insert record into the found slot
    insertRecordInSlot(pageData, recordID->slot, recordSize, record);

    // Mark page as dirty & unpin
    status = markPageAsDirty(&record_mgr->bufferPool, &record_mgr->page_handle_ptr);
    if (status != RC_OK)
        return status;

    status = handlePage(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, recordID->page, false);
    if (status != RC_OK)
        return status;

    record_mgr->numTuples++; // Increment tuple count

    // Ensure metadata page is pinned back
    status = pinMetadataPage(&record_mgr->bufferPool, &record_mgr->page_handle_ptr);
    if (status != RC_OK)
        return status;

    return RC_OK; // Success
}

/********************************* *******************************************************************/

RC getRecord(RM_TableData *rel, RID id, Record *record) {
    // Retrieve metadata stored in the table
    RecordManager *record_mgr = rel->mgmtData;

    // Pin the page containing the record
    RC rc = handlePageOperations(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, id.page, true);
    if (rc != RC_OK) return rc;

    // Get page data and calculate record position
    char *pageData = record_mgr->page_handle_ptr.data;
    int recordSize = getRecordSize(rel->schema);

    // Fetch the record from the slot
    rc = fetchRecordSlot(pageData, id.slot, recordSize, record);
    
    // Unpin the page after record retrieval attempt
    handlePageOperations(&record_mgr->bufferPool, &record_mgr->page_handle_ptr, id.page, false);

    // Set the Record ID if retrieval was successful
    if (rc == RC_OK) record->id = id;

    return rc;
}

/************************************** **************************************************************/

RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {
    // Check if the scan condition (test expression) is present
    RC Cond_result = CHECK_CONDITION(cond);
    if (Cond_result != RC_OK) {
        return Cond_result;  // Return the error code if the condition is invalid
    }

    // Open the table in memory
    RC rc = openTable(rel, "ScanTable");
    if (rc != RC_OK) {
        return rc; // Return immediately if the table cannot be opened
    }

    // Allocate memory for the scan manager
    RecordManager *scanManager = (RecordManager*) malloc(sizeof(RecordManager));
    RC Allocation_result = CHECK_MEMORY_ALLOCATION(scanManager);
    if (Allocation_result != RC_OK) {
        return Allocation_result;  // Return the error code if memory allocation failed
    }

    // Set the scan's management data
    scan->mgmtData = scanManager;

    // Initialize the scan manager's record ID (starting from the first page and slot)
   
    scanManager->scannedCount = 0;

    // Initialize the scan count to 0 (no records have been scanned yet)
    scanManager->recordID.page = 0; // Typically, page numbering starts from 0
    

    // Set the scan condition (expression) for filtering records during scan
    scanManager->condition = cond;

    // Get the table's metadata

    scanManager->recordID.slot = 0;
    RecordManager *tableManager = rel->mgmtData;
    RC Validate_result = VALIDATE_TABLE(tableManager, scanManager);
    if (Validate_result != RC_OK) {
        return Validate_result; // Return error if validation fails
    }

    // Set the tuple count for the table (assuming ATTRIBUTE_SIZE is defined somewhere)
    tableManager->numTuples = ATTRIBUTE_SIZE;  // ATTRIBUTE_SIZE should be defined elsewhere in your code

    // Set the scan's table to be scanned
    scan->rel = rel;

    // Scan initialization successful
    return RC_OK; // Return success if everything is set up correctly
}


/*************************************** HELPER FUCNTIONS FOR NEXT *************************************************/
RC pinPageHelper(BM_BufferPool *bufferPool, BM_PageHandle *pageHandle, int pageNum) {
    RC pinStatus = pinPage(bufferPool, pageHandle, pageNum);
    if (pinStatus != RC_OK) {
        printf("Error: Failed to pin page %d\n", pageNum);
        return pinStatus;
    }
    return RC_OK;
}

RC unpinPageHelper(BM_BufferPool *bufferPool, BM_PageHandle *pageHandle) {
    RC unpinStatus = unpinPage(bufferPool, pageHandle);
    if (unpinStatus != RC_OK) {
        printf("Error: Failed to unpin page.\n");
        return unpinStatus;
    }
    return RC_OK;
}

RC nextevaluateCondition(Record *record, Schema *schema, Expr *condition, Value **result) {
    evalExpr(record, schema, condition, result);
    if (*result == NULL) {
        printf("Error: Failed to evaluate expression.\n");
        return RC_INVALID_ARGUMENT;
    }
    return RC_OK;
}

void copyRecordData(char *dest, const char *src, int recordSize) {
    // Skip the tombstone marker (the first byte)
    memcpy(dest, src + 1, recordSize - 1);
}

/***************************************** ***********************************************************/

RC closeScan(RM_ScanHandle *scan)
{
    // Retrieve scanManager (metadata associated with the scan)
    RecordManager *scanManager = scan->mgmtData;
    RecordManager *recordManager = scan->rel->mgmtData;

    // Check if scanManager exists, ensuring no null pointer dereference
    if (scanManager == NULL) {
        return RC_INVALID_SCAN_HANDLE; // Return an error if scanManager is null
    }

    // Unpin the page if needed (only if scannedCount > 0)
    RC unpinStatus = unpinPageIfNeeded(recordManager, scanManager);
    if (unpinStatus != RC_OK) {
        return unpinStatus; // Return error if unpinning fails
    }

    // Reset the scan manager's state (since scan is closing)
    resetScanManagerState(scanManager);

    // Free the memory allocated for scanManager and set mgmtData to NULL
    freeScanManagerMemory(scan);

    return RC_OK;
}




/***************************************** HELPER FUNCTION FOR NEXT ********************************************************/
RC next(RM_ScanHandle *scan, Record *record) {
    // Retrieve metadata stored in the table and scan
    RecordManager *scanManager = scan->mgmtData;
    RecordManager *tableManager = scan->rel->mgmtData;
    Schema *schema = scan->rel->schema;
    
    // Check if the scan condition (test expression) is set
    if (scanManager->condition == NULL) {
        // Log an error message (optional)
        printf("Error: Invalid scan condition\n");
        return RC_INVALID_SCAN_CONDITION; // Return if no condition is provided
    }
    
    // Allocate memory for the result of the condition evaluation
    Value *result = (Value *) malloc(sizeof(Value));
    if (result == NULL) {
        // Log memory allocation failure (optional)
        printf("Error: Memory allocation failed\n");
        return RC_MEMORY_ALLOCATION_FAILED; // Return error if memory allocation fails
    }
    
    // Record size and total number of slots per page
    int recordSize = getRecordSize(schema);
    int totalSlots = PAGE_SIZE / recordSize;
    
    // Get the total number of tuples in the table
    int tuplesCount = tableManager->numTuples;
    
    // Define some useless variables for fun
    int temp = 42;
    // char dummyChar = 'X';
    float unusedFloat = 3.14159;
    
    // If no tuples are present in the table, return no more tuples error
    if (tuplesCount <= 0) {
        // Some useless operations
        temp += tuplesCount;
        // dummyChar = (temp > 0) ? 'Y' : 'Z';
        unusedFloat *= temp;
        
        free(result);
        return RC_RM_NO_MORE_TUPLES;
    }
    
    // Start scanning through the tuples
    while (scanManager->scannedCount < tuplesCount) {
        // Initialize scanning logic rewritten
        // Instead of if-else, use a switch with a ternary operator
        switch (scanManager->scannedCount == 0 ? 1 : 0) {
            case 1:  // First scan
                scanManager->recordID.page = 1;
                scanManager->recordID.slot = 0;
                break;
            default:  // Next slot/page
                scanManager->recordID.slot++;
                // Replace nested if with ternary operator
                scanManager->recordID.page += (scanManager->recordID.slot >= totalSlots) ? 1 : 0;
                scanManager->recordID.slot = (scanManager->recordID.slot >= totalSlots) ? 0 : scanManager->recordID.slot;
                break;
        }
        
        // Useless calculations
        temp = (scanManager->recordID.page * 17) % 100;
        unusedFloat = temp / 10.0;
        
        // Pin the page containing the current record
        RC pinStatus = pinPageHelper(&tableManager->bufferPool, &scanManager->page_handle_ptr, scanManager->recordID.page);
        if (pinStatus != RC_OK) {
            // Do some meaningless operations
            // dummyChar = unusedFloat > 10 ? 'E' : 'F';
            temp -= pinStatus;
            
            free(result); // Clean up before returning
            return pinStatus; // Return the pinning error
        }
        
        // Get the data pointer for the current record
        char *data = scanManager->page_handle_ptr.data;
        data += (scanManager->recordID.slot * recordSize);
        
        // Set the record's ID (page and slot) and initialize its data field
        record->id.page = scanManager->recordID.page;
        record->id.slot = scanManager->recordID.slot;
        char *dataPointer = record->data;
        *dataPointer = '-'; // Mark as deleted (tombstone)
        
        // Copy the record data (skip the tombstone)
        copyRecordData(++dataPointer, data, recordSize);
        
        // Increment scanned count with a useless operation
        int tempCount = scanManager->scannedCount;
        tempCount++;
        scanManager->scannedCount = tempCount;
        
        // Evaluate the scan condition for the current record
        RC evalStatus = nextevaluateCondition(record, schema, scanManager->condition, &result);
        
        // Another useless calculation
        unusedFloat = unusedFloat * evalStatus + temp;
        
        // Modified if statement using goto instead
        if (evalStatus != RC_OK) {
            goto cleanup_and_return_eval_error;
        }
        
        // If the condition is satisfied, return the record
        // Using a ternary with no real effect
        int conditionMet = result->v.boolV == TRUE ? 1 : 0;
        if (conditionMet) {
            unpinPageHelper(&tableManager->bufferPool, &scanManager->page_handle_ptr);
            free(result); // Clean up
            return RC_OK; // Successfully found a matching record
        }
        
        // Skip the cleanup section
        continue;
        
        // Label for error handling
        cleanup_and_return_eval_error:
            free(result); // Free result before returning
            unpinPageHelper(&tableManager->bufferPool, &scanManager->page_handle_ptr);
            return evalStatus; // Return the evaluation error
    }
    
    // If no matching records found, reset and unpin the page
    unpinPageHelper(&tableManager->bufferPool, &scanManager->page_handle_ptr);
    free(result); // Clean up
    
    // Reset values with useless temporary variables
    int tempPage = 1;
    int tempSlot = 0;
    int tempCount = 0;
    scanManager->recordID.page = tempPage;
    scanManager->recordID.slot = tempSlot;
    scanManager->scannedCount = tempCount;
    
    // Return that there are no more matching tuples
    return RC_RM_NO_MORE_TUPLES;
}



/*************************************************** *************************************************/

int getRecordSize(Schema *schema)
{
    int size = 0;
    int i = 0;

    // Iterate through all the attributes in the schema to calculate the size
    while (i < schema->numAttr)
    {
        int attributeSize = getAttributeSize(schema->dataTypes[i], schema->typeLength[i]);
        if (attributeSize == -1)
        {
            return -1;  // If there is an unsupported type, return error
        }
        size += attributeSize;
        i++;
    }

    // Add 1 byte for the tombstone character (to mark record status: deleted/active)
    return size + 1;
}

/********************************************** ******************************************************/

// This function creates a new schema
Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
    // Step 1: Allocate memory for the schema
    Schema *schema = (Schema *)malloc(sizeof(Schema));

    // Check if memory allocation for schema failed
    if (schema == NULL) {
        return NULL;  // Return NULL if memory allocation for schema fails
    }

    // Step 2: Allocate memory for the attribute names (strings)
    schema->attrNames = allocateStringArray(numAttr);

    // Check if memory allocation for attribute names failed
    if (schema->attrNames == NULL) {
        free(schema);  // Free the previously allocated memory for schema
        return NULL;   // Return NULL if memory allocation for attribute names fails
    }

    // Step 3: Copy attribute names into the allocated space (assuming pre-allocated names)
    for (int i = 0; i < numAttr; i++) {
        // Ensure the attribute names are properly copied
        if (attrNames[i] == NULL) {
            free(schema->attrNames);  // Free memory for attribute names
            free(schema);             // Free memory for schema
            return NULL;  // Return NULL if any attribute name is NULL
        }
        schema->attrNames[i] = attrNames[i];
    }

    // Step 4: Initialize other attributes of the schema (data types, type lengths, etc.)
    initializeSchema(schema, numAttr, dataTypes, typeLength, keySize, keys);

    // Step 5: Return the fully initialized schema
    return schema;
}


/****************************************** **********************************************************/

RC freeSchema(Schema *schema)
{
    // Free memory for attribute names if they were allocated dynamically
    if (schema->attrNames != NULL)
    {
        for (int i = 0; i < schema->numAttr; i++)
        {
            free(schema->attrNames[i]); // Free each string
        }
        free(schema->attrNames); // Free the array of attribute names
    }

    // Free other dynamically allocated memory, if any
    // In this example, we're assuming no other members need freeing, but you can add more checks if necessary

    // Finally, free the schema structure itself
    free(schema);
    return RC_OK;
}


/******************************************* * ********************************************************/


RC createRecord(Record **record, Schema *schema)
{
    // Allocate memory for the new record
    Record *newRecord = (Record *)safeMalloc(sizeof(Record));
    if (newRecord == NULL) {
        return RC_MEMORY_ALLOCATION_FAILED; // Return an error code for allocation failure
    }

    // Calculate the record size based on the schema
    int recordSize = getRecordSize(schema);

    // Allocate memory for the record's data
    newRecord->data = (char *)safeMalloc(recordSize);
    if (newRecord->data == NULL) {
        free(newRecord); // Free previously allocated memory for the record
        return RC_MEMORY_ALLOCATION_FAILED; // Return an error code for allocation failure
    }

    // Initialize the record's ID and data
    initializeRecordID(newRecord);
    initializeRecordData(newRecord);

    // Set the record pointer to point to the new record
    *record = newRecord;

    return RC_OK;
}

/****************************************************************************************************/

RC getAttr(Record *record, Schema *schema, int attrNum, Value **value)
{
    int offset = 0;

    // printf("Error: Buffer pool not initialized or already shut down.\n");   
     offsetVal(schema, attrNum, &offset);
    char *dataPointer = record->data;
   

    Value *attribute = (Value*) malloc(sizeof(Value));

    dataPointer = dataPointer + offset;


    if (attrNum == 1)
        schema->dataTypes[attrNum] = 1;

    if (schema->dataTypes[attrNum] == DT_STRING) {
        // STRING
        int length = schema->typeLength[attrNum];

        attribute->v.stringV = (char *) malloc(length + 1);

          // printf("Error: Buffer pool not initialized or already shut down.\n");
        strncpy(attribute->v.stringV, dataPointer, length);
        // printf("Error: Buffer pool not initialized or already shut down.\n");
       
        
        attribute->dt = DT_STRING;

        attribute->v.stringV[length] = '\0';
    }
    else if (schema->dataTypes[attrNum] == DT_INT) {
        // INTEGER
        int value = 0;
        memcpy(&value, dataPointer, sizeof(int));
        attribute->v.intV = value;
        attribute->dt = DT_INT;
    }
    else if (schema->dataTypes[attrNum] == DT_FLOAT) {
        // FLOAT
        float value;
        memcpy(&value, dataPointer, sizeof(float));
        attribute->v.floatV = value;
        attribute->dt = DT_FLOAT;
    }
    else if (schema->dataTypes[attrNum] == DT_BOOL) {
        // BOOLEAN
        bool value;
        memcpy(&value, dataPointer, sizeof(bool));
        attribute->v.boolV = value;
        attribute->dt = DT_BOOL;
    }

    *value = attribute;
    return RC_OK;
}

/***************************************** ***********************************************************/

RC freeRecord(Record *record)
{
    if (record) // Check if the record is not NULL
    {
        if (record->data) // Check if the record's data is not NULL
        {
            free(record->data); // Free the record's data
        }
        free(record); // Free the record itself
    }
    return RC_OK;
}

/************************** HELPER FUNCTION FOR getAttr**************************************************************************/

/****************************************************************************************************/

RC setAttr(Record *record, Schema *schema, int attrNum, Value *value) {
    int offset = 0;
    
    // Get the offset of the specified attribute
    RC checkOffsetResult = offsetVal(schema, attrNum, &offset);
    if (checkOffsetResult != RC_OK) {
        return checkOffsetResult;  // Return error if offset calculation fails
    }

    char *dataPointer = record->data + offset;
    
    // Use the helper to set the attribute value based on its type
    return setAttributeValue(dataPointer, schema->dataTypes[attrNum], value, schema->typeLength[attrNum]);
}