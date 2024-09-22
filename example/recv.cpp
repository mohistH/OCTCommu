#include <OCTCommu.h>
#include <iostream>
#include <thread>
#include <string.h>

int on_recv_data(const unsigned char*pdata, const size_t len)
{
	std::cout << " \nrecv_data_func, thread id =" << std::this_thread::get_id();
	return 0;
}

class RecvData
{
public:
	int on_recv_data(const unsigned char* pdata, size_t len, std::string ip, int port, int family)
	{
		if (nullptr == psp_)
		{
			std::cout << std::hex << " \n-------------------------------\nrecv_data_func, thread id ="
				<< std::this_thread::get_id() << ", data_len=" << len << "\n";
			for (size_t index = 0; index < len; ++index)
			{
				size_t value_tmp = pdata[index];
				std::cout << value_tmp << " ";
			}
			return 0;
		}





		if (0 == cur_len_)
		{
			recv_data_[cur_len_] = pdata[0];
			cur_len_ += 1;

			/// 下次再接收一个字节
			psp_->setNextReadLen(1);
			
			return 0;
		}
		
		if (1 == cur_len_)
		{
			recv_data_[cur_len_] = pdata[0];
			cur_len_ += 1;

			/// 下次再接收8字节
			psp_->setNextReadLen(8);
			return 0;
		}

		if (8 != len)
		{
			std::cout << "\n len != 8, len=" << len;
			return 0;
		}

		/// 将数据全部接收
		memcpy(recv_data_ + cur_len_, pdata, len);
		cur_len_ += len;






		//return 0;
		
		std::cout << std::hex << " \n-------------------------------\nrecv_data_func, thread id =" 
			<< std::this_thread::get_id() << ", data_len=" << len << "\n";
		for (size_t index = 0; index < len; ++index)
		{	
			size_t value_tmp = recv_data_[index];
			std::cout << std::hex << value_tmp << " ";
		}

		cur_len_ = 0;
		psp_->setNextReadLen(1);


		return 0;
	}

	void setSP(oct_commu::ISP* psp)
	{
		psp_ = psp;
	}

private:
	oct_commu::ISP* psp_ = nullptr;

	unsigned char recv_data_[10] = {0};
	int cur_len_ = 0;
};


