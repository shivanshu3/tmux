#pragma once

typedef int tcflag_t;
typedef char cc_t;
typedef int speed_t;

#define NCCS 64

struct termios
{
	tcflag_t c_iflag;      /* input modes */
	tcflag_t c_oflag;      /* output modes */
	tcflag_t c_cflag;      /* control modes */
	tcflag_t c_lflag;      /* local modes */
	cc_t c_cc[NCCS];   /* special characters */
};

struct winsize
{
	int ws_row;
	int ws_col;
};

typedef struct TERMINAL
{
	int a;
} TERMINAL;

extern TERMINAL *cur_term;

// WIN32_TODO: Code which calls these functions likely needs to be rearchitected
int tcgetattr(int fd, struct termios *termios_p);
char* ttyname(int fd);
speed_t cfgetispeed(const struct termios *termios_p);
speed_t cfgetospeed(const struct termios *termios_p);
int cfsetispeed(struct termios *termios_p, speed_t speed);
int cfsetospeed(struct termios *termios_p, speed_t speed);
int setupterm(const char *term, int filedes, int *errret);
int tigetflag(const char *capname);
int tigetnum(const char *capname);
char *tigetstr(const char *capname);
int del_curterm(TERMINAL *oterm);
char *tparm(const char *str, ...);
