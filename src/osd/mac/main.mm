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

int main(int argc, char * argv[])
{
	[NSApplication sharedApplication];
	[NSApp setDelegate: [MAMEAppDelegate new]];

	return NSApplicationMain(argc, (const char**)argv);
}
