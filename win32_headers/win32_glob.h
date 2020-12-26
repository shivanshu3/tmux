#pragma once

typedef struct glob_t
{
	size_t gl_pathc; // Count of paths matched by pattern.
	char **gl_pathv; // Pointer to a list of matched pathnames.
	size_t gl_offs; // Slots to reserve at the beginning of gl_pathv.
} glob_t;

#define GLOB_APPEND 1
#define GLOB_DOOFFS 2
#define GLOB_ERR 3
#define GLOB_MARK 4
#define GLOB_NOCHECK 5
#define GLOB_NOESCAPE 6
#define GLOB_NOSORT 7

#define GLOB_ABORTED 8
#define GLOB_NOMATCH 9
#define GLOB_NOSPACE 10

int glob(const char *pattern, int flags, int (*errfunc) (const char *epath, int eerrno), glob_t *pglob);
void globfree(glob_t *pglob);
