
#pragma once

#include <boost/asio/ip/address.hpp>

boost::asio::ip::address_v4 getifaddrv4(std::string ifname);

boost::asio::ip::address_v6 getifaddrv6(std::string ifname);
