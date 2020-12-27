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
#include <io.h>
#include <errno.h>
#include <process.h>

typedef int pid_t;
typedef int mode_t;

#define close _close
#define getpid _getpid

// WIN32_TODO: Maybe rearchitect the code which uses these macros?
#define WIFEXITED(x) (1)
#define WEXITSTATUS(x) (1)
#define WIFSTOPPED(x) (1)
#define WSTOPSIG(x) (1)

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define TIOCSWINSZ 1

#define SIGTTIN 1
#define SIGTTOU 1

#define SIGCONT 1

#define SHUT_WR 1

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

int TmuxWin32PosixDup2(int oldfd, int newfd);

int TmuxWin32PosixOpen(const char *pathname, int flags, mode_t mode);

int TmuxWin32PosixExecl(const char *path, const char *arg, ...);

int ioctl(int fd, int cmd, ...);
