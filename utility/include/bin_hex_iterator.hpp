﻿
#pragma once

#include <boost/archive/iterators/transform_width.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator.hpp>

namespace boost{
template<typename Base>
struct  bin_from_hex   :  public boost::iterator_adaptor <
	bin_from_hex<Base>,
	Base,
	uint8_t,
	boost::single_pass_traversal_tag,
	uint8_t
>
{
	friend class boost::iterator_core_access;
	typedef BOOST_DEDUCED_TYPENAME boost::iterator_adaptor <
		bin_from_hex<Base>,
		Base,
		uint8_t,
		boost::single_pass_traversal_tag,
		uint8_t
	> super_t;

	typedef bin_from_hex<Base> this_t;
	typedef uint8_t CharType;

	typedef typename boost::iterator_value<Base>::type base_value_type;

	Base m_base;

	static uint8_t hex_to_int(const char c)
	{
		switch (c)
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			return c - '0';
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			return c - 'a' + 10;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			return c - 'A' + 10;
		}
		return '?';
	}

	CharType dereference() const {
		return hex_to_int(*m_base);
	}

	// standard iterator interface
	bool equal(const this_t & rhs) const {
		return m_base == rhs.m_base;
	}

	void increment(){
		++m_base;
	}

	bin_from_hex(const Base & _base)
		: m_base(_base)
	{
	}
};

template<typename Base, typename _CharType = char>
struct  hex_from_bin
{
	typedef hex_from_bin<Base> this_t;
	typedef _CharType CharType;

	typedef typename boost::iterator_value<Base>::type base_value_type;

	typedef std::input_iterator_tag iterator_category;
	typedef char value_type;

	typedef typename Base::difference_type difference_type;
	typedef typename Base::reference reference;
	typedef typename Base::pointer pointer;

	Base m_base;

	static inline char char_to_hex(unsigned c)
	{
		static char table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
		return table[c];
	}

	CharType dereference() const {
		return char_to_hex(*m_base);
	}

	CharType operator * ()
	{
		return dereference();
	}

	// standard iterator interface
	bool equal(const this_t & rhs) const {
		return m_base == rhs.m_base;
	}

	bool operator == (const this_t & rhs)
	{
		return equal(rhs);
	}

	bool operator != (const this_t & rhs)
	{
		return ! equal(rhs);
	}


	void increment(){
		++m_base;
	}

	void operator ++ ()
	{
		increment();
	}

	hex_from_bin(Base const & _base)
		: m_base(_base)
	{
	}
};

static uint8_t hex_to_int(const char c)
{
	switch (c)
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return c - '0';
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return c - 'a' + 10;
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return c - 'A' + 10;
	}
	return '?';
}

template<class BaseIterator>
using basic_hex_from_bin_iterator = boost::hex_from_bin<boost::archive::iterators::transform_width<BaseIterator, 4, 8>, char>;

typedef basic_hex_from_bin_iterator<std::string::iterator> hex_from_bin_iterator;

template<class Iter>
static std::string bin2hex(Iter first, Iter last)
{
	return {
		basic_hex_from_bin_iterator<Iter>(first),
		basic_hex_from_bin_iterator<Iter>(last),
	};
}

static std::string hex2bin(std::string hex)
{
	if (hex.length() % 2 != 0)
	{
		return {};
	}

	std::string result;
	result.reserve(hex.length() / 2);
	for (auto i = 0; i < hex.size(); i += 2)
	{
		char c = hex_to_int(hex[i]) << 4 | hex_to_int(hex[i + 1]);
		result.push_back(c);
	}
	return result;
}

}
