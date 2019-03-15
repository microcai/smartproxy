
#pragma once

#include <mutex>
#include <boost/lexical_cast.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/endian/arithmetic.hpp>
#include <string>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/beast/http.hpp>

#include "fields_alloc.hpp"
#include "streambufalloc.hpp"

#include "proxyconfig.hpp"

#include <iostream>
#include "steady_clock.hpp"
#include "splice.hpp"

#include "getifaddr.hpp"

template<typename F>
void call_once(std::atomic_flag & flag, F&& f)
{
	if (!flag.test_and_set())
		f();
}

namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

namespace boost { namespace asio {

	struct match_socks5_ok
	{
		template <typename Iterator>
		std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
		{
			if ((end - begin) < 2)
				return std::make_pair(end, false);

			Iterator i = begin;

			auto  buffer = i;

			auto count = buffer[1];

			if ((end - begin) < count + 2)
				return std::make_pair(end, false);

			return std::make_pair(begin + 2 + count, true);
		}

		//typedef std::pair<Iterator, bool> result_type;
	};

	template<>
	struct is_match_condition<match_socks5_ok>
	{
		static const bool value = true;
	};

}};

class Socks5Session : public boost::enable_shared_from_this<Socks5Session>
{
	using request_body_t = http::string_body;
	using alloc_t = fields_alloc<char>;

public:
	Socks5Session(boost::asio::ip::tcp::socket&& socket, proxyconfig&& cfg, const char* preReadBuf, std::size_t preReadBufLength)
		: m_socket(std::move(socket))
		, m_io(m_socket.get_io_context())
		, cfg(cfg)
	{
		m_recbuf.sputn(preReadBuf, preReadBufLength);
		one_upstream.clear();
	}

	Socks5Session(boost::asio::ip::tcp::socket&& socket, proxyconfig& cfg, const char* preReadBuf, std::size_t preReadBufLength)
		: m_socket(std::move(socket))
		, m_io(m_socket.get_io_context())
		, cfg(cfg)
	{
		m_recbuf.sputn(preReadBuf, preReadBufLength);
		one_upstream.clear();
	}


	void start()
	{
		auto self = shared_from_this();

		boost::asio::async_read_until(m_socket, m_recbuf, boost::asio::match_socks5_ok(), [self, this](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if(ec)
				return;

			const boost::uint8_t* buffer = boost::asio::buffer_cast<const boost::uint8_t*>(m_recbuf.data());
			int count =  buffer[1];

			if (std::find(&buffer[1], &buffer[count+2], 0) !=  &buffer[count+2])
			{
				m_recbuf.consume(bytes_transferred);

				// send out
				m_socket.async_write_some(boost::asio::buffer("\005\000",2),
					boost::bind(&Socks5Session::handle_handshake_write, self, _1, _2)
				);
			};

		});
	}

private:
	void handle_handshake_write(const boost::system::error_code & ec, std::size_t bytes_transferred)
	{
		if (!ec)
			m_socket.async_read_some(m_recbuf.prepare(5),
				 boost::bind(&Socks5Session::handle_read_socks5_magic, shared_from_this(), _1, _2)
			);
	}

	void handle_read_socks5_magic(const boost::system::error_code & ec, std::size_t bytes_transferred)
	{
		if( ec || bytes_transferred < 5)
		{
			std::cerr << ec.message() << std::endl;
			return;
		}

		m_recbuf.commit(bytes_transferred);

		const boost::uint8_t* buffer = boost::asio::buffer_cast<const boost::uint8_t*>(m_recbuf.data());

		if(buffer[0]==5 && buffer[1] == 1)
		{
			int type = buffer[3];
			switch(type)
			{
			case 1: // IPv4
				m_recbuf.consume(4);
				m_socket.async_read_some(m_recbuf.prepare(5),
					boost::bind(&Socks5Session::handle_read_socks5_ipv4host, shared_from_this(), _1, _2)
				);
				break;
			case 4:
				m_recbuf.consume(4);
				m_socket.async_read_some(m_recbuf.prepare(17),
					boost::bind(&Socks5Session::handle_read_socks5_ipv6host, shared_from_this(), _1, _2)
				);
				break;
			case 3: // DNS 地址.
				{
					if (m_recbuf.size() < 5)
						break;
					int dnshost_len = buffer[4]+2;
					m_recbuf.consume(m_recbuf.size());
					m_socket.async_read_some(m_recbuf.prepare(dnshost_len),
						boost::bind(&Socks5Session::handle_read_socks5_dnshost, shared_from_this(), _1, _2)
					);
				}
 				break;
			}
		}
	}

