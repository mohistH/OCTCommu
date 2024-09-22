/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include "Timer.h"

namespace toolkit
{

	Timer::Timer(float second, const std::function<bool()>& cb, const EventPoller::Ptr& poller)
	{
		_poller = poller;
		if (!_poller)
		{
			_poller = EventPollerPool::Instance().getPoller();
		}

		_tag = _poller->doDelayTask((uint64_t)(second * 1000), [cb, second]()
			{
				try
				{
					if (cb())
					{
						//重复的任务
						return (uint64_t)(1000 * second);
					}
					//该任务不再重复
					return (uint64_t)0;
				}
				catch (std::exception& ex)
				{
					ErrorL << "执行定时器任务捕获到异常:" << ex.what();
					return (uint64_t)(1000 * second);
				}
			});
	}


	/// --------------------------------------------------------------------------------
	/// @brief: Timer::init
	/// --------------------------------------------------------------------------------
	int Timer::init(const float second, const std::function<bool()>& cb, const EventPoller::Ptr& poller)
	{
		end();

		if (0.0f >= second)
		{
			return 1;
		}

		timer_period_ = second;
		timeout_cb_ = cb;
		_poller = poller;
		if (!_poller)
		{
			_poller = EventPollerPool::Instance().getPoller();
		}

		return 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: Timer::begin
	/// --------------------------------------------------------------------------------
	int Timer::begin()
	{
		TimeoutFunc& tf = timeout_cb_;
		const float& tp = timer_period_;

		_tag = _poller->doDelayTask((uint64_t)(tp * 1000), [tf, tp]()
			{
				try
				{
					if (tf())
					{
						//重复的任务
						return (uint64_t)(1000 * tp);
					}
					//该任务不再重复
					return (uint64_t)0;
				}
				catch (std::exception& ex)
				{
					ErrorL << "执行定时器任务捕获到异常:" << ex.what();
					return (uint64_t)(1000 * tp);
				}
			});


		return 0;
	}


	/// --------------------------------------------------------------------------------
	/// @brief: Timer::end
	/// --------------------------------------------------------------------------------
	int Timer::end()
	{
		auto tag = _tag.lock();
		if (tag)
		{
			tag->cancel();
		}

		return 0;
	}

	Timer::~Timer()
	{
		end();
	}

}  // namespace toolkit
