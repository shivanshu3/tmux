#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <io.h>
#include <errno.h>

typedef HANDLE pid_t;

#define close _close

// TODO: Return 1024
int getdtablesize(void);
