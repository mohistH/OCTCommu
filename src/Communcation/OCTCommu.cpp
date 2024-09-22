#include <OCTCommu.h>
#include <UDPImp.h>
#include <TCPImp.h>
#include <SPWin.h>
#include <SPUnix.h>
#include <memory>

namespace oct_commu
{
	/// --------------------------------------------------------------------------------
	/// @brief: IUDP::IUDP
	/// --------------------------------------------------------------------------------
	IUDP::IUDP()
	{
		d_ptr_ = new(std::nothrow) UDPImp;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: IUDP::~IUDP
	/// --------------------------------------------------------------------------------
	IUDP::~IUDP()
	{
		uninit();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: IUDP::init
	/// --------------------------------------------------------------------------------
	int IUDP::init(const UDPInit& param)
	{
		if (d_ptr_)
		{
			return d_ptr_->init(param);
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: IUDP::recvSpeed
	/// --------------------------------------------------------------------------------
	int IUDP::recvSpeed()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (d_ptr_)
		{
			return d_ptr_->recvSpeed();
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: IUDP::sendSpeed
	/// --------------------------------------------------------------------------------
	int IUDP::sendSpeed()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (d_ptr_)
		{
			return d_ptr_->sendSpeed();
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: IUDP::send
	/// --------------------------------------------------------------------------------
	int IUDP::send(const char* pdata, const size_t len)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (d_ptr_)
		{
			return d_ptr_->send(pdata, len);
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: IUDP::shutdown
	/// --------------------------------------------------------------------------------
	int IUDP::shutdown()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (d_ptr_)
		{
			return d_ptr_->shutdown();
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: IUDP::getLastErrorMsg
	/// --------------------------------------------------------------------------------
	std::string IUDP::lastErrorMsg()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (d_ptr_)
		{
			return d_ptr_->getLastErrorMsg();
		}

		return std::string{""};
	}

	/// --------------------------------------------------------------------------------
	/// @brief: IUDP::setOnRead
	/// --------------------------------------------------------------------------------
	int IUDP::setOnReadFunc(OnRecvDataFunc cb)
	{
		if (d_ptr_)
		{
			return d_ptr_->setOnRead(std::move(cb));
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: IUDP::uninit
	/// --------------------------------------------------------------------------------
	void IUDP::uninit()
	{
		try
		{
			if (d_ptr_)
			{
				delete d_ptr_;
				d_ptr_ = nullptr;
			}
		}
		catch (...)
		{

		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::ITCP
	/// --------------------------------------------------------------------------------
	ITCP::ITCP()
	{
		if (nullptr == d_ptr_)
		{
			d_ptr_ = new(std::nothrow) TCPImp;
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::~ITCP
	/// --------------------------------------------------------------------------------
	ITCP::~ITCP()
	{
		uninit();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::init
	/// --------------------------------------------------------------------------------
	int ITCP::init(const TCPInit& init_param)
	{
		/// 如果 传递TCP类型错误
		if ((TT_CLIENT != init_param.tcp_type_) && (TT_SERVER != init_param.tcp_type_))
		{
			return -2;
		}

		if (d_ptr_)
		{
			return d_ptr_->init(init_param);
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::recvSpeed
	/// --------------------------------------------------------------------------------
	int ITCP::recvSpeed()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (d_ptr_)
		{
			return d_ptr_->recvSpeed();
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::sendSpeed
	/// --------------------------------------------------------------------------------
	int ITCP::sendSpeed()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (d_ptr_)
		{
			return d_ptr_->sendSpeed();
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::send
	/// --------------------------------------------------------------------------------
	int ITCP::send(const char* pdata, const size_t len)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (d_ptr_)
		{
			return d_ptr_->send(pdata, len);
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::shutdown
	/// --------------------------------------------------------------------------------
	int ITCP::shutdown()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (d_ptr_)
		{
			return d_ptr_->shutdown();
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::getLastErrorMsg
	/// --------------------------------------------------------------------------------
	std::string ITCP::lastErrorMsg()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (d_ptr_)
		{
			return d_ptr_->getLastErrorMsg();
		}

		return std::string{"failed to create d_ptr_"};
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::setOnReadFunc
	/// --------------------------------------------------------------------------------
	int ITCP::setOnReadFunc(OnRecvDataFunc cb)
	{
		if (d_ptr_)
		{
			return d_ptr_->setOnReadFunc(std::move(cb));
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::setOnErrorFunc
	/// --------------------------------------------------------------------------------
	int ITCP::setOnErrorFunc(OnTCPErrorFunc cb)
	{
		if (d_ptr_)
		{
			return d_ptr_->setOnErrorFunc(std::move(cb));
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int ITCP::setOnConnectFunc(OnTCPConnectFunc cb)
	{
		if (d_ptr_)
		{
			return d_ptr_->setOnConnectFunc(std::move(cb));
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int ITCP::setOnFlushFunc(OnTCPFlush cb)
	{
		if (d_ptr_)
		{
			return d_ptr_->setOnFlushFunc(std::move(cb));
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ITCP::uninit
	/// --------------------------------------------------------------------------------
	void ITCP::uninit()
	{
		try
		{
			if (d_ptr_)
			{
				delete d_ptr_;
				d_ptr_ = nullptr;
			}
		}
		catch (...)
		{

		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ISP::ISP
	/// --------------------------------------------------------------------------------
	ISP::ISP()
	{
#ifdef os_is_win
		d_ptr_ = new(std::nothrow) SPWin;
#else
		d_ptr_ = new(std::nothrow) SPUnix;
#endif ///!
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ISP::~ISP
	/// --------------------------------------------------------------------------------
	ISP::~ISP()
	{
		d_ptr_->shutdown();
		delete d_ptr_;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ISP::init
	/// --------------------------------------------------------------------------------
	int ISP::init(const SPInit& sp_init)
	{
		if (d_ptr_)
		{
			return d_ptr_->init(sp_init);
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ISP::recvSpeed
	/// --------------------------------------------------------------------------------
	int ISP::recvSpeed()
	{
		if (d_ptr_)
		{
			return d_ptr_->recvSpeed();
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ISP::sendSpeed
	/// --------------------------------------------------------------------------------
	int ISP::sendSpeed()
	{
		if (d_ptr_)
		{
			return d_ptr_->sendSpeed();
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ISP::send
	/// --------------------------------------------------------------------------------
	int ISP::send(const char* pdata, size_t len)
	{
		if (d_ptr_)
		{
			return d_ptr_->send(pdata, len);
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ISP::shutdown
	/// --------------------------------------------------------------------------------
	int ISP::shutdown()
	{
		if (d_ptr_)
		{
			return d_ptr_->shutdown();
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ISP::getLastErrorMsg
	/// --------------------------------------------------------------------------------
	std::string ISP::lastErrorMsg()
	{
		if (d_ptr_)
		{
			return d_ptr_->lastErrorMsg();
		}

		return std::string{""};
	}

	/// --------------------------------------------------------------------------------
	/// @brief: ISP::setOnReadFunc
	/// --------------------------------------------------------------------------------
	int ISP::setOnReadFunc(OnRecvDataFunc cb)
	{
		if (d_ptr_)
		{
			return d_ptr_->setOnReadFunc(cb);
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int ISP::setNextReadLen(unsigned int len)
	{
		if (d_ptr_)
		{
			return d_ptr_->setNextReadLen(len);
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int ISP::suspend(bool flag)
	{
		if (d_ptr_)
		{
			return d_ptr_->suspend(flag);
		}

		return -1;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int ISP::setTimeouts(const SPTimeouts& st)
	{
		return d_ptr_->setTimeouts(st);
	}

	/// --------------------------------------------------------------------------------
	/// @brief: 
	/// --------------------------------------------------------------------------------
	int ISP::timeouts(SPTimeouts& st)
	{
		return d_ptr_->timeouts(st);
	}

}