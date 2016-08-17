// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  consoleview.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "consoleview.h"

#include "debug/debugvw.h"


@implementation MAMEConsoleView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_CONSOLE machine:m wholeLineScroll:NO]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end
