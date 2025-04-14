# CS525 Advanced Database Organization – ADO Assignments

This repository contains implementations of four core assignments for the **CS525: Advanced Database Organization** course at **Illinois Institute of Technology**.

## 🧑‍💻 Team Members
- **Atharv Patil**
- **Manthan Surjuse**
- **Avneet Singh**

> Implemented in **C/C++**, this project demonstrates fundamental components of database internals.

---

## 📚 Assignments Overview

### 1. 📦 Storage Manager
Implements low-level file management using fixed-size pages. Core functionalities include:
- Creating/opening/closing files
- Reading/writing specific pages
- Ensuring persistence and page alignment

### 2. 🧠 Buffer Manager
Simulates a buffer pool that caches disk pages in memory with replacement strategies like:
- FIFO
- LRU
- Clock (optional)

Features include:
- Pinning/unpinning pages
- Tracking dirty pages and hit count
- Page eviction and flushing

### 3. 📄 Record Manager
Built on top of the storage and buffer managers, this module supports:
- Record schema definition
- Insert/update/delete/search records
- Record scanning with condition expressions

### 4. 🌳 B+ Tree Indexing
Implements a disk-based **B+ Tree** for indexing and fast lookups. Includes:
- Key insertion and node splitting
- Index-based search and deletion
- Tree traversal and visualization (optional)

---

## 🛠️ Technologies
- **C/C++**
- Manual memory management
- File I/O
- Custom data structures (linked lists, tree nodes, etc.)

---

## 📁 Folder Structure

```
├── ASSIGNMENT 1/ assign1_storage_manager/
├── ASSIGNMENT 2/ assign2_buffer_manager/
├── ASSIGNMENT 3/ assign3_record_manager/
├── ASSIGNMENT 4/ assign4_btree_Implementation/
└── README.md
```

## 📌 Notes
- This project was completed as part of coursework and is not intended for production use.
- All code was written from scratch without using external DBMS libraries.

---

## 📫 Contact

For academic inquiries or contributions, feel free to reach out via linkedin or GitHub.

| Name | Linkedin | Github
|---|---|---                                                     
**Atharv Patil** | [Linkedin](https://www.linkedin.com/in/atharv-patil-414b531b4/) | [Github](https://github.com/AtharvPat) |
**Manthan Surjuse** | [Linkedin](https://www.linkedin.com/in/manthan-surjuse/) | [Github](https://github.com/Manthan0120)   
**Avneet Singh** | [Linkedin](https://www.linkedin.com/in/avneet-singh120/) | [Github](https://github.com/asingh180)  