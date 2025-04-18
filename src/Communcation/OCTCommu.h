#ifndef OCT_COMMU_H_
#define OCT_COMMU_H_
#include <string>
#include <functional>
#include <memory>

namespace oct_commu
{
#if defined(_WIN32) || defined(_WIN64)

#ifndef lib_export
#define lib_commu_api __declspec(dllexport)
#else
#define lib_commu_api __declspec(dllimport)

#endif ///

#ifndef os_is_win
	#define os_is_win
#endif ///

#elif defined(__linux__) || defined(__unix__)
#ifndef lib_export
#define lib_commu_api __attribute__((visibility("default")))
#endif ///

#ifndef os_is_linux
#define os_is_linux
#endif ///

#elif defined(__APPLE__)
	#ifndef os_is_mac
		#define os_is_mac
	#endif ///

	#ifndef lib_export
	#define lib_commu_api __attribute__((visibility("default")))
	#endif ///

#endif ///

	/// ----------------------------------------------------------------------------------------
	///				---------------------  分界线  ---------------------------
	/// ----------------------------------------------------------------------------------------

	/// 参数1-收到的数据
	/// 参数2-收到的数据长度
	/// 参数3-谁发过来的，（若时串口，该参数为空）
	/// 参数4-谁从哪个端口发过来的（若为串口，该参数为0）
	/// 参数5-地址族（若为串口，该参数为0）
	

	/// ----------------------------------------------------
	/// @brief 数据接收注册函数原型
	/// 
	/// @param data - 接收的数据内容
	/// @param data_len - 接收的数据长度，单位： 字节数
	/// @param ip - 从哪个地址接收的数据（网络）
	/// @param port - 从哪个端口接收的数据（网络）
	/// @param family - 网络通信使用的地址族
	/// @return int 
	/// ----------------------------------------------------
	using OnNetLinkRecvDataFunc = std::function<int(const unsigned char* data, size_t data_len, std::string ip, int port, int family)>;

	/// 串口链路接收数据函数
	/// @param spName - 串口号
	/// @param baud - 波特率
	/// @param parity - 校验
	/// @param dataBit - 数据位
	/// @param stopBit - 停止位
	/// @param flowCtrl - 流控制
	using OnSPLinkRecvDataFunc = std::function<void(const unsigned char* pdata, const size_t dataLen, const std::string spName, const int baud, const int parity, const int dataBit, const int stopBit, const int flowctrl)>;

	/// tcp connect触发的函数
	using OnTCPConnectFunc = std::function<void(std::string& msg)>;

	/// 发送数据阻塞后， 清空缓存事件
	using OnTCPFlush = std::function<void()>;
	
	/// TCP错误事件
	using OnTCPErrorFunc = std::function<void(const std::string& error_msg)>;

	/// 
	using OnErrorFunc = OnTCPErrorFunc;
	using OnSendResultFunc = std::function<void(const unsigned char* pdata,  size_t len, const bool is_success)>;

	/// ----------------------------------------------------------------------------------------
	/// @brief: 
	/// ----------------------------------------------------------------------------------------
	class lib_commu_api NonCopyable
	{
	protected:
		NonCopyable() {}
		virtual ~NonCopyable() {}

	private:
		NonCopyable(const NonCopyable&);
		NonCopyable& operator=(const NonCopyable&);
	};

	/// ----------------------------------------------------------------------------
	/// @brief: UDP类型
	/// ----------------------------------------------------------------------------
	enum CAST_TYPE : int
	{
		/// 单播
		CT_UNI = 1,
		/// 组播
		CT_MULTI = 2,
		/// 广播
		CT_BROAD = 3,
	};

	/// ----------------------------------------------------------------------------
	/// @brief: UDP初始化参数
	/// ----------------------------------------------------------------------------
	struct UDPInit
	{
	public:
		/// 组播类型
		CAST_TYPE		cast_type_{ CT_UNI };
		/// 本机IP
		std::string		local_ip_{"127.0.0.1"};
		/// 目标IP
		std::string		target_ip_{"127.0.0.1"};
		/// 目标端口
		uint16_t		target_port_{ 10086 };
		/// 是否接收数据
		bool			is_to_recv_ = false;
		/// 数据接收线程是否与轮询分离。 如果设置为分离，及轮询线程和函数回调线程位于同一个线程
		bool			is_detach_recv_ = false;
	};

