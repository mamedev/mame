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

-(instancetype)initWithScreen:(NSScreen *)screen
{
	if (screen == nil)
	{
		screen = [NSScreen mainScreen];
	}

	// the frame rect is in screen coordinates, which puts the window on the right display
	NSRect screenRect = [screen frame];
	self = [super initWithContentRect:screenRect
				styleMask:NSWindowStyleMaskBorderless
				backing:NSBackingStoreBuffered
				defer:YES];

	// Set the window level to be above the menu bar
	[self setLevel:NSMainMenuWindowLevel+1];
	[self setOpaque:YES];
	[self setHidesOnDeactivate:YES];
	[self setReleasedWhenClosed:NO];
	[self setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces | NSWindowCollectionBehaviorFullScreenAuxiliary];

	return self;
}

-(instancetype)init
{
	return [self initWithScreen:nil];
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
