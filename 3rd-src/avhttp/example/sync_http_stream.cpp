#include <iostream>
#include <boost/array.hpp>
#include "avhttp.hpp"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "usage: " << argv[0] << " <url>\n";
		return -1;
	}

	try {
		boost::asio::io_service io;
		avhttp::http_stream h(io);

		//  可以设置请求选项.
		// avhttp::request_opts opt;
		// opt.insert("Connection", "Keep-Alive");
		// h.request_options(opt);

		h.open(argv[1]);

		boost::array<char, 1024> buf;
		boost::system::error_code ec;
		std::size_t file_size = 0;
		while (!ec)
		{
			std::size_t bytes_transferred = 0;
			// 也可以: bytes_transferred = boost::asio::read(h, boost::asio::buffer(buf), ec);
			bytes_transferred = h.read_some(boost::asio::buffer(buf), ec);
			file_size += bytes_transferred;
			std::cout.write(buf.data(), bytes_transferred);
		}
		std::cout.flush();

		h.close(ec);

		io.run();
	}
	catch (boost::system::system_error &e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
