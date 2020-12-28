#pragma once

struct utsname {
	char sysname[64];    /* Operating system name (e.g., "Linux") */
	char nodename[64];   /* Name within "some implementation-defined network" */
	char release[64];    /* Operating system release (e.g., "2.6.28") */
	char version[64];    /* Operating system version */
	char machine[64];    /* Hardware identifier */
};

int uname(struct utsname *buf);
