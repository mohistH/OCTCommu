#include <StreamBuffer.h>

namespace oct_commu
{

	/// --------------------------------------------------------------------------------
	/// @brief: StreamBuffer::StreamBuffer
	/// --------------------------------------------------------------------------------
	StreamBuffer::StreamBuffer()
	{

	}

	/// --------------------------------------------------------------------------------
	/// @brief: StreamBuffer::~StreamBuffer
	/// --------------------------------------------------------------------------------
	StreamBuffer::~StreamBuffer()
	{

	}

	/// --------------------------------------------------------------------------------
	/// @brief: StreamBuffer::init
	/// --------------------------------------------------------------------------------
	bool StreamBuffer::init(const std::string& sp_head, const int frame_len)
	{
		if (0 >= frame_len)
		{
			return false;
		}
		
		{
			std::vector<std::string> after_str = str_split(sp_head, " ");

			/// 解析后的帧头大小
			int head_size = after_str.size();
			if (FRAME_HEAD_LEN < head_size)
			{
				return 1;
			}

			int index = 0;
			for (auto& item_head : after_str)
			{
				m_head[index] = strtol(item_head.c_str(), nullptr, 16);
				++index;
			}

			m_headValidLen = index;
		}

		m_frameLen = frame_len;
		m_startPos = 0;
		m_endPos = 0;

		return true;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: StreamBuffer::recvData
	/// --------------------------------------------------------------------------------
	void StreamBuffer::append(const unsigned char* pdata, const int data_len)
	{
		if (data_len >= m_totalBufferLen)
		{
			return;
		}

		int remain_len = m_totalBufferLen - m_endPos;

		if (remain_len >= data_len)
		{
			memcpy(m_dataBuffer + m_endPos, pdata, data_len);
			m_endPos += data_len;
		}
		else
		{
			int current_data_len = m_endPos - m_startPos;

			if (0 < current_data_len)
			{
#ifdef os_is_win
				memmove_s(m_dataBuffer, m_totalBufferLen, m_dataBuffer + m_startPos, current_data_len);
#else 
				memmove(m_dataBuffer, m_dataBuffer + m_startPos, current_data_len);
#endif ///!
				m_endPos = current_data_len;
				m_startPos = 0;
			}
			else
			{
				m_startPos = 0;
				m_endPos = 0;
			}

			memcpy(m_dataBuffer + m_endPos, pdata, data_len);
			m_endPos += data_len;
		}
	}

	/// --------------------------------------------------------------------------------
	/// @brief: StreamBuffer::getNextFrame
	/// --------------------------------------------------------------------------------
	bool StreamBuffer::getNextFrame(unsigned char*& frame, int& frame_len)
	{
		if (m_frameLen > (m_endPos - m_startPos))
		{
			return false;
		}

		if (true == findStartFrameHeaderPos())
		{
			if (m_frameLen > (m_endPos - m_startPos))
			{
				return false;
			}

			frame = m_dataBuffer + m_startPos;
			frame_len = m_frameLen;
			m_startPos = m_startPos + m_frameLen;
			return true;
		}

		return false;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: StreamBuffer::release
	/// --------------------------------------------------------------------------------
	void StreamBuffer::release()
	{
		memset(m_head, 0, sizeof(m_head));
		m_headValidLen = 0;
		m_frameLen = 0;
		m_startPos = 0;
		m_endPos = 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: StreamBuffer::clearBuffer
	/// --------------------------------------------------------------------------------
	void StreamBuffer::clearBuffer()
	{
		memset(m_dataBuffer, 0, sizeof(m_dataBuffer));
		m_startPos = 0;
		m_endPos = 0;
	}

	/// --------------------------------------------------------------------------------
	/// @brief: StreamBuffer::findStartFrameHeaderPos
	/// --------------------------------------------------------------------------------
	bool StreamBuffer::findStartFrameHeaderPos()
	{
		if (0 == m_headValidLen)
		{
			return true;
		}
		
		for (int stop_pos = m_endPos - m_headValidLen; m_startPos <= stop_pos; ++m_startPos)
		{
			if (0 == memcmp(m_dataBuffer + m_startPos, m_head, m_headValidLen))
			{
				return true;
			}
		}

		return false;
	}

}