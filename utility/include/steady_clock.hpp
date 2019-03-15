//
// Copyright (C) 2013 Jack.
//
// Author: jack
// Email:  jack.wgm at gmail dot com
//

#pragma once

#include <utility>

#include <boost/system/error_code.hpp>
#include <boost/chrono/duration.hpp>
#include <boost/chrono/time_point.hpp>
#include <boost/asio/basic_waitable_timer.hpp>

#include <time.h>

#if defined BOOST_WINDOWS
#	include <mmsystem.h>	// for windows.
#	pragma comment(lib, "winmm.lib")
#endif

#if defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

namespace utility {

class steady_clock
{
public:
	typedef boost::chrono::nanoseconds duration;
	typedef duration::rep rep;
	typedef duration::period period;
	typedef boost::chrono::time_point<steady_clock> time_point;
	BOOST_STATIC_CONSTEXPR bool is_steady = true;

	static BOOST_CHRONO_INLINE time_point now() BOOST_NOEXCEPT
	{
		int64_t time;
#if defined(BOOST_WINDOWS)
		time = ::timeGetTime() * 1000000LL;
#elif defined(__MACH__)
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
		clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);
		time = mts.tv_sec * 1000000000LL + mts.tv_nsec;
#else
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		time = ts.tv_sec * 1000000000LL + ts.tv_nsec;
#endif // BOOST_WINDOWS
		return time_point(duration(static_cast<rep>(time)));
	}

#if !defined BOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING
	static BOOST_CHRONO_INLINE time_point now(boost::system::error_code & ec)
	{
		int64_t time;
#if defined(BOOST_WINDOWS)
		time = ::timeGetTime() * 1000000LL;
#else
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		time = ts.tv_sec * 1000000000LL + ts.tv_nsec;
#endif // BOOST_WINDOWS
		return time_point(duration(static_cast<rep>(time)));
	}
#endif
};

typedef boost::asio::basic_waitable_timer<steady_clock> steady_timer;

}
