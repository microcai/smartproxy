
#pragma once

#include <string>

#include <boost/multiprecision/cpp_dec_float.hpp>

std::string dec_fmt(boost::multiprecision::cpp_dec_float_50 num);

boost::multiprecision::cpp_dec_float_50 dec_round(boost::multiprecision::cpp_dec_float_50 val, boost::multiprecision::cpp_dec_float_50 round_step);

boost::multiprecision::cpp_dec_float_50 dec_round(boost::multiprecision::cpp_dec_float_50 val, const char* round_step);

boost::multiprecision::cpp_dec_float_50 dec_round_down(boost::multiprecision::cpp_dec_float_50 val, boost::multiprecision::cpp_dec_float_50 round_step);
boost::multiprecision::cpp_dec_float_50 dec_round_down(boost::multiprecision::cpp_dec_float_50 val, const char* round_step);
