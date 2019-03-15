
#pragma once

#include <string>
#include <map>
#include "httpd/http_helper.hpp"
#include "pay_utility.hpp"

namespace url_reencode
{
	std::string reencode(const std::string& url_raw)
	{
		auto query_string_pos = url_raw.find('?');
		if (query_string_pos == std::string::npos)
			return url_raw;

		http::http_form qs(url_raw.substr(query_string_pos + 1), {});
		std::map<std::string, std::string> tmp;
		for (auto kv : qs.headers)
		{
			tmp.emplace(kv);
		}

		std::string encoded = pay_utility::map_to_httpxform_with_empty(tmp);
		return url_raw.substr(0, query_string_pos) + encoded;
	}
}