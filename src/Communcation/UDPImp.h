#ifndef UDP_IMP_H_
#define UDP_IMP_H_
#include <OCTCommu.h>
#include <Socket.h>
#include <memory>
#include <SocketToken.h>
#include <Buffer.h>
#include <NetCallback.h>
#include <BlockingQueue.h>
#include <FixedBuffer.h>

namespace oct_commu
{
	/// IPVX地址
	template<typename TSockAddrType>
	struct TSockAddr_s
	{
	public:
		TSockAddr_s()
		{
			memset(this, 0, sizeof(TSockAddr_s));
		}

		size_t sockAddrLen()
		{
			return sizeof(TSockAddrType);
		}

		/// 目标地址
		TSockAddrType	m_destSockAddr;
		/// 本机地址
		TSockAddrType	m_localSockAddr;
	};

	/// ----------------------------------------------------------------------------------------
	/// @brief: 地址信息: 目标IP和本机IP
	/// ----------------------------------------------------------------------------------------
	struct SockAddress_
	{
	public:
		SockAddress_()
		{
			memset(this, 0, sizeof(SockAddress_));
		}

		TSockAddr_s<struct sockaddr_in>			ipv4_;
		TSockAddr_s<struct sockaddr_in6>		ipv6_;
	};

	typedef SockAddress_ SockAddress;

	/// ----------------------------------------------------------------------------------------
	/// @brief: UDP实现类
	/// ----------------------------------------------------------------------------------------
	class UDPImp : public SocketToken
	{
		enum
		{
			RECV_BUF_LEN = 1024 * 256,
		};

		using RecvBuf = oct_tk::FixedBuffer<RECV_BUF_LEN>;

	public:
		UDPImp();
		virtual ~UDPImp();

		UDPImp(const UDPImp& instance) = delete;
		UDPImp(const UDPImp&& instance) = delete;
		UDPImp& operator = (const UDPImp& instance) = delete;
		UDPImp& operator = (const UDPImp&& instance) = delete;

		/// --------------------------------------------------------------------------------
		/// @brief: init
		/// --------------------------------------------------------------------------------
		int init(const UDPInit& param);

		/// --------------------------------------------------------------------------------
		/// @brief: send
		/// --------------------------------------------------------------------------------
		int send(const char* pdata, const size_t len);

		/// --------------------------------------------------------------------------------
		/// @brief: unInit
		/// --------------------------------------------------------------------------------
		int shutdown();

		/// --------------------------------------------------------------------------------
		/// @brief: getLastErrorMsg
		/// --------------------------------------------------------------------------------
		std::string getLastErrorMsg();


		/// --------------------------------------------------------------------------------
		/// @brief: recvSpeed
		/// --------------------------------------------------------------------------------
		int recvSpeed();


		/// --------------------------------------------------------------------------------
		/// @brief: sendSpeed
		/// --------------------------------------------------------------------------------
		int sendSpeed();

		int setOnRead(OnRecvDataFunc cb);

	private:
		/// -------------------------------------------------------------------------------
		/// @brief:		初始化套接字
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int initSocket();

		/// -------------------------------------------------------------------------------
		/// @brief:		地址转换
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int toSockAddress();

		/// -------------------------------------------------------------------------------
		/// @brief:		绑定套接字
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int bindSock();

		/// -------------------------------------------------------------------------------
		/// @brief:		设置回调
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int setOnReadDataFunc();

		/// -------------------------------------------------------------------------------
		/// @brief:		initCast
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int initCast();

		/// -------------------------------------------------------------------------------
		/// @brief:		设置错误消息
		/// @param: 	const std::string & str - 新值
		///  @ret:		void
		///				
		/// -------------------------------------------------------------------------------
		void setErrorMsg(const std::string& str);

		void OnReadDataFunc(const toolkit::Buffer::Ptr& buf, struct sockaddr* addr, int addr_len);

		///线程池函数
		void recvWorkerThread();

	private:
		/// 初始化参数
		UDPInit		m_udpIntParam;

		/// 是否已经初始化
		bool				m_hasInited = false;

		/// 通信socket
		std::shared_ptr<toolkit::Socket>	m_pUDP = nullptr;

		/// 网络地址
		SockAddress			m_sockAddress;

		/// 错误消息
		std::string			m_errorMsg;

		/// 
		std::recursive_mutex	m_errorMsgMutex;

		/// 
		std::recursive_mutex	m_send_mutex;

		/// sock
		struct sockaddr*			m_pSendAddr = nullptr;
		int						m_sockAddrLen = 0;

		/// 接收数据回调函数
		std::mutex				m_recvDataFunxMtx;
		OnRecvDataFunc			m_recvDataFunc = nullptr;
		/// 数据接收缓存
		BlockingQueue<RecvBuf, 128>		block_queue_recv_buf_;

		toolkit::ThreadPool		m_recvThreadPool{ 1, toolkit::ThreadPool::PRIORITY_NORMAL, false };
		/// 是否为IPV6 , 默认 IPv4
		std::atomic_bool		m_isIPV6 = false;
	};
}


#endif /// UDP_IMP_H_