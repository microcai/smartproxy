#pragma once

#include <string>
#include <map>
#include <boost/format.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include "escape_string.hpp"
#include <json11.hpp>

using decimal = boost::multiprecision::cpp_dec_float_50;
using money = decimal;
using bigint = boost::multiprecision::cpp_int;

namespace pay_utility
{
	using stringmap = std::map<std::string, std::string>;
	using paramvec = std::vector<std::pair<std::string, std::string>>;

	std::map<std::string, std::string> map_from_json(json11::Json::object jsonmap);
	std::map<std::string, std::string> parse_xml_to_map(std::string xmldoc);

	template<class MapType, char sep_char = '&'>
	std::string mapvalue_to_string(MapType contentmap)
	{
		std::string content;
		for (const auto & p : contentmap)
		{
			if (p.second.empty())
				continue;
			if (!content.empty())
			{
				if (sep_char !=0)
					content.push_back(sep_char);
			}

			content.append(p.second);
		}
		return content;
	}

	template<class MapType, char sep_char = '&'>
	std::string map_to_string(MapType contentmap)
	{
		std::string content;
		for (const auto & p : contentmap)
		{
			if (p.second.empty())
				continue;
			if (!content.empty())
			{
				content.push_back(sep_char);
			}

			if (p.first.empty() || p.second.empty())
				continue;
			content.append(p.first);
			content.push_back('=');
			content.append(p.second);
		}
		return content;
	}

	template<class MapType, char sep_char = '&'>
	std::string map_to_string_with_empty(MapType contentmap)
	{
		std::string content;
		for (const auto & p : contentmap)
		{
			if (!content.empty())
			{
				content.push_back(sep_char);
			}

			content.append(p.first);
			content.push_back('=');
			content.append(p.second);
		}
		return content;
	}

	template<class MapType, char sep_char = '&'>
	std::string map_to_httpxform(MapType contentmap)
	{
		std::string content;
		for (const auto & p : contentmap)
		{
			if (p.second.empty())
				continue;
			if (!content.empty())
			{
				content.push_back(sep_char);
			}
			content.append(p.first);
			content.push_back('=');
			content.append(string_util::escape_path(p.second));
		}
		return content;
	}

	template<class MapType, char sep_char = '&'>
	std::string map_to_httpxform_with_empty(MapType contentmap)
	{
		std::string content;
		for (const auto & p : contentmap)
		{
			if (!content.empty())
			{
				content.push_back(sep_char);
			}
			content.append(p.first);
			content.push_back('=');
			content.append(string_util::escape_path(p.second));
		}
		return content;
	}

	template<class MapType, typename Escaper>
	std::string map_to_httpxform_with_escaper(MapType contentmap, Escaper escaper)
	{
		std::string content;
		for (const auto & p : contentmap)
		{
			if (!content.empty())
			{
				content.push_back('&');
			}
			content.append(p.first);
			content.push_back('=');
			content.append(escaper(p.second));
		}
		return content;
	}

	inline std::string money_to_string(money m, bool fixed = true)
	{
		std::stringstream ss;
		if (fixed)
		{
			ss.precision(2);
			ss << std::fixed << m; // Output to stringstream.
		}
		else
		{
			ss.precision(std::numeric_limits<money>::max_digits10);  // Ensure all potentially significant bits are output.
			ss << m; // Output to stringstream.
		}
		return ss.str();
	}

	inline std::string money_to_cent_string(money m)
	{
		std::stringstream ss;
		ss << (m * 100);
		return ss.str();
	}

	money money_from_string(std::string m);

	inline money money_from_cent_string(std::string m)
	{
		return money_from_string(m) / 100;
	}

	inline std::string cent_string_to_yuan_string(std::string cents)
	{
		return money_to_string(money_from_cent_string(cents));
	}

	template<typename String>
	std::string aliqr_uri_transform(const String& qr_code)
	{
		auto should_open = boost::str(boost::format("alipayqr://platformapi/startapp?saId=10000007&clientVersion=3.7.0.0718&qrcode=%s&_t=%d")
			% string_util::escape_path(qr_code+ "?_s=web-other") % std::time(nullptr));
		return should_open;
	}

	template<typename String>
	std::string aliqr_uri_transform2(const String& qr_code)
	{
		auto should_open = boost::str(boost::format("alipayqr://platformapi/startapp?saId=10000007&clientVersion=3.7.0.0718&qrcode=%s&_t=%d")
			% string_util::escape_path(qr_code) % std::time(nullptr));
		return should_open;
	}

	template<typename String>
	std::vector<char> string_to_char_vector(String&& in)
	{
		std::vector<char> ret;
		ret.reserve(in.size());
		for (auto&& k : in)
			ret.push_back(k);
		return ret;
	}

	std::map<std::string, std::string> parse_query_string(const std::string& text);

	boost::multiprecision::cpp_dec_float_50 parse_perent_to_decimal(const std::string&);

	boost::multiprecision::cpp_dec_float_50 get_channel_fee_rate_from_json_config(const json11::Json&, const std::string& payment_method);
}
