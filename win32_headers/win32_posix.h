#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <windows.h>
#include <winsock2.h>
#include <Mswsock.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <afunix.h>
#include <io.h>
#include <errno.h>
#include <process.h>
#include <stdio.h>
#include <sys/types.h>

typedef int pid_t;
typedef int mode_t;

#define close _close
#define fdopen _fdopen
#define getpid _getpid
#define unlink _unlink

// WIN32_TODO: Maybe rearchitect the code which uses these macros?
#define WIFEXITED(x) (1)
#define WEXITSTATUS(x) (1)
#define WIFSTOPPED(x) (1)
#define WSTOPSIG(x) (1)
#define WIFSIGNALED(x) (1)
#define WTERMSIG(x) (1)

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define TIOCSWINSZ 1

#define SIGCHLD 1
#define SIGCONT 2
#define SIGHUP 3
#define SIGPIPE 4
#define SIGQUIT 5
#define SIGTSTP 6
#define SIGTTIN 7
#define SIGTTOU 8
#define SIGUSR1 9
#define SIGUSR2 10
#define SIGWINCH 11

#define SIG_BLOCK 1
#define SIG_UNBLOCK 1
#define SIG_SETMASK 1

#define SA_RESTART 1

#define SHUT_WR 1

#define ICRNL 1
#define IXANY 1
#define OPOST 1
#define ONLCR 1
#define CREAD 1
#define CS8 1
#define HUPCL 1
#define VMIN 1
#define VTIME 1
#define TCSANOW 1
#define TCSAFLUSH 1
#define O_NONBLOCK 1
#define WNOHANG 1
#define VERASE 1
#define _POSIX_VDISABLE 1

typedef struct uid_t
{
	int a;
} uid_t;

struct passwd
{
	char *pw_dir; // home directory
};

struct timezone
{
	int a;
};

typedef struct sigset_t
{
	int a;
} sigset_t;

union sigval
{
	int a;
};

typedef struct siginfo_t
{
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
} siginfo_t;

struct sigaction
{
	void     (*sa_handler)(int);
	void     (*sa_sigaction)(int, siginfo_t *, void *);
	sigset_t   sa_mask;
	int        sa_flags;
	void     (*sa_restorer)(void);
};

// WIN32_TODO: Return 1024
int getdtablesize(void);

// WIN32_TODO: https://stackoverflow.com/a/7850494/1544818
int getpagesize(void);

uid_t getuid(void);

struct passwd *getpwnam(const char *name);

struct passwd *getpwuid(uid_t uid);

int kill(pid_t pid, int sig);

int gettimeofday(struct timeval *tv, struct timezone *tz);

int socketpair(int domain, int type, int protocol, int sv[2]);

pid_t fork(void);

int chdir(const char *path);

int TmuxWin32PosixDup(int);

int TmuxWin32PosixDup2(int oldfd, int newfd);

#ifndef TMUX_CXX_DISABLED
int TmuxWin32PosixOpen(const char *pathname, int flags, mode_t mode = 0);
#endif

int TmuxWin32PosixExecl(const char *path, const char *arg, ...);

int ioctl(int fd, int cmd, ...);

int fseeko(FILE *stream, off_t offset, int whence);

off_t ftello(FILE *stream);

int mkstemp(char *template_);

int sigemptyset(sigset_t *set);

int sigfillset(sigset_t *set);

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

int killpg(int pgrp, int sig);

int tcgetattr(int fd, struct termios *termios_p);

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);

pid_t getppid(void);

char *strsignal(int sig);

pid_t waitpid(pid_t pid, int *status, int options);
