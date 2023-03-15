// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  registerpointsview.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "registerpointsview.h"

#include "debug/debugvw.h"


@implementation MAMERegisterpointsView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_REGISTER_POINTS machine:m wholeLineScroll:YES]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end
