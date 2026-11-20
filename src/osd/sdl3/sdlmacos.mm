// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
//============================================================
//
//  sdlmacos.mm - macOS application activation helpers
//
//============================================================

#include "sdlmacos.h"

#import <Cocoa/Cocoa.h>


bool sdl_macos_app_active()
{
	return [NSApp isActive];
}


bool sdl_macos_app_frontmost()
{
	return [[[NSWorkspace sharedWorkspace] frontmostApplication] isEqual:[NSRunningApplication currentApplication]];
}


void sdl_macos_activate_dock()
{
	NSRunningApplication *const dock = [[NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.apple.dock"] firstObject];
	[dock activateWithOptions:0];
}


void sdl_macos_activate_self()
{
	[[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateAllWindows];
}