	void handle_read_socks5_ipv4host(const boost::system::error_code & ec, std::size_t bytes_transferred)
	{
		if(ec)
			return;
		m_recbuf.commit(bytes_transferred);
		const char* buffer = boost::asio::buffer_cast<const char*>(m_recbuf.data());

		boost::asio::ip::address_v4::bytes_type ip;
		m_recbuf.sgetn((char*) &ip, 4);

		boost::asio::ip::address_v4 host(ip);

		boost::endian::big_int16_t port_networkbytesorder;
		m_recbuf.sgetn((char*) &port_networkbytesorder, 2);

		int port = port_networkbytesorder;
		// 好的，目的地址和端口都获得了， 开启全部的 upstream，让 upstream 来干接下来的脏活.
		m_recbuf.consume(m_recbuf.size());

		std::cerr << "receive proxy request to : " << host.to_string() << ": " << port << std::endl;
		send_out_all_upstream(host.to_string(), port);
	}

	void handle_read_socks5_ipv6host(const boost::system::error_code & ec, std::size_t bytes_transferred)
	{
		if(ec)
			return;
		m_recbuf.commit(bytes_transferred);
		const char* buffer = boost::asio::buffer_cast<const char*>(m_recbuf.data());

		boost::asio::ip::address_v6::bytes_type ip;
		m_recbuf.sgetn((char*) &ip, 16);

		boost::asio::ip::address_v6 host(ip);

		boost::endian::big_int16_t port_networkbytesorder;
		m_recbuf.sgetn((char*) &port_networkbytesorder, 2);

		int port = port_networkbytesorder;
		// 好的，目的地址和端口都获得了， 开启全部的 upstream，让 upstream 来干接下来的脏活.
		m_recbuf.consume(m_recbuf.size());

		std::cerr << "receive proxy request to : " << host.to_string() << ": " << port << std::endl;
		send_out_all_upstream(host.to_string(), port);
	}
	void handle_read_socks5_dnshost(const boost::system::error_code & ec, std::size_t bytes_transferred)
	{
		if(ec)
			return;
		m_recbuf.commit(bytes_transferred);
		const char* buffer = boost::asio::buffer_cast<const char*>(m_recbuf.data());

		std::string host;
		host.assign(buffer, m_recbuf.size()-2);
		int port = boost::endian::big_to_native( *(boost::uint16_t*)(buffer+ m_recbuf.size()-2));
		// 好的，目的地址和端口都获得了， 开启全部的 upstream，让 upstream 来干接下来的脏活.
		m_recbuf.consume(m_recbuf.size());

		std::cerr << "receive proxy request to : " << host << ": " << port << std::endl;

		send_out_all_upstream(host, port);
	}

	void send_out_all_upstream(std::string host, int port)
	{
		struct upstream_visitor
		{
			void operator()(const upstream_direct_connect_via_binded_address& up) const
			{
				boost::asio::spawn(
					_this->m_io,
					[this, up](boost::asio::yield_context yield_context)
					{
						_this->direct_connect_coroutine(host, port, up, yield_context);
					});
			}

			void operator()(const upstream_direct_connect_via_binded_interface& up) const
			{
				boost::asio::spawn(
					_this->m_io,
					[this, up](boost::asio::yield_context yield_context)
					{
						_this->direct_connect_coroutine(host, port, up, yield_context);
					});
			}

			void operator()(const upstream_socks5& up) const
			{
				boost::asio::spawn(_this->m_io, boost::bind(&Socks5Session::socks5_connect_coroutine, _this->shared_from_this(), host, port, up, _1));
			}

			mutable Socks5Session* _this;
			std::string host;
			int port;

			upstream_visitor(Socks5Session* parent, std::string host, int port)
				: _this(parent)
				, host(host)
				, port(port)
			{
			}

		};

		// spawn all coroutines to connect to upstream, and select the fasted one.
		for ( auto u : cfg.upstreams)
		{
			std::visit(upstream_visitor(this, host, port), u);
		}
	}

	void socks5_connect_coroutine(std::string host, int port, upstream_socks5& up, boost::asio::yield_context yield_context)
	{

		std::cerr << "\ttry use socks5 to connect:" << host << "\n";

		boost::system::error_code ec;

		boost::asio::ip::tcp::resolver resolver(m_io);

		utility::steady_timer delay_timer(m_io);

		delay_timer.expires_from_now(boost::chrono::milliseconds(250));

		delay_timer.async_wait(yield_context[ec]);

		auto endpoints_range = resolver.async_resolve(boost::asio::ip::tcp::resolver::query(up.sock_host, up.sock_port), yield_context[ec]);

		if (ec)
			return;

		for (auto remote_to_connect : endpoints_range)
		{
			boost::asio::spawn(
				m_io,
				boost::bind(&Socks5Session::socks5_all_dnsresult_coroutine, shared_from_this(), remote_to_connect, up, host, port, _1)
			);
		}
	}

	void socks5_all_dnsresult_coroutine(boost::asio::ip::tcp::endpoint remote_to_connect, upstream_socks5& up, std::string host, int port, boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;
		boost::asio::ip::tcp::socket client_sock(m_io);

		client_sock.open(remote_to_connect.protocol());
		client_sock.async_connect(remote_to_connect, yield_context[ec]);

		if (ec)
			return;

		// now , check the first that returns!

		// then send request to socks5 server.

		boost::asio::async_write(client_sock, boost::asio::buffer("\x05\x01\x00", 3), yield_context[ec]);

		if (ec)
			return;
		// wait for socks5 auth ok.

		unsigned char buf[2];

		boost::asio::async_read(client_sock, boost::asio::buffer(buf), yield_context[ec]);
		if (ec)
			return;
		// check for 0500
		if (buf[0] == 0x05 && buf[1] == 0x00)
		{
			// write out
			unsigned char req_buf[64];
			int len = 0;

			req_buf[0] = 0x05;
			req_buf[1] = 0x01;
			req_buf[2] = 0x00;

			req_buf[3] = 0x03;
			req_buf[4] = host.length();
			std::copy(host.begin(), host.end(), &req_buf[5]);

			boost::endian::big_int16_t nport = port;

			memcpy(req_buf + 5 + host.length(), &nport, 2);

			len = host.length() + 7;

			boost::asio::async_write(client_sock, boost::asio::buffer(req_buf, len), boost::asio::transfer_exactly(len), yield_context[ec]);
			if (ec)
				return;
			// and then waiting for reply
			unsigned char rep_buf_head[5];
			boost::asio::async_read(client_sock, boost::asio::buffer(rep_buf_head), boost::asio::transfer_at_least(4),yield_context[ec]);

			if (ec)
				return;

			if (rep_buf_head[0] == 0x5 && rep_buf_head[1] == 0x00 && rep_buf_head[2] == 0x00)
			{
				switch(rep_buf_head[3])
				{
					case 1:
					{
						char buf[6];
						buf[0] = rep_buf_head[4];
						boost::asio::async_read(client_sock, boost::asio::buffer(buf+1, 5), boost::asio::transfer_exactly(5), yield_context[ec]);
					}
					break;
					case 3:
					{
						int l = rep_buf_head[4];
#ifdef _MSC_VER
						char buf[100];
#else
						char buf[l + 2];
#endif
						boost::asio::async_read(client_sock, boost::asio::buffer(buf, l + 2), boost::asio::transfer_exactly(l + 2), yield_context[ec]);
					}
					break;
					case 4:
					{
						char buf[18];
						buf[0] = rep_buf_head[4];
						boost::asio::async_read(client_sock, boost::asio::buffer(buf + 1, 17), boost::asio::transfer_exactly(17), yield_context[ec]);
					}
					break;
					default:
						return;
				}
			}

			if (!ec)
				call_once(one_upstream, boost::bind(&Socks5Session::handlesocks5_connection_success, shared_from_this(), boost::ref(client_sock), host, port, up, yield_context));
		}

	}

	void handlesocks5_connection_success(boost::asio::ip::tcp::socket& client_sock, std::string host, int port, upstream_socks5& up, boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;
		// 向 client 返回链接成功信息.
		m_socket.async_write_some(asio::buffer("\005\000\000\001\000\000\000\000\000\000",10), yield_context[ec]);

		std::cerr << "fastest connect to " << host << ":" << port << " via socks5 proxy: " << up.sock_host << ":" << up.sock_port << "\n";

		boost::shared_ptr<avsocks::splice<Socks5Session, boost::asio::ip::tcp::socket&, boost::asio::ip::tcp::socket>> splice_ptr;

		if (!ec)
		{
			// start doing splice now.
			splice_ptr.reset(
				new avsocks::splice<Socks5Session, boost::asio::ip::tcp::socket&, boost::asio::ip::tcp::socket>(shared_from_this(), m_socket, std::move(client_sock))
			);

			splice_ptr->start();
		}
	}

	template<typename UPSTREAM_DESC>
	void direct_connect_coroutine(std::string host, int port, UPSTREAM_DESC&& up, boost::asio::yield_context yield_context)
	{
		std::string port_s = std::to_string(port);

		boost::asio::ip::tcp::resolver resolver(m_io);

		boost::system::error_code ec;

		std::cerr << "\ttry direct connecting to " << host << "\n";

		auto endpoints_range = resolver.async_resolve(boost::asio::ip::tcp::resolver::query(host, port_s), yield_context[ec]);

		if (ec)
		{
			std::cerr << ec.message() << "\n";
			return;
		}

		for (auto remote_to_connect : endpoints_range)
		{
			boost::asio::spawn(
				m_io, [this, remote_to_connect, up, _this = shared_from_this()](boost::asio::yield_context yield_context)
				{
					connect_all_dnsresult_coroutine(remote_to_connect, up, yield_context);
				}
			);
		}
	}

	void connect_all_dnsresult_coroutine(boost::asio::ip::tcp::endpoint remote_to_connect, const upstream_direct_connect_via_binded_address& up, boost::asio::yield_context yield_context)
	{
		auto bind_addr = boost::asio::ip::address::from_string(up.bind_addr);

		boost::asio::ip::tcp::socket client_sock(m_io);

		if (bind_addr.is_v4() && remote_to_connect.protocol() == boost::asio::ip::tcp::v6())
		{
			std::cerr << "not using " << bind_addr << " to connect " << remote_to_connect << ", because ipv4 can not connect to ipv6 site" << std::endl;
			return;
		}

		if (bind_addr.is_v6() && remote_to_connect.protocol() == boost::asio::ip::tcp::v4())
		{
//			std::cerr << "not using " << bind_addr << " to connect " << remote_to_connect << ", because ipv6 can not connect to ipv4 site" << std::endl;
			return;
		}

		client_sock.open(bind_addr.is_v6() ? boost::asio::ip::tcp::v6() : boost::asio::ip::tcp::v4());
		client_sock.bind(boost::asio::ip::tcp::endpoint(bind_addr, 0));

		boost::system::error_code ec;

		client_sock.async_connect(remote_to_connect, yield_context[ec]);

		if (ec)
			return;

		call_once(one_upstream, [&client_sock, &up, &yield_context, this](){
			handle_connection_success(client_sock, up, yield_context);
		});

	}

	void connect_all_dnsresult_coroutine(boost::asio::ip::tcp::endpoint remote_to_connect, const upstream_direct_connect_via_binded_interface& up, boost::asio::yield_context yield_context)
	{
		boost::asio::ip::tcp::socket client_sock(m_io);

		client_sock.open(remote_to_connect.protocol());

		boost::asio::ip::address bind_addr;

		if (remote_to_connect.address().is_v6())
			bind_addr = getifaddrv6(up.bindiface);
		else
			bind_addr = getifaddrv4(up.bindiface);


		client_sock.bind(boost::asio::ip::tcp::endpoint(bind_addr, 0));

		boost::system::error_code ec;

		client_sock.async_connect(remote_to_connect, yield_context[ec]);

		if (ec)
			return;

		// now , check the first that returns!
		call_once(one_upstream, [&client_sock, &up, &yield_context, this](){
			handle_connection_success(client_sock, up, yield_context);
		});
	}

	void handle_connection_success(boost::asio::ip::tcp::socket& client_sock, const upstream_direct_connect_via_binded_address& up, boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;
		if (ec)
			return;
		// 向 client 返回链接成功信息.
		m_socket.async_write_some(asio::buffer("\005\000\000\001\000\000\000\000\000\000",10), yield_context[ec]);

		std::cerr << "fastest connect to " << client_sock.remote_endpoint(ec) << " via " << client_sock.local_endpoint() << "\n";

		boost::shared_ptr<avsocks::splice<Socks5Session, boost::asio::ip::tcp::socket&, boost::asio::ip::tcp::socket>> splice_ptr;

		// start doing splice now.
		splice_ptr.reset(
			new avsocks::splice<Socks5Session, boost::asio::ip::tcp::socket&, boost::asio::ip::tcp::socket>(shared_from_this(), m_socket, std::move(client_sock))
		);

		splice_ptr->start();
	}

	void handle_connection_success(boost::asio::ip::tcp::socket& client_sock, const upstream_direct_connect_via_binded_interface& up, boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;
		if (ec)
			return;
		// 向 client 返回链接成功信息.
		m_socket.async_write_some(asio::buffer("\005\000\000\001\000\000\000\000\000\000",10), yield_context[ec]);

		std::cerr << "fastest connect to " << client_sock.remote_endpoint(ec) << " via " << client_sock.local_endpoint() << "\n";

		boost::shared_ptr<avsocks::splice<Socks5Session, boost::asio::ip::tcp::socket&, boost::asio::ip::tcp::socket>> splice_ptr;

		// start doing splice now.
		splice_ptr.reset(
			new avsocks::splice<Socks5Session, boost::asio::ip::tcp::socket&, boost::asio::ip::tcp::socket>(shared_from_this(), m_socket, std::move(client_sock))
		);

		splice_ptr->start();
	}

private:
	boost::asio::ip::tcp::socket m_socket;
	boost::asio::io_context& m_io;

	std::atomic_flag one_upstream;

	streambufstorage recv_real_buffer;

	boost::asio::basic_streambuf<streambufalloc<char>> m_recbuf  {recv_real_buffer.max_size(), streambufalloc<char>(recv_real_buffer)};

	proxyconfig cfg;
};
