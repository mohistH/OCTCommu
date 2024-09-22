/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TASKQUEUE_H_
#define TASKQUEUE_H_

#include <mutex>
#include "Util/List.h"
#include "semaphore.h"

namespace toolkit
{

	//ʵ����һ�����ں�������������жӣ����ж����̰߳�ȫ�ģ������ж����������ź�������
	template<typename T>
	class TaskQueue
	{
	public:
		//�����������ж�
		template<typename C>
		void push_task(C&& task_func)
		{
			{
				std::lock_guard<decltype(_mutex)> lock(_mutex);
				_queue.emplace_back(std::forward<C>(task_func));
			}
			_sem.post();
		}

		template<typename C>
		void push_task_first(C&& task_func)
		{
			{
				std::lock_guard<decltype(_mutex)> lock(_mutex);
				_queue.emplace_front(std::forward<C>(task_func));
			}
			_sem.post();
		}

		//��������ж�
		void push_exit(size_t n)
		{
			_sem.post(n);
		}

		//���жӻ�ȡһ��������ִ���߳�ִ��
		bool get_task(T& tsk)
		{
			_sem.wait();
			std::lock_guard<decltype(_mutex)> lock(_mutex);
			if (_queue.empty())
			{
				return false;
			}
			tsk = std::move(_queue.front());
			_queue.pop_front();
			return true;
		}

		size_t size() const
		{
			std::lock_guard<decltype(_mutex)> lock(_mutex);
			return _queue.size();
		}

	private:
		List <T> _queue;
		mutable std::mutex _mutex;
		semaphore _sem;
	};

} /* namespace toolkit */
#endif /* TASKQUEUE_H_ */
