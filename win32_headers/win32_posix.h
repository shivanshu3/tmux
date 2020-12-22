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

// TODO: Return 1024
int getdtablesize(void);
