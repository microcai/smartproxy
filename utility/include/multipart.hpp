
#pragma once

#include <random>
#include <tuple>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>

template<typename FormData>
std::tuple<std::string, std::string> build_multi_part_form(const FormData& formdata)
{
	thread_local static std::mt19937 mt = std::mt19937(std::random_device()());

	static const auto encode_chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	// boost::format("%s&time=%s") callback_data
	std::uniform_int_distribution<int> ud(0, 61);

	std::string boundry = "--";

	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);
	boundry.push_back(encode_chars[ud(mt)]);

	boundry += "--";

	std::string content_type = "multipart/form-data; boundary=" + boundry;

	boundry = "--" + boundry;	// 之后都是单行的分隔.

	std::stringstream http_content_body;

	for ( auto & form_item : formdata)
	{

		http_content_body << boundry << "\r\n";
		http_content_body << "Content-Disposition: form-data; name=\"" << form_item.first << "\"\r\n";

		if (form_item.first == "json")
			http_content_body << "Content-Type: application/json\r\n";

		http_content_body << "\r\n";
		http_content_body << form_item.second;
		http_content_body << "\r\n";
	}

	http_content_body << boundry << "--\r\n";

	return std::make_tuple(http_content_body.str(), content_type);
}
