
#pragma once

#include <array>
#include <atomic>
#include <vector>
#include <functional>

#include <boost/asio.hpp>

struct multiread_first_tag
{
	std::atomic_flag first_flag;

	bool readed = false;

	std::array<char, 4096> buf;

	std::size_t bytes_transfered;
	boost::system::error_code ec;

	std::mutex lock;

	struct queued_reader
	{
		std::function<void(boost::system::error_code, std::size_t)> handler;
		boost::asio::mutable_buffer buffer;
	};

	std::vector<queued_reader> queued_handlers;
};

namespace detail {

	template<class Stream, typename MutableBuffers, typename Handler>
	struct multiread_first_pkg_op : boost::asio::coroutine
	{
		multiread_first_pkg_op(multiread_first_tag& tag, Stream& s, const MutableBuffers& buffers, Handler handler)
			: tag(tag)
			, s(s)
			, buffers(buffers)
			, handler(handler)
		{

		}

		Handler handler;
		multiread_first_tag& tag;
		Stream& s;
		const MutableBuffers& buffers;

		typedef void result_type;

		void operator()(boost::system::error_code ec = {} , std::size_t bytes_transfered = 0)
		{
			BOOST_ASIO_CORO_REENTER(this)
			{

				if (!tag.first_flag.test_and_set())
				{
					BOOST_ASIO_CORO_YIELD s.async_read_some(boost::asio::buffer(tag.buf), *this);

					boost::asio::buffer_copy(buffers, boost::asio::buffer(tag.buf, bytes_transfered));

					handler(ec, bytes_transfered);

					{

						std::lock_guard<std::mutex> l(tag.lock);

						for (auto h : tag.queued_handlers)
						{
							boost::asio::buffer_copy(h.buffer, boost::asio::buffer(tag.buf, bytes_transfered));

							boost::asio::post(s.get_executor(), [ec, bytes_transfered, h]()
							{
								h.handler(ec, bytes_transfered);
							});
						}

						tag.readed = true;
						tag.ec = ec;
						tag.bytes_transfered = bytes_transfered;

					}
				}
				else
				{
					// 挂入列队.
					{
						std::lock_guard<std::mutex> l(tag.lock);
						if (tag.readed)
						{
							// 过了, 所以直接返回.
							boost::asio::buffer_copy(buffers,  boost::asio::buffer(tag.buf, tag.bytes_transfered));
							handler(tag.ec, tag.bytes_transfered);
						}
						else
						{
							tag.queued_handlers.push_back(multiread_first_tag::queued_reader{
								handler,
								buffers
							});
						}
					}
				}
			}
		}

	};

}

template<class Stream, typename MutableBuffers, typename Handler>
	BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code, std::size_t))
multiread_first_pkg(multiread_first_tag& tag, Stream& s, const MutableBuffers& buffers, Handler&& handler)
{
	// ASYNC_HANDLER_TYPE_CHECK(Handler, void(boost::system::error_code, std::size_t));
	boost::asio::async_completion<Handler, void(boost::system::error_code, std::size_t)> init(handler);

	detail::multiread_first_pkg_op<Stream, MutableBuffers, decltype(init.completion_handler)>(tag, s, buffers, init.completion_handler)();

	return init.result.get();
}