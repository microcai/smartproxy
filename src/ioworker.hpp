
#pragma once

#include <boost/beast.hpp>

#include "steady_clock.hpp"

#include <boost/asio.hpp>
#include <boost/signals2/signal.hpp>

class ioworker
{
	using tcp = boost::asio::ip::tcp;

public:
	ioworker(boost::asio::ip::tcp::acceptor& acceptor)
		: acceptor_(acceptor)
		, socket_(acceptor.get_executor())
	{
	}

	void start()
	{
		accept();
	}

private:
	// The acceptor used to listen for incoming connections.
	boost::asio::ip::tcp::acceptor& acceptor_;

	// The socket for the currently connected client.
	boost::asio::ip::tcp::socket socket_;

	// The buffer for performing reads
	std::array<char, 4096> buffer_;

	// The timer putting a time limit on requests.
	utility::steady_timer request_deadline_{
		acceptor_.get_executor(), (utility::steady_clock::time_point::max)()};

	void accept()
	{

		// Clean up any previous connection.
		boost::beast::error_code ec;
		socket_.close(ec);

		acceptor_.async_accept(
			socket_,
			[this](boost::beast::error_code ec)
			{
				if (ec)
				{
					accept();
				}
				else
				{
					// Request must be fully processed within 60 seconds.
					request_deadline_.expires_after(
						boost::chrono::seconds(4));

					request_deadline_.async_wait(boost::bind(&ioworker::check_deadline, this, boost::placeholders::_1));

					read_request();
				}
			});
	}

	void read_request()
	{
		// read the first packet. check for the protocol that has been send.

		socket_.async_read_some(boost::asio::buffer(buffer_), std::bind(&ioworker::process_request, this, std::placeholders::_1, std::placeholders::_2));
	}

	enum protocol_t
	{
		PROTO_HTTP,
		PROTO_SOCKS5,
	};

	protocol_t check_protocol()
	{
		if (buffer_[0] == 0x5)
			return PROTO_SOCKS5;
		return PROTO_HTTP;
	}

	void process_request(boost::system::error_code ec, std::size_t bytes_transfered)
	{
		request_deadline_.expires_at(utility::steady_clock::time_point::max());
		if (ec)
		{
			accept();
			return;
		}

		switch(check_protocol())
		{
			case PROTO_SOCKS5:
				on_accept_socks5(boost::ref(socket_), &buffer_[0], bytes_transfered);
				break;
			case PROTO_HTTP:
				on_accept_http(boost::ref(socket_), &buffer_[0], bytes_transfered);
				break;
		}

		accept();
	}

	void check_deadline(boost::beast::error_code ec)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;
		// The deadline may have moved, so check it has really passed.
		if (request_deadline_.expiry() <= utility::steady_clock::now())
		{
			// Close socket to cancel any outstanding operation.
			boost::beast::error_code ec;
			socket_.close();

			// Sleep indefinitely until we're given a new deadline.
			request_deadline_.expires_at(
				utility::steady_clock::time_point::max());
		}
	}

public:
	boost::signals2::signal<void(boost::asio::ip::tcp::socket&, const char*, std::size_t)> on_accept_socks5;
	boost::signals2::signal<void(boost::asio::ip::tcp::socket&, const char*, std::size_t)> on_accept_http;
};
