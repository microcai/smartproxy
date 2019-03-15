#include "avhttp.hpp"
#include <iostream>
#include <boost/asio.hpp>
namespace asio = boost::asio;

int main()
{
	asio::io_service io;
	avhttp::http_stream stream(io);
	std::map<std::string, std::string> key_values;
	key_values["name"] = "hyq";
	key_values["age"] = "25";
	avhttp::post_form(stream, key_values);
	stream.open("http://localhost/");
	std::cout << &stream;
}
