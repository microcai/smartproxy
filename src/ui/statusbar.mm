
#include "statusbar.h"

void show_statusbar()
{
    NSStatusBar *bar = [NSStatusBar systemStatusBar];

    theItem = [bar statusItemWithLength:NSVariableStatusItemLength];
    [theItem retain];

    [theItem setTitle: NSLocalizedString(@"SmartProxy",@"")];
    [theItem setHighlightMode:YES];
//    [theItem setMenu:theMenu];
}
