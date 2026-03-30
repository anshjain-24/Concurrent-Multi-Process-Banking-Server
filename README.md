# Thread-Safe-Multi-Client-Banking-Server
A Concurrent, Multi-Client, Socket-Based Transaction Processing System with Synchronization Control

# 💳 Banking Management System (Concurrent & Socket-Based)

![C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)
![Sockets](https://img.shields.io/badge/Networking-Sockets-green.svg)
![Concurrency](https://img.shields.io/badge/Concurrency-Mutex%20%7C%20Semaphores-orange.svg)

---

## ✨ Overview

A **multi-client Banking Management System** built using **Sockets and Concurrency (Mutex & Semaphores)**.

This project simulates a **real-world banking backend** where:
- Only **one server** runs at a time 🖥️
- Multiple **clients can connect concurrently** 👥
- Supports **Admin & User roles**
- Ensures **safe transaction handling** using synchronization 🔒

---

## 🚀 Features

### 👨‍💼 Admin Functionalities
- ➕ Add Users (Regular / Joint Account)
- 🗑️ Delete Users
- ✏️ Update User Details
- 📋 View All Users

### 👤 User Functionalities
- 🔐 Secure Login
- 💰 Deposit Money
- 💸 Withdraw Money
- 📊 View Balance
- 📜 Transaction History
- 🔑 Change Password

---

## 🧠 Core Concepts Used

| Concept | Description |
|--------|------------|
| 🧵 Multithreading | Handle multiple clients simultaneously |
| 🔒 Mutex | Prevent race conditions |
| 🚦 Semaphores | Control critical section access |
| 🌐 Sockets | Client-Server communication |
| 💾 File Handling | Persistent storage using binary files |

---

## ⚙️ System Design
Client 1 ─┐

Client 2 ─┼──> 🌐 Server (Multithreaded)

Client 3 ─┘ │


💾 Binary Database ▼

(ACCOUNTS / USERS / TRANSACTIONS)


---

## 🔧 Improvement (Ongoing C++ Version)

🚀 Rewriting the system in **C++ with better concurrency design**:

### Goals:
- ✅ Fine-grained locking (account-level instead of global)
- ✅ Independent transactions for different accounts
- ✅ Improved performance & scalability
- ✅ Cleaner object-oriented design

---

## 🗂️ Data Storage

All data is stored locally in binary files:

    /db

        ├── ACCOUNTS

        ├── USERS

        ├── TRANSACTIONS
## 🛠️ Tech Stack

- 💻 Language: C / C++
- 🌐 Networking: POSIX Sockets
- 🧵 Concurrency: Mutex, Semaphores
- 💾 Storage: Binary Files
- 🖥️ OS: Linux / Unix

---
<br>

<p align="center">
  © 2026 Ansh Jain (anshjain-24). All rights reserved.
</p>