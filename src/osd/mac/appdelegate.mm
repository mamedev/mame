// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  appdelegate.mm - Cocoa app delegate
//
//  Mac OSD by R. Belmont
//
//============================================================

#import "appdelegate.h"

// implemented in window.cpp
extern "C" int MacRequestMachineExit();
extern "C" void MacOrderWindowsFront();

@interface MAMEAppDelegate ()
@end

@implementation MAMEAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
	MacOrderWindowsFront();
}

- (void)applicationDidResignActive:(NSNotification *)notification
{
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSNotification *)notification
{
	if (MacRequestMachineExit())
	{
		return NSTerminateCancel;
	}

	return NSTerminateNow;
}
@end
