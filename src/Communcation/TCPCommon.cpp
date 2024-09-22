#include <TCPCommon.h>

namespace oct_commu
{

	/// --------------------------------------------------------------------------------
	/// @brief: TCPCommon::TCPCommon
	/// --------------------------------------------------------------------------------
	TCPCommon::TCPCommon()
	{
		is_to_recv_ = false;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPCommon::~TCPCommon
	/// --------------------------------------------------------------------------------
	TCPCommon::~TCPCommon()
	{

	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPCommon::setOnReadFunc
	/// --------------------------------------------------------------------------------
	int TCPCommon::setOnReadFunc(OnRecvDataFunc cb)
	{
		std::unique_lock<std::mutex> lock(mtx_recv_data_func_);
		on_recv_data_func_ = std::move(cb);
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPCommon::setOnConnectFunc
	/// --------------------------------------------------------------------------------
	int TCPCommon::setOnConnectFunc(OnTCPConnectFunc cb)
	{
		std::unique_lock<std::mutex> lock(mtx_connect_func_);
		on_connect_func_ = std::move(cb);
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPCommon::setOnFlushFunc
	/// --------------------------------------------------------------------------------
	int TCPCommon::setOnFlushFunc(OnTCPFlush cb)
	{
		std::unique_lock<std::mutex> lock(mtx_flush_func_);
		on_tcp_flush_func_ = std::move(cb);
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPCommon::setOnErrorFunc
	/// --------------------------------------------------------------------------------
	int TCPCommon::setOnErrorFunc(OnTCPErrorFunc cb)
	{
		std::unique_lock<std::mutex> lock(mtx_error_func_);
		on_tcp_error_func_ = std::move(cb);
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 接收函数数据触发函数
	/// --------------------------------------------------------------------------------
	void TCPCommon::recvDataWorker()
	{
		std::unique_lock<std::mutex> lock(mtx_recv_data_func_);
		if (on_recv_data_func_)
		{
			/// 1. 取出来
			RecvBuffer rb = recv_blocking_queue_.take();
			if (0 == rb.currentLen())
			{
				return;
			}
			on_recv_data_func_((const unsigned char*)rb.data(), (size_t)rb.currentLen(), 
			rb.targetIP(), rb.targetPort(), rb.family(), 
			std::string{}, 0, 0, 0, 0, 0);
		
		}
	}

}