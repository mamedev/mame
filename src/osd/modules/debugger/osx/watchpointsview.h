// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  watchpointsview.h - MacOS X Cocoa debug window handling
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
