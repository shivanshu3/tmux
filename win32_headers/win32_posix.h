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

#define close _close
#define getpid _getpid

typedef struct uid_t
{
	int a;
} uid_t;

struct passwd
{
	char *pw_dir; // home directory
};

// WIN32_TODO: Return 1024
int getdtablesize(void);

// WIN32_TODO: https://stackoverflow.com/a/7850494/1544818
int getpagesize(void);

uid_t getuid(void);

struct passwd *getpwnam(const char *name);

struct passwd *getpwuid(uid_t uid);

int kill(pid_t pid, int sig);
