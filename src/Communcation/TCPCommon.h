#ifndef TCP_COMMON_H_
#define TCP_COMMON_H_
#include <OCTCommu.h>
#include <mutex>
#include <atomic>
#include <BlockingQueue.h>
#include <FixedBuffer.h>

namespace oct_commu
{
	class TCPCommon
	{
	public:
		enum
		{
			MAX_RECV_BUF_LEN = 1024 * 256,
		};

		using RecvBuffer = oct_tk::FixedBuffer<MAX_RECV_BUF_LEN>;

		using RecvBufferQueue = BlockingQueue<RecvBuffer, 128>;

	public:
		TCPCommon();
		virtual ~TCPCommon();

		/// -------------------------------------------------------------------------------
		/// @brief:		指定接收数据函数
		/// @param: 	OnRecvDataFunc cb - 回调函数,
		///  @ret:		void
		///				
		/// -------------------------------------------------------------------------------
		int setOnReadFunc(OnNetLinkRecvDataFunc cb);

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

		/// -------------------------------------------------------------------------------
		/// @brief:		TCP错误事件处理
		/// @param: 	OnTCPError cb - 
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int setOnErrorFunc(OnTCPErrorFunc cb);

		/// 接收函数数据触发函数
		void recvDataWorker();

	public:
		std::mutex			mtx_recv_data_func_;

		/// 接收数据
		OnNetLinkRecvDataFunc		on_recv_data_func_ = nullptr;
		
		std::mutex			mtx_connect_func_;
		/// tcp连接
		OnTCPConnectFunc	on_connect_func_ = nullptr;
		std::mutex			mtx_flush_func_;
		/// 
		OnTCPFlush			on_tcp_flush_func_ = nullptr;
		
		std::mutex			mtx_error_func_;
		///
		OnTCPErrorFunc			on_tcp_error_func_ = nullptr;
		
		/// 数据接收buffer
		RecvBufferQueue			recv_blocking_queue_;
		/// 是否接收数据
		std::atomic_bool		is_to_recv_;

		/// 是否为IPV4
		std::atomic_bool		is_ipv4_ = true;
		/// 链路对应的配置
		TCPInit					tcp_init_;
	
	};
}

#endif ///!TCP_COMMON_H_