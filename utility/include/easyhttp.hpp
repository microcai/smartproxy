#pragma once

#include <string>
#include <boost/system/error_code.hpp>
#include "avhttp.hpp"

void avhttp_enable_ssl(avhttp::http_stream& h);
void avhttp_set_proxy(avhttp::http_stream& h, std::string proxy);

void easy_http_get(boost::asio::io_context& io, std::string url, std::function<void(boost::system::error_code, std::string)>, std::string use_proxy = "");
void easy_http_get(boost::asio::io_context& io, std::string url, std::vector<std::pair<std::string, std::string>> additional_headers, std::function<void(boost::system::error_code, std::string)>, std::string use_proxy = "");

void easy_http_post(boost::asio::io_context& io, std::string url,
	std::pair<std::string, std::string> post_content,
	std::function<void(boost::system::error_code, std::string)>, std::string use_proxy = "");
