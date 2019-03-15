
#pragma once

#include <type_traits>
#include "smart_timer.hpp"

template<typename TimerType, typename... T>
class smart_timer_var
{
public:
	explicit smart_timer_var(boost::asio::io_context& io)
		: io(io)
		, m_working_timer(io)
	{}

	template<typename... TT>
	void expires_from_now(TT...  arg)
	{
		m_working_timer.expires_from_now(arg...);
	}

	template<typename Handler>
	void async_wait(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		m_working_timer.async_wait([this, handler](boost::system::error_code ec, std::tuple<T...> arg)
		{
			// handler()
			call_handler_with_tuple(ec, io.wrap(handler), arg, std::index_sequence_for<T...>());
		});
	}

	void wake_up(T... arg)
	{
		m_working_timer.wake_up(std::make_tuple(arg...));
	}

	void clear_wake_up_thing()
	{
		m_working_timer.clear_wake_up_thing();
	}

	void cancel()
	{
		m_working_timer.cancel();
	}

	template<typename... TT>
	void cancel(TT... arg)
	{
		m_working_timer.cancel(arg...);
	}

private:
		//The helper method.
	template<typename Handler, std::size_t... Is>
	static void call_handler_with_tuple(boost::system::error_code ec, Handler handler, const std::tuple<T...>& tuple, std::index_sequence<Is...>)
	{
		handler(ec, std::get<Is>(tuple)...);
	}

private:
	boost::asio::io_context& io;
	smart_timer::smart_timer<TimerType, std::tuple<T...>> m_working_timer;
};

