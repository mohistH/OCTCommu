#ifndef FIXED_BUFFER_H_
#define FIXED_BUFFER_H_
#include <string.h>
// #include "typedef.h"

namespace oct_tk
{
	template<typename To, typename From>
	inline To implicit_cast(From const& f)
	{
		return f;
	}

	class SockAddrInfo
	{
	public:
		void setTargetIP(const std::string& ip)
		{
			target_ip_ = std::move(ip);
		}

		void setTargetPort(int port)
		{
			target_port_ = port;
		}

		void setFamily(int family)
		{
			family_ = family;
		}

		int& targetPort()
		{
			return target_port_;
		}

		int& family()
		{
			return family_;
		}

		std::string& targetIP()
		{
			return target_ip_;
		}

	private:
		int		target_port_ = 0;
		int		family_ = 0;
		std::string		target_ip_ = {};
	};

	template<size_t SIZE>
	class FixedBuffer : public SockAddrInfo
	{
	public:
		FixedBuffer()
			: cur_(data_)
		{
			
		}

		~FixedBuffer()
		{
			
		}

		void append(const char* /*restrict*/ buf, size_t len)
		{
			// FIXME: append partially
			if (implicit_cast<size_t>(avail()) > len)
			{
				memcpy(cur_, buf, len);
				cur_ += len;
				current_len_ += len;
			}
		}

		const char* data() const 
		{
			return data_; 
		}
		
		int length() const 
		{ 
			return static_cast<int>(cur_ - data_); 
		}

		// write to data_ directly
		char* current() 
		{ 
			return cur_; 
		}
		
		int avail() const 
		{ 
			return static_cast<int>(end() - cur_); 
		}

		void moveTo(size_t len)
		{ 
			cur_ += len; 
			current_len_ += len;
		}

		void setValidLen(size_t len)
		{
			current_len_ = len;
		}

		void reset() 
		{ 
			cur_ = data_; 
			current_len_ = 0;
		}
		
		// void bzero() 
		// { 
		// 	memset(data_, 0, sizeof(SIZE));
		// 	current_len_ = 0;
		// }

		void setEndChar()
		{
			data_[current_len_] = '\0';
		}

		//// for used by unit test
		//std::string toString() const 
		//{ 
		//	return std::string(data_, length());
		//}

		size_t currentLen()
		{
			return current_len_;
		}

		size_t maxLen()
		{
			return SIZE;
		}

		void swap(FixedBuffer& ins)
		{
			if (this == &ins)
			{
				return;
			}

			memcpy(data_, ins.data(), SIZE);
			current_len_ = ins.currentLen();
			cur_ = data_ + current_len_;

			ins.reset();
		}

	private:
		const char* end() const 
		{ 
			return data_ + sizeof(data_); 
		}

	private:
		char data_[SIZE] = {0};
		char* cur_ = nullptr;
		size_t	current_len_ = 0;
	};
}
#endif ///!FIXED_BUFFER_H_
