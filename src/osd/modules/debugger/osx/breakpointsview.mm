// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  breakpointsview.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "breakpointsview.h"

#include "debug/debugvw.h"


@implementation MAMEBreakpointsView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_BREAK_POINTS machine:m wholeLineScroll:YES]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end
