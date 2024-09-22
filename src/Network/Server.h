/*
 * Copyright (c) 2021 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLTOOLKIT_SERVER_H
#define ZLTOOLKIT_SERVER_H

#include <unordered_map>
#include "Util/mini.h"
#include "Session.h"

namespace toolkit
{

	// ȫ�ֵ� Session ��¼����, ����������
	// �̰߳�ȫ��
	class SessionMap : public std::enable_shared_from_this<SessionMap>
	{
	public:
		friend class SessionHelper;
		using Ptr = std::shared_ptr<SessionMap>;

		//����
		static SessionMap& Instance();
		~SessionMap() = default;

		//��ȡSession
		Session::Ptr get(const std::string& tag);
		void for_each_session(const std::function<void(const std::string& id, const Session::Ptr& session)>& cb);

	private:
		SessionMap() = default;

		//�Ƴ�Session
		bool del(const std::string& tag);
		//���Session
		bool add(const std::string& tag, const Session::Ptr& session);

	private:
		std::mutex _mtx_session;
		std::unordered_map<std::string, std::weak_ptr<Session> > _map_session;
	};

	class Server;

	class SessionHelper
	{
	public:
		using Ptr = std::shared_ptr<SessionHelper>;

		SessionHelper(const std::weak_ptr<Server>& server, Session::Ptr session);
		~SessionHelper();

		const Session::Ptr& session() const;

	private:
		std::string _identifier;
		Session::Ptr _session;
		SessionMap::Ptr _session_map;
		std::weak_ptr<Server> _server;
	};

	// server ����, ��ʱ�����ڰ��� SessionHelper �� TcpServer ������
	// ������ TCP �� UDP ����ͨ�ò��ּӵ�����.
	class Server : public std::enable_shared_from_this<Server>, public mINI
	{
	public:
		using Ptr = std::shared_ptr<Server>;

		explicit Server(EventPoller::Ptr poller = nullptr);
		virtual ~Server() = default;

		/**
		 * �����������
		 * @param buf ���ݣ������ظ�ʹ���ڴ���,���ɱ�����ʹ��
		 */
		virtual void onRecv(const Buffer::Ptr& buf) {};

		/**
		 * �յ� eof �������������� Server �¼��Ļص�
		 * �յ����¼�ʱ, �ö���һ�㽫����������
		 * @param err ԭ��
		 */
		virtual void onError(const SockException& err) {};

		/**
		 * ÿ��һ��ʱ�䴥��, ��������ʱ����
		 */
		virtual void onManager() {};

	protected:
		EventPoller::Ptr _poller;
	};

} // namespace toolkit

#endif // ZLTOOLKIT_SERVER_H