	class UDPImp;
	/// ----------------------------------------------------------------------------
	/// @brief: udp通信类
	///			使用顺序：	1. init初始化
	///						2. setOnRead设置是否接收数据
	///						3. 调用send发送数据
	///						5. 调用shutdown断开
	/// ----------------------------------------------------------------------------
	class lib_commu_api IUDP : public NonCopyable
	{
	public:
		IUDP();
		virtual ~IUDP();

		/// ----------------------------------------------------------------------------
		/// @brief：		初始化
		/// @param: 	const UDPInitParams & param - UDP初始化参数
		/// @return: 	int 
		///					0 - 成功
		///					!= 0 - 失败， 内部错误，请调用 [getLastErrorMsg] 获取错误信息
		/// ----------------------------------------------------------------------------
		int init(const UDPInit& param);

		/// --------------------------------------------------------------------------------
		/// @brief: 获取接收速率，单位： bytes/s
		/// --------------------------------------------------------------------------------
		int recvSpeed();

		/// --------------------------------------------------------------------------------
		/// @brief: 获取发送速率，单位： bytes/s
		/// --------------------------------------------------------------------------------
		int sendSpeed();

		/// --------------------------------------------------------------------------------
		/// @brief: 发送数据,
		///			发送成功-返回数值大于0， 表示已经发送得字节数
		///			发送失败-返回数值为非正整数
		/// --------------------------------------------------------------------------------
		int send(const char* pdata, const size_t len);

		/// --------------------------------------------------------------------------------
		/// @brief: 释放资源
		/// --------------------------------------------------------------------------------
		int shutdown();

		/// --------------------------------------------------------------------------------
		/// @brief: 获取错误信息
		/// --------------------------------------------------------------------------------
		std::string lastErrorMsg();

		/// --------------------------------------------------------------------------------
		/// @brief: 设置接收函数
		/// --------------------------------------------------------------------------------
		int setOnReadFunc(OnNetLinkRecvDataFunc cb);

	private:
		void uninit();

	private:
		UDPImp*		d_ptr_ = nullptr;
	};

	enum TCP_TYPE : int
	{
		/// 服务端
		TT_SERVER = 1,
		/// 客户端
		TT_CLIENT = 2,
	};

	/// ----------------------------------------------------------------------------------------
	/// @brief: TCP初始化参数
	/// ----------------------------------------------------------------------------------------
	struct TCPInit
	{
		/// TCP类型， 默认， TCP客户端
		TCP_TYPE	tcp_type_{ TT_CLIENT };
		/// 目标IP
		std::string	target_ip_{"127.0.0.1"};
		/// 目标端口. 
		uint16_t	target_port_{10086};
		/// 本机IP
		std::string local_ip_{"127.0.0.1"};
		/// 是否接收数据: true -接收， false-不接收
		bool		is_to_recv_ = false;
		/// 数据接收线程是否与轮询分离。 如果设置为分离，及轮询线程和函数回调线程位于同一个线程
		bool			is_detach_recv_ = false;
	};

	class TCPImp;
	/// ----------------------------------------------------------------------------------------
	/// @brief: TCP使用
	///			使用顺序：	1. init初始化
	///						2. setOnRead设置是否接收数据 和 setOnErrorFunc 设置当出现错误时得回调函数
	///						3. 调用send发送数据
	///						5. 调用shutdown断开
	/// ----------------------------------------------------------------------------------------
	/// ----------------------------------------------------------------------------------------
	class lib_commu_api ITCP : public NonCopyable
	{
	public:
		ITCP();
		virtual ~ITCP();

		/// -------------------------------------------------------------------------------
		/// @brief:		初始化
		/// @param: 	const TCPInit & init_param - 初始化参数
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int init(const TCPInit& init_param);

		/// --------------------------------------------------------------------------------
		/// @brief: 获取接收速率， 单位：bytes/s
		/// --------------------------------------------------------------------------------
		int recvSpeed();

