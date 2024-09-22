#ifndef __BLOCKING_QUEUE__
#define __BLOCKING_QUEUE__
#include <mutex>
#include <deque>
#include <condition_variable>
#include <chrono>

template<typename   T, int queue_max_count = 128>
class BlockingQueue
{
public:
	BlockingQueue(const BlockingQueue<T>& other) = delete;
	BlockingQueue<T>& operator=(const BlockingQueue<T>& other) = delete;

	BlockingQueue()
		:mtx_()
		, not_empty_()
		, queue_()
	{

	}
	~BlockingQueue()
	{

	}

	void put(const T& x)
	{
		/// 向队列中加入元素
		std::unique_lock<std::mutex> lock(mtx_);
		while (queue_max_count < queue_.size())
		{
			queue_.pop_front();
		}
		queue_.push_back(x);
		/// 加入元素后，通知线程来取
		not_empty_.notify_all();
	}

	void put(T&& x)
	{
		/// 向队列中加入元素
		std::unique_lock<std::mutex> lock(mtx_);
		while (queue_max_count < queue_.size())
		{
			queue_.pop_front();
		}

		queue_.push_back(std::move(x));
		/// 加入元素后，通知线程来取
		not_empty_.notify_all();
	}

	T take()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		/// 如果队列为空，则等待
		// while (true == queue_.empty())
		// {
		//     if (std::cv_status::timeout ==  not_empty_.wait_for(lock, std::chrono::milliseconds(4)))
		//     {
		//         break;
		//     }
		// }

		if (true == queue_.empty())
		{
			T ret;
			return ret;
		}

		T front(std::move(queue_.front()));
		queue_.pop_front();

		return front;
	}

	size_t size() const
	{
		std::unique_lock<std::mutex> lock(mtx_);
		return queue_.size();
	}

private:
	std::mutex      mtx_;
	std::condition_variable not_empty_;
	std::deque<T>   queue_;
};

#endif /// __BLOCKING_QUEUE__