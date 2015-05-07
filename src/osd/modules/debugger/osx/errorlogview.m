// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  errorlogview.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "errorlogview.h"

#include "debug/debugvw.h"


@implementation MAMEErrorLogView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_LOG machine:m wholeLineScroll:NO]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end
