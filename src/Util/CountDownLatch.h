#ifndef COUNT_DOWN_LATCH_H_
#define COUNT_DOWN_LATCH_H_
#include <mutex>
#include <condition_variable>

class CountDownLatch 
{
public:
	explicit CountDownLatch();

	void setCount(const int ocunt);

	/// -------------------------------------------------------------------------------
	/// @brief:		wait
	///  @ret:		void
	///				
	/// -------------------------------------------------------------------------------
	void wait();

	void countDown();

	int getCount();

private:
	mutable std::mutex mutex_;
	std::condition_variable condi_;
	int count_{ 0 };
};

#endif ///!
