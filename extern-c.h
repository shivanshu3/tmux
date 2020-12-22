#pragma once

#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#define EXTERN_C extern "C"
#define EXTERN_CPP extern "C++"
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#define EXTERN_C
#define EXTERN_CPP
#endif

