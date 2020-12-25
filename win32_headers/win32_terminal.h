#pragma once

struct termios
{
	int a;
};

// WIN32_TODO: Code which calls these functions likely needs to be rearchitected
int tcgetattr(int fd, struct termios *termios_p);
