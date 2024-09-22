#ifndef STREAM_BUFFER_H_
#define STREAM_BUFFER_H_
#include <OCTCommu.h>
#include <vector>
#include <iterator>
#include <regex>

namespace oct_commu
{
	class StreamBuffer
	{
	public:
		enum
		{
			FRAME_HEAD_LEN = 64,
			MAX_BUFFER_LEN = 1024 * 80,
		};

	public:
		StreamBuffer();
		~StreamBuffer();

		bool init(const std::string& sp_head, const int frame_len);

		void append(const unsigned char* pdata, const int data_len);

		bool getNextFrame(unsigned char*& frame, int& frame_len);

		void release();

		void clearBuffer();
	private:
		bool findStartFrameHeaderPos();

		std::vector<std::string> str_split(const std::string& in, const std::string& delim)
		{
			std::regex re{ delim };
			// 调用 std::vector::vector (InputIterator first, InputIterator last,const allocator_type& alloc = allocator_type())
			// 构造函数,完成字符串分割
			return std::vector<std::string>
			{
				std::sregex_token_iterator(in.begin(), in.end(), re, -1),
					std::sregex_token_iterator()
			};
		}

	public:
		unsigned char	m_head[FRAME_HEAD_LEN] = {0};
		int		m_headValidLen = 0;
		int		m_frameLen = 0;
		unsigned char	m_dataBuffer[MAX_BUFFER_LEN] = { 0 };
		int		m_startPos = 0;
		int		m_endPos = 0;
		int		m_totalBufferLen = MAX_BUFFER_LEN;
	};

}

#endif///!STREAM_BUFFER_H_