void call_udp()
{
	/// 接收对象
	using namespace oct_commu;
	IUDP* pudp = new IUDP;

	UDPInit param;
	param.cast_type_ = oct_commu::CT_MULTI;
	//param.m_localIP = std::string("192.168.200.129");
	// param.m_localIP = std::string("192.168.200.1");
<<<<<<< HEAD
	param.m_localIP = std::string("127.0.0.1");
	param.m_targetIP = std::string("230.0.0.30");
	param.m_targetPort = 10010;
=======
	param.local_ip_ = std::string("A838:B0FC:EC04:8271:19AC:207A:091F:0738");
	/// 
	//param.target_ip_ = std::string("230.0.0.30");
	param.target_ip_ = std::string("FF02::01");
	param.target_port_ = 10010;
>>>>>>> f1313d60f84a4cfd84eb3138de628c1411a759eb
	
	int ret = pudp->init(param);
	if (0 != ret)
	{
		std::cout << "\n init failed,  ret=" << ret << ", msg=" << pudp->lastErrorMsg().c_str() << "\n";
	}
	else
	{
		RecvData rd;
		auto onread = std::bind(&RecvData::on_recv_data, &rd, std::placeholders::_1, std::placeholders::_2
			, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
		pudp->setOnReadFunc(onread);

		// std::cout << "\n\n\n----------------------wait ----- to ----recv--(5S)------------------\n\n";
		// for (int index = 0; index < 10; ++index)
		// {
		// 	std::this_thread::sleep_for(std::chrono::seconds(1));
		// 	std::cout << "\n index=" << index << " second(s)";
		// }

		std::cout << "\n---------its waiting for data = main thread id = " << std::this_thread::get_id() << ".......----------\n";

		// const size_t len = 10;
		// char arr[len] = {0 };
		// arr[0] = 0xAA;
		// arr[1] = 0x55;
		// for (size_t index = 2; index < len; ++index)
		// {
		// 	arr[index] = 170 + index;
		// }

		//for (int index = 0; index < 30 * 60; ++index)
		//{
		//	/// 发送
		//	int rettt = pudp->send(arr, len);
		//	if (0 != rettt)
		//	{
		//		std::cout << "\n=========== send index= "<< index << ",  failed to send, ret=" << rettt << ", msg=" << pudp->getLastErrorMsg().c_str() << "\n";
		//	}

		//	std::this_thread::sleep_for(std::chrono::seconds(1));
		//}

      
         for (int index =0 ; index < 60; ++ index)
         {
		 	if (3 >= (60 * 60 - index))
		 	{
		 		std::cout << "\n wait for recv, index=" << index << ", thread id =" << std::this_thread::get_id();
		 	}

		 	//std::cout << "\nsend_sppeedd=" << pudp->sendSpeed() << ", recv_speed=" << pudp->recvSpeed() << "\n";

             //
             std::this_thread::sleep_for(std::chrono::seconds(1));
         }
	}

	delete pudp;
}


void call_tcp_server()
{
	/// 接收对象
	using namespace oct_commu;

	TCPInit init_param;
	init_param.local_ip_ = {"10.0.0.4"};
	init_param.target_port_ = 10010;
	init_param.target_ip_ = {"10.0.0.4"};
	init_param.tcp_type_ = TT_SERVER;

	ITCP* tcp_server = new(std::nothrow) ITCP;
	RecvData rd;
	auto onread = std::bind(&RecvData::on_recv_data, &rd, std::placeholders::_1, std::placeholders::_2
		, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
	tcp_server->setOnReadFunc(onread);

	int ret = tcp_server->init(init_param);
	if (0 != ret)
	{
		std::cout << "\n tcp init failed\n";
	}
	else
	{
		

		// std::cout << "\n\n\n----------------------wait ----- to ----recv--(5S)------------------\n\n";
		// for (int index = 0; index < 10; ++index)
		// {
		// 	std::this_thread::sleep_for(std::chrono::seconds(1));
		// 	std::cout << "\n index=" << index << " second(s)";
		// }

		std::cout << "\n---------its waiting for data = main thread id = " << std::this_thread::get_id() << ".......----------\n";

		const size_t len = 10;
		char arr[len] = { 0 };
		for (size_t index = 0; index < len; ++index)
		{
			arr[index] = 30 + index;
		}

		 for (int index = 0; index < 60 * 10; ++index)
		 {
		 //	/// 发送
			// int rettt = 0;/// tcp_server->send(arr, len);
		 //	if (0 != rettt)
		 //	{
		 //		std::cout << "\n=========== send index= "<< index << ",  failed to send, ret=" << rettt << ", msg=" << tcp_server->getLastErrorMsg().c_str() << "\n";
		 //	}

			//std::cout	<< "\nsend_sppeedd=" << tcp_server->sendSpeed() 
			//			<< ", recv_speed=" << tcp_server->recvSpeed() << "\n";

			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	delete tcp_server;
}

void call_tcp_client()
{
	/// 接收对象
	using namespace oct_commu;

	TCPInit init_param;
	init_param.local_ip_ = { "172.20.64.1" };
	init_param.target_port_ = 10086;
	init_param.target_ip_ = { "172.20.64.1" };
	init_param.tcp_type_ = TT_CLIENT;
	init_param.is_to_recv_ = true;

	//ITCP* tcp_client = new(std::nothrow) ITCP;
	std::unique_ptr<ITCP> tcp_client = std::make_unique<ITCP>();
	int ret = tcp_client->init(init_param);
	if (0 != ret)
	{
		std::cout << "\n tcp init failed\n";
	}
	else
	{
		RecvData rd;
		auto onread = std::bind(&RecvData::on_recv_data, &rd, std::placeholders::_1, std::placeholders::_2
			, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
		tcp_client->setOnReadFunc(onread);

		// std::cout << "\n\n\n----------------------wait ----- to ----recv--(5S)------------------\n\n";
		// for (int index = 0; index < 10; ++index)
		// {
		// 	std::this_thread::sleep_for(std::chrono::seconds(1));
		// 	std::cout << "\n index=" << index << " second(s)";
		// }

		std::cout << "\n---------its waiting for data = main thread id = " << std::this_thread::get_id() << ".......----------\n";

		const size_t len = 10;
		char arr[len] = { 0 };
		for (size_t index = 0; index < len; ++index)
		{
			arr[index] = 30 + index;
		}

		for (int index = 0; index < 60; ++index)
		{
			///// 发送
			//int rettt = tcp_client->send(arr, len);
			//if (0 == rettt)
			//{
			//	std::cout << "\n=========== send index= " << index << ",  failed to send, ret=" << rettt << ", msg=" 
			//					<< tcp_client->lastErrorMsg().c_str() << "\n";
			//}
			//else
			//{
			//	std::cout << "\n , send_ret=" << rettt;
			//}

			//std::cout << "\nsend_sppeedd=" << tcp_client->sendSpeed()
			//	<< ", recv_speed=" << tcp_client->recvSpeed() << "\n";

			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	//delete tcp_client;
}


void call_sp()
{
	using namespace oct_commu;
	/// 串口初始化参数
	SPInit sp_init;
	sp_init.baud_ = 9600;
	//sp_init.name_ = std::string{"/dev/ttyS0"};
	sp_init.name_ = std::string{ "COM20" };
	sp_init.is_to_recv_ = true;

	ISP* psp = new(std::nothrow) ISP;
	if (nullptr == psp)
	{
		return;
	}

	RecvData rd;
	auto onread = std::bind(&RecvData::on_recv_data, &rd, std::placeholders::_1, std::placeholders::_2
		, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
	psp->setOnReadFunc(onread);
	psp->setNextReadLen(1);
	rd.setSP(psp);
	int init_ret = psp->init(sp_init);
	if (0 != init_ret)
	{
		std::cout << "\n sp- init-false = " << init_ret << ", eror=" << psp->lastErrorMsg().c_str() << "\n";
	}
	/// 初始化成功, 发送数据
	else
	{
		const size_t len = 10;
		char arr[len] = { 0 };
		for (size_t index = 0; index < len; ++index)
		{
			arr[index] = 30 + index;
		}

		std::cout << "\n is to send\n";
		for (int index = 0; index < 10 * 5; ++index)
		{
			 std::cout <<"\n index=" << index; 
			 /// 发送
			 int rettt = psp->send(arr, len);
			 if (0 != rettt)
			 {
			 	std::cout << "\n=========== send index= " << index << ",  failed to send, ret=" << rettt;
			 }
		
			// {
			// 	std::cout << "\nindex=" << index << ", send ret = " << rettt << ",error=" << psp->lastErrorMsg().c_str() << std::endl;
			// }

			// std::cout << "\nindex" << index << ", recv_speed = " << psp->recvSpeed();

			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	psp->shutdown();
	delete psp;
}

int main(void)
{
<<<<<<< HEAD
	call_udp();
	//call_tcp_server();
	//call_tcp_client();
	// call_sp();
=======
	 call_udp();
	//call_tcp_server();
	//call_tcp_client();
	//call_sp();
>>>>>>> f1313d60f84a4cfd84eb3138de628c1411a759eb

	return 0;
}