		/// --------------------------------------------------------------------------------
		/// @brief: 发送速率： 单位： bytes/s
		/// --------------------------------------------------------------------------------
		int sendSpeed();

		/// --------------------------------------------------------------------------------
		/// @brief: 发送数据,
		///			发送成功-返回数值大于0， 表示已经发送得字节数
		///			发送失败-返回数值为非正整数
		/// --------------------------------------------------------------------------------
		int send(const char* pdata, const size_t len);

		/// --------------------------------------------------------------------------------
		/// @brief: 释放资源
		/// --------------------------------------------------------------------------------
		int shutdown();

		/// --------------------------------------------------------------------------------
		/// @brief: 获取错误信息
		/// --------------------------------------------------------------------------------
		std::string lastErrorMsg();

		/// -------------------------------------------------------------------------------
		/// @brief:		指定接收数据函数
		/// @param: 	OnRecvDataFunc cb - 回调函数,
		///  @ret:		void
		///				
		/// -------------------------------------------------------------------------------
		int setOnReadFunc(OnNetLinkRecvDataFunc cb);

		/// -------------------------------------------------------------------------------
		/// @brief:		TCP错误事件处理
		/// @param: 	OnTCPError cb - 
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int setOnErrorFunc(OnTCPErrorFunc cb);

		/// -------------------------------------------------------------------------------
		/// @brief:		连接事件处理函数
		/// @param: 	OnTCPConnectFunc cb - 
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int setOnConnectFunc(OnTCPConnectFunc cb);

		/// -------------------------------------------------------------------------------
		/// @brief:		设置发送数据阻塞后清空缓存事件处理函数
		/// @param: 	OnTCPFlush cb - 
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int setOnFlushFunc(OnTCPFlush cb);

	private:
		void uninit();

	private:
		TCPImp* d_ptr_ = nullptr;
	};

	/// ----------------------------------------------------------------------------------------
	/// @brief: 串口初始化参数定义开始
	/// ----------------------------------------------------------------------------------------
	
	/// ----------------------------------------------------------------------------------------
	/// @brief: 校验
	/// ----------------------------------------------------------------------------------------
	enum SP_Parity : int 
	{
		SPP_None	= 0,  ///< No Parity 无校验
		SPP_Odd		= 1,  ///< Odd Parity 奇校验
		SPP_Even	= 2,  ///< Even Parity 偶校验
		SPP_Mark	= 3,  ///< Mark Parity 1校验
		SPP_Space	= 4,  ///< Space Parity 0校验
	};

	enum SP_DataBits : int 
	{
		SPDB_5 = 5, ///< 5 data bits 5位数据位
		SPDB_6 = 6, ///< 6 data bits 6位数据位
		SPDB_7 = 7, ///< 7 data bits 7位数据位
		SPDB_8 = 8  ///< 8 data bits 8位数据位
	};


	enum SP_StopBits : int 
	{
		SPSB_One = 0,        ///< 1 stop bit 1位停止位
		SPSB_OneAndHalf = 1, ///< 1.5 stop bit 1.5位停止位 - This is only for the Windows platform
		SPSB_Two = 2         ///< 2 stop bit 2位停止位
	};

	enum SP_FlowControl : int 
	{
		SPFC_None = 0,     ///< No flow control 无流控制
		SPFC_Hardware = 1, ///< Hardware(RTS / CTS) flow control 硬件流控制
		SPFC_Software = 2  ///< Software(XON / XOFF) flow control 软件流控制
	};

	/// ----------------------------------------------------------------------------------------
	/// @brief: 串口初始化参数
	/// ----------------------------------------------------------------------------------------
	struct SP_INIT
	{
		/// 串口号
		std::string		name_={""};
		/// 波特率
		size_t			baud_ = 9600;
		/// 校验位
		SP_Parity		parity_ = SPP_Even;
		/// 数据位
		SP_DataBits		data_bits_ = SPDB_8;
		/// 停止位
		SP_StopBits		stop_bits_ = SPSB_One;
		/// 流控制
		SP_FlowControl	flow_ctrl_ = SPFC_None;
#if defined(os_is_linux)
		/// 轮询超时，单位： 毫秒
		size_t			recv_interval_timeout_ = 3;
#endif ///!os_is_linux
		/// 是否接收数据
		bool			is_to_recv_ = false;
	};
	using SPInit = SP_INIT;

