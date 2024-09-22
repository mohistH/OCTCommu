#include <SPUtil.h>
#include <SPTypeDef.h>
#include <OCTCommu.h>


#ifdef os_is_mac
#include <sys/param.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>

#define CBAUD 0030017
#endif///!os_is_mac

#if defined(os_is_linux) || defined(os_is_mac)
#include <linux/serial.h>

#ifndef BOTHER
#define BOTHER 0010000
#endif

// linux/include/uapi/asm-generic/ioctls.h
#ifndef TCGETS2
#define TCGETS2 _IOR('T', 0x2A, struct termios2)
#endif

#ifndef TCSETS2
#define TCSETS2 _IOW('T', 0x2B, struct termios2)
#endif




/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::rate2Constant(int baudrate)
{
#define B(x) \
    case x:  \
        return B##x

	switch (baudrate)
	{
#ifdef B50
		B(50);
#endif
#ifdef B75
		B(75);
#endif
#ifdef B110
		B(110);
#endif
#ifdef B134
		B(134);
#endif
#ifdef B150
		B(150);
#endif
#ifdef B200
		B(200);
#endif
#ifdef B300
		B(300);
#endif
#ifdef B600
		B(600);
#endif
#ifdef B1200
		B(1200);
#endif
#ifdef B1800
		B(1800);
#endif
#ifdef B2400
		B(2400);
#endif
#ifdef B4800
		B(4800);
#endif
#ifdef B9600
		B(9600);
#endif
#ifdef B19200
		B(19200);
#endif
#ifdef B38400
		B(38400);
#endif
#ifdef B57600
		B(57600);
#endif
#ifdef B115200
		B(115200);
#endif
#ifdef B230400
		B(230400);
#endif
#ifdef B460800
		B(460800);
#endif
#ifdef B500000
		B(500000);
#endif
#ifdef B576000
		B(576000);
#endif
#ifdef B921600
		B(921600);
#endif
#ifdef B1000000
		B(1000000);
#endif
#ifdef B1152000
		B(1152000);
#endif
#ifdef B1500000
		B(1500000);
#endif
#ifdef B2000000
		B(2000000);
#endif
#ifdef B2500000
		B(2500000);
#endif
#ifdef B3000000
		B(3000000);
#endif
#ifdef B3500000
		B(3500000);
#endif
#ifdef B4000000
		B(4000000);
#endif
		default:
			return 0;
	}

#undef B
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setBaud(int fd, const int baud, struct termios* option)
{
	int ret = 0;

	int baudRateConstant = rate2Constant(baud);
	if (0 != baudRateConstant)
	{
		cfsetispeed(option, baudRateConstant);
		cfsetospeed(option, baudRateConstant);
	}
	else
	{

		struct termios2 tio2;

		if (-1 != ioctl(fd, TCGETS2, &tio2))
		{
			tio2.c_cflag &= ~CBAUD; // remove current baud rate
			tio2.c_cflag |= BOTHER; // allow custom baud rate using int input

			tio2.c_ispeed = baud; // set the input baud rate
			tio2.c_ospeed = baud; // set the output baud rate

			if (-1 == ioctl(fd, TCSETS2, &tio2) || -1 == ioctl(fd, TCGETS2, &tio2))
			{
				fprintf(stderr, "termios2 set custom baudrate error\n");
				ret = -1;
			}
		}
		else
		{
			fprintf(stderr, "termios2 ioctl error\n");
			ret = -2;
		}
	}

	return ret;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setParity(int fd, const int parity, struct termios* poption)
{
	struct termios& options = *poption;
	// 设置校验位
	switch (parity)
	{
		// 无奇偶校验位
		case oct_commu::SPP_None:
			options.c_cflag &= ~PARENB; // PARENB：产生奇偶位，执行奇偶校验
			options.c_cflag &= ~INPCK;  // INPCK：使奇偶校验起作用
			break;
			// 设置奇校验
		case oct_commu::SPP_Odd:
			options.c_cflag |= PARENB; // PARENB：产生奇偶位，执行奇偶校验
			options.c_cflag |= PARODD; // PARODD：若设置则为奇校验,否则为偶校验
			options.c_cflag |= INPCK;  // INPCK：使奇偶校验起作用
			options.c_cflag |= ISTRIP; // ISTRIP：若设置则有效输入数字被剥离7个字节，否则保留全部8位
			break;
			// 设置偶校验
		case oct_commu::SPP_Even:
			options.c_cflag |= PARENB;  // PARENB：产生奇偶位，执行奇偶校验
			options.c_cflag &= ~PARODD; // PARODD：若设置则为奇校验,否则为偶校验
			options.c_cflag |= INPCK;   // INPCK：使奇偶校验起作用
			options.c_cflag |= ISTRIP;  // ISTRIP：若设置则有效输入数字被剥离7个字节，否则保留全部8位
			break;
			// 设置0校验
		case oct_commu::SPP_Space:
			options.c_cflag &= ~PARENB; // PARENB：产生奇偶位，执行奇偶校验
			options.c_cflag &= ~CSTOPB; // CSTOPB：使用两位停止位
			break;
		default:
			fprintf(stderr, "unknown parity\n");
			return -1;
	}

	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setDataBit(int fd, const int dataBits, struct termios* poption)
{
	struct termios& options = *poption;
	switch (dataBits)
	{
		case oct_commu::SPDB_5:
			options.c_cflag &= ~CSIZE; //屏蔽其它标志位
			options.c_cflag |= CS5;
			break;
		case oct_commu::SPDB_6:
			options.c_cflag &= ~CSIZE; //屏蔽其它标志位
			options.c_cflag |= CS6;
			break;
		case oct_commu::SPDB_7:
			options.c_cflag &= ~CSIZE; //屏蔽其它标志位
			options.c_cflag |= CS7;
			break;
		case oct_commu::SPDB_8:
			options.c_cflag &= ~CSIZE; //屏蔽其它标志位
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr, "unknown data bits\n");
			return -1;
	}

	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setStopbit(int fd, const int stopbits, struct termios* poption)
{
	struct termios& options = *poption;

	// 停止位
	switch (stopbits)
	{
		case oct_commu::SPSB_One:
			options.c_cflag &= ~CSTOPB; // CSTOPB：使用两位停止位
			break;
		case oct_commu::SPSB_OneAndHalf:
			fprintf(stderr, "POSIX does not support 1.5 stop bits\n");
			return -1;
		case oct_commu::SPSB_Two:
			options.c_cflag |= CSTOPB; // CSTOPB：使用两位停止位
			break;
		default:
			fprintf(stderr, "unknown stop\n");
			return -1;
	}

	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setFlowCtrl(int fd, const int fc, struct termios* poption)
{
	struct termios& options = *poption;
	// 流控制
	switch (fc)
	{
		case oct_commu::SPFC_None: ///< No flow control 无流控制
			options.c_cflag &= ~CRTSCTS;
			break;
		case oct_commu::SPFC_Hardware: ///< Hardware(RTS / CTS) flow control 硬件流控制
			options.c_cflag |= CRTSCTS;
			break;
		case oct_commu::SPFC_Software: ///< Software(XON / XOFF) flow control 软件流控制
			options.c_cflag |= IXON | IXOFF | IXANY;
			break;
		default:
			fprintf(stderr, "unknown flow control\n");
			return -1;
	}

	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setOutputMode(struct termios* option)
{
	// 设置输出模式为原始输出
	option->c_oflag &= ~OPOST; // OPOST：若设置则按定义的输出处理，否则所有c_oflag失效
	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setLocalMode(struct termios* option)
{
	/*
 *ICANON：允许规范模式进行输入处理
 *ECHO：允许输入字符的本地回显
 *ECHOE：在接收EPASE时执行Backspace,Space,Backspace组合
 *ISIG：允许信号
 */

 //设置本地模式为原始模式
	option->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setInputMode(struct termios* option)
{
	/*
 *BRKINT：如果设置了IGNBRK，BREAK键输入将被忽略
 *ICRNL：将输入的回车转化成换行（如果IGNCR未设置的情况下）(0x0d => 0x0a)
 *INPCK：允许输入奇偶校验
 *ISTRIP：去除字符的第8个比特
 *IXON：允许输出时对XON/XOFF流进行控制 (0x11 0x13)
 */
	option->c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setWaitTime(struct termios* option, const int time)
{
	option->c_cc[VTIME] = time; // 可以在select中设置
	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setAtLeastRead(struct termios* option, const int read_min_len)
{
	option->c_cc[VMIN] = read_min_len;  // 最少读取一个字符

	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setDataOverflowFlag(int fd)
{
	//如果发生数据溢出，只接受数据，但是不进行读操作
	tcflush(fd, TCIFLUSH);
	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::activeOption(int fd, const struct termios* option)
{
	// 激活配置
	if (tcsetattr(fd, TCSANOW, option) < 0)
	{
		perror("tcsetattr failed");
		return -1;
	}

	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setCtrolMode(struct termios* option)
{
	//控制模式
	option->c_cflag |= CLOCAL; //保证程序不占用串口
	option->c_cflag |= CREAD;  //保证程序可以从串口中读取数据

	return 0;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::openSP(const std::string& sp_name)
{
	// std::cout << "\nname=" << sp_name.c_str() << "\n";
	/// //O_NONBLOCK设置为非阻塞模式，在read时不会阻塞住，在读的时候将read放在while循环中，
	return ::open(sp_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::closeSP(int fd)
{
	if (0 < fd)
	{
		::close(fd);
		fd = -1;
	}
	else
	{
		fd = -1;
	}

	return fd;
}


/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
int SPUtil::setBufferSize(int fd, int size)
{
	struct serial_struct ss;
	if (0 != ioctl(fd, TIOCGSERIAL, &ss))
	{
		return 1;
	}
	ss.xmit_fifo_size = size;
	if (0 != ioctl(fd, TIOCSSERIAL, &ss))
	{
		return 2;
	}

	return 0;
}

#endif ///!os_is_linux