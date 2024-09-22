/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include <iostream>
#include "TcpClient.h"

using namespace std;

namespace toolkit
{

	StatisticImp(TcpClient);

	TcpClient::TcpClient(const EventPoller::Ptr& poller) : SocketHelper(nullptr)
	{
		//_timer = std::make_shared<Timer>();
		//_timer_reconnect = std::make_shared<Timer>();

		setPoller(poller ? poller : EventPollerPool::Instance().getPoller());
		setOnCreateSocket([](const EventPoller::Ptr& poller)
			{
				//TCP客户端默认开启互斥锁
				return Socket::createSocket(poller, true);
			});
	}

	TcpClient::~TcpClient() 
	{
		_timer->end();
		_timer.reset();
	}

	void TcpClient::shutdown(const SockException& ex)
	{
		_timer.reset();
		SocketHelper::shutdown(ex);
	}

	bool TcpClient::alive() const
	{
		if (_timer)
		{
			//连接中或已连接
			return true;
		}
		//在websocket client(zlmediakit)相关代码中，
		//_timer一直为空，但是socket fd有效，alive状态也应该返回true
		auto sock = getSock();
		return sock && sock->rawFD() >= 0;
	}

	void TcpClient::setNetAdapter(const string& local_ip)
	{
		_net_adapter = local_ip;
	}

	void TcpClient::startConnect(const string& url, uint16_t port, float timeout_sec, uint16_t local_port)
	{
		_url = url;
		_port = port;
		_local_port = local_port;
		_timeout_seconds	= timeout_sec;

		weak_ptr<TcpClient> weak_self = shared_from_this();

		_timer = std::make_shared<Timer>(2.0f, [weak_self]()
			{
				auto strong_self = weak_self.lock();
				if (!strong_self)
				{
					return false;
				}
				strong_self->onManager();
				return true;
			}, getPoller());

		setSock(createSocket());

		auto sock_ptr = getSock().get();
		sock_ptr->setOnErr([weak_self, sock_ptr](const SockException& ex)
			{
				auto strong_self = weak_self.lock();
				if (!strong_self)
				{
					return;
				}
				if (sock_ptr != strong_self->getSock().get())
				{
					//已经重连socket，上次的socket的事件忽略掉
					return;
				}
				strong_self->_timer.reset();
				strong_self->onErr(ex);
			});

		_timer_reconnect = std::make_shared<Timer>(5.0f, std::bind(&TcpClient::reConnectServer, this), getPoller());
		reConnectServer();
		// sock_ptr->connect(url, port, [weak_self](const SockException& err)
		// 	{
		// 		auto strong_self = weak_self.lock();
		// 		if (strong_self)
		// 		{
		// 			strong_self->onSockConnect(err);
		// 		}
		// 	}, timeout_sec, _net_adapter, local_port);
	}


	/// --------------------------------------------------------------------------------
	/// @brief: 重启连接服务器的功能
	/// --------------------------------------------------------------------------------
	void TcpClient::restartConnectTimer()
	{
		//static int xxxxx = 0;
		//++xxxxx;
		//std::cout << __LINE__ << __FUNCTION__ << "=" << xxxxx << "\n";
		_timer_reconnect.reset();
		_timer_reconnect = std::make_shared<Timer>(5.0f, std::bind(&TcpClient::reConnectServer, this), getPoller());
	}

	void TcpClient::onSockConnect(const SockException& ex)
	{
		if (ex)
		{
			onConnect(ex);

			//std::cout << "\n" << __LINE__ << " , " << __FUNCTION__ << ", RECONNECTING" << "3333 \n\n";
			//连接失败
			//_timer.reset(new(std::nothrow) Timer);
			_timer.reset();

			/// --------------------------------
			/// 连接失败，重启定时器，开启连接
			//_timer_reconnect->begin();
			// reConnectServer();
			/// _timer.reset();
			
			return;
		}

		//std::cout << "\n" << __LINE__ << " , " << __FUNCTION__ << ", RECONNECTING" << "44444 \n\n";

		_timer_reconnect.reset();

		/// --------------------------------
		/// 停止定时器，改为使用定时向服务器发送manager
		//_timer.reset();
		//----------------------------------

		auto sock_ptr = getSock().get();
		weak_ptr<TcpClient> weak_self = shared_from_this();

		sock_ptr->setOnFlush([weak_self, sock_ptr]()
			{
				auto strong_self = weak_self.lock();
				if (!strong_self)
				{
					return false;
				}
				if (sock_ptr != strong_self->getSock().get())
				{
					//已经重连socket，上传socket的事件忽略掉
					return false;
				}
				strong_self->onFlush();
				return true;
			});

		sock_ptr->setOnRead([weak_self, sock_ptr](const Buffer::Ptr& pBuf, struct sockaddr* addr, int addr_len)
			{
				auto strong_self = weak_self.lock();
				if (!strong_self)
				{
					return;
				}
				if (sock_ptr != strong_self->getSock().get())
				{
					//已经重连socket，上传socket的事件忽略掉
					return;
				}
				try
				{
					strong_self->onRecv(pBuf, addr, addr_len);
				}
				catch (std::exception& ex)
				{
					strong_self->shutdown(SockException(Err_other, ex.what()));
				}
			});

		onConnect(ex);
	}

	bool TcpClient::reConnectServer()
	{
		auto sock_ptr = getSock().get();
		weak_ptr<TcpClient> weak_self = shared_from_this();

		sock_ptr->connect(_url, _port, [weak_self](const SockException& err)
		{
			//std::cout << "\n\n" << __LINE__ << ", " << __FUNCTION__ << " 22222 \n\n";
			auto strong_self = weak_self.lock();
			if (strong_self)
			{
				strong_self->onSockConnect(err);
			}
		}, _timeout_seconds, _net_adapter, _local_port);

		return true;
	}
} /* namespace toolkit */
