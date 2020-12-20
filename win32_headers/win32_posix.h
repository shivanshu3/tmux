#pragma once

#include <Windows.h>
#include <io.h>
#include <errno.h>

typedef HANDLE pid_t;

#define close _close

// TODO: Return 1024
int getdtablesize(void);
