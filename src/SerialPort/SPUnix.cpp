#include <SPUnix.h>
#include <OCTCommu.h>

#if  defined(os_is_linux) || defined(os_is_mac)
#include <SPUtil.h>
#include <SPParamHelper.h>
//#include <iostream>

namespace oct_commu
{

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	SPUnix::SPUnix(void)
		: mtx_recv_func_()
		, recv_func_(nullptr)
		, error_str_()
		, sp_init_()
		, next_read_len_(1)
		, sp_fd_(-1)
		, mtx_error_()
		, mtx_fd_()
		, thread_poller_(std::bind(&SPUnix::pollerThreadFunc, this), "tmpsp")
		, recv_buffer_()
		, recv_thread_is_running_(false)
		, listen_to_write_(false)
		, send_l2_cache_()
		, send_l1_cache_()
		, send_swap_cache_()
		, mtx_send_l2_cache_()
		, mtx_send_l1_cache_()
		, sp_buf_()
		, mtx_sp_read_buf_()
		, sp_suspend_(false)
		, sent_bytes_(0)
		, recv_bytes_(0)
	{
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	SPUnix::~SPUnix(void)
	{
		shutdown();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPUnix::init(const SPInit& sp_init)
	{
		// 检查参数
		if (0 != SPParamHelper::ins().paramIsOk(sp_init))
		{
			setErrorMsg(SPParamHelper::ins().errorStr());
			return 1;
		}

		sp_init_ = sp_init;

		return initSP();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPUnix::recvSpeed()
	{
		int64_t diff = m_ElapsedTimerRecv.elapsed_s();

		if (0 >= diff)
		{
			diff = 1;
		}
		int ret = recv_bytes_ / diff;
		recv_bytes_ = 0;
		m_ElapsedTimerRecv.reset();

		return ret;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPUnix::sendSpeed()
	{
		int64_t diff = m_ElapsedTimerSend.elapsed_s();
		if (0 >= diff)
		{
			diff = 1;
		}
		int ret = sent_bytes_ / diff;
		sent_bytes_ = 0;
		m_ElapsedTimerSend.reset();

		return ret;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPUnix::send(const char* pdata, const size_t len)
	{
		if (false == isOpen())
		{
			return -1;
		}

		/// 如果是挂起
		if (sp_suspend_)
		{
			setErrorMsg("the serial has suspended");
			return 0;
		}

		/// 如果长度大于缓存最大长度
		if (MAX_SEND_BUFFER_LEN < len)
		{
			setErrorMsg("failed to send, MAX_SEND_BUFFER_LEN(1024 * 10) < len");
			return -2;
		}

		/// 将数据写入一级缓存
		{
			std::unique_lock<std::recursive_mutex> lock(mtx_send_l1_cache_);
			send_l1_cache_.reset();
			send_l1_cache_.append(pdata, len);
			/// 监听写
			listen_to_write_ = true;
		}

		//size_t has_sent_len = 0;
		//bool is_done = false;
		///// 避免多次发送一直无法结束，如果重发10次，还没有将数据发出，则break
		//char break_count = 0;

		//while (false == is_done)
		//{
		//	if (10 == break_count)
		//	{
		//		/// 发送失败
		//		setErrorMsg(std::string("failed to send data，written times: 10"));
		//		break;
		//	}

		//	/// 指向待发送数据的起始地址
		//	char* reset_send_data = (char*)pdata[has_sent_len];
		//	/// 剩下待发送的数据
		//	size_t rest_len = len - has_sent_len;
		//	/// 再次发送数据
		//	int sent_len = write(sp_fd_, reset_send_data, rest_len);
		//	/// 只有部分数据发送成功
		//	if (sent_len < (int)rest_len)
		//	{
		//		/// 写数据出差错
		//		if (0 > sent_len)
		//		{
		//			if (EAGAIN == errno)
		//			{
		//				std::cout << "\n breal_count = " << break_count;
		//				// outputstring("1111");
		//				++ break_count;
		//				/// try again
		//				std::this_thread::sleep_for(std::chrono::milliseconds(3));
		//			}
		//			else
		//			{
		//				/// 发送失败
		//				setErrorMsg(std::string("failed to send data，please check [errno] to get more info, error=") + std::to_string(errno));
		//				break;
		//			}
		//			
		//		}
		//		else
		//		{
		//			/// 统计发送次数
		//			++break_count;
		//			/// 统计本次已经发送了多少字节
		//			has_sent_len += sent_len;
		//		}				
		//	}
		//	else
		//	{
		//		is_done = true;
		//		break;
		//	}
		//}
		
		/// 返回发送的实际长度
		return len;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPUnix::shutdown()
	{
		if (false == isOpen())
		{
			return 1;
		}

		std::unique_lock<std::mutex> lock(mtx_fd_);
		listen_to_write_ = false;
		if (sp_init_.is_to_recv_)
		{
			recv_thread_is_running_ = false;
			/// 停止线程
			thread_poller_.shutdown();
			
		}
		
		sp_fd_ = SPUtil::closeSP(sp_fd_);

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	std::string SPUnix::lastErrorMsg()
	{
		std::string ret;

		{
			std::unique_lock<std::mutex> lock(mtx_error_);
			ret = error_str_;
		}

		return ret;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPUnix::setOnReadFunc(OnSPLinkRecvDataFunc cb)
	{
		 std::unique_lock<std::mutex> lock(mtx_recv_func_);
		recv_func_ = std::move(cb);
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPUnix::setNextReadLen(const unsigned int len)
	{
		next_read_len_ = len;
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPUnix::suspend(const bool flag)
	{
		if (false == isOpen())
		{
			return 1;
		}
		/// linux暂不支持挂起串口。模拟实现，即不响应发送和接收
		sp_suspend_ = flag;

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPUnix::setTimeouts(const SPTimeouts& st)
	{
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int SPUnix::timeouts(SPTimeouts& out_value)
	{
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	bool SPUnix::isOpen()
	{
		const bool ret = (-1 == sp_fd_ ? false : true);
		if (false == ret)
		{
			setErrorMsg(std::string("sp doesnt open"));
		}

		return ret;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	void SPUnix::setErrorMsg(const std::string& str)
	{
		 std::unique_lock<std::mutex> lock(mtx_error_);
		error_str_ = str;
	}

	int SPUnix::initSP()
	{
		sp_fd_ = SPUtil::openSP(sp_init_.name_);
		/// 如果打开串口失败，则反馈
		if (0 >= sp_fd_)
		{
			std::string str_tmp = strerror(errno);

			sp_fd_ = SPUtil::closeSP(sp_fd_);
			setErrorMsg("failed to open serial port");
			return 2;
		}

		/// 激活串口属性失败
		if (0 != initSPProperty())
		{
			sp_fd_ = SPUtil::closeSP(sp_fd_);
			return 1;
		}

		if (sp_init_.is_to_recv_)
		{
			recv_thread_is_running_ = true;
			/// 启动线程
			thread_poller_.start();

			/// 启动线程
			recv_thread_pool_.start();

			m_ElapsedTimerRecv.reset();
		}
		
		m_ElapsedTimerSend.reset();

		return 0;
	}

	int SPUnix::initSPProperty()
	{
		struct termios options;

		// 获取终端属性
		if (tcgetattr(sp_fd_, &options) < 0)
		{
			setErrorMsg("tcgetattr error");
			return -1;
		}

		/// 设置波特率
		if (0 != SPUtil::setBaud(sp_fd_, sp_init_.baud_, &options))
		{
			setErrorMsg("set baud error");
			return 1;
		}

		/// 设置校验位
		if (0 != SPUtil::setParity(sp_fd_, sp_init_.parity_, &options))
		{
			setErrorMsg("set parity error");
			return 2;
		}

		/// 设置数据位
		if (0 != SPUtil::setDataBit(sp_fd_, sp_init_.data_bits_, &options))
		{
			setErrorMsg("set databit error");
			return 3;
		}

		/// 停止位
		if (0 != SPUtil::setStopbit(sp_fd_, sp_init_.stop_bits_, &options))
		{
			setErrorMsg("set stopbit error");
			return 5;
		}
		/// flow ctrl
		if (0 != SPUtil::setFlowCtrl(sp_fd_, sp_init_.flow_ctrl_, &options))
		{
			setErrorMsg("set flowctrl error");
			return 6;
		}

		/// 控制模式
		SPUtil::setCtrolMode(&options);
		/// 设置输出模式
		SPUtil::setOutputMode(&options);
		/// 设置输入模式
		SPUtil::setInputMode(&options);
		/// 设置本地模式
		SPUtil::setLocalMode(&options);
		/// 设置等待时间
		SPUtil::setWaitTime(&options, 0);
		/// 设置至少读取字节数
		SPUtil::setAtLeastRead(&options, 1);
		/// 设置数据发生溢出时的处理
		SPUtil::setDataOverflowFlag(sp_fd_);
		/// 设置缓冲区大小: 10 * 1024 字节
		SPUtil::setBufferSize(sp_fd_, MAX_RECV_BUFFER_LEN);
		/// 激活配置
		if (0 != SPUtil::activeOption(sp_fd_, &options))
		{
			setErrorMsg("active option error");
			return 7;
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: poller线程
	/// --------------------------------------------------------------------------------
	void SPUnix::pollerThreadFunc()
	{
		//latch_.wait();
	
		
		int mill_second = sp_init_.recv_interval_timeout_;
		if ( 2 >= mill_second)
		{
			mill_second = 2;
		}
		int select_err_count = 0;
		while (recv_thread_is_running_)
		{
			fd_set read_fd_sets;
			fd_set write_fd_sets;

			/// 毫秒
			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = mill_second * 1000;

			FD_ZERO(&read_fd_sets); //每次循环都要清空集合，否则不能检测描述符变化  
			FD_ZERO(&write_fd_sets);

			FD_SET(sp_fd_, &read_fd_sets); //添加描述符 
			FD_SET(sp_fd_, &write_fd_sets); //添加描述符 
 
			int select_result = 0;
			if (false == listen_to_write_)
			{
				select_result = select(sp_fd_ + 1, &read_fd_sets, nullptr, nullptr, &timeout);
			}
			else
			{
				select_result = select(sp_fd_ + 1, &read_fd_sets, &write_fd_sets, nullptr, &timeout);
			}

			switch (select_result)
			{
				/// select错误
			case -1: 
			{	
				if (10 == select_err_count)
				{
					setErrorMsg("failed to call select, restart to call 10 times, they were all failure");
					break;
				}
				else
				{
					++select_err_count;
				}
			}
			break;
			case 0:
				break; //再次轮询  
			default:

				/// 不是挂起，再次轮询
				if (false == sp_suspend_)
				{
					/// 检测读取
					readEvent(&read_fd_sets);
					/// 检测写
					writeEvent(&write_fd_sets);
				}
				
				break;
				
			}// end switch  
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 读取事件
	/// --------------------------------------------------------------------------------
	void SPUnix::readEvent(fd_set* fds)
	{
		if (FD_ISSET(sp_fd_, fds)) //测试fd是否可读  
		{
			/// 获取串口中的当前可读取的字符数量
			int num_of_bytes_available = 0;
			if (ioctl(sp_fd_, FIONREAD, &num_of_bytes_available) < 0)
			{
				return;
			}

			if (0 >= num_of_bytes_available || MAX_RECV_BUFFER_LEN < num_of_bytes_available)
			{
				return;
			}

			/// 重置接收buffer
			recv_buffer_.reset();

			//for (int index = 0; index < );
			/// 当前已经读取了多少字节
			int current_read_count = 0;

			do 
			{
				/// 还剩下这么多字节没有读取
				int rest_read_count = num_of_bytes_available - current_read_count;
				/// 读取结束
				if (0 == rest_read_count)
				{
					break;
				}

				/// 读取这么多字节
				int is_to_read_count = read(sp_fd_, (void*)recv_buffer_.current(), rest_read_count);

				/// 读取成功
				if (is_to_read_count > 0)
				{
					/// 全部读取
					if (is_to_read_count == rest_read_count)
					{
						recv_buffer_.setValidLen(num_of_bytes_available);
						break;
					}
					/// 部分读取，则继续读取
					else
					{
						/// 统计当前读取了多少字节
						current_read_count = is_to_read_count;
						/// 移动
						recv_buffer_.moveTo(is_to_read_count);
					}
				}
				else
				{
					if (-1 == is_to_read_count && EAGAIN == errno)
					{
						/// 继续读取
					}
					else
					{
						break;
					}
				}

			} while (1);

			if (0 != recv_buffer_.currentLen())
			{
				onReadData((const unsigned char*)recv_buffer_.data(), recv_buffer_.currentLen(), "", 0, 0,
							sp_init_.name_, sp_init_.baud_, sp_init_.parity_, sp_init_.data_bits_, sp_init_.stop_bits_, sp_init_.flow_ctrl_);
			}
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	void SPUnix::writeEvent(fd_set* write_fd_set)
	{
		if (false == listen_to_write_)
		{
			return;
		}

		if (FD_ISSET(sp_fd_, write_fd_set)) 
		{
			flushAllData();
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	void SPUnix::flushAllData()
	{
		/// 1. 首先，将二级缓存中的数据换出来。
		send_swap_cache_.reset();
		{
			std::unique_lock<std::recursive_mutex> lock(mtx_send_l2_cache_);
			if (0 != send_l2_cache_.length())
			{
				/// 如果二级缓存中存在数据
				send_swap_cache_.swap(send_l2_cache_);
			}
		}

		/// 如果二级缓存为空
		if (0 == send_swap_cache_.currentLen())
		{
			do 
			{
				/// 消费一级缓存，将以一级缓存中的数据置换到二级缓存
				std::unique_lock<std::recursive_mutex> lock(mtx_send_l1_cache_);
				if (0 != send_l1_cache_.length())
				{
					send_swap_cache_.swap(send_l1_cache_);
					break;
				}

				/// 关闭写监听事件
				listen_to_write_ = false;
			} while (0);
		}

		/// 缓存不为空，则先消费缓存中的数据
		{
			/// 当前发送了多少字节
			int sent_count = 0;

			while (1)
			{
				/// 剩余多少字节数据
				int rest_count = send_swap_cache_.currentLen() - sent_count;
				/// 全部发送完毕
				if (0 == rest_count)
				{
					break;
				}

				/// 当前要发送数据的其实地址
				char* psend_data = (char*)send_swap_cache_.data() + sent_count;

				/// 当前已经发送了多少字节的数据
				int is_to_send_count = write(sp_fd_, psend_data, rest_count);

				if (0 < is_to_send_count)
				{
					/// 如果全部发送成功
					if (is_to_send_count == rest_count)
					{
						break;
					}
					/// 只发送了部分数据， 则继续发送，将数据写入
					else
					{
						/// 当前发送了多少字节
						sent_count += is_to_send_count;
						sent_bytes_ += is_to_send_count;
					}
				}
				else
				{
					/// 继续发送
					if (-1 == is_to_send_count && EAGAIN == errno)
					{

					}
					else
					{
						/// 出现错误，放弃发送，
						break;
					}
				}
			}

			/// 如果已经发送的字节数与待发送数据的长度相同，则全部发送，关闭监听写
			if (send_swap_cache_.length() == sent_count)
			{
				/// 再调用一次，
				flushAllData();
			}
			/// 没有全部发送， 说明当前出现了错误，
			else
			{
				/// 关闭监听写， 
				listen_to_write_ = false;
			}
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 处理接收的数据
	/// --------------------------------------------------------------------------------
	void SPUnix::onReadData(const unsigned char* pdata, const size_t len)
	{
		recv_bytes_ += len;
		{
			std::unique_lock<std::recursive_mutex> lock(mtx_sp_read_buf_);
			sp_buf_.append(pdata, len);
		}

		/// 将接收数据的传给应用层，打入任务队列线程池
		recv_thread_pool_.async(std::bind(&SPUnix::pushData2Client, this), false);
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 将数据处理后交给应用层
	/// --------------------------------------------------------------------------------
	void SPUnix::pushData2Client()
	{
		decltype(sp_buf_) sp_buf_tmp;
		{
			std::unique_lock<std::recursive_mutex> lock(mtx_sp_read_buf_);
			//sp_buf_.append(pdata, len);
			if (0 != sp_buf_.data(sp_buf_tmp))
			{
				return;
			}
		}
		
		//std::cout << "\n================= pushData2Client ==================\n";
		do 
		{
			/// 获取当前数据缓存中的有效长度
			size_t cur_valid_len = sp_buf_tmp.validLen();

			/// 将要读取的数据长度
			size_t next_read_len = next_read_len_;

			
			/// 缓存数据不足将要接收的数据长度，则等待
			if (cur_valid_len < next_read_len)
			{
				break;
			}

			/// 缓存中的长度至少满足当前需要将要读取的数据长度，则将数据传给应用层
			/// 获取当前的数据，
			const unsigned char* read_data = sp_buf_tmp.current();
			{
				std::unique_lock<std::mutex> lock(mtx_recv_func_);
				if (recv_func_)
				{
					recv_func_(read_data, (size_t)next_read_len,
						sp_init_.name_, sp_init_.baud_, sp_init_.parity_, sp_init_.data_bits_, sp_init_.stop_bits_, sp_init_.flow_ctrl_);
				}
			}
			/// 用完后，移动当前指针
			sp_buf_tmp.addBegin(next_read_len);
		} while (1);
	}

}

#endif ///!os_is_linux