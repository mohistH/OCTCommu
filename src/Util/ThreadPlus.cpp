#include "ThreadPlus.h"

namespace oct_tk
{
	struct ThreadData
	{
		using ThreadFunc =  ThreadPlus::ThreadFunc ;
		ThreadFunc func_;
		//string name_;
		//pid_t* tid_;
		CountDownLatch* latch_;

		ThreadData(ThreadFunc func,
			//const std::string& name,
			//pid_t* tid,
			CountDownLatch* latch)
			: func_(std::move(func)),
			//name_(name),
			//tid_(tid),
			latch_(latch)
		{
		}

		void runInThread()
		{
			latch_->countDown();
			latch_ = nullptr;

			func_();
		}
	};

	void* startThread(void* obj)
	{
		ThreadData* data = static_cast<ThreadData*>(obj);
		data->runInThread();
		delete data;
		return nullptr;
	}
}


/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
ThreadPlus::ThreadPlus(ThreadFunc func, const std::string& name)
	:func_(std::move(func))
	, started_(false)
	, joined_(false)
	, latch_()
	, name_(name)
{

}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
ThreadPlus::~ThreadPlus()
{
	if (started_ && !joined_)
	{
		thread_->detach();
	}
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
void ThreadPlus::start()
{
	if (nullptr == func_)
	{
		return;
	}

	started_ = true;

	oct_tk::ThreadData* data = new(std::nothrow) oct_tk::ThreadData(func_, &latch_);
	thread_.reset(new(std::nothrow)  std::thread(oct_tk::startThread, data));
	if (nullptr == thread_)
	{
		started_ = false; 
		delete data;
		data = nullptr;
	}
	else
	{
		latch_.wait();
	}
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
void ThreadPlus::shutdown()
{
	if (thread_->joinable())
	{
		thread_->join();
	}

	joined_ = true;
}

/// --------------------------------------------------------------------------------
/// @brief: 
/// --------------------------------------------------------------------------------
bool ThreadPlus::isStarted()
{
	return started_;
}
