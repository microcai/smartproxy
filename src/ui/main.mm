
#include <Cocoa/Cocoa.h>
#include <thread>
#include <sys/resource.h>
#include <iostream>

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

extern int proxy_main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    ulimit_limit();
    [NSApplication sharedApplication];

    NSStatusBar *bar = [NSStatusBar systemStatusBar];

    auto theItem = [bar statusItemWithLength:NSVariableStatusItemLength];
    [theItem retain];

    [theItem setTitle: NSLocalizedString(@"SmartProxy",@"")];
    [theItem setHighlightMode:YES];
//    [theItem setMenu:theMenu];
    

    auto network = std::thread([argc, argv](){
      
        argv[0];
        proxy_main(argc, argv);
    });

    [NSApp run];
    network.join();
}

