#pragma once

#include "extern-c.h"

#define b64_ntop __b64_ntop
#define b64_pton __b64_pton

EXTERN_C int b64_ntop (const unsigned char *, size_t, char *, size_t);
EXTERN_C int b64_pton (char const *, unsigned char *, size_t);

