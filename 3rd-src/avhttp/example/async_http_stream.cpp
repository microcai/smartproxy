#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "avhttp.hpp"

class downloader : public boost::enable_shared_from_this<downloader>
{
public:
	downloader(boost::asio::io_service &io)
		: m_io_service(io)
		, m_stream(io)
	{}
	~downloader()
	{}

public:
	void start(const std::string &url)
	{
		avhttp::proxy_settings p;
		p.type = avhttp::proxy_settings::http;
		p.hostname = "218.188.13.237";
		p.port = 8888;

		// 设置代理服务器.
		// m_stream.proxy(p);

		// 设置处理https不进行认证.
		m_stream.check_certificate(false);

		avhttp::request_opts opt;
		opt.insert(avhttp::http_options::accept_encoding, "gzip");
		// 设置接受gzip格式, 需要启用AVHTTP_ENABLE_ZLIB.
		// m_stream.request_options(opt);

		m_stream.async_open(url,
			boost::bind(&downloader::handle_open, shared_from_this(), boost::asio::placeholders::error));
	}

	void handle_open(const boost::system::error_code &ec)
	{
		if (!ec)
		{
			m_stream.async_read_some(boost::asio::buffer(m_buffer, 1024),
					boost::bind(&downloader::handle_read, shared_from_this(),
					boost::asio::placeholders::bytes_transferred,
					boost::asio::placeholders::error
				)
			);
//			也可以:
// 			boost::asio::async_read(m_stream, boost::asio::buffer(m_buffer, 597),
// 				boost::bind(&downloader::handle_read, shared_from_this(),
// 				boost::asio::placeholders::bytes_transferred,
// 				boost::asio::placeholders::error));
		}
	}

	void handle_read(int bytes_transferred, const boost::system::error_code &ec)
	{
		if (!ec)
		{
			std::cout.write(m_buffer.data(), bytes_transferred);
			m_stream.async_read_some(boost::asio::buffer(m_buffer),
					boost::bind(&downloader::handle_read, shared_from_this(),
					boost::asio::placeholders::bytes_transferred,
					boost::asio::placeholders::error
				)
			);
		}
		else if (ec == boost::asio::error::eof)
		{
			std::cout.write(m_buffer.data(), bytes_transferred);
		}
		std::cout.flush();
	}

private:
	boost::asio::io_service &m_io_service;
	avhttp::http_stream m_stream;
	boost::array<char, 1024> m_buffer;
};

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "usage: " << argv[0] << " <url>\n";
		std::cerr << "usage: " << argv[0] << " <url_list.txt>\n";
		return -1;
	}

	boost::asio::io_service io;
	std::fstream file;
	std::string filename = argv[1];
	bool urllist = false;

	// 如果是存在的文件, 则查看是否是txt文件, 允许.txt文件中多个url.
	// 在一个txt文件中可以保存多行url, 比如文件urls.txt中可以是:
	// http://www.boost.org/LICENSE_1_0.txt
	// http://www.microsoft.com/en-us/default.aspx
	// 本程序将同时下载这两个链接, 并且是单线程的哦!!!
	if (boost::filesystem::exists(filename))
	{
		if (boost::filesystem::extension(filename) == ".txt")
		{
			urllist = true;
			file.open(filename.c_str());
		}
	}

	for (;;)
	{
		if (!urllist)	// 单个文件下载.
		{
			boost::shared_ptr<downloader> d(new downloader(io));
			d->start(filename);
			break;
		}
		else			// 从url列表中下载, txt文件中的每一行是一个url.
		{
			std::string url;
			std::getline(file, url);
			if (url.empty())
				break;
			boost::shared_ptr<downloader> d(new downloader(io));
			d->start(url);
		}
	}

	io.run();

	return 0;
}
