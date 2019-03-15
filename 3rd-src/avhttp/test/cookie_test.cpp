#include <iostream>
#include <boost/assert.hpp>
#include "avhttp.hpp"


int main(int argc, char** argv)
{
	avhttp::cookies cookie;
	cookie("sf_mirror_attempt=avplayer:optimate|softlayer-ams:/avplayer/exe/release-2013-03-13.7z; expires=Tue, 3-Dec-2013 14:52:55 GMT; Path=/");
	BOOST_ASSERT(cookie["sf_mirror_attempt"] == std::string("avplayer:optimate|softlayer-ams:/avplayer/exe/release-2013-03-13.7z"));
	return 0;
}
