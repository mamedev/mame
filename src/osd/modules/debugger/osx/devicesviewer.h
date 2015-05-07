// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  devicesviewer.h - MacOS X Cocoa debug window handling
//
//============================================================

#import "debugosx.h"

#import "debugwindowhandler.h"

#include "emu.h"

#import <Cocoa/Cocoa.h>


@class MAMEDebugConsole, MAMEDeviceWrapper;

@interface MAMEDevicesViewer : MAMEAuxiliaryDebugWindowHandler <NSOutlineViewDataSource>
{
	MAMEDeviceWrapper   *root;
	NSOutlineView       *devicesView;
}

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c;

- (IBAction)showDeviceDetail:(id)sender;

@end
