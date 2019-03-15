//
// Copyright (C) 2013 Jack.
//
// Author: jack
// Email:  jack.wgm@gmail.com
//

#pragma once

#include <map>
#include <cmath>
#include <iostream>
#include <vector>
#include <deque>
#include <cinttypes>

#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>

#if defined (_WIN32) || defined (WIN32)
// # define WIN32_LEAN_AND_MEAN
// # include <windows.h>
# include <mmsystem.h>
# pragma comment(lib, "Winmm.lib")
#endif // _WIN32

namespace misc {

	using boost::int8_t;
	using boost::uint8_t;
	using boost::int16_t;
	using boost::uint16_t;
	using boost::int32_t;
	using boost::uint32_t;
	using boost::int64_t;
	using boost::uint64_t;


#ifdef WIN32

	static const unsigned __int64 epoch = 116444736000000000L;	/* Jan 1, 1601 */
	typedef union {
		unsigned __int64 ft_scalar;
		FILETIME ft_struct;
	} FLT_FT;

	inline int gettimeofday(struct timeval * tp, struct timezone * tzp)
	{
		FILETIME	file_time;
		SYSTEMTIME	system_time;
		ULARGE_INTEGER ularge;

		GetSystemTime(&system_time);
		SystemTimeToFileTime(&system_time, &file_time);
		ularge.LowPart = file_time.dwLowDateTime;
		ularge.HighPart = file_time.dwHighDateTime;

		tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
		tp->tv_usec = (long)(system_time.wMilliseconds * 1000);

		return 0;
	}

#endif // WIN32

	inline int64_t gettime()
	{
#if defined (WIN32) || defined (_WIN32)
		static int64_t system_start_time = 0;
		if (system_start_time == 0)
		{
			FLT_FT nt_time;
			GetSystemTimeAsFileTime(&(nt_time.ft_struct));
			int64_t tim = (__int64)((nt_time.ft_scalar - epoch) / 10000i64);
			system_start_time = tim - timeGetTime();
#if 0
			struct timeval tv;
			gettimeofday(&tv, NULL);
			int64_t millsec = ((int64_t)tv.tv_sec * 1000000 + tv.tv_usec) / 1000;
			system_start_time = millsec - timeGetTime();
#endif
		}
		return system_start_time + timeGetTime();
#elif defined (__linux__)
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return ((int64_t)tv.tv_sec * 1000000 + tv.tv_usec) / 1000;
#endif
	}

	inline std::string time_to_string(int64_t time)
	{
		std::string ret;
		std::time_t rawtime = time / 1000;
		struct tm * ptm = std::localtime(&rawtime);
		if (!ptm)
			return ret;
		char buffer[1024];
		sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
			ptm->tm_year + 1900, ptm->tm_mon, ptm->tm_mday,
			ptm->tm_hour, ptm->tm_min, ptm->tm_sec, (int)(time % 1000));
		ret = buffer;
		return ret;
	}

	inline std::string to_string(int v, int width)
	{
		std::stringstream s;
		s.flags(std::ios_base::right);
		s.width(width);
		s.fill(' ');
		s << v;
		return s.str();
	}

	inline std::string to_string(float v, int width, int precision = 3)
	{
		char buf[20] = { 0 };
		std::sprintf(buf, "%*.*f", width, precision, v);
		return std::string(buf);
	}

	inline std::string add_suffix(float val, char const* suffix = 0)
	{
		std::string ret;

		const char* prefix[] = { "kB", "MB", "GB", "TB" };
		const int num_prefix = sizeof(prefix) / sizeof(const char*);
		for (int i = 0; i < num_prefix; ++i)
		{
			val /= 1024.f;
			if (std::fabs(val) < 1024.f)
			{
				ret = to_string(val, 4);
				ret += prefix[i];
				if (suffix) ret += suffix;
				return ret;
			}
		}
		ret = to_string(val, 4);
		ret += "PB";
		if (suffix) ret += suffix;
		return ret;
	}

	template <class EndPoint>
	std::string endpoint_to_string(const EndPoint& endp)
	{
		std::ostringstream oss;
		oss << endp.address().to_string() << " : " << endp.port();
		return oss.str();
	}

} // namespace libstream
