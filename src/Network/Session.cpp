/*
 * Copyright (c) 2021 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include <atomic>
#include "Session.h"
#include <TcpServer.h>

using namespace std;

namespace toolkit
{

	Session::Session(const Socket::Ptr& sock) 
		: SocketHelper(sock) {}
	Session::~Session() = default;


	/// --------------------------------------------------------------------------------
	/// @brief: Session::onRecv
	/// --------------------------------------------------------------------------------
	void Session::onRecv(const Buffer::Ptr& buf, struct sockaddr* addr, int len)
	{
		if (recv_data_func_)
		{
			recv_data_func_((const unsigned char*)buf->data(), (const size_t)buf->size(), addr, len );
		}
	}


	/// --------------------------------------------------------------------------------
	/// @brief: Session::onError
	/// --------------------------------------------------------------------------------
	void Session::onError(const SockException& err)
	{
		if (tcp_error_func_)
		{
			tcp_error_func_(std::string(err.what()));
		}
	}


	/// --------------------------------------------------------------------------------
	/// @brief: Session::onManager
	/// --------------------------------------------------------------------------------
	void Session::onManager()
	{	
		;
	}

	static atomic<uint64_t> s_session_index{ 0 };


	/// --------------------------------------------------------------------------------
	/// @brief: Session::attachServer
	/// --------------------------------------------------------------------------------
	void Session::attachServer(std::weak_ptr<Server> server)
	{
		_server = server;
	}

	string Session::getIdentifier() const
	{
		if (_id.empty())
		{
			_id = to_string(++s_session_index) + '-' + to_string(getSock()->rawFD());
		}
		return _id;
	}

	void Session::safeShutdown(const SockException& ex)
	{
		std::weak_ptr<Session> weakSelf = shared_from_this();
		async_first([weakSelf, ex]()
			{
				auto strongSelf = weakSelf.lock();
				if (strongSelf)
				{
					strongSelf->shutdown(ex);
				}
			});
	}

	StatisticImp(Session);
	StatisticImp(UdpSession);
	StatisticImp(TcpSession);

} // namespace toolkit
