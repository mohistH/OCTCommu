#ifndef TCP_SERVER_IMP_H_
#define TCP_SERVER_IMP_H_
#include <OCTCommu.h>
#include <Network/TcpServer.h>
#include <TCPCommon.h>
#include <mutex>
#include <ThreadPool.h>
#include <BlockingQueue.h>

namespace oct_commu
{
	class TCPServerImp : public toolkit::TcpSession, public TCPCommon
	{
	public:
		TCPServerImp(const toolkit::Socket::Ptr& sock = nullptr);
		~TCPServerImp();

		/// --------------------------------------------------------------------------------
		/// @brief: init
		/// --------------------------------------------------------------------------------
		int init(const TCPInit& init_param);

		/// --------------------------------------------------------------------------------
		/// @brief: send
		/// --------------------------------------------------------------------------------
		int sendData(const char* pdata, const size_t len);

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

		/**
		 * 接收数据入口
		 * @param buf 数据，可以重复使用内存区,不可被缓存使用
		 */
		void MyonRecv(const unsigned char* pdata, const size_t len, struct sockaddr* addr, int adr_len);

		/**
		 * 收到 eof 或其他导致脱离 Server 事件的回调
		 * 收到该事件时, 该对象一般将立即被销毁
		 * @param err 原因
		 */
		void MyonError(const std::string& err);

		/**
		 * 每隔一段时间触发, 用来做超时管理
		 */
		void MyonManager();

	private:
		void setErrorStr(const std::string& str);

	private:
		toolkit::TcpServer::Ptr		tcp_server_ = nullptr;
	private:
		/// 
		std::mutex			mtx_error_;
		std::string			error_msg_{};
		/// 接收函数
		std::mutex			recv_func_mutex_;
		std::mutex			error_func_mutex_;
		toolkit::ThreadPool m_recvThreadPool{1, toolkit::ThreadPool::PRIORITY_NORMAL, false};
	};
}

#endif ///!TCP_SERVER_IMP_H_