	/// ----------------------------------------------
	/// @brief: 串口超时设置
	/// ----------------------------------------------
	struct SP_TIMEOUTS
	{
#if defined(os_is_win)
		uint32_t		read_internal_timeout_ = 0;
		uint32_t		read_total_timeout_multiple_ = 0;
		uint32_t		read_total_timeout_constant_ = 0;
		uint32_t		write_total_timeout_multiple_ = 0;
		uint32_t		write_total_timeout_constant_ = 0;
#endif ///!os_is_win
	};

	using SPTimeouts = SP_TIMEOUTS;

#ifdef os_is_win
	class SPWin;
#else
	class SPUnix;
#endif ///!

	/// ----------------------------------------------------------------------------------------
	/// @brief: 串口通信协议
	/// ----------------------------------------------------------------------------------------
	class lib_commu_api ISP : public NonCopyable
	{ 
	public:
		ISP();
		virtual ~ISP();

		/// -------------------------------------------------------------------------------
		/// @brief:		初始化
		/// @param: 	const SPInit & sp_init - 
		///  @ret:		int
		///				0 - 成功
		/// -------------------------------------------------------------------------------
		int init(const SPInit& sp_init);

		/// --------------------------------------------------------------------------------
		/// @brief: 获取接收速率， 单位：bytes/s
		/// --------------------------------------------------------------------------------
		int recvSpeed();

		/// --------------------------------------------------------------------------------
		/// @brief: 发送速率： 单位： bytes/s
		/// --------------------------------------------------------------------------------
		int sendSpeed();

		/// --------------------------------------------------------------------------------
		/// @brief: 发送数据,
		///			发送成功-返回数值大于0， 表示已经发送的字节数
		///			发送失败-返回数值为非正整数, 调用 get lasterrormsg获取错误信息
		/// --------------------------------------------------------------------------------
		int send(const char* pdata, size_t len);

		/// --------------------------------------------------------------------------------
		/// @brief: 释放资源
		/// --------------------------------------------------------------------------------
		int shutdown();

		/// --------------------------------------------------------------------------------
		/// @brief: 获取错误信息
		/// --------------------------------------------------------------------------------
		std::string lastErrorMsg();

		/// -------------------------------------------------------------------------------
		/// @brief:		指定接收数据函数
		/// @param: 	OnSPLinkRecvDataFunc cb - 回调函数,
		///  @ret:		void
		///				0 - 成功
		///				!= 0 失败 			
		/// -------------------------------------------------------------------------------
		int setOnReadFunc(OnSPLinkRecvDataFunc cb);

		/// -------------------------------------------------------------------------------
		/// @brief:		设置下一次读取数据长度
		/// @param: 	const unsigned int len - 读取数据长度
		///  @ret:		int
		///				0 - 成功
		///				!= 0 失败 			
		/// -------------------------------------------------------------------------------
		int setNextReadLen(unsigned int len);

		/// -------------------------------------------------------------------------------
		/// @brief:		挂起串口
		/// @param: 	const bool flag - true-挂起， false-恢复
		///  @ret:		int
		///				0 - 成功
		///				!= 0 失败 
		/// -------------------------------------------------------------------------------
		int suspend(bool flag);
		

		/// -------------------------------------------------------------------------------
		/// @brief:		设置超时
		/// @param: 	const SP_TIMEOUTS & st - 
		///  @ret:		int
		///				0 - 成功
		///				!= 0 失败 	
		/// -------------------------------------------------------------------------------
		int setTimeouts(const SPTimeouts& st);

		/// -------------------------------------------------------------------------------
		/// @brief:		获取超时
		/// @param: 	SPTimeouts & out_value - 超时结果
		///  @ret:		int
		///				0 - 成功
		///				!= 0 失败 
		/// -------------------------------------------------------------------------------
		int timeouts(SPTimeouts& out_value);
	private:
#ifdef os_is_win
		SPWin* d_ptr_;
#else
		SPUnix* d_ptr_;
#endif ///!
	};

}

#endif