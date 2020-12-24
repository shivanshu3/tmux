#pragma once

#include "tmux.h"

enum tty_code_type {
	TTYCODE_NONE = 0,
	TTYCODE_STRING,
	TTYCODE_NUMBER,
	TTYCODE_FLAG,
};

struct tty_code {
	enum tty_code_type	type;
	union {
		char	       *string;
		int		number;
		int		flag;
	} value;
};

struct tty_term_code_entry {
	enum tty_code_type	type;
	const char	       *name;
};

extern const struct tty_term_code_entry tty_term_codes[];

