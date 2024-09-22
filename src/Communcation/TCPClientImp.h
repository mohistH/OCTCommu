#ifndef TCP_CLIENT_IMP_H_
#define TCP_CLIENT_IMP_H_
#include <OCTCommu.h>
#include <Network/TcpClient.h>
#include <TCPCommon.h>
#include <ThreadPool.h>

namespace oct_commu
{
	class TCPClientImp : public toolkit::TcpClient, public TCPCommon
	{
	public:
		TCPClientImp();
		~TCPClientImp();

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

	protected:
		/// --------------------------------------------------------------------------------
		/// @brief: 连接上服务器回调
		/// --------------------------------------------------------------------------------
		virtual void onConnect(const toolkit::SockException& ex) override;


		/// --------------------------------------------------------------------------------
		/// @brief: 数据接收函数
		/// --------------------------------------------------------------------------------
		virtual void onRecv(const toolkit::Buffer::Ptr& buf, struct sockaddr* addr, int addr_len) override;


		/// --------------------------------------------------------------------------------
		/// @brief: 被动断开时的回调
		/// --------------------------------------------------------------------------------
		virtual void onErr(const toolkit::SockException& ex) override;


		/// --------------------------------------------------------------------------------
		/// @brief: 数据发送
		/// --------------------------------------------------------------------------------
		virtual void onFlush() override;

		void setErrorStr(const std::string& str);

	private:
		std::mutex mtx_erro_msg_;
		/// 、错误信息
		std::string error_msg_{""};
		/// 
		toolkit::ThreadPool m_recvThreadPool{ 1, toolkit::ThreadPool::PRIORITY_NORMAL, false };
	};
}

#endif /// !TCP_CLIENT_IMP_H_