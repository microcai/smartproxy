
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;
using po::options_description;
using po::variables_map;

#include "ioworker.hpp"
#include "stack_storage.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/pool/pool_alloc.hpp>

#include "proxyconfig.hpp"

#include "Socks5Session.hpp"

static proxyconfig parse_config(std::string configfile);

static boost::pool_allocator<Socks5Session> clientalloc;

static void process_socks5_client(boost::asio::io_context& io, boost::asio::ip::tcp::socket& socket, const char* preReadBuf, std::size_t preReadBufLength, proxyconfig& cfg)
{
	try{
		std::cerr << "new client\n";
		auto s = boost::allocate_shared<Socks5Session>(clientalloc, io, static_cast<boost::asio::ip::tcp::socket&&>(socket), cfg, preReadBuf, preReadBufLength);
		s->start();
	}catch(std::bad_alloc&)
	{
		std::cerr << "bad_alloc"  << std::endl;
	}
}


int main(int argc, char* argv[])
{
	std::string config;

	options_description desc("options");
	desc.add_options()
		("help,h", "help message")
		("version,v", "current version")
		("config", po::value<std::string>(&config), "config file")
		;

	variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << desc << "\n";
		return 0;
	}

	proxyconfig cfg = parse_config(config);

	boost::asio::io_context io;

	stack_storage<boost::asio::ip::tcp::acceptor, 20> stor_for_accept_socket;
	stack_storage<ioworker, 500> stor;

	std::vector<ioworker, stack_allocator<ioworker>> workers(stor);
	workers.reserve(500);

	std::vector<boost::asio::ip::tcp::acceptor, stack_allocator<boost::asio::ip::tcp::acceptor>> accept_sockets(stor_for_accept_socket);
	accept_sockets.reserve(20);

	accept_sockets.emplace_back(io);
	accept_sockets.emplace_back(io);



	accept_sockets[1].open(boost::asio::ip::tcp::v6());
	accept_sockets[1].set_option(boost::asio::detail::socket_option::integer<IPPROTO_IPV6, IPV6_V6ONLY>(1));
	accept_sockets[1].set_option(boost::asio::ip::tcp::acceptor::reuse_address(1));
	accept_sockets[1].bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), cfg.listenport));
	accept_sockets[1].listen();

	accept_sockets[0].open(boost::asio::ip::tcp::v4());
	accept_sockets[0].set_option(boost::asio::ip::tcp::acceptor::reuse_address(1));
	accept_sockets[0].bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), cfg.listenport));
	accept_sockets[0].listen();



	for (int i=0; i< 250; i++)
	{
		workers.emplace_back(accept_sockets[0]);

		workers.back().on_accept_socks5.connect(boost::bind(process_socks5_client, boost::ref(io), _1, _2, _3, boost::ref(cfg)));

//		workers.back().on_accept_http.connect(boost::bind(process_http_client, _1, _2, _3, boost::ref(cfg)));

		workers.back().start();

		workers.emplace_back(accept_sockets[1]);

		workers.back().on_accept_socks5.connect(boost::bind(process_socks5_client, boost::ref(io), _1, _2, _3, boost::ref(cfg)));

//		workers.back().on_accept_http.connect(boost::bind(process_http_client, _1, _2, _3, boost::ref(cfg)));

		workers.back().start();
	}

	io.run();
	return 0;
}

#include <fstream>
#include <boost/filesystem.hpp>
#include "json11.hpp"

static proxyconfig parse_config(std::string configfile)
{
	upstream_direct_connect_via_binded_address ud;
	ud.bind_addr = "::";

	proxyconfig cfg;
	cfg.listenport = 1810;
	cfg.upstreams.push_back(ud);
	ud.bind_addr = "0.0.0.0";
	cfg.upstreams.push_back(ud);

	upstream_socks5 ud2;
	ud2.sock_host = "127.0.0.1";
	ud2.sock_port = "1080";
	cfg.upstreams.push_back(ud2);

	ud2.sock_host = "127.0.0.1";
	ud2.sock_port = "1081";
	cfg.upstreams.push_back(ud2);

	ud2.sock_host = "127.0.0.1";
	ud2.sock_port = "1082";
	cfg.upstreams.push_back(ud2);

	std::ifstream ifile(configfile);

	if (ifile.is_open())
	{

		std::string config_file_content;

		config_file_content.resize(boost::filesystem::file_size(configfile));
		ifile.read(&config_file_content[0], config_file_content.capacity());

		std::string err;

		auto cfgjson = json11::Json::parse(config_file_content, err);

		if (cfgjson["listen"].is_number())
			cfg.listenport = cfgjson["listen"].number_value().convert_to<int>();

		if (cfgjson["upstreams"].is_array())
		{
			cfg.upstreams.clear();

			for (auto up : cfgjson["upstreams"].array_items())
			{

				if (!up["bind"].is_null())
				{
					upstream_direct_connect_via_binded_address ud;
					ud.bind_addr = up["bind"].string_value();

					cfg.upstreams.push_back(ud);
				}
				else if (!up["interface"].is_null())
				{
					upstream_direct_connect_via_binded_interface ud;
					ud.bindiface = up["interface"].string_value();
					cfg.upstreams.push_back(ud);
				}
				else if (!up["socks5"].is_null())
				{
					upstream_socks5 ud;
					ud.sock_host = up["socks5"].string_value();
					ud.sock_port = up["socks5port"].string_value();
					cfg.upstreams.push_back(ud);
				}
			}
		}
	}

	return cfg;
}
