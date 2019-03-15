
#include "easyhttp.hpp"

void avhttp_set_proxy(avhttp::http_stream& h, std::string use_proxy)
{
	if (use_proxy.size())
	{
		boost::system::error_code ec;
		avhttp::url proxy_string = avhttp::url::from_string(use_proxy, ec);

		avhttp::proxy_settings s;

		if (proxy_string.protocol() == "socks5")
		{
			s.type = avhttp::proxy_settings::socks5;
		}
		else if (proxy_string.protocol() == "http")
		{
			s.type = avhttp::proxy_settings::http;
		}

		s.hostname = proxy_string.host();
		s.port = proxy_string.port();

		h.proxy(s);
	}

}

void avhttp_enable_ssl(avhttp::http_stream& h)
{
#ifdef _WIN32
	h.check_certificate(false);
#else
	if (boost::filesystem::exists("/etc/ssl/certs/ca-bundle.crt"))
		h.load_verify_file("/etc/ssl/certs/ca-bundle.crt");
	else
		h.add_verify_path("/etc/ssl/certs");
	h.check_certificate(true);
#endif
}

void easy_http_get(boost::asio::io_context& io, std::string url,
	std::vector<std::pair<std::string, std::string>> additional_headers,
	   std::function<void(boost::system::error_code, std::string)> handler, std::string use_proxy)
{
	auto m_http_stream = std::make_shared<avhttp::http_stream>(io);
	auto m_readbuf = std::make_shared<boost::asio::streambuf>();

	avhttp::request_opts opt;

	opt(avhttp::http_options::user_agent, "mozilla");

	for (auto kv : additional_headers)
		opt.insert(kv);

	m_http_stream->request_options(opt);

	avhttp_set_proxy(*m_http_stream, use_proxy);
	avhttp_enable_ssl(*m_http_stream);

	avhttp::async_read_body(*m_http_stream, url, *m_readbuf, [m_readbuf, m_http_stream, handler](boost::system::error_code ec, std::size_t bytes_transfered)
	{
		if (ec || bytes_transfered <= 0)
		{
			handler(ec, "");
			return;
		}

		// decode the returned data

		std::string responseStr;
		responseStr.resize(bytes_transfered);
		m_readbuf->sgetn(&responseStr[0], bytes_transfered);

		handler(ec, responseStr);
	});

}

void easy_http_get(boost::asio::io_context& io, std::string url, std::function<void(boost::system::error_code, std::string)> handler, std::string use_proxy)
{
	easy_http_get(io, url, {}, handler, use_proxy);
}

void easy_http_post(boost::asio::io_context& io, std::string url, std::pair<std::string, std::string> post_content,
	std::function<void(boost::system::error_code, std::string)> handler, std::string use_proxy)
{
	auto m_http_stream = std::make_shared<avhttp::http_stream>(io);
	auto m_readbuf = std::make_shared<boost::asio::streambuf>();

	avhttp::request_opts opt;

	opt(avhttp::http_options::user_agent, "mozilla");
	opt(avhttp::http_options::request_method, "POST");
	opt(avhttp::http_options::request_body, post_content.second);
	opt(avhttp::http_options::content_type, post_content.first);
	opt(avhttp::http_options::content_length, std::to_string(post_content.second.length()));

	m_http_stream->request_options(opt);

	avhttp_set_proxy(*m_http_stream, use_proxy);
	avhttp_enable_ssl(*m_http_stream);

	avhttp::async_read_body(*m_http_stream, url, *m_readbuf, [m_readbuf, m_http_stream, handler](boost::system::error_code ec, std::size_t bytes_transfered)
	{
		if (ec || bytes_transfered <= 0)
		{
			handler(ec, "");
			return;
		}

		// decode the returned data

		std::string responseStr;
		responseStr.resize(bytes_transfered);
		m_readbuf->sgetn(&responseStr[0], bytes_transfered);

		handler(ec, responseStr);
	});
}
