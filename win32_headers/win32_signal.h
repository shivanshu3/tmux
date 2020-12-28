#pragma once

#include "win32_headers/win32_posix.h"

struct sigset_t {
	int a;
};

union sigval {
	int a;
};

struct siginfo_t {
	int      si_signo;     /* Signal number */
	int      si_errno;     /* An errno value */
	int      si_code;      /* Signal code */
	int      si_trapno;    /* Trap number that caused hardware-generated signal (unused on most architectures) */
	pid_t    si_pid;       /* Sending process ID */
	uid_t    si_uid;       /* Real user ID of sending process */
	int      si_status;    /* Exit value or signal */
	clock_t  si_utime;     /* User time consumed */
	clock_t  si_stime;     /* System time consumed */
	union sigval si_value; /* Signal value */
	int      si_int;       /* POSIX.1b signal */
	void    *si_ptr;       /* POSIX.1b signal */
	int      si_overrun;   /* Timer overrun count; POSIX.1b timers */
	int      si_timerid;   /* Timer ID; POSIX.1b timers */
	void    *si_addr;      /* Memory location which caused fault */
	long     si_band;      /* Band event (was int in glibc 2.3.2 and earlier) */
	int      si_fd;        /* File descriptor */
	short    si_addr_lsb;  /* Least significant bit of address (since Linux 2.6.32) */
	void    *si_lower;     /* Lower bound when address violation occurred (since Linux 3.19) */
	void    *si_upper;     /* Upper bound when address violation occurred (since Linux 3.19) */
	int      si_pkey;      /* Protection key on PTE that caused fault (since Linux 4.6) */
	void    *si_call_addr; /* Address of system call instruction (since Linux 3.5) */
	int      si_syscall;   /* Number of attempted system call (since Linux 3.5) */
	unsigned int si_arch;  /* Architecture of attempted system call (since Linux 3.5) */
};

struct sigaction {
	void     (*sa_handler)(int);
	void     (*sa_sigaction)(int, siginfo_t *, void *);
	sigset_t   sa_mask;
	int        sa_flags;
	void     (*sa_restorer)(void);
};

int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int killpg(int pgrp, int sig);

#define SIG_BLOCK 1
#define SIG_UNBLOCK 1
#define SIG_SETMASK 1

#define SA_RESTART 1
