// upload image to imagebin.org
#include <iostream>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "avhttp.hpp"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "usage: " << argv[0] << " <filename> [nickname]\n";
		return -1;
	}

	std::string filename = std::string(argv[1]);
	std::string extension = boost::filesystem::path(filename).extension().string();
	boost::to_lower(extension);
	if (extension != ".png" &&
		extension != ".jpg" &&
		extension != ".jpeg" &&
		extension != ".gif" &&
		extension != ".jpe")
	{
		std::cerr << "You must provide a image!\n";
		return -1;
	}

	boost::asio::io_service io;
	avhttp::file_upload upload(io);

	avhttp::request_opts opts;
	opts.insert("Referer", "http://imagebin.org/index.php?page=add");
	opts.insert(avhttp::http_options::user_agent, "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/28.0.1500.72 Safari/537.36");
	opts.insert("Origin", "http://imagebin.org");
	opts.insert("Cache-Control", "max-age=0");
	opts.insert("Accept-Language", "zh-CN,zh;q=0.8");

	upload.request_option(opts);

	avhttp::file_upload::form_args args;
	args["nickname"] = "Cai";
	args["remember_nickname"] = "Y";
	args["title"] = boost::filesystem::path(filename).leaf().string();
	args["description"] = "Upload by avhttp";
	args["disclaimer_agree"] = "Y";
	args["Submit"] = "Submit";
	args["mode"] = "add";
	avhttp::http_stream& http = upload.get_http_stream();
	http.max_redirects(0);
	boost::system::error_code ec;
	upload.open("http://imagebin.org/index.php", filename, "image", args, ec);
	if (ec)
	{
		return -1;
	}
	// start upload image.
	avhttp::default_storge file;
	file.open(filename, ec);
	if (ec)
	{
		return -1;
	}

	boost::array<char, 1024> buffer;
	while (!file.eof())
	{
		int readed = file.read(buffer.data(), 1024);
		boost::asio::write(upload, boost::asio::buffer(buffer, readed), ec);
		if (ec)
		{
			return -1;
		}
	}
	upload.write_tail(ec);
	if (ec)
	{
		return -1;
	}

	// output image url.
	std::string path = http.location();
	std::cout << path << std::endl;

	return 0;
}
