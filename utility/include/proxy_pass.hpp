
#pragma once

#include "avhttp.hpp"
#include "httpd/http_connection.hpp"
#include "easyhttp.hpp"

namespace utility { namespace impl
{
/*
	proxy_pass_handler(boost::system::error_code, std::size_t bytes_transfered);
 */
template <typename Handler>
inline BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
proxy_pass(avhttp::url&& proxyed_url, libhttpd::http_connection_ptr con, Handler&& handler)
{
	struct proxy_pass_op : boost::asio::coroutine
	{
		avhttp::url proxyed_url;
		libhttpd::http_connection_ptr con;
		Handler handler;

		std::shared_ptr<avhttp::http_stream> m_http;
		std::shared_ptr<boost::asio::streambuf> m_readbuf;

		void operator()(boost::system::error_code ec = {}, std::size_t bytes_transfered = 0)
		{
			std::string retdata;
			avhttp::request_opts opt;


			BOOST_ASIO_CORO_REENTER(this)
			{
				opt(avhttp::http_options::user_agent, "mozilla");
				opt(avhttp::http_options::host, proxyed_url.host());
				opt("X-Forwarded-For", con->http_header("x-forwarded-for"));

				if (con->m_http_parser.method == HTTP_POST)
				{
					opt(avhttp::http_options::request_method, "POST");
					opt(avhttp::http_options::request_body, con->post_body());
					opt(avhttp::http_options::content_length, std::to_string(con->post_body().length()));
				}

				m_http->request_options(opt);

				m_readbuf = std::make_shared<boost::asio::streambuf>();

				BOOST_ASIO_CORO_YIELD avhttp::async_read_body(*m_http, proxyed_url, *m_readbuf, * this);

				if (ec || bytes_transfered <= 0)
				{
					handler(ec, bytes_transfered);
					return;
				}

				// decode the returned data
				retdata.resize(bytes_transfered);
				m_readbuf->sgetn(&retdata[0], bytes_transfered);
				if (con)
					con->write_response(retdata);
				handler(ec, bytes_transfered);
			}
		}

		proxy_pass_op(avhttp::url&& a, libhttpd::http_connection_ptr b, Handler&& c)
			: proxyed_url(a)
			, con(b)
			, handler(c)
			, m_http(std::make_shared<avhttp::http_stream>(con->m_io_service))
		{
			avhttp_enable_ssl(*m_http);
			//avhttp_set_proxy(*m_http);
		}
	};

	proxy_pass_op(std::move(proxyed_url), con, BOOST_ASIO_MOVE_CAST(Handler)(handler))();
}

}
}

/*
	proxy_pass_handler(boost::system::error_code, std::size_t bytes_transfered);
 */
template <typename Handler>
inline BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
proxy_pass(avhttp::url proxyed_url, libhttpd::http_connection_ptr con, Handler&& handler)
{
	using namespace boost::asio;

	BOOST_ASIO_WRITE_HANDLER_CHECK(Handler, handler) type_check;

	boost::asio::detail::async_result_init<
		Handler, void(boost::system::error_code, std::size_t)> init(
		BOOST_ASIO_MOVE_CAST(Handler)(handler));

	utility::impl::proxy_pass<BOOST_ASIO_HANDLER_TYPE(Handler, void(boost::system::error_code, std::size_t))>
		(std::move(proxyed_url), con, BOOST_ASIO_MOVE_CAST(BOOST_ASIO_HANDLER_TYPE(Handler, void(boost::system::error_code, std::size_t)))(init.handler));

	return init.result.get();
}
