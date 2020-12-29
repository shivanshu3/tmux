#pragma once

#include <stdint.h>

typedef struct regex_t {
	size_t re_nsub; // Number of parenthesized subexpressions
} regex_t;

typedef int64_t regoff_t;

typedef struct regmatch_t {
	regoff_t rm_so; // Byte offset from start of string to start of substring.
	regoff_t rm_eo; // Byte offset from start of string of the first character after the end of substring.
} regmatch_t;

#define REG_EXTENDED 1
#define REG_ICASE 1
#define REG_NOSUB 1
#define REG_NEWLINE 1
#define REG_NOTBOL 1

int regcomp(regex_t *, const char *, int);
int regexec(const regex_t* preg, const char* string, size_t nmatch, regmatch_t pmatch[], int eflags);
void regfree(regex_t *);
