
#pragma once

#include <string>
#include <random>

std::mt19937& get_local_mt();

template<int length>
inline std::string generate_random_str()
{
	static auto base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
										"abcdefghijklmnopqrstuvwxyz"
										"0123456789";

	std::string ret;
	ret.reserve(length);

	std::uniform_int_distribution<> ud(0, 26*2+10-1);

	for (int i=0; i<length; i++)
	{
		ret.push_back(base64_chars[ud(get_local_mt())]);
	}

	return ret;
}
