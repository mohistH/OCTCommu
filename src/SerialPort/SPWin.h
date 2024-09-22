#ifndef SP_WIN_H_
#define SP_WIN_H_
#include <OCTCommu.h>
#ifdef os_is_win
#include <atomic>
#include <mutex>
#include <wtypes.h>
#include <ThreadPlus.h>
#include <ElapsedTimer.h>

namespace oct_commu
{
	class SPWin
	{
	private:
		enum OPERATE_MODE
		{
			/// <summary>
			///  异步
			/// </summary>
			OM_ASYNC = 1,
			/// <summary>
			///  同步
			/// </summary>
			OM_SYNC = 2,

		};

	public:
		explicit SPWin(void);
		virtual ~SPWin(void);

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
		int setOnReadFunc(OnSPLinkRecvDataFunc cb);

		/// -------------------------------------------------------------------------------
		/// @brief:		设置下一次读取数据长度
		/// @param: 	const unsigned int len - 读取数据长度
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int setNextReadLen(unsigned int len);

		/// -------------------------------------------------------------------------------
		/// @brief:		挂起串口
		/// @param: 	const bool flag - true-挂起， false-恢复
		///  @ret:		int
		///				
		/// -------------------------------------------------------------------------------
		int suspend(bool flag);

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
		void recvTheadFunc();

	private:
		bool isOpen();

		int open();

		int realOpen();

		int activeConfig();

		int setFlowCtrl();

		void setErrorMsg(const std::string& str);


	private:
		/// 接收数据函数
		OnSPLinkRecvDataFunc		recv_func_ = nullptr;
		/// 错误信息
		std::string			error_str_ = std::string("");
		/// 初始化参数
		SPInit				sp_init_;
		/// 线程是否运行
		std::atomic_bool	recv_thread_is_running_ = false;

		CountDownLatch		latch_;
		/// 线程
		ThreadPlus			recv_thread_;
		/// 下一次读取数据长度
		std::atomic_uint32_t	next_read_len_ = 1;

		/// 同步还是异步
		OPERATE_MODE		operate_mode_ = OM_ASYNC;

		HANDLE				sp_handle_ = INVALID_HANDLE_VALUE;

		/// 
		COMMCONFIG			sp_config_;
		COMMTIMEOUTS		sp_timeouts_;

		OVERLAPPED			overlapped_write_;

		std::mutex			mutex_fd_;
		std::mutex			mutex_error_;
		std::mutex			mutex_recv_func_;

		/// 单位时间内接收字节数
		std::atomic_uint				recv_bytes_count_{ 0 };
		/// 单位时间内发送字节数
		std::atomic_uint				send_bytes_count_{ 0 };
		/// elapsed timer
		ElapsedTimer					elapsed_timer_send_;
		ElapsedTimer					elapsed_timer_recv_;
	};

}

#endif ///!os_is_win


#endif ///!SP_WIN_H_