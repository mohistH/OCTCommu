#ifndef THREAD_PLUS_H_
#define THREAD_PLUS_H_
#include <functional>
#include <thread>
#include <atomic>
#include <memory>

#include "CountDownLatch.h"

class ThreadPlus
{
public:
	using ThreadFunc = std::function<void()>;
public:
	ThreadPlus(ThreadFunc func, const std::string& name);
	~ThreadPlus();

	void start();
	void shutdown();
	bool isStarted();

private:
	ThreadFunc			func_;
	std::atomic_bool	started_;
	std::atomic_bool	joined_;
	std::thread::id		thread_id_{};
	std::string			name_{};
	CountDownLatch		latch_;
	std::unique_ptr<std::thread>	thread_;
};


#endif ///!THREAD_PLUS_H_