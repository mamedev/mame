// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  consoleview.m - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "consoleview.h"

#include "debug/debugvw.h"


@implementation MAMEConsoleView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_CONSOLE machine:m]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end
