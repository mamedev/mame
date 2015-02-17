// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  watchpointsview.m - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "watchpointsview.h"

#include "debug/debugvw.h"


@implementation MAMEWatchpointsView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_WATCH_POINTS machine:m]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end
