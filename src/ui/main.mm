
#include <Cocoa/Cocoa.h>

extern int proxy_main(int argc, char* argv[]);

void main(int argc, char* argv[])
{
    NSStatusBar *bar = [NSStatusBar systemStatusBar];

    auto theItem = [bar statusItemWithLength:NSVariableStatusItemLength];
    [theItem retain];

    [theItem setTitle: NSLocalizedString(@"SmartProxy",@"")];
    [theItem setHighlightMode:YES];
//    [theItem setMenu:theMenu];

	proxy_main(argc, argv);
}

