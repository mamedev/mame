// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  exceptionpointsview.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "exceptionpointsview.h"

#include "debug/debugvw.h"


@implementation MAMEExceptionpointsView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_EXCEPTION_POINTS machine:m wholeLineScroll:YES]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end
