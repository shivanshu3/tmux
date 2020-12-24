#pragma once

#ifdef __cplusplus
	#define TMUX_EXTERN_C_BEGIN extern "C" {
	#define TMUX_EXTERN_C_END }
	#define TMUX_EXTERN_C extern "C"
	#define TMUX_EXTERN_CPP extern "C++"
	#define QUALIFIED_MEMBER(x,y) x::y
	#ifdef _WIN32
		#define TMUX_THROW
	#else
		#define TMUX_THROW __THROW
	#endif
#else
	#define TMUX_EXTERN_C_BEGIN
	#define TMUX_EXTERN_C_END
	#define TMUX_EXTERN_C
	#define TMUX_EXTERN_CPP
	#define QUALIFIED_MEMBER(x,y) y
	#define TMUX_THROW
#endif
