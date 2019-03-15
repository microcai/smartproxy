
#pragma once

#include <tuple>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace smart_timer {

namespace errc {

/// HTTP error codes.
/**
 * The enumerators of type @c errc_t are implicitly convertible to objects of
 * type @c boost::system::error_code.
 *
 * @par Requirements
 * @e Header: @c <error_codec.hpp> @n
 * @e Namespace: @c avhttp::errc
 */
enum errc_t
{
	/// wake_up called
	timer_wakeup = 1,
};

} // namespace errc

namespace detail {

	class error_category_impl
		: public boost::system::error_category
{
	virtual const char* name() const BOOST_SYSTEM_NOEXCEPT
	{
		return "HTTP";
	}

	virtual std::string message(int e) const
	{
		switch (e)
		{
		case errc::timer_wakeup:
			return "wakeup called ";
		default:
			return "Unknown smart_timer error";
		}
	}
};

}

template<class error_category>
const boost::system::error_category& error_category_single()
{
	static error_category error_category_instance;
	return reinterpret_cast<const boost::system::error_category&>(error_category_instance);
}

inline const boost::system::error_category& error_category()
{
	return error_category_single<detail::error_category_impl>();
}

namespace errc
{

/// Converts a value of type @c errc_t to a corresponding object of type
/// @c boost::system::error_code.
/**
 * @par Requirements
 * @e Header: @c <error_codec.hpp> @n
 * @e Namespace: @c avhttp::errc
 */
inline boost::system::error_code make_error_code(errc_t e)
{
	return boost::system::error_code(static_cast<int>(e), smart_timer::error_category());
}

}


template<typename TimerType, typename T>
class smart_timer
{
	struct timer_handler_op_base
	{
		std::atomic<bool> no_handler;

		timer_handler_op_base()
			: no_handler(false)
		{}

		virtual void handle_wait(boost::system::error_code) = 0;
		virtual void handle_waitup(boost::system::error_code ec, T arg) = 0;
	};

	template<typename Handler>
	struct timer_handler_op : public timer_handler_op_base
	{
		timer_handler_op(Handler handler)
			: m_handler(handler)
		{

		}

		virtual void handle_wait(boost::system::error_code ec) override
		{
			if (ec == boost::asio::error::operation_aborted)
			{
				if (timer_handler_op_base::no_handler.load())
					return;
			}
			m_handler(ec, T());
		}

		virtual void handle_waitup(boost::system::error_code ec, T arg) override
		{
			timer_handler_op_base::no_handler.store(true);
			m_handler(ec, arg);
		}

		Handler m_handler;
	};

	struct timer_handler_op_wrap
	{
		typedef void result_type;

		boost::shared_ptr<timer_handler_op_base> m_op;

		timer_handler_op_wrap(boost::shared_ptr<timer_handler_op_base> _op)
			: m_op(_op)
		{
		}

		void operator()(boost::system::error_code ec)
		{
			m_op->handle_wait(ec);
		}

		void operator()(boost::system::error_code ec, T arg)
		{
			m_op->handle_waitup(ec, arg);
		}
	};

public:
	explicit smart_timer(boost::asio::io_context& io)
		: io(io)
		, m_timer(io)
	{}

	template<typename... TT>
	void expires_from_now(TT...  arg)
	{
		m_timer.expires_from_now(arg...);
	}

	template<typename Handler>
	void async_wait(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		boost::shared_ptr<timer_handler_op_base> _handler_op;

		std::unique_lock<std::mutex> l(m);

		if (wake_up_thing)
		{
			_handler_op.reset(new timer_handler_op<Handler>(handler));
			T arg = *wake_up_thing;
			io.post(boost::asio::detail::bind_handler(timer_handler_op_wrap(_handler_op), errc::make_error_code(errc::timer_wakeup), arg));
			return;
		}

		_handler_op.reset(new timer_handler_op<Handler>(handler));
		m_handler = _handler_op;
		m_timer.async_wait(timer_handler_op_wrap(_handler_op));
	}

	void wake_up(T arg)
	{
		std::unique_lock<std::mutex> l(m);

		auto handler = m_handler.lock();

		if (handler)
		{
			handler->no_handler.store(true);
			io.post(boost::asio::detail::bind_handler(timer_handler_op_wrap(handler), errc::make_error_code(errc::timer_wakeup), arg));
			boost::system::error_code ignore;
			m_timer.cancel(ignore);
			m_handler.reset();
		}
		else
		{
			wake_up_thing.reset(new T(arg));
		}
	}

	void clear_wake_up_thing()
	{
		std::unique_lock<std::mutex> l(m);
		wake_up_thing.reset();
	}

	void cancel()
	{
		m_timer.cancel();
	}

	template<typename... TT>
	void cancel(TT... arg)
	{
		m_timer.cancel(arg...);
	}

private:
	boost::asio::io_context& io;
	TimerType m_timer;
	boost::weak_ptr<timer_handler_op_base> m_handler;
	std::mutex m;
	std::unique_ptr<T> wake_up_thing;
};

} // namespace smart_timer


namespace boost {
namespace system {

template<> struct is_error_code_enum<smart_timer::errc::errc_t>
{
  static const bool value = true;
};

}
}
