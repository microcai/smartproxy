
#include "getifaddr.hpp"

#ifndef _WIN32

#include <ifaddrs.h>

boost::asio::ip::address_v4 getifaddrv4(std::string ifname)
{
	std::shared_ptr<ifaddrs> auto_free;

	{
		struct ifaddrs *ifaddr;

		if (getifaddrs(&ifaddr) == -1)
		{
			perror("getifaddrs");
			exit(EXIT_FAILURE);
		}

		auto_free.reset(ifaddr, freeifaddrs);
	}


	for (auto ifaddr_iterator = auto_free.get(); ifaddr_iterator != NULL; ifaddr_iterator = ifaddr_iterator->ifa_next)
	{
		if (ifaddr_iterator->ifa_addr == NULL)
			continue;

		if ( ifaddr_iterator->ifa_addr->sa_family==AF_INET )
		{
			if (ifname.empty() || ifname == ifaddr_iterator->ifa_name)
			{
				sockaddr_in * soaddr4 = (sockaddr_in * )ifaddr_iterator->ifa_addr;

				boost::asio::ip::address_v4::bytes_type rawbytes_of_addr;
				memcpy(rawbytes_of_addr.data(), & soaddr4->sin_addr, 4);

				boost::asio::ip::address_v4 v4addr(rawbytes_of_addr);

				if (rawbytes_of_addr[0] == 0xfd || rawbytes_of_addr[0] == 0xfc)
					continue;

				return v4addr;
			}
		}
	}
	return boost::asio::ip::make_address_v4("0.0.0.0");
}

boost::asio::ip::address_v6 getifaddrv6(std::string ifname)
{
	std::shared_ptr<ifaddrs> auto_free;

	{
		struct ifaddrs *ifaddr;

		if (getifaddrs(&ifaddr) == -1)
		{
			perror("getifaddrs");
			exit(EXIT_FAILURE);
		}

		auto_free.reset(ifaddr, freeifaddrs);
	}

	struct addr_info {
		boost::asio::ip::address_v6 addr_v6;
		int valid_lft;
	};

	std::vector<addr_info> addr_infos;

	for (auto ifaddr_iterator = auto_free.get(); ifaddr_iterator != NULL; ifaddr_iterator = ifaddr_iterator->ifa_next)
	{
		if (ifaddr_iterator->ifa_addr == NULL)
			continue;

		if ( ifaddr_iterator->ifa_addr->sa_family==AF_INET6 )
		{
			if (ifname.empty() || ifname == ifaddr_iterator->ifa_name)
			{
				sockaddr_in6 * soaddr6 = (sockaddr_in6 * )ifaddr_iterator->ifa_addr;

				if (soaddr6->sin6_scope_id == 0)
				{
					boost::asio::ip::address_v6::bytes_type rawbytes_of_addr;
					memcpy(rawbytes_of_addr.data(), soaddr6->sin6_addr.s6_addr, 16);

					boost::asio::ip::address_v6 v6addr(rawbytes_of_addr, soaddr6->sin6_scope_id);

					if (rawbytes_of_addr[0] == 0xfd || rawbytes_of_addr[0] == 0xfc)
						continue;

					printf("\tInterface : <%s>\n",ifaddr_iterator->ifa_name );
					printf("\t  Address : <%s>\n", v6addr.to_string().c_str());

					return v6addr;
				}
			}
		}
	}
	return boost::asio::ip::make_address_v6("::");
}
#else
boost::asio::ip::address_v4 getifaddrv4(std::string ifname)
{
	return boost::asio::ip::make_address_v4("0.0.0.0");
}

boost::asio::ip::address_v6 getifaddrv6(std::string ifname)
{
	return boost::asio::ip::make_address_v6("::");
}
#endif