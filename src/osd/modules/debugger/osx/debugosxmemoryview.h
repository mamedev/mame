// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugosxmemoryview.h - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "debugosx.h"
#import "debugosxdebugview.h"

#include "emu.h"

#import <Cocoa/Cocoa.h>


@interface MAMEMemoryView : MAMEDebugView <MAMEDebugViewSubviewSupport, MAMEDebugViewExpressionSupport>
{
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m;

- (NSSize)maximumFrameSize;

- (NSString *)selectedSubviewName;
- (int)selectedSubviewIndex;
- (void)selectSubviewAtIndex:(int)index;
- (void)selectSubviewForCPU:(device_t *)device;

- (NSString *)expression;
- (void)setExpression:(NSString *)exp;

- (IBAction)showChunkSize:(id)sender;
- (IBAction)showPhysicalAddresses:(id)sender;
- (IBAction)showReverseView:(id)sender;
- (IBAction)showReverseViewToggle:(id)sender;
- (IBAction)changeBytesPerLine:(id)sender;

- (void)insertActionItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index;
- (void)insertSubviewItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index;

@end
