/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef UTIL_WORKTHREADPOOL_H_
#define UTIL_WORKTHREADPOOL_H_

#include <memory>
#include "Poller/EventPoller.h"

namespace toolkit
{

	class WorkThreadPool : public std::enable_shared_from_this<WorkThreadPool>, public TaskExecutorGetterImp
	{
	public:
		using Ptr = std::shared_ptr<WorkThreadPool>;

		~WorkThreadPool() override = default;

		/**
		 * ��ȡ����
		 */
		static WorkThreadPool& Instance();

		/**
		 * ����EventPoller��������WorkThreadPool��������ǰ��Ч
		 * �ڲ����ô˷���������£�Ĭ�ϴ���thread::hardware_concurrency()��EventPollerʵ��
		 * @param size EventPoller���������Ϊ0��Ϊthread::hardware_concurrency()
		 */
		static void setPoolSize(size_t size = 0);

		/**
		 * �ڲ������߳��Ƿ�����cpu�׺��ԣ�Ĭ������cpu�׺���
		 */
		static void enableCpuAffinity(bool enable);

		/**
		 * ��ȡ��һ��ʵ��
		 * @return
		 */
		EventPoller::Ptr getFirstPoller();

		/**
		 * ���ݸ��������ȡ�Ḻ�ص�ʵ��
		 * ������ȷ��ص�ǰ�̣߳���ô�᷵�ص�ǰ�߳�
		 * ���ص�ǰ�̵߳�Ŀ����Ϊ������̰߳�ȫ��
		 * @return
		 */
		EventPoller::Ptr getPoller();

	protected:
		WorkThreadPool();
	};

} /* namespace toolkit */
#endif /* UTIL_WORKTHREADPOOL_H_ */
