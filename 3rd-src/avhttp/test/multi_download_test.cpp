#include <iostream>
#include <boost/array.hpp>
#include <cmath>

#include "avhttp.hpp"

std::string to_string(int v, int width)
{
	std::stringstream s;
	s.flags(std::ios_base::right);
	s.width(width);
	s.fill(' ');
	s << v;
	return s.str();
}

std::string& to_string(float v, int width, int precision = 3)
{
	// this is a silly optimization
	// to avoid copying of strings
	enum { num_strings = 20 };
	static std::string buf[num_strings];
	static int round_robin = 0;
	std::string& ret = buf[round_robin];
	++round_robin;
	if (round_robin >= num_strings) round_robin = 0;
	ret.resize(20);
	int size = sprintf(&ret[0], "%*.*f", width, precision, v);
	ret.resize((std::min)(size, width));
	return ret;
}

std::string add_suffix(float val, char const* suffix = 0)
{
	std::string ret;
	if (val == 0)
	{
		ret.resize(4 + 2, ' ');
		if (suffix) ret.resize(4 + 2 + strlen(suffix), ' ');
		return ret;
	}

	const char* prefix[] = {"kB", "MB", "GB", "TB"};
	const int num_prefix = sizeof(prefix) / sizeof(const char*);
	for (int i = 0; i < num_prefix; ++i)
	{
		val /= 1024.f;
		if (std::fabs(val) < 1024.f)
		{
			ret = to_string(val, 4);
			ret += prefix[i];
			if (suffix) ret += suffix;
			return ret;
		}
	}
	ret = to_string(val, 4);
	ret += "PB";
	if (suffix) ret += suffix;
	return ret;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "usage: " << argv[0] << " <url>\n";
		return -1;
	}
	try {
		boost::asio::io_service io;
		avhttp::multi_download d(io);

		avhttp::settings s;
		// s.m_download_rate_limit = 102400;

		d.start(argv[1], s);

		if (d.file_size() != -1)
			std::cout << "file \'" << d.file_name().c_str() <<
			"\' size is: " << "(" << d.file_size() << " bytes) " << add_suffix(d.file_size()).c_str() << std::endl;

		boost::thread t(boost::bind(&boost::asio::io_service::run, &io));

		if (d.file_size() != -1)
		{
			printf("\n");
			int percent = 0;
			boost::int64_t file_size = d.file_size();
			boost::int64_t bytes_download = 0;
			while (percent != 100)
			{
				bytes_download = d.bytes_download();
				percent = ((double)bytes_download / (double)file_size) * 100.0f;
				boost::this_thread::sleep(boost::posix_time::millisec(200));
				printf("\r");
				printf("%3d%% [", percent);
				int progress = percent / 2;
				for (int i = 0; i < progress; i++)
					printf("=");
				if (progress != 50)
					printf(">");
				for (int i = 0; i < 49 - progress; i++)
					printf(" ");
				printf("]  %s  %s/s", add_suffix(bytes_download).c_str(), add_suffix(d.download_rate()).c_str());
			}
			printf("\n");
		}

		t.join();

		std::cout << "\n*** download completed! ***\n";
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
