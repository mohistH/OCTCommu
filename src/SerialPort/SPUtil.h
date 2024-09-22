#ifndef SP_UTIL_H_
#define SP_UTIL_H_
#include <OCTCommu.h>
#if defined(os_is_linux) || defined(os_is_mac)


#include <string>
#include <sys/ioctl.h> // ioctl
#include <termios.h>   // POSIX terminal control definitions
#include <fcntl.h>     // File control definitions
#include <unistd.h>    // UNIX standard function definitions
#include <errno.h>     // Error number definitions
#include <unistd.h> // usleep

class SPUtil
{
public:
	/// 波特率转为常量，比如传入9600， 返回B9600
	static int rate2Constant(int baudrate);

	static int setBaud(int fd, const int baud, struct termios* option);

	static int setParity(int fd, const int parity, struct termios* option);

	static int setDataBit(int fd, const int db, struct termios* option);

	static int setStopbit(int fd, const int stopbits, struct termios* option);

	static int setFlowCtrl(int fd, const int fc, struct termios* option);

	static int setOutputMode(struct termios* option);

	static int setLocalMode(struct termios* option);

	static int  setInputMode(struct termios* option);

	static int setWaitTime(struct termios* option, const int time);

	static int setAtLeastRead(struct termios* option, const int read_min_len);

	static int setDataOverflowFlag(int fd);

	static int activeOption(int fd, const struct termios* option);

	static int setCtrolMode(struct termios* option);

	static int openSP(const std::string& sp_name);

	static int closeSP(int fd);

	static int setBufferSize(int fd, int size);
};

#endif /// !os_is_linux

#endif ///SP_UTIL_H_