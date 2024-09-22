#ifndef SP_UNIX_H_
#define SP_UNIX_H_
#include <mutex>
#include <FixedBuffer.h>
#include <OCTCommu.h>
#include <ThreadPlus.h>
#include <StreamBuffer.h>
#include <ThreadPool.h>

#if  defined(os_is_linux) || defined(os_is_mac)
#include <ElapsedTimer.h>

namespace oct_sp
{
	template<size_t SIZE=1024 * 4>
	class SPBuffer
	{
	public:
		SPBuffer() 
		{
			reset();
		}
		~SPBuffer() {}


		int data(SPBuffer & ins)
		{
			if (this == &ins)
			{
				return 2;
			}
			//if (0 == avail())
			//{
			//	return 1;
			//}

			ins.reset();
			ins.append(current(), validLen());

			begin_ = end_;

			return 0;
		}

		/// 追加数据
		int append(const unsigned char* pdata, const size_t len)
		//void operator << (const unsigned char* pdata, const size_t len)
		{
			if (len > SIZE)
			{
				return 1;
			}

			/// 放不下数据了
			if (len > avail())
			{
				/// 将数据移动到0的起始位置
				if (0 != begin_)
				{
					size_t valid_len = validLen();
					memmove(data_ + 0, data_ + begin_, valid_len);
					/// 重置计数器
					begin_ = 0;
					end_ = valid_len;

					/// 移动后，如还是放不下，则返回
					if (len > avail())
					{
						return 2;
					}
				}
				/// 起始位置为0， 无法存放当前数据
				else
				{
					return 3;
				}
			}

			/// 能放下，将数据放入buf中
			memcpy(data_ + begin_, pdata, len);
			/// 移动计数器
			end_ += len;

			return 0;
		}

		size_t validLen()
		{
			return end_ - begin_;
		}

		const unsigned char* current() const 
		{
			return data_ + begin_;
		}

		bool addBegin(size_t offset)
		{
			if (end_ < (begin_ + offset))
			{
				return false;
			}

			begin_ += offset;
			return true;
		}

		void reset()
		{
			memset(data_, 0, SIZE);
			begin_ = 0;
			end_ = 0;
		}

	private:
		size_t avail()
		{
			return SIZE - end_;
		}


	private:
		unsigned char data_[SIZE] = {0};
		size_t				begin_ = 0;
		size_t				end_ = 0;
	};
}

namespace oct_commu
{
	class SPUnix
	{
		enum
		{
			/// 最大接收缓存大小： 单位：字节
			MAX_RECV_BUFFER_LEN = 10 * 1024,
			/// 发送缓存
			MAX_SEND_BUFFER_LEN = 1024 * 10,
		};
	public:
		explicit SPUnix(void);
		virtual ~SPUnix(void);

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
		int setOnReadFunc(OnRecvDataFunc cb);

		/// -------------------------------------------------------------------------------
		/// @brief:		设置下一次读取数据长度
		/// @param: 	const unsigned int len - 读取数据长度
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int setNextReadLen(const unsigned int len);

		/// -------------------------------------------------------------------------------
		/// @brief:		挂起串口
		/// @param: 	const bool flag - true-挂起， false-恢复
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int suspend(const bool flag);

		/// -------------------------------------------------------------------------------
		/// @brief:		设置超时
		/// @param: 	const SP_TIMEOUTS & st - 
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int setTimeouts(const SPTimeouts& st);

		/// -------------------------------------------------------------------------------
		/// @brief:		获取超时
		/// @param: 	SPTimeouts & out_value - 
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int timeouts(SPTimeouts& out_value);

	private:
		/// -------------------------------------------------------------------------------
		/// @brief:		串口是否打开
		///  @ret:		bool
		///				
		/// -------------------------------------------------------------------------------
		bool isOpen();

		/// -------------------------------------------------------------------------------
		/// @brief:		错误信息
		/// @param: 	const std::string & str - 
		///  @ret:		void
		///				
		/// -------------------------------------------------------------------------------
		void setErrorMsg(const std::string& str);

		/// -------------------------------------------------------------------------------
		/// @brief:		初始化串口信息
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int initSP();

		/// -------------------------------------------------------------------------------
		/// @brief:		设置串口属性
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int initSPProperty();

	private:
		/// -------------------------------------------------------------------------------
		/// @brief:		poller线程
		///  @ret:		void
		///				
		/// -------------------------------------------------------------------------------
		void pollerThreadFunc();

		/// -------------------------------------------------------------------------------
		/// @brief:		读取事件
		///  @ret:		void
		///				
		/// -------------------------------------------------------------------------------
		void readEvent(fd_set* rd_set);
		
		///
		void writeEvent(fd_set* rd_set);

		/// 刷入数据
		void flushAllData();

		/// 处理接收的数据
		void onReadData(const unsigned char* pdata, const size_t len);

		/// -------------------------------------------------------------------------------
		/// @brief:		将数据处理后交给应用层
		/// @param: 	const unsigned char * pdata - 
		/// @param: 	const size_t len - 
		///  @ret:		void
		///				
		/// -------------------------------------------------------------------------------
		void pushData2Client();

	private:
		/// 接收函数
		std::mutex			mtx_recv_func_;
		/// 接收数据函数
		OnRecvDataFunc		recv_func_ = nullptr;
		/// 错误信息
		std::string			error_str_ = std::string("");
		/// 初始化参数
		SPInit				sp_init_;
		/// 下一次读取数据长度
		std::atomic_uint32_t	next_read_len_;
		/// 串口标识
		int					sp_fd_ = -1;
		/// 错误信息 互斥
		std::mutex			mtx_error_;
		/// 当前操作
		std::mutex			mtx_fd_;
		/// 接收数据线程，
		ThreadPlus			thread_poller_;
		/// 接收数据缓冲使用的buffer, 目前使用1024 * 10 = 10K大小
		oct_tk::FixedBuffer<MAX_RECV_BUFFER_LEN>	recv_buffer_;
		/// 接收线程是否运行
		std::atomic_bool	recv_thread_is_running_;
		/// 是否开启监听写
		std::atomic_bool	listen_to_write_;

		/// 发送二级缓存
		oct_tk::FixedBuffer<MAX_SEND_BUFFER_LEN>	send_l2_cache_;
		/// 发送一级缓存
		oct_tk::FixedBuffer<MAX_SEND_BUFFER_LEN>	send_l1_cache_;
		/// 用作发送时切换发送缓存使用
		oct_tk::FixedBuffer<MAX_SEND_BUFFER_LEN>	send_swap_cache_;

		/// 
		std::recursive_mutex			mtx_send_l2_cache_;
		std::recursive_mutex			mtx_send_l1_cache_;

		/// 用于模拟串口接收定长数据
		oct_sp::SPBuffer<MAX_SEND_BUFFER_LEN>		sp_buf_;
		std::recursive_mutex			mtx_sp_read_buf_;

		/// 是否发挂起串口
		std::atomic_bool				sp_suspend_;

		ElapsedTimer					m_ElapsedTimerRecv;
		ElapsedTimer					m_ElapsedTimerSend;

		/// 发送字节数
		std::atomic_uint64_t			sent_bytes_;
		/// 接收字节数
		std::atomic_uint64_t			recv_bytes_;

		toolkit::ThreadPool		recv_thread_pool_{ 1, toolkit::ThreadPool::PRIORITY_NORMAL, false };
		

	};
}
#endif /// os_is_linux

#endif ///!SP_UNIX_H_