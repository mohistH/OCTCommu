#include <TCPClientImp.h>
#include <FixedBuffer.h>
#include <thread>
#include <iostream>
//#include <constdef.h>
namespace oct_commu
{

	/// --------------------------------------------------------------------------------
	/// @brief: TCPClientImp::TCPClientImp
	/// --------------------------------------------------------------------------------
	TCPClientImp::TCPClientImp()
	{

	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPClientImp::~TCPClientImp
	/// --------------------------------------------------------------------------------
	TCPClientImp::~TCPClientImp()
	{
		
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPClientImp::init
	/// --------------------------------------------------------------------------------
	int TCPClientImp::init(const TCPInit& init_param)
	{
		is_to_recv_ = init_param.is_to_recv_;

		/// 调试发现，下面两行代码【不能】交换， 否则， 发送数据接口将返回-1
		/// 1. 先调用 setNetAdapter
		setNetAdapter(init_param.local_ip_);
		/// 2. 再调用  startConnect
		startConnect(init_param.target_ip_, init_param.target_port_, 5.0f);//, init_param.target_port_);

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

		
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPClientImp::send
	/// --------------------------------------------------------------------------------
	int TCPClientImp::sendData(const char* pdata, const size_t len)
	{
		if (true == alive())
		{
			return send(pdata, len);
		}

		setErrorStr("its connecting or the connection is cut off");
		return -2;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPClientImp::getLastErrorMsg
	/// --------------------------------------------------------------------------------
	std::string TCPClientImp::getLastErrorMsg()
	{
		std::string ret;
		{
			std::unique_lock<std::mutex> lock(mtx_erro_msg_);
			ret = error_msg_;
		}
		
		return ret;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPClientImp::onConnect
	/// --------------------------------------------------------------------------------
	void TCPClientImp::onConnect(const toolkit::SockException& ex)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		std::unique_lock<std::mutex> lock(mtx_connect_func_);
		if (on_connect_func_)
		{
			std::string str_errror_tmp(ex.what());

			setErrorStr(str_errror_tmp);

			OnTCPConnectFunc& thread_func = on_connect_func_;
		
			/// 分离
			if (true == tcp_init_.is_detach_recv_)
			{
				/// 线程执行函数函数
				auto thread_task = [&thread_func, &str_errror_tmp]()
				{
					thread_func(str_errror_tmp);
				};

				/// 将接收数据的传给应用层，打入任务队列线程池
				m_recvThreadPool.async(thread_task, false);
			}
			else
			{
				thread_func(str_errror_tmp);
			}
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPClientImp::onRecv
	/// --------------------------------------------------------------------------------
	void TCPClientImp::onRecv(const toolkit::Buffer::Ptr& buf, struct sockaddr* addr, int addr_len)
	{
		if (false == is_to_recv_)
		{
			return;
		}

		{
			RecvBuffer fb;
			fb.append(buf->data(), (size_t)buf->size());
			fb.setEndChar();

			toolkit::SockUtil::get_ip_port_from_addr(addr, is_ipv4_, fb.targetIP(), fb.targetPort(), fb.family());

			/// 如果分离
			if (true == tcp_init_.is_detach_recv_)
			{
				recv_blocking_queue_.put(fb);

				/// 将接收数据的传给应用层，打入任务队列线程池
				m_recvThreadPool.async(std::bind(&TCPClientImp::recvDataWorker, this), false);
			}
			else
			{
				on_recv_data_func_((unsigned char*)fb.data(), fb.length(), fb.targetIP(), fb.targetPort(), fb.family());
			}		
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPClientImp::onErr
	/// --------------------------------------------------------------------------------
	void TCPClientImp::onErr(const toolkit::SockException& ex)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		// mtx_error_func_;
		// std::unique_lock<std::mutex> lock(mtx_error_func_);
		std::lock_guard<std::mutex> lock(mtx_error_func_);
		if (on_tcp_error_func_)
		{
			const std::string str_errror_tmp(ex.what());
			setErrorStr(str_errror_tmp);

			OnTCPErrorFunc& thread_func = on_tcp_error_func_;

			// 分离
			if (true == tcp_init_.is_detach_recv_)
			{
				/// 线程执行函数函数
				auto thread_task = [&thread_func, &str_errror_tmp]()
				{
					thread_func(str_errror_tmp);
				};

				/// 将接收数据的传给应用层，打入任务队列线程池
				m_recvThreadPool.async(thread_task, false);
			}
			else
			{
				thread_func(str_errror_tmp);
			}
		}

		/// 重启连接服务器的定时器
		restartConnectTimer();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPClientImp::onFlush
	/// --------------------------------------------------------------------------------
	void TCPClientImp::onFlush()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		std::unique_lock<std::mutex> lock(mtx_flush_func_);
		if (on_tcp_flush_func_)
		{
			OnTCPFlush& thread_func = on_tcp_flush_func_;

			// 分离
			if (true == tcp_init_.is_detach_recv_)
			{
				/// 线程执行函数函数
				auto thread_task = [&thread_func]()
				{
					thread_func();
				};

				/// 将接收数据的传给应用层，打入任务队列线程池
				m_recvThreadPool.async(thread_task, false);
			}
			else
			{
				thread_func();
			}
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	void TCPClientImp::setErrorStr(const std::string& str)
	{
		std::unique_lock<std::mutex> lock(mtx_erro_msg_);
		error_msg_ = str;
	}

}