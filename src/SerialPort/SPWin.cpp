#include <SPWin.h>

#ifdef os_is_win
#include <FixedBuffer.h>
#include <SPParamHelper.h>

namespace oct_commu
{

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	SPWin::SPWin(void)
		: recv_func_(nullptr)
		, error_str_()
		, sp_init_()
		, recv_thread_is_running_(false)
		, latch_()
		, recv_thread_(std::bind(&SPWin::recvTheadFunc, this), "tmpsp")
		, next_read_len_(1)
		, operate_mode_(OM_ASYNC)
		, sp_handle_(INVALID_HANDLE_VALUE)
		, sp_config_()
		, sp_timeouts_()
		, mutex_error_()
		, mutex_recv_func_()
		, recv_bytes_count_(0)
		, send_bytes_count_(0)
		, elapsed_timer_send_()
		, elapsed_timer_recv_()
	{
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	SPWin::~SPWin(void)
	{
		shutdown();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::init(const SPInit& sp_init)
	{
		// 检查参数
		if (0 != SPParamHelper::ins().paramIsOk(sp_init))
		{
			setErrorMsg(SPParamHelper::ins().errorStr());
			return 1;
		}

		sp_init_ = sp_init;

		return open();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::recvSpeed()
	{
		int64_t diff = elapsed_timer_recv_.elapsed_s();

		if (0 >= diff)
		{
			diff = 1;
		}
		int ret = recv_bytes_count_ / diff;
		recv_bytes_count_ = 0;
		elapsed_timer_recv_.reset();

		return ret;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::sendSpeed()
	{
		int64_t diff = elapsed_timer_send_.elapsed_s();
		if (0 >= diff)
		{
			diff = 1;
		}
		int ret = send_bytes_count_ / diff;
		send_bytes_count_ = 0;
		elapsed_timer_send_.reset();

		return ret;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::send(const char* pdata, const size_t len)
	{
		if (false == isOpen())
		{
			return -1;
		}

		if (OM_ASYNC == operate_mode_)
		{
			overlapped_write_.Internal = 0;
			overlapped_write_.InternalHigh = 0;
			overlapped_write_.Offset = 0;
			overlapped_write_.OffsetHigh = 0;
			overlapped_write_.hEvent = CreateEvent(NULL, true, false, NULL);

			DWORD send_bytes = 0;
			if (FALSE == WriteFile(sp_handle_, (void*)pdata, (DWORD)len, &send_bytes, &overlapped_write_))
			{
				if (ERROR_IO_PENDING == GetLastError())
				{
					GetOverlappedResult(sp_handle_, &overlapped_write_, &send_bytes, true);
				}
				else
				{
					setErrorMsg(std::string("failed to send data, async_mode"));
				}
			}

			CloseHandle(overlapped_write_.hEvent);

			send_bytes_count_ += send_bytes;
			return (int)send_bytes;
		}
	
		DWORD send_bytes = 0;
		if (FALSE == WriteFile(sp_handle_, (void*)pdata, (DWORD)len, &send_bytes, NULL))
		{
			/// 发送失败
			setErrorMsg(std::string("failed to send data, sync_mode"));
		}

		send_bytes_count_ += send_bytes;

		return (int)send_bytes;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::shutdown()
	{
		if (false == isOpen())
		{
			return 1;
		}
		std::unique_lock<std::mutex> lock(mutex_fd_);

		if (true == sp_init_.is_to_recv_)
		{
			recv_thread_is_running_ = false;
			/// 停止线程
			recv_thread_.shutdown();
		}

		SetCommMask(sp_handle_, 0);
		PurgeComm(sp_handle_, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);
		CloseHandle(sp_handle_);
		sp_handle_ = INVALID_HANDLE_VALUE;

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	std::string SPWin::lastErrorMsg()
	{
		std::string ret;
		{
			std::unique_lock<std::mutex> lock(mutex_error_);
			{
				ret = error_str_;
			}
		}

		return ret;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::setOnReadFunc(OnRecvDataFunc cb)
	{
		std::unique_lock<std::mutex> lock(mutex_recv_func_);
		recv_func_ = std::move(cb);
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::setNextReadLen(unsigned int len)
	{
		next_read_len_ = len;
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::suspend(bool flag)
	{
		std::unique_lock<std::mutex> lock(mutex_fd_);

		if (true == isOpen())
		{
			const BOOL ret = (true == flag ? SetCommBreak(sp_handle_) : ClearCommBreak(sp_handle_));

			return !ret;
		}

		return 1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::setTimeouts(const SPTimeouts& st)
	{
		std::lock_guard<std::mutex> lock(mutex_fd_);
		if (false == isOpen())
		{
			return 1;
		}

		COMMTIMEOUTS ct;
		ct.ReadIntervalTimeout = st.read_internal_timeout_;
		ct.ReadTotalTimeoutConstant = st.read_total_timeout_constant_;
		ct.ReadTotalTimeoutMultiplier = st.read_total_timeout_multiple_;
		ct.WriteTotalTimeoutConstant = st.write_total_timeout_constant_;
		ct.WriteTotalTimeoutMultiplier = st.write_total_timeout_multiple_;
		SetCommTimeouts(sp_handle_, &ct);

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::timeouts(SPTimeouts& st)
	{
		std::lock_guard<std::mutex> lock(mutex_fd_);
		if (false == isOpen())
		{
			return 1;
		}

		COMMTIMEOUTS ct;
		GetCommTimeouts(sp_handle_, &ct);

		st.read_internal_timeout_ = ct.ReadIntervalTimeout ;
		st.read_total_timeout_constant_ = ct.ReadTotalTimeoutConstant ;
		st.read_total_timeout_multiple_ = ct.ReadTotalTimeoutMultiplier  ;
		st.write_total_timeout_constant_ = ct.WriteTotalTimeoutConstant ;
		st.write_total_timeout_multiple_ = ct.WriteTotalTimeoutMultiplier ;

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	void SPWin::recvTheadFunc()
	{
		///阻塞latch .wait);
		unsigned int dwError = 0; 
		COMSTAT comstat;
		unsigned int eventMask = 0;

		HANDLE sp_handle = sp_handle_;

		const int sp_recv_max_len = 1024 * 60;
		oct_tk::FixedBuffer<sp_recv_max_len> recv_data_buffer;


		if (OM_ASYNC == operate_mode_)
		{
			while (true == recv_thread_is_running_)
			{
				DWORD wCount = next_read_len_;
				DWORD error_flag = 0;

				COMSTAT com_stat;
				OVERLAPPED overlapped_read;
				memset(&overlapped_read, 0, sizeof(overlapped_read));

				overlapped_read.hEvent = CreateEvent(NULL, TRUE, FALSE, "READ_EVENT");

				ClearCommError(sp_handle, &error_flag, &com_stat);

				if (!com_stat.cbInQue)
				{
					ClearCommError(sp_handle, &error_flag, &com_stat);
					CloseHandle(overlapped_read.hEvent);
				}
				else
				{
					const BOOL ret = ReadFile(sp_handle, (void*)recv_data_buffer.current(), next_read_len_, &wCount, &overlapped_read);
					if (FALSE == ret)
					{
						if (ERROR_IO_PENDING == GetLastError())
						{
							GetOverlappedResult(sp_handle, &overlapped_read, &wCount, TRUE);
						}
						else
						{
							ClearCommError(sp_handle, &error_flag, &com_stat);
							CloseHandle(overlapped_read.hEvent);
						}
					}
					else
					{
						recv_bytes_count_ += next_read_len_;
						std::unique_lock<std::mutex> lock(mutex_recv_func_);
						if (recv_func_)
						{
							recv_func_((const unsigned char*)recv_data_buffer.data(), (size_t)next_read_len_, "", 0, 0, 
									sp_init_.name_, sp_init_.baud_, sp_init_.parity_, sp_init_.data_bits_, sp_init_.stop_bits_, sp_init_.flow_ctrl_);
							recv_data_buffer.reset();
						}
					}
				}
			}
		}
		else
		{
			while (true == recv_thread_is_running_)
			{
				DWORD real_read = 0;
				if (TRUE == ReadFile(sp_handle, (void*)recv_data_buffer.current(), next_read_len_, &real_read, NULL))
				{
					recv_bytes_count_ += real_read;
					std::unique_lock<std::mutex> lock(mutex_recv_func_);
					if (recv_func_)
					{
						recv_func_((const unsigned char*)recv_data_buffer.data(), (size_t)next_read_len_, "", 0, 0,
						sp_init_.name_, sp_init_.baud_, sp_init_.parity_, sp_init_.data_bits_, sp_init_.stop_bits_, sp_init_.flow_ctrl_);
						recv_data_buffer.reset();
					}
				}				
			}
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	bool SPWin::isOpen()
	{
		const bool ret = (INVALID_HANDLE_VALUE == sp_handle_ ? false : true);
		if (false == ret)
		{
			setErrorMsg(std::string("sp doesnt open"));
		}

		return ret;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::open()
	{
		if (0 != realOpen())
		{
			return 1;
		}

		return activeConfig();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::realOpen()
	{
		if (isOpen())
		{
			return 1;
		}

		/// 同步还是异步
		const unsigned int mode = (OM_ASYNC == operate_mode_ ? FILE_FLAG_OVERLAPPED : NULL);
		/// 串口名称
		const std::string sp_name = std::string("\\\\.\\") + sp_init_.name_;
		
		sp_handle_ = CreateFileA(sp_name.c_str(),	/// 串口名称
			GENERIC_READ | GENERIC_WRITE,			/// 读写
			0,										/// 独占
			NULL,									/// 安全属性指针 
			OPEN_EXISTING,							/// 打开现有串口
			mode,									/// 同步还是异步
			NULL);									/// 复制文件句柄， 串口这里设置为NULL
		
		if (INVALID_HANDLE_VALUE == sp_handle_)
		{
			std::string str("ERROR_UNKNOW");
			const DWORD err_id = GetLastError();
			switch (err_id)
			{
				/// 串口不存在
			case ERROR_FILE_NOT_FOUND:
			{
				str = "ERROR_FILE_NOT_FOUND";
			}
			break;

			case ERROR_ACCESS_DENIED:
				str = "ERROR_ACCESS_DENIED";
				break;

			default:
				break;
			}

			setErrorMsg(str);
			return 2;
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::activeConfig()
	{
		if (false == isOpen())
		{
			return 1;
		}

		DWORD config_size = sizeof(COMMCONFIG);

		sp_config_.dwSize = config_size;
		GetCommConfig(sp_handle_, &sp_config_, &config_size);
		GetCommState(sp_handle_, &(sp_config_.dcb));

		sp_config_.dcb.BaudRate = sp_init_.baud_;
		sp_config_.dcb.ByteSize = sp_init_.data_bits_;
		sp_config_.dcb.Parity = sp_init_.parity_;
		sp_config_.dcb.StopBits = sp_init_.stop_bits_;

		sp_config_.dcb.fBinary = true;
		sp_config_.dcb.fInX = false;
		sp_config_.dcb.fOutX = false;
		sp_config_.dcb.fAbortOnError = false;
		sp_config_.dcb.fNull = false;

		setFlowCtrl();
		SetCommConfig(sp_handle_, &sp_config_, config_size);

		
		if (OM_ASYNC == operate_mode_)
		{
			sp_timeouts_.ReadIntervalTimeout         = MAXDWORD;
			sp_timeouts_.ReadTotalTimeoutMultiplier  = 0;
			sp_timeouts_.ReadTotalTimeoutConstant    = 0;
			sp_timeouts_.WriteTotalTimeoutMultiplier = 0;
			sp_timeouts_.WriteTotalTimeoutConstant   = 0;

			SetCommTimeouts(sp_handle_, &sp_timeouts_);
			PurgeComm(sp_handle_, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);
			
			
			if (true == sp_init_.is_to_recv_)
			{
				recv_thread_is_running_ = true;
				recv_thread_.start();
				elapsed_timer_recv_.reset();
			}

			elapsed_timer_send_.reset();
		}
		else
		{
			sp_timeouts_.ReadIntervalTimeout = MAXDWORD;
			sp_timeouts_.ReadTotalTimeoutMultiplier = 0;
			sp_timeouts_.ReadTotalTimeoutConstant = 0;
			sp_timeouts_.WriteTotalTimeoutMultiplier = 100;
			sp_timeouts_.WriteTotalTimeoutConstant = 500;

			SetCommTimeouts(sp_handle_, &sp_timeouts_);
			PurgeComm(sp_handle_, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPWin::setFlowCtrl()
	{
		switch (sp_init_.flow_ctrl_)
		{
			case SPFC_None: // No flow control
			{
				sp_config_.dcb.fOutxCtsFlow = FALSE;
				sp_config_.dcb.fRtsControl = RTS_CONTROL_DISABLE;
				sp_config_.dcb.fInX = FALSE;
				sp_config_.dcb.fOutX = FALSE;
				SetCommConfig(sp_handle_, &sp_config_, sizeof(COMMCONFIG));
			}
				break;

			case SPFC_Software: // Software(XON / XOFF) flow control
			{
				sp_config_.dcb.fOutxCtsFlow = FALSE;
				sp_config_.dcb.fRtsControl = RTS_CONTROL_DISABLE;
				sp_config_.dcb.fInX = TRUE;
				sp_config_.dcb.fOutX = TRUE;
				SetCommConfig(sp_handle_, &sp_config_, sizeof(COMMCONFIG));
			}
				break;

			case SPFC_Hardware: // Hardware(RTS / CTS) flow control
			{
				sp_config_.dcb.fOutxCtsFlow = TRUE;
				sp_config_.dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
				sp_config_.dcb.fInX = FALSE;
				sp_config_.dcb.fOutX = FALSE;
				SetCommConfig(sp_handle_, &sp_config_, sizeof(COMMCONFIG));
			}
				break;

			default:
				return 1;
				break;
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	void SPWin::setErrorMsg(const std::string& str)
	{
		std::unique_lock<std::mutex> lock(mutex_error_);
		error_str_ = str;
	}
}

#endif ///!os_is_win