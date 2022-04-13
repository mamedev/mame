// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  mamewfsindow.mm - our fullscreen window, subclassed from NSWindow
//
//  Mac OSD by R. Belmont
//
//============================================================

#import "mamefswindow.h"

@implementation MAMEFSWindow

-(instancetype)init
{
	NSRect screenRect = [[NSScreen mainScreen] frame];
	self = [super initWithContentRect:screenRect
				styleMask:NSBorderlessWindowMask
				backing:NSBackingStoreBuffered
				defer:YES];

	// Set the window level to be above the menu bar
	[self setLevel:NSMainMenuWindowLevel+1];
	[self setOpaque:YES];
	[self setHidesOnDeactivate:YES];

	return self;
}

-(BOOL)canBecomeKeyWindow
{
	return YES;
}

- (void)keyDown:(NSEvent *)event
{
	[[self windowController] keyDown:event];
}

@end
