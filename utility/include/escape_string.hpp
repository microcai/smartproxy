//
// Copyright (C) 2013 Jack.
//
// Author: jack
// Email:  jack.wgm@gmail.com
//

#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <sstream>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/hex.hpp>

#include "utf8.hpp"

namespace string_util {

static const char hex_chars[] = "0123456789abcdef";
static const char HEX_chars[] = "0123456789ABCDEF";

inline bool is_char(int c)
{
	return c >= 0 && c <= 127;
}

inline bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

inline bool is_ctl(int c)
{
	return (c >= 0 && c <= 31) || c == 127;
}

inline bool is_tspecial(int c)
{
	switch (c)
	{
		case ' ': case '`': case '{': case '}': case '^': case '|': case '\"' : case '\'': case ':':
		return true;
	default:
		return false;
	}
}

inline bool should_not_be_escaped(int c)
{
	if (c >= 'a' && c <= 'z')
		return true;
	if (c >= 'A' && c <= 'Z')
		return true;
	if (c >= '0' && c <= '9')
		return true;

	switch (c)
	{
		case '-': case '_': case '.': case '~':
		return true;
	}
	return false;
}

inline std::string to_hex(char i)
{
	std::string ret;
	ret.reserve(2);

	ret += hex_chars[((unsigned char)i) >> 4];
	ret += hex_chars[((unsigned char)i) & 0xf];

	return ret;
}

inline std::string to_HEX(char i)
{
	std::string ret;
	ret.reserve(2);

	ret += HEX_chars[((unsigned char)i) >> 4];
	ret += HEX_chars[((unsigned char)i) & 0xf];

	return ret;
}

inline bool is_print(char c)
{
	return c >= 32 && c < 127;
}

inline bool tolower_compare(char a, char b)
{
	return std::tolower(a) == std::tolower(b);
}


inline std::string escape_path(const std::string& s)
{
	std::string ret;

	for (char t : s)
	{
		if (!should_not_be_escaped(t))
			ret += "%" + to_hex(t);
		else
			ret += t;
	}

	return ret;
}

inline std::string escape_path_upper_encode(const std::string& s)
{
	std::string ret;

	for (char t : s)
	{
		if (!should_not_be_escaped(t))
			ret += "%" + to_HEX(t);
		else
			ret += t;
	}

	return ret;
}


inline bool unescape_path(const std::string& in, std::string& out)
{
	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i)
	{
		switch (in[i])
		{
		case '%':
			if (i + 3 <= in.size())
			{
				unsigned int value = 0;
				for (std::size_t j = i + 1; j < i + 3; ++j)
				{
					switch (in[j])
					{
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						value += in[j] - '0';
						break;
					case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
						value += in[j] - 'a' + 10;
						break;
					case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
						value += in[j] - 'A' + 10;
						break;
					default:
						return false;
					}
					if (j == i + 1)
						value <<= 4;
				}
				out += static_cast<char>(value);
				i += 2;
			}
			else
				return false;
			break;
		case '+':
			out += ' ';
			break;
		case '-': case '_': case '.': case '!': case '~': case '*':
		case '\'': case '(': case ')': case ':': case '@': case '&':
		case '=': case '$': case ',': case '/': case ';':
			out += in[i];
			break;
		default:
			if (!std::isalnum((unsigned char)in[i]))
				return false;
			out += in[i];
			break;
		}
	}
	return true;
}

// template <typename Source>
// std::string encode_base64(const Source& s)
// {
// 	using namespace boost::archive::iterators;
// 	typedef typename Source::const_iterator source_const_iterator;
// 	typedef base64_from_binary<
// 		transform_width<
// 			source_const_iterator,
// 			6,
// 			8
// 		>
// 	> base64_text;
// 	std::stringstream ss;
// 	std::copy(base64_text(s.begin()), base64_text(s.end()),
// 		boost::archive::iterators::ostream_iterator<char>(ss));
// 	std::string result = ss.str();
// 	int padding = 4 - result.size() % 4;
// 	for (int i = 0; i < padding; i++)
// 		result += "=";
// 	return result;
// }

static const std::string base64_chars =	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
										"abcdefghijklmnopqrstuvwxyz"
										"0123456789+/";

static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

inline std::string base64_encode(unsigned char const* bytes_to_encode, std::size_t in_len)
{
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}

inline std::string base64_decode(std::string const& encoded_string)
{
	size_t in_len = encoded_string.size();
	size_t i = 0;
	size_t j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

} // namespace string_util
