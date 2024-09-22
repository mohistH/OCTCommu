#ifndef SP_PARAM_HELPER_H_
#define SP_PARAM_HELPER_H_
#include <OCTCommu.h>
#include <unordered_map>

namespace oct_commu
{
	/// ----------------------------------------------------------------------------------------
	/// @brief: 检查串口初始化参数是否正确
	/// ----------------------------------------------------------------------------------------
	class SPParamHelper
	{
		using HashIntInt = std::unordered_map<int, int>;
	public:
		SPParamHelper()
		{
			hash_parity_.emplace(SPP_None, 0);
			hash_parity_.emplace(SPP_Odd,	0);
			hash_parity_.emplace(SPP_Even,	0);
			hash_parity_.emplace(SPP_Mark,	0);
			hash_parity_.emplace(SPP_Space, 0);

			hash_data_bits_.emplace(SPDB_5, 0);
			hash_data_bits_.emplace(SPDB_6, 0);
			hash_data_bits_.emplace(SPDB_7, 0);
			hash_data_bits_.emplace(SPDB_8, 0);

			hash_stop_bits_.emplace(SPSB_One, 0);
			hash_stop_bits_.emplace(SPSB_OneAndHalf, 0);
			hash_stop_bits_.emplace(SPSB_Two, 0);

			hash_flow_.emplace(SPFC_None, 0);
			hash_flow_.emplace(SPFC_Hardware, 0);
			hash_flow_.emplace(SPFC_Software, 0);

		}
		~SPParamHelper() = default;

		static SPParamHelper& ins() { static SPParamHelper instance; return instance; }

		int paramIsOk(const oct_commu::SPInit& sp)
		{
			/// 校验错误
			if (false == contain(std::ref(hash_parity_), sp.parity_))
			{
				error_str_ = std::string("parity is false");
				return 1;
			}
			/// 数据位错误
			if (false == contain(std::ref(hash_data_bits_), sp.data_bits_))
			{
				error_str_ = std::string("data bits is false");
				return 2;
			}
			/// 停止位错误
			if (false == contain(std::ref(hash_stop_bits_), sp.stop_bits_))
			{
				error_str_ = std::string("stop bits is false");
				return 3;
			}
			/// 流控制
			if (false == contain(std::ref(hash_flow_), sp.flow_ctrl_))
			{
				error_str_ = std::string("flow ctrl is false");
				return 5;
			}

			if (0 == sp.name_.length())
			{
				error_str_ = std::string{ "serial port name is empty" };
				return 6;
			}

			//if (1024 * 10 < sp.recv_data_buffer_size_)
			//{
			//	error_str_ = std::string{ "recv_data_buffer_size is greate than 1024 * 10" };
			//	return 7;
			//}

			return 0;
		}

		std::string errorStr()
		{
			return error_str_;
		}

	private:

		bool contain(HashIntInt& hash_name, const int find_value)
		{
			HashIntInt::iterator find_it = hash_name.find(find_value);
			return (find_it == hash_name.end() ? false : true);
		}

	private:

		/// 校验
		HashIntInt		hash_parity_;
		/// 数据位
		HashIntInt		hash_data_bits_;
		/// 停止位
		HashIntInt		hash_stop_bits_;
		/// 流
		HashIntInt		hash_flow_;
		std::string		error_str_ = { "" };
	};

}

#endif ///!SP_PARAM_HELPER_H_