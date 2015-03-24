// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  watchpointsview.h - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "debugosx.h"

#import "debugview.h"

#include "emu.h"

#import <Cocoa/Cocoa.h>


@interface MAMEWatchpointsView : MAMEDebugView
{
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m;

@end
