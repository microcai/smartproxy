
#pragma once

#include <variant>
#include <vector>
#include <string>

struct upstream_direct_connect_via_binded_address
{
	std::string bind_addr; // the address that the outgoing socket bindto.
};

struct upstream_socks5
{
	std::string sock_host;
	std::string sock_port;
};

typedef std::variant<upstream_direct_connect_via_binded_address, upstream_socks5>  upstream_desc;

struct proxyconfig
{
	std::vector<upstream_desc> upstreams;
	int listenport;
};
