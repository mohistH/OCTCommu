#ifndef NET_CALL_BACK_H_
#define NET_CALL_BACK_H_
#include <functional>
#include <string>

namespace toolkit
{
	/// 参数1： 收到的数据， 参数2， 收到的数据长度
	using NetRecvDataFunc = std::function<void(const unsigned char*, const unsigned int, struct sockaddr* addr, int len)>;

	/// tcp connect触发的函数
	using NetTCPConnectFunc = std::function<void(std::string& msg)>;

	/// 发送数据完成后， 清空缓存事件
	using NetTCPFlush = std::function<void()>;

	/// TCP错误事件
	using NetTCPError = std::function<void(const std::string& error_msg)>;
}

#define OCT_LOCK_GUARD(mtx) std::lock_guard<decltype(mtx)> lck(mtx)

#endif /// !NET_CALL_BACK_H_