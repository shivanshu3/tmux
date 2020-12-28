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
	#define TMUX_PTR_TO_INT(int_type, var) (static_cast<int_type>(reinterpret_cast<uint64_t>(var)))
#else
	#define TMUX_EXTERN_C_BEGIN
	#define TMUX_EXTERN_C_END
	#define TMUX_EXTERN_C
	#define TMUX_EXTERN_CPP
	#define QUALIFIED_MEMBER(x,y) y
	#define TMUX_THROW
	#define TMUX_PTR_TO_INT(int_type, var) ((int_type)var)
#endif
