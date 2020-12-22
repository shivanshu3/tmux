#pragma once

struct event_base;

char* osdep_get_name(int, char *);
char* osdep_get_cwd(int);
struct event_base *osdep_event_init(void);
