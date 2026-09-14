# Concurrent-Multi-Process-Banking-Server

A Concurrent, Multi-Client, Socket-Based Transaction Processing System with Process-Level Synchronization Control

![C++](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)
![Sockets](https://img.shields.io/badge/Networking-POSIX%20Sockets-green.svg)
![Concurrency](https://img.shields.io/badge/Concurrency-Multi--Process%20(fork)-orange.svg)
![Sync](https://img.shields.io/badge/Synchronization-Semaphores%20%7C%20fcntl%20Record%20Locks-red.svg)
![Storage](https://img.shields.io/badge/Storage-Binary%20Flat%20Files-lightgrey.svg)
![OS](https://img.shields.io/badge/OS-Linux%20%2F%20Unix-informational.svg)

---

## ✨ Overview

A **multi-client Banking Management System** built entirely on **raw POSIX system calls**, sockets, `fork()`, `fcntl()` file locks, and System V semaphores.

This project simulates a **real banking backend** where:
- 🖥️ One server process listens for connections
- 👥 Multiple clients connect and transact **concurrently**
- 🧑‍💼 Both **Admin** and **User** roles are supported over the same socket protocol
- 🔒 Every read/write to shared account data is protected against race conditions — across **separate OS processes**, not just separate threads in one process

---

## 🚀 Features

### 👨‍💼 Admin Functionalities
- ➕ Add Account (Regular or Joint) + create the associated User record(s)
- 🔍 Get Account Details
- 📜 Get Transaction Details for any account
- 🪪 Get User Details
- 🗑️ Delete (deactivate) an Account
- ✏️ Modify User Information (name / age / gender)

### 👤 User Functionalities
- 🔐 Secure login (login ID + password, no echo on password entry)
- 💰 Deposit Money
- 💸 Withdraw Money
- 📊 Get Balance
- 📜 View Transaction History
- 🔑 Change Password

---

## 🧠 Core Concepts Used


| Concept | Where it shows up | Notes |
|---|---|---|
| 🌐 **Socket Programming** | `Server.cpp`, `Client.cpp`, `common/Connection.hpp` | Raw BSD sockets: `socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `read()`/`write()`. Custom lightweight text protocol with `^` / `$` / `#` control sentinels. |
| 🧬 **Multi-Processing (`fork()`)** | `Server.cpp` accept loop | Every accepted connection is handed off to a freshly forked **child process** — not a thread. Each client gets its own isolated memory space; a crash in one session can't corrupt another's. |
| ⚙️ **System Calls** | Throughout `common/` and `services/` | `open()`, `read()`, `write()`, `lseek()`, `close()`, `fcntl()`, `fork()`, `semget()`/`semop()`/`semctl()`/`ftok()` — no C++ streams, no library-level file wrappers for the database layer. |
| 🚦 **Concurrency** | Whole system | Many client processes can be reading/writing the same account data at the same time; correctness is enforced at the file/record level, not by serializing clients. |
| 🔒 **Semaphores (System V, `sys/sem.h`)** | `common/CriticalSection.hpp` | One binary semaphore per account (`ftok`-keyed), guarding the full read-modify-write of deposit / withdraw / password-change as an atomic critical section across processes. RAII-wrapped (`AccountSemaphore` + `CriticalSection`) so lock/unlock can't be mismatched. |
| 🔐 **Record-Level File Locking (`fcntl`)** | `common/RecordLock.hpp`, `common/RecordFile.hpp` | Byte-range `F_RDLCK`/`F_WRLCK` locks scoped to the exact record being read or written, not the whole file — also RAII-wrapped. |
| 💾 **File Handling** | `common/RecordFile.hpp` | Accounts, Users, and Transactions are stored as flat binary files of fixed-size POD structs, indexed directly by record number × `sizeof(Record)` byte offset — no database engine, no ORM. |
| 🧱 **RAII / Modern C++ Design** | `common/FileDescriptor.hpp`, `RecordLock.hpp`, `CriticalSection.hpp` | File descriptors, `fcntl` locks, and semaphore critical sections are all owned by objects whose destructors guarantee cleanup — replacing the original C code's manual, hand-paired open/close and lock/unlock calls. |

---

## ⚙️ System Design

```
 Client 1 ─┐
 Client 2 ─┼──▶ 🌐 Server (fork() per connection)
 Client 3 ─┘         │
                      ├── Child Process 1 ──┐
                      ├── Child Process 2 ──┼──▶ 🔒 fcntl record locks
                      └── Child Process 3 ──┘     🚦 per-account semaphore
                                             │
                                             ▼
                                   💾 Binary Database (flat files)
                                      ├── ACCOUNT_FILE
                                      ├── USER_FILE
                                      └── TRANSACTION_FILE
```

Each client connection is a **separate process**, not a thread — the server's `while(1) { accept(); fork(); }` loop is the entire concurrency model. Two children touching the *same* account still have to go through the same `fcntl` locks and semaphore as they would with threads; what changes is that a bug or crash in one child can't reach across into another child's memory.

---

## 🗂️ Data Storage

All data lives in flat binary files under `db/`, written and read as raw fixed-size struct records — no serialization library, no text format:

```
/db
 ├── ACCOUNT_FILE      (struct Account, one fixed-size record per account number)
 ├── USER_FILE         (struct User, one fixed-size record per user ID)
 └── TRANSACTION_FILE  (struct Transaction, append-only log)
```

Deletion is soft: records are never removed or compacted — an "deleted" account is just one with `active_status = false`, rewritten in place.

---

## 🏗️ Project Structure

```
OnlineBankingCPP/
 ├── db/          Account / User / Transaction record structs + admin credentials
 ├── messages/    All protocol & UI text, centralized in one header
 ├── common/      Reusable infrastructure: FileDescriptor, RecordLock, RecordFile<T>,
 │                Connection, AccountSemaphore / CriticalSection — all RAII
 ├── services/    Business logic: LoginService, AccountService, AdminService, UserService
 ├── Server.cpp   Listens, accepts, forks, dispatches to Admin/User session
 ├── Client.cpp   Connects and drives the text protocol (with hidden password entry)
 └── Makefile
```

---

## 🛠️ Tech Stack

- 💻 **Language:** C++17
- 🌐 **Networking:** POSIX Sockets (BSD socket API)
- 🧬 **Concurrency model:** Multi-processing via `fork()`
- 🚦 **Synchronization:** System V semaphores + `fcntl()` byte-range record locks
- 💾 **Storage:** Raw binary flat files (fixed-size POD records)
- 🧱 **Design:** RAII resource management, service-oriented class structure
- 🖥️ **OS:** Linux / Unix (POSIX-dependent — requires WSL on Windows)

---

## 🔭 Possible Future Directions

Ideas worth exploring, not yet implemented:

- 🧵 **Thread-per-connection model**: swap `fork()` for `std::thread` to reduce per-connection overhead. Would *not* remove the need for `fcntl` locks or semaphores (the shared state lives in files, not memory), but could add `std::mutex`-protected in-memory caching on top.
- 🔎 **Finer-grained locking**: the current per-account semaphore already avoids a global lock; a next step could reduce lock scope further for high-traffic multi-owner accounts.
- 🗃️ **Structured storage**: replacing flat binary files with SQLite or a similar embedded database, trading some raw-syscall transparency for built-in indexing and crash recovery.

---

<p align="center">
  © 2026 Ansh Jain (anshjain-24). All rights reserved.
</p>
