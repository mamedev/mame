// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  disassemblyviewer.h - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "debugosx.h"

#import "debugwindowhandler.h"

#include "emu.h"

#import <Cocoa/Cocoa.h>


@class MAMEDebugConsole, MAMEDisassemblyView;

@interface MAMEDisassemblyViewer : MAMEExpressionAuxiliaryDebugWindowHandler
{
	MAMEDisassemblyView	*dasmView;
	NSPopUpButton		*subviewButton;
}

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c;

- (BOOL)selectSubviewForDevice:(device_t *)device;
- (BOOL)selectSubviewForSpace:(address_space *)space;

- (IBAction)changeSubview:(id)sender;

@end
