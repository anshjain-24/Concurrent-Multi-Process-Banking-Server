#pragma once
#include <sys/ipc.h>
#include <sys/sem.h>
#include <cstdio>
#include <unistd.h>

namespace bank {

#if !defined(_SEM_SEMUN_UNDEFINED) && !defined(__APPLE__)
// glibc doesn't define "union semun" for you (POSIX leaves it to the
// caller); most modern glibc *does* define it if _GNU_SOURCE is set, but
// we declare it explicitly here so this compiles the same way everywhere.
#endif
union SemUnion {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};

// Gets (creating + initializing to 1 if necessary) a single binary
// semaphore keyed off ftok(keyPath, keyId). One of these is created per
// logged-in user session, keyed by account number, matching the original
// server's per-account semaphore. This is the "joint account" protection
// layer sitting above the per-record fcntl() locks: it serializes the
// full read-modify-write of a deposit/withdraw/password-change, not just
// the individual file record access.
class AccountSemaphore {
public:
    AccountSemaphore(const char* keyPath, int keyId) {
        key_t key = ftok(keyPath, keyId);
        semId_ = semget(key, 1, 0);
        if (semId_ == -1) {
            semId_ = semget(key, 1, IPC_CREAT | 0700);
            if (semId_ == -1) {
                perror("Error while creating semaphore!");
                _exit(1);
            }
            SemUnion arg{};
            arg.val = 1; // binary semaphore, starts unlocked
            if (semctl(semId_, 0, SETVAL, arg) == -1) {
                perror("Error while initializing a binary semaphore!");
                _exit(1);
            }
        }
    }

    int id() const { return semId_; }

private:
    int semId_;
};

// RAII lock/unlock around an AccountSemaphore -- replaces the manual
// lock_critical_section()/unlock_critical_section() pair, and the several
// "unlock_critical_section(&sem_op); return false;" call sites that had to
// remember to pair every early return with an unlock.
class CriticalSection {
public:
    explicit CriticalSection(const AccountSemaphore& sem) : semId_(sem.id()) {
        struct sembuf op{0, -1, SEM_UNDO};
        if (semop(semId_, &op, 1) == -1) {
            perror("Error while locking critical section");
            locked_ = false;
        }
    }

    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;

    ~CriticalSection() {
        if (locked_) {
            struct sembuf op{0, 1, SEM_UNDO};
            if (semop(semId_, &op, 1) == -1)
                perror("Error while unlocking critical section");
        }
    }

    bool ok() const { return locked_; }

private:
    int semId_;
    bool locked_ = true;
};

} // namespace bank
