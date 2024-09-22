#include <TCPImp.h>
#include <Network/sockutil.h>

namespace oct_commu
{
	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::TCPImp
	/// --------------------------------------------------------------------------------
	TCPImp::TCPImp()
	{
		tcp_client_imp_ = std::make_shared<TCPClientImp>();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::~TCPImp
	/// --------------------------------------------------------------------------------
	TCPImp::~TCPImp()
	{
		shutdown();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::init
	/// --------------------------------------------------------------------------------
	int TCPImp::init(const TCPInit& init_param)
	{
		tcp_type_ = init_param.tcp_type_;

		if (TT_CLIENT == tcp_type_)
		{
			if (tcp_client_imp_)
			{
				return tcp_client_imp_->init(init_param);
			}

			return -2;
		}
		
		return tcp_server_imp_.init(init_param);
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::recvSpeed
	/// --------------------------------------------------------------------------------
	int TCPImp::recvSpeed()
	{
		if (TT_CLIENT == tcp_type_)
		{
			//return tcp_client_imp_.recvSpeed();
			if (tcp_client_imp_)
			{
				return tcp_client_imp_->recvSpeed();
			}

			return 0;
		}
		
		return tcp_server_imp_.recvSpeed();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::sendSpeed
	/// --------------------------------------------------------------------------------
	int TCPImp::sendSpeed()
	{
		if (TT_CLIENT == tcp_type_)
		{
			if (tcp_client_imp_)
			{
				return tcp_client_imp_->sendSpeed();
			}

			return 0;
		}
		
		return tcp_server_imp_.sendSpeed();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::send
	/// --------------------------------------------------------------------------------
	int TCPImp::send(const char* pdata, const size_t len)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (TT_CLIENT == tcp_type_)
		{
			if (tcp_client_imp_)
			{
				return tcp_client_imp_->send(pdata, len);
			}

			return -2;
		}
		
		return tcp_server_imp_.sendData(pdata, len);
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::unInit
	/// --------------------------------------------------------------------------------
	int TCPImp::shutdown()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		if (TT_CLIENT == tcp_type_)
		{
			if (tcp_client_imp_)
			{
				tcp_client_imp_->shutdown();
				return 0;
			}

			return -2;
		}
		else
		{
			tcp_server_imp_.shutdown();
		}
		
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::getLastErrorMsg
	/// --------------------------------------------------------------------------------
	std::string TCPImp::getLastErrorMsg()
	{
		if (TT_CLIENT == tcp_type_)
		{
			//return tcp_client_imp_.getLastErrorMsg();
			if (tcp_client_imp_)
			{
				return tcp_client_imp_->getLastErrorMsg();
			}

			return std::string{""};
		}
		
		return tcp_server_imp_.getLastErrorMsg();
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::setOnReadFunc
	/// --------------------------------------------------------------------------------
	int TCPImp::setOnReadFunc(OnNetLinkRecvDataFunc cb)
	{
		if (TT_CLIENT == tcp_type_)
		{
			//tcp_client_imp_.setOnReadFunc(cb);
			tcp_client_imp_->setOnReadFunc(cb);
		}
		else
		{
			tcp_server_imp_.setOnReadFunc(cb);
		}
		
		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::setOnConnectFunc
	/// --------------------------------------------------------------------------------
	int TCPImp::setOnConnectFunc(OnTCPConnectFunc cb)
	{
		if (TT_CLIENT == tcp_type_)
		{
			tcp_client_imp_->setOnConnectFunc(cb);
		}
		else
		{
			tcp_server_imp_.setOnConnectFunc(cb);
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::setOnFlushFunc
	/// --------------------------------------------------------------------------------
	int TCPImp::setOnFlushFunc(OnTCPFlush cb)
	{
		if (TT_CLIENT == tcp_type_)
		{
			//tcp_client_imp_.setOnFlushFunc(cb);
			tcp_client_imp_->setOnFlushFunc(cb);
		}
		else
		{
			tcp_server_imp_.setOnFlushFunc(cb);
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: TCPImp::setOnErrorFunc
	/// --------------------------------------------------------------------------------
	int TCPImp::setOnErrorFunc(OnTCPErrorFunc cb)
	{
		if (TT_CLIENT == tcp_type_)
		{
			//tcp_client_imp_.setOnErrorFunc(cb);
			tcp_client_imp_->setOnErrorFunc(cb);
		}
		else
		{
			tcp_server_imp_.setOnErrorFunc(cb);
		}

		return 0;

	}

}