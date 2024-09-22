#ifndef ELAPSED_TIMER_H_
#define ELAPSED_TIMER_H_
#include <chrono>

namespace oct_commu
{
	class ElapsedTimer
	{
	public:
		ElapsedTimer()
			: m_begin(std::chrono::high_resolution_clock::now())
		{
		}

		void reset()
		{
			m_begin = std::chrono::high_resolution_clock::now();
		}
		int64_t elapsed_ns() const
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
		}
		//us
		int64_t elapsed_us() const
		{
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
		}
		//ms
		int64_t elapsed_ms() const
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
		}
		//s
		int64_t elapsed_s() const
		{
			return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
		}
		//m
		int64_t elapsed_min() const
		{
			return std::chrono::duration_cast<std::chrono::minutes>(std::chrono::high_resolution_clock::now() - m_begin).count();
		}
		//h
		int64_t elapsed_hours() const
		{
			return std::chrono::duration_cast<std::chrono::hours>(std::chrono::high_resolution_clock::now() - m_begin).count();
		}


	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_begin;
	};
}


#endif ///£¡ELAPSED_TIMER_H_