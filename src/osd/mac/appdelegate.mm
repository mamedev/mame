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

extern int mac_run_emulator();

@interface MAMEThread : NSThread
@end

@implementation MAMEThread
- (void)main
{
	mac_run_emulator();
}
@end

@interface MAMEAppDelegate ()
@end

@implementation MAMEAppDelegate
MAMEThread *appThread;

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	// run MAME on a thread so event dispatching happens normally
	appThread = [[MAMEThread alloc] init];
	[appThread start];
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
