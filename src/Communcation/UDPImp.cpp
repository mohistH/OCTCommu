#include <UDPImp.h>
#include <sockutil.h>
#include <functional>
#include <iostream>

#include <Buffer.h>
namespace oct_commu
{

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::UDPImp
	/// --------------------------------------------------------------------------------
	UDPImp::UDPImp()
	{
		if (nullptr == m_pUDP)
		{
			m_pUDP = toolkit::Socket::createSocket();
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::~UDPImp
	/// --------------------------------------------------------------------------------
	UDPImp::~UDPImp()
	{
		shutdown();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::init
	/// --------------------------------------------------------------------------------
	int UDPImp::init(const UDPInit& param)
	{
		if (nullptr == m_pUDP)
		{
			setErrorMsg("Failed to initialze, it cannot create socket(system error)");
			return -1;
		}

		OCT_LOCK_GUARD(m_send_mutex);
		m_udpIntParam = param;

		return initSocket();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::send
	/// --------------------------------------------------------------------------------
	int UDPImp::send(const char* pdata, const size_t len)
	{
		OCT_LOCK_GUARD(m_send_mutex);

		/// 如果没有初始化成功，不能发送数据
		if (false == m_hasInited)
		{
			setErrorMsg("Failed to send, please call [ init ] interface at first");
			return -1;
		}

		/// 参数检查
		if ((0 == len) || (nullptr == pdata))
		{
			setErrorMsg("Failed to send, the param len is zero or pdata is nullptr");
			return -2;
		}

		/// 调用接口发送数据
		const int64_t&& ret = m_pUDP.get()->send(pdata, len, m_pSendAddr, m_sockAddrLen);

		if (-1 == ret)
		{
			setErrorMsg("Failed to send, socket is invalid");
			return -3;
		}
		else if (0 == ret)
		{
			setErrorMsg("Failed to send, the length of data is zero");
			return -5;
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::unInit
	/// --------------------------------------------------------------------------------
	int UDPImp::shutdown()
	{
		try
		{
			OCT_LOCK_GUARD(m_send_mutex);
			/// 通知底层停止接收
			if (m_pUDP)
			{
				m_pUDP.get()->enableRecv(false);
				m_pUDP.get()->closeSock();
			}

			m_hasInited = false;

			m_recvThreadPool.stop();
		}
		catch (...)
		{
			;
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::getLastErrorMsg
	/// --------------------------------------------------------------------------------
	std::string UDPImp::getLastErrorMsg()
	{
		std::string retValue{};
		{
			OCT_LOCK_GUARD(m_errorMsgMutex);
			retValue = m_errorMsg;
			m_errorMsg.clear();
		}

		return retValue;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::recvSpeed
	/// --------------------------------------------------------------------------------
	int UDPImp::recvSpeed()
	{
		if (m_pUDP)
		{
			return m_pUDP.get()->getRecvSpeed();
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::sendSpeed
	/// --------------------------------------------------------------------------------
	int UDPImp::sendSpeed()
	{
		if (m_pUDP)
		{
			return m_pUDP.get()->getSendSpeed();
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::initSocket
	/// --------------------------------------------------------------------------------
	int UDPImp::initSocket()
	{
		shutdown();

		UDPInit& param = m_udpIntParam;

		/// 地址转换
		toSockAddress();

		int retValue = 0;
		/// 1. 绑定套接字
		retValue = bindSock();
		if (0 != retValue)
		{
			return 1;
		}

		/// 3. 初始化casttype
		retValue = initCast();
		if (0 != retValue)
		{
			return 3;
		}

		if (m_pUDP)
		{
			toolkit::Socket* psocket = m_pUDP.get();
			if (0 != psocket->setAttach())
			{
				return 5;
			}

		}

		if (true == m_udpIntParam.is_to_recv_)
		{
			setOnReadDataFunc();
			if (true == m_udpIntParam.is_detach_recv_)
			{
				/// 启动线程
				m_recvThreadPool.start();
			}			
		}

		m_hasInited = true;

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::toSockAddress
	/// --------------------------------------------------------------------------------
	int UDPImp::toSockAddress()
	{
		/// IPv6的本机IP中含有 ":"
		m_isIPV6 = false;
		std::atomic_bool& is_ipv6 = m_isIPV6;
		{
			const int find_pos = (const int)(m_udpIntParam.target_ip_.find(':'));
			if (0 <= find_pos)
			{
				is_ipv6 = true;
			}
		}

		if (false == is_ipv6)
		{
			m_sockAddrLen = (int)m_sockAddress.ipv4_.sockAddrLen();
			int ret = 0;
			toolkit::SockUtil::to_sockaddr_in(m_udpIntParam.target_ip_, m_udpIntParam.target_port_, &m_sockAddress.ipv4_.m_destSockAddr);

			m_pSendAddr = (struct sockaddr*)&m_sockAddress.ipv4_.m_destSockAddr;
			return ret;
		}
		else
		{
			m_sockAddrLen = (int)m_sockAddress.ipv6_.sockAddrLen();
			int ret = toolkit::SockUtil::to_sockaddr_in6(m_udpIntParam.target_ip_, m_udpIntParam.target_port_, &m_sockAddress.ipv6_.m_destSockAddr);
			if (0 != ret)
			{
				setErrorMsg("the target_ip_ is false");
			}
			m_pSendAddr = (struct sockaddr*)&m_sockAddress.ipv6_.m_destSockAddr;
			return ret;
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::bindSock
	/// --------------------------------------------------------------------------------
	int UDPImp::bindSock()
	{
		UDPInit& param = m_udpIntParam;

		/// 1. 绑定套接字
		toolkit::Socket* ptr = m_pUDP.get();
		//if (false == ptr->bindUdpSock(param.m_targetPort, param.m_localIP, true))
		if (false == ptr->newUdpSockWithBinding(param.target_port_, param.local_ip_, true))
		{
			setErrorMsg("Failed to bind udp, please check the local Ip and targetPort");
			return 2;
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 设置回调
	/// --------------------------------------------------------------------------------
	int UDPImp::setOnReadDataFunc()
	{
		//OnRecvDataFunc& recv_func = m_recvDataFunc;

		/// 1. 绑定套接字
		toolkit::Socket* ptr = m_pUDP.get();

		auto on_read_func = std::bind(&UDPImp::OnReadDataFunc, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		/// 设置接收回调
		//ptr->setOnRead([&recv_func](const toolkit::Buffer::Ptr& buf, struct sockaddr* addr, int recvLen)
		//	{
		//		if (recv_func)
		//		{
		//			recv_func((uchar*)buf->data(), (uint)buf->size());
		//		}
		//	});

		ptr->setOnRead(on_read_func);

		/// 开启接收
		ptr->enableRecv(true);

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::setOnRead
	/// --------------------------------------------------------------------------------
	int UDPImp::setOnRead(OnRecvDataFunc cb)
	{
		if (nullptr == cb)
		{
			setErrorMsg("cb param is nullptr");
			return 1;
		}

		std::unique_lock<std::mutex> lock(m_recvDataFunxMtx);
		m_recvDataFunc = std::move(cb);

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::initCast
	/// --------------------------------------------------------------------------------
	int UDPImp::initCast()
	{
		UDPInit& param = m_udpIntParam;
		toolkit::Socket* pscok = m_pUDP.get();

		switch (param.cast_type_)
		{
			/// 广播
			case CT_BROAD:
			{
				return pscok->setBroadcast(true);
			}
			break;

			/// 组播
			case CT_MULTI:
			{
				/// 这里，第二个参数须指定为true, 否则， 无法收到数据
				int retValue = pscok->setMultiProperties(param.local_ip_, true, 128, m_isIPV6);

				if (0 != retValue)
				{
					setErrorMsg("faild to setMultiProperties, please check the local ip and target ip");

					return retValue;
				}

				retValue = pscok->joinMultiAddr(param.target_ip_.c_str(), param.local_ip_.c_str());
				if (0 != retValue)
				{
					setErrorMsg("faild to joinMultiAddr, please check the local ip and target ip");
				}

				return retValue;
			}
			break;

			/// 单播
			case CT_UNI:
				return 0;
				break;

				/// 其他，错误
			default:
				break;
		}

		setErrorMsg("Error, [UDPInitParams.m_castType] is false, please check");

		return -101;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::setErrorMsg
	/// --------------------------------------------------------------------------------
	void UDPImp::setErrorMsg(const std::string& str)
	{
		OCT_LOCK_GUARD(m_errorMsgMutex);
		m_errorMsg = str;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: UDPImp::OnRecvData
	/// --------------------------------------------------------------------------------
	void UDPImp::OnReadDataFunc(const toolkit::Buffer::Ptr& buf, struct sockaddr* addr, int addr_len)
	{
		if (m_recvDataFunc)
		{
			using namespace oct_tk;
			RecvBuf fb;
			fb.append(buf->data(), (size_t)buf->size());
			fb.setEndChar();
		
			//sockaddr_
			toolkit::SockUtil::get_ip_port_from_addr(addr, m_isIPV6, fb.targetIP(), fb.targetPort(), fb.family());

			if (true == m_udpIntParam.is_detach_recv_)
			{
				block_queue_recv_buf_.put(fb);
				/// 将接收数据的传给应用层，打入任务队列线程池
				m_recvThreadPool.async(std::bind(&UDPImp::recvWorkerThread, this), false);
			}
			else
			{
				m_recvDataFunc((unsigned char*)fb.data(), fb.length(), fb.targetIP(), fb.targetPort(), fb.family(), 
								std::string{}, 0, 0, 0, 0, 0);
			}
		}
	}

	void UDPImp::recvWorkerThread()
	{
		/// 从缓存中获取数据， 如果为空，则pass
		RecvBuf rb = block_queue_recv_buf_.take();
		if (0 == rb.length())
		{
			return;
		}

		std::unique_lock<std::mutex> lock(m_recvDataFunxMtx);
		if (m_recvDataFunc)
		{
			m_recvDataFunc((unsigned char*)rb.data(), rb.currentLen(), rb.targetIP(), rb.targetPort(), rb.family(),
							std::string{}, 0, 0, 0, 0, 0);
		}

	}

}