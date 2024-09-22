#include <TCPServerImp.h>
#include <FixedBuffer.h>
//#include <constdef.h>

namespace oct_commu
{
	/// --------------------------------------------------------------------------------
	/// @brief: TCPServerImp::TCPServerImp
	/// --------------------------------------------------------------------------------
	TCPServerImp::TCPServerImp(const toolkit::Socket::Ptr& sock) 
		: toolkit::TcpSession(sock)
	{
		tcp_server_ = std::make_shared<toolkit::TcpServer>();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPServerImp::~TCPServerImp
	/// --------------------------------------------------------------------------------
	TCPServerImp::~TCPServerImp()
	{

	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPServerImp::init
	/// --------------------------------------------------------------------------------
	int TCPServerImp::init(const TCPInit& init_param)
	{
		tcp_init_ = init_param;

		is_to_recv_ = init_param.is_to_recv_;

		if (tcp_server_)
		{
			{
				auto func = std::bind(&TCPServerImp::MyonRecv, this, std::placeholders::_1, std::placeholders::_2
				, std::placeholders::_3, std::placeholders::_4);
				tcp_server_->setOnRecvDataFunc(func);
			}
			{
				auto func = std::bind(&TCPServerImp::MyonError, this, std::placeholders::_1);
				tcp_server_->setTCPErrorFunc(func);
			}

			if (true == is_to_recv_ && true == tcp_init_.is_detach_recv_)
			{
				m_recvThreadPool.start();
			}

			{
				is_ipv4_ = true;
				int find_pos = init_param.target_ip_.find(':');
				if (0 <= find_pos)
				{
					is_ipv4_ = false;
				}
			}

			tcp_server_->start<TCPServerImp>(init_param.target_port_, init_param.local_ip_);
			return 0;
		}

		setErrorStr(std::string("failed to crerate tcp server"));
		return -2;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPServerImp::sendData
	/// --------------------------------------------------------------------------------
	int TCPServerImp::sendData(const char* pdata, const size_t len)
	{
		if (tcp_server_)
		{
			return  (int)send(pdata, len);
		}

		setErrorStr(std::string("failed to crerate tcp server"));
		return -2;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPServerImp::getLastErrorMsg
	/// --------------------------------------------------------------------------------
	std::string TCPServerImp::getLastErrorMsg()
	{
		std::string ret;
		{
			std::unique_lock<std::mutex> lock(mtx_error_);
			ret = error_msg_;
		}
		return ret;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPServerImp::recvSpeed
	/// --------------------------------------------------------------------------------
	int TCPServerImp::recvSpeed()
	{
		if (tcp_server_)
		{
			return tcp_server_->recvSpeed();
		}
		
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPServerImp::sendSpeed
	/// --------------------------------------------------------------------------------
	int TCPServerImp::sendSpeed()
	{
		if (tcp_server_)
		{
			return tcp_server_->sendSpeed();
		}
		return 0;
	}


	/// --------------------------------------------------------------------------------
	/// @brief: TCPServerImp::MyonRecv
	/// --------------------------------------------------------------------------------
	void TCPServerImp::MyonRecv(const unsigned char* pdata, const size_t len, struct sockaddr* addr, int adr_len)
	{
		OCT_LOCK_GUARD(recv_func_mutex_);
		if (on_recv_data_func_)
		{
			RecvBuffer fb;
			fb.append((char*)pdata, len);
			fb.setEndChar();

			toolkit::SockUtil::get_ip_port_from_addr(addr, is_ipv4_, fb.targetIP(), fb.targetPort(), fb.family());

			/// 如果分离
			if (true == tcp_init_.is_detach_recv_)
			{
				recv_blocking_queue_.put(fb);

				/// 将接收数据的传给应用层，打入任务队列线程池
				m_recvThreadPool.async(std::bind(&TCPServerImp::recvDataWorker, this), false);
			}
			else
			{
				on_recv_data_func_((unsigned char*)fb.data(), fb.length(), fb.targetIP(), fb.targetPort(), fb.family());
			}			
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPServerImp::MyonError
	/// --------------------------------------------------------------------------------
	void TCPServerImp::MyonError(const std::string& err)
	{
		OCT_LOCK_GUARD(error_func_mutex_);
		if (on_tcp_error_func_)
		{
			setErrorStr(err);

			OnTCPErrorFunc& thread_func = on_tcp_error_func_;

			// 分离
			if (true == tcp_init_.is_detach_recv_)
			{
				/// 线程执行函数函数
				auto thread_task = [&thread_func, &err]()
				{
					thread_func(err);
				};

				/// 将接收数据的传给应用层，打入任务队列线程池
				m_recvThreadPool.async(thread_task, false);
			}
			else
			{
				thread_func(err);
			}
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPServerImp::MyonManager
	/// --------------------------------------------------------------------------------
	void TCPServerImp::MyonManager()
	{
		// todo
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	void TCPServerImp::setErrorStr(const std::string& str)
	{
		std::unique_lock<std::mutex> lock(mtx_error_);
		error_msg_ = str;
	}

}