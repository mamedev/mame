// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  deviceinfoviewer.h - MacOS X Cocoa debug window handling
//
//============================================================

#import "debugosx.h"

#import "debugwindowhandler.h"

#include "emu.h"

#import <Cocoa/Cocoa.h>


@class MAMEDebugConsole, MAMEDeviceWrapper;

@interface MAMEDeviceInfoViewer : MAMEAuxiliaryDebugWindowHandler
{
	device_t    *device;
}

- (id)initWithDevice:(device_t &)d machine:(running_machine &)m console:(MAMEDebugConsole *)c;

@end
