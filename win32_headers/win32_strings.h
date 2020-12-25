#pragma once

#include "extern-c.h"

TMUX_EXTERN_C int strcasecmp(const char *s1, const char *s2);
TMUX_EXTERN_C int strncasecmp(const char *s1, const char *s2, size_t n);
TMUX_EXTERN_C int fnmatch(const char *pattern, const char *string, int flags);
