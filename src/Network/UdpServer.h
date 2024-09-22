/*
 * Copyright (c) 2021 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TOOLKIT_NETWORK_UDPSERVER_H
#define TOOLKIT_NETWORK_UDPSERVER_H

#include "Server.h"
#include "Session.h"

namespace toolkit
{

	class UdpServer : public Server
	{
	public:
		using Ptr = std::shared_ptr<UdpServer>;
		using PeerIdType = std::string;
		using onCreateSocket = std::function<Socket::Ptr(const EventPoller::Ptr&, const Buffer::Ptr&, struct sockaddr*, int)>;

		explicit UdpServer(const EventPoller::Ptr& poller = nullptr);
		~UdpServer() override;

		/**
		 * @brief ��ʼ����������
		 */
		template<typename SessionType>
		void start(uint16_t port, const std::string& host = "::")
		{
			// Session ������, ͨ����������ͬ���͵ķ�����
			_session_alloc = [](const UdpServer::Ptr& server, const Socket::Ptr& sock)
			{
				auto session = std::make_shared<SessionType>(sock);
				auto sock_creator = server->_on_create_socket;
				session->setOnCreateSocket([sock_creator](const EventPoller::Ptr& poller)
					{
						return sock_creator(poller, nullptr, nullptr, 0);
					});
				return std::make_shared<SessionHelper>(server, session);
			};
			start_l(port, host);
		}

		/**
		 * @brief ��ȡ�����������˿ں�, ����������ѡ���������˿�
		 */
		uint16_t getPort();

		/**
		 * @brief �Զ���socket������Ϊ
		 */
		void setOnCreateSocket(onCreateSocket cb);

	protected:
		virtual Ptr onCreatServer(const EventPoller::Ptr& poller);
		virtual void cloneFrom(const UdpServer& that);

	private:
		/**
		 * @brief ��ʼudp server
		 * @param port �����˿ڣ�0�����
		 * @param host ��������ip
		 */
		void start_l(uint16_t port, const std::string& host = "::");

		/**
		 * @brief ��ʱ���� Session, UDP �Ự��Ҫ������Ҫ����ʱ
		 */
		void onManagerSession();

		void onRead(const Buffer::Ptr& buf, struct sockaddr* addr, int addr_len);

		/**
		 * @brief ���յ�����,��������server fd��Ҳ��������peer fd
		 * @param is_server_fd ʱ��Ϊserver fd
		 * @param id �ͻ���id
		 * @param buf ����
		 * @param addr �ͻ��˵�ַ
		 * @param addr_len �ͻ��˵�ַ����
		 */
		void onRead_l(bool is_server_fd, const PeerIdType& id, const Buffer::Ptr& buf, struct sockaddr* addr, int addr_len);

		/**
		 * @brief ���ݶԶ���Ϣ��ȡ�򴴽�һ���Ự
		 */
		const Session::Ptr& getOrCreateSession(const PeerIdType& id, const Buffer::Ptr& buf, struct sockaddr* addr, int addr_len, bool& is_new);

		/**
		 * @brief ����һ���Ự, ͬʱ���б�Ҫ������
		 */
		const Session::Ptr& createSession(const PeerIdType& id, const Buffer::Ptr& buf, struct sockaddr* addr, int addr_len);

		/**
		 * @brief ����socket
		 */
		Socket::Ptr createSocket(const EventPoller::Ptr& poller, const Buffer::Ptr& buf = nullptr, struct sockaddr* addr = nullptr, int addr_len = 0);

	private:
		bool _cloned = false;
		Socket::Ptr _socket;
		std::shared_ptr<Timer> _timer;
		onCreateSocket _on_create_socket;
		//cloned server������server��session map����ֹ�����ڲ�ͬserver��Ư��
		std::shared_ptr<std::recursive_mutex> _session_mutex;
		std::shared_ptr<std::unordered_map<PeerIdType, SessionHelper::Ptr> > _session_map;
		//��server����cloned server������
		std::unordered_map<EventPoller*, Ptr> _cloned_server;
		std::function<SessionHelper::Ptr(const UdpServer::Ptr&, const Socket::Ptr&)> _session_alloc;
		// �������ͳ��
		ObjectStatistic<UdpServer> _statistic;
	};

} // namespace toolkit

#endif // TOOLKIT_NETWORK_UDPSERVER_H
