///这个示例是用于从IP138.COM网站获得IP或域名所对应的地理位置.
#include <iostream>
#include <boost/array.hpp>
#include <boost/regex.hpp>
#include "avhttp.hpp"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "usage: " << argv[0] << " <domain/ip>\n";
		return -1;
	}

	boost::asio::io_service io;
	boost::system::error_code ec;

	std::string ip = argv[1];
	boost::trim(ip);
	// 构造查询IP的URL.
	std::string query_url = "http://ip138.com/ips138.asp?ip=" + ip;

	avhttp::http_stream h(io);
	h.open(query_url, ec);
	if (ec)
		return -1;

	std::string result;
	boost::asio::streambuf response;
	std::istream is(&h);

	try
	{
		while (is.good())
		{
			std::getline(is, result);
			std::size_t pos = result.find("<ul class=\"ul1\"><li>");
			if (pos == std::string::npos)
				continue;
			// 匹配出地址信息.
			boost::cmatch what;
			boost::regex ex("<ul class=\"ul1\"><li>(.*)?<\\/li><li>");
			if(boost::regex_search(result.c_str(), what, ex))
			{
				result = std::string(what[1]);
				std::string gbk_ex;
				gbk_ex.push_back('\xA3');
				gbk_ex.push_back('\xBA');
				gbk_ex += "(.*)";
				ex.assign(gbk_ex);
				if(boost::regex_search(result.c_str(), what, ex))
				{
					result = std::string(what[1]);
					// 输出地址信息.
					if (!result.empty())
						std::cout << result << std::endl;
				}
				break;
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
