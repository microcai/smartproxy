
#pragma once

#include <sstream>
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace currency
{
	class cny{};
}

template<typename Currency>
class money;

template<typename Currency>
money<Currency> operator + (const money<Currency>& a, const money<Currency>& b);

template<typename Currency>
class money
{
public:
    money()
    {}

    template<typename T>
    money(T&&a)
        : m_value(static_cast<T&&>(a))
    {}

	operator std::string() const
	{
		std::stringstream ss;
		ss << m_value;
		return ss.str();
	}

    friend money<Currency> operator + (const money<Currency>& a, const money<Currency>& b);
private:
    boost::multiprecision::cpp_dec_float_50 m_value;
};

template<typename Currency>
money<Currency> operator + (const money<Currency>& a, const money<Currency>& b)
{
    return money<Currency>(a.m_value + b.m_value);
}

using CNY = money<currency::cny>;
