#ifndef SP_TYPEDEF_H_
#define SP_TYPEDEF_H_
#include <OCTCommu.h>

#ifdef os_is_mac
#include <sys/param.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#endif///!os_is_mac

#ifndef _WIN32

// termios2 for custom baud rate at least linux kernel 2.6.32 (RHEL 6.0)

// linux/include/uapi/asm-generic/termbits.h
struct termios2
{
	tcflag_t c_iflag; /* input mode flags */
	tcflag_t c_oflag; /* output mode flags */
	tcflag_t c_cflag; /* control mode flags */
	tcflag_t c_lflag; /* local mode flags */
	cc_t c_line;      /* line discipline */
	cc_t c_cc[19];    /* control characters */
	speed_t c_ispeed; /* input speed */
	speed_t c_ospeed; /* output speed */
};

#endif ///!
#endif ///!SP_TYPEDEF_H_