Name | function | record_mgr.c
--- | --- | --- 
Avneet | '1. value equals' , '2. value smaller' | '3. createTable' , '8. FlushDataToPage' , '12. updateRecord', '13.getRecord' , 17. getRecordSize', '18. *createSchema', '23. getNumAttr' , '24. getAttr'
Atharv | '3. boolNot' , '4. boolAnd' , '5. boolOr' | '1. initRecordManager' , '2. shutdownRecordManager' , '4. openTable' , '7. getNumTuples', '9.getRecords', '10.insertRecord' , '16.closeScan' , '19. freeSchema' , '21. freeRecord' , '22. getStringAttr' , '27. createPageDirectoryNode'
Manthan | '6. evalExpr' , '7. freeExpr' | '5. closeTable' , '6.deleteTable', '11. deleteRecord' , '14. startScan' , '15. next' , '20. createRecord' , '25. intToString' , '26. setAttr'

## changes in variables of record_mgr.c
1. fHandle -> fileHandle
2. name -> char_name
3. schema -> scheme
4. schema_info -> scheme_info

## take reference from the following code
https://github.com/hanggrian/IIT-CS525/tree/main/assign3
