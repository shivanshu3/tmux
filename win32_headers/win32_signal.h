#pragma once

struct sigset_t {
	int a;
};

int sigfillset(sigset_t *set);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int killpg(int pgrp, int sig);

#define SIG_BLOCK 1
#define SIG_UNBLOCK 1
#define SIG_SETMASK 1
