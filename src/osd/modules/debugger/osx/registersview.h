// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  registersview.h - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "debugosx.h"

#import "debugview.h"

#include "emu.h"

#import <Cocoa/Cocoa.h>


@interface MAMERegistersView : MAMEDebugView <MAMEDebugViewSubviewSupport>
{
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m;

- (NSSize)maximumFrameSize;

- (NSString *)selectedSubviewName;
- (int)selectedSubviewIndex;
- (void)selectSubviewAtIndex:(int)index;
- (void)selectSubviewForDevice:(device_t *)device;

@end
