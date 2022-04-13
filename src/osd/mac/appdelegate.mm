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
}

- (void)applicationDidResignActive:(NSNotification *)notification
{
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSNotification *)notification
{
	return NSTerminateNow;
}
@end
