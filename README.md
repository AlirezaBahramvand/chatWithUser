# 👻 Ghost Chat

A full-duplex, real-time, terminal-based two-player chat application written in **C** using **Linux/Unix System Programming** concepts. The project utilizes **System V Message Queues** for Inter-Process Communication (IPC) between two concurrent terminal instances and features a self-destructing **Burn-After-Reading** messaging mechanism.

---

## ✨ Key Features

* **Automatic Role Detection:** Uses file-based locking (`O_CREAT | O_EXCL`) to automatically assign the first instance as **User 1** (Host) and the second instance as **User 2** (Peer).
* **Full-Duplex Real-Time IPC:** Leverages process concurrency via `fork()` to decouple message sending (Parent process) and receiving (Child process) for a smooth, non-blocking chat experience.
* **Burn-After-Reading Messages:** Supports timed, self-destructing messages that auto-erase from the recipient's screen after a specified duration and bypass local chat logs.
* **Local Chat History Logging:** Automatically logs standard chat interactions locally into `user1History.txt` and `user2History.txt`.
* **Clean Terminal UI:** Uses ANSI Escape Sequences (`\033[H\033[J`, `\033[A\r\033[K`) for screen clearing and smooth prompt redrawing.
* **Graceful Exit & Resource Cleanup:** Safely terminates processes using POSIX signals (`SIGTERM`) and deletes active IPC queues, session state files, and history logs upon typing `EXIT`.

---

## 🛠 Tech Stack & System Calls

* **Language:** C (Standard C Library & POSIX System Calls)
* **Inter-Process Communication (IPC):** System V Message Queues (`msgget`, `msgsnd`, `msgrcv`, `msgctl`)
* **Process Management:** `fork`, `wait`, `kill`, `_exit`, `getppid`
* **Low-Level I/O:** `open`, `read`, `write`, `close`, `unlink`
* **Signals:** `SIGTERM` for inter-process teardown synchronization

---

## 🚀 How to Build & Run

### Prerequisites
* A Unix-like operating system (Linux, macOS, or WSL on Windows).
* `gcc` compiler.

### 1. Compilation
Open your terminal in the project directory and run:

```bash
gcc -o ghost_chat main.c
