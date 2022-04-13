// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  main.mm - main file for MAME on Mac
//
//  Mac OSD by R. Belmont
//
//============================================================

#import "appdelegate.h"

extern int mac_run_emulator(int argc, char *argv[]);

int main(int argc, char * argv[])
{
	[NSApplication sharedApplication];
	[NSApp setDelegate: [MAMEAppDelegate new]];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	[NSApp activateIgnoringOtherApps:YES];
	[NSApp finishLaunching];
	[[NSNotificationCenter defaultCenter]
		postNotificationName:NSApplicationWillFinishLaunchingNotification
		object:NSApp];
	[[NSNotificationCenter defaultCenter]
		postNotificationName:NSApplicationDidFinishLaunchingNotification
		object:NSApp];
	id quitMenuItem = [NSMenuItem new];
	[quitMenuItem
		initWithTitle:@"Quit"
		action:@selector(terminate:)
		keyEquivalent:@"q"];
	id appMenu = [NSMenu new];
	[appMenu addItem:quitMenuItem];
	id appMenuItem = [NSMenuItem new];
	[appMenuItem setSubmenu:appMenu];
	id menubar = [[NSMenu new] autorelease];
	[menubar addItem:appMenuItem];
	[NSApp setMainMenu:menubar];

	return mac_run_emulator(argc, argv);
}
