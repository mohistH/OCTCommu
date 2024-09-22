/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TCPSERVER_TCPSERVER_H
#define TCPSERVER_TCPSERVER_H

#include <memory>
#include <functional>
#include <unordered_map>
#include "Server.h"
#include "TcpSession.h"
#include "Poller/Timer.h"
#include "Util/util.h"

namespace toolkit
{

	//TCP�������������õģ�����ͨ��TcpSession::attachServer�������ݸ��Ự����
	class TcpServer : public Server
	{
	public:
		using Ptr = std::shared_ptr<TcpServer>;

		/**
		 * ����tcp��������listen fd��accept�¼�����뵽���е�poller�߳��м���
		 * �ڵ���TcpServer::start����ʱ���ڲ��ᴴ�������TcpServer����
		 * ��Щ��TcpServer����ͨ��Socket�����¡�ķ�ʽ�ڶ��poller�߳��м���ͬһ��listen fd
		 * �������TCP����������ͨ����ռʽaccept�ķ�ʽ�ѿͻ��˾��ȵķֲ�����ͬ��poller�߳�
		 * ͨ���÷�ʽ��ʵ�ֿͻ��˸��ؾ����Լ�������ӽ����ٶ�
		 */
		explicit TcpServer(const EventPoller::Ptr& poller = nullptr);
		~TcpServer() override;

		/**
		* @brief ��ʼtcp server
		* @param port �����˿ڣ�0�����
		* @param host ��������ip
		* @param backlog tcp listen backlog
		*/
		template<typename SessionType>
		void start(uint16_t port, const std::string& host = "::", uint32_t backlog = 1024)
		{
			//TcpSession��������ͨ����������ͬ���͵ķ�����
			/// ����1�� �������� ����2-�ͻ���socket
			_session_alloc = [](const TcpServer::Ptr& server, const Socket::Ptr& sock)
			{
				auto session = std::make_shared<SessionType>(sock);
				session->setOnCreateSocket(server->_on_create_socket);
				return std::make_shared<SessionHelper>(server, session);
			};
			start_l(port, host, backlog);
		}

		/**
		 * @brief ��ȡ�����������˿ں�, ����������ѡ���������˿�
		 */
		uint16_t getPort();

		/**
		 * @brief �Զ���socket������Ϊ
		 */
		void setOnCreateSocket(Socket::onCreateSocket cb);

		/// --------------------------------------------------------------------------------
		/// @brief: recvSpeed
		/// --------------------------------------------------------------------------------
		int recvSpeed();

		/// --------------------------------------------------------------------------------
		/// @brief: sendSpeed
		/// --------------------------------------------------------------------------------
		int sendSpeed();



		void setOnRecvDataFunc(NetRecvDataFunc cb)
		{
			recv_data_func_ = cb;
		}

		void setTCPErrorFunc(NetTCPError cb)
		{
			tcp_error_func_ = cb;
		}

		void setTcpFlushFunc(NetTCPFlush cb)
		{
			tcp_flush_func_ = cb;
		}

		void setTcpConnectFunc(NetTCPConnectFunc cb)
		{
			tcp_connect_func_ = cb;
		}


	protected:
		virtual void cloneFrom(const TcpServer& that);
		virtual TcpServer::Ptr onCreatServer(const EventPoller::Ptr& poller);

		virtual void onAcceptConnection(const Socket::Ptr& sock);
		virtual Socket::Ptr onBeforeAcceptConnection(const EventPoller::Ptr& poller);

	private:
		void onManagerSession();
		Socket::Ptr createSocket(const EventPoller::Ptr& poller);
		void start_l(uint16_t port, const std::string& host, uint32_t backlog);
		Ptr getServer(const EventPoller*) const;

	private:
		bool _is_on_manager = false;
		const TcpServer* _parent = nullptr;
		Socket::Ptr _socket;
		std::shared_ptr<Timer> _timer;
		Socket::onCreateSocket _on_create_socket;
		std::unordered_map<SessionHelper*, SessionHelper::Ptr> _session_map;
		std::function<SessionHelper::Ptr(const TcpServer::Ptr& server, const Socket::Ptr&)> _session_alloc;
		std::unordered_map<const EventPoller*, Ptr> _cloned_server;
		//�������ͳ��
		ObjectStatistic<TcpServer> _statistic;


	private:
		NetRecvDataFunc		recv_data_func_ = nullptr;
		NetTCPError			tcp_error_func_ = nullptr;
		NetTCPFlush			tcp_flush_func_ = nullptr;
		NetTCPConnectFunc	tcp_connect_func_ = nullptr;
	};

} /* namespace toolkit */
#endif /* TCPSERVER_TCPSERVER_H */
