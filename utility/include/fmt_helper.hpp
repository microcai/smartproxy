#pragma once

#include <boost/format.hpp>

namespace fmt_helper_detail {

	static inline boost::format& formatImpl(boost::format& f)
	{
		return f;
	}

	template <typename Head, typename... Tail>
	static inline boost::format& formatImpl(boost::format& f, const Head & head, Tail&&... tail)
	{
		return formatImpl(f % head, std::forward<Tail>(tail)...);
	}

}

template <typename... Args>
static inline std::string format_msg(std::string formatString, Args&&... args)
{
	try
	{
		boost::format f(std::move(formatString));
		return fmt_helper_detail::formatImpl(f, std::forward<Args>(args)...).str();
	}
	catch (...)
	{

	}
	return "erro when formating the string : " + formatString;
}
