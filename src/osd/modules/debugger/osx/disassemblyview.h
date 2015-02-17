// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  disassemblyview.h - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "debugosx.h"

#import "debugview.h"

#include "emu.h"
#include "debug/dvdisasm.h"

#import <Cocoa/Cocoa.h>


@interface MAMEDisassemblyView : MAMEDebugView <MAMEDebugViewSubviewSupport, MAMEDebugViewExpressionSupport>
{
	BOOL	useConsole;
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m useConsole:(BOOL)uc;

- (NSSize)maximumFrameSize;

- (NSString *)selectedSubviewName;
- (int)selectedSubviewIndex;
- (void)selectSubviewAtIndex:(int)index;
- (BOOL)selectSubviewForDevice:(device_t *)device;
- (BOOL)selectSubviewForSpace:(address_space *)space;

- (NSString *)expression;
- (void)setExpression:(NSString *)exp;

- (debug_view_disasm_source const *)source;

- (IBAction)debugToggleBreakpoint:(id)sender;
- (IBAction)debugToggleBreakpointEnable:(id)sender;
- (IBAction)debugRunToCursor:(id)sender;

- (IBAction)showRightColumn:(id)sender;

- (void)insertActionItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index;
- (void)insertSubviewItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index;

@end
