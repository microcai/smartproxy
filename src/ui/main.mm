
#include <Cocoa/Cocoa.h>
#include <boost/thread.hpp>
#include <sys/resource.h>
#include <iostream>
#include <vector>

void ulimit_limit()
{
    struct rlimit rlp;
    
    getrlimit(RLIMIT_NOFILE, &rlp);
    
    if (rlp.rlim_cur < 10000)
    {
        rlp.rlim_cur = 10000;
        setrlimit(RLIMIT_NOFILE, &rlp);
    }
    getrlimit(RLIMIT_NOFILE, &rlp);
    std::cout << "rlimit changed to " << rlp.rlim_cur << std::endl;
}

int proxy_main(std::vector<std::string> argv);

int main(int argc, char* argv[])
{
	std::vector<std::string> args;

	for (int i = 0; i < argc; i++)
	{
		args.push_back(argv[i]);
	}

	ulimit_limit();

	std::thread([]()
	{
		[NSApplication sharedApplication];

		NSStatusBar *bar = [NSStatusBar systemStatusBar];

		auto theItem = [bar statusItemWithLength:NSVariableStatusItemLength];
		[theItem retain];

		[theItem setTitle: NSLocalizedString(@"SmartProxy",@"")];
		[theItem setHighlightMode:YES];
	//  [theItem setMenu:theMenu];
		[NSApp run];
	}).detach();

	proxy_main(args);
}

