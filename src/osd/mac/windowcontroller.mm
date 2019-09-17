// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  windowcontroller.mm - our window/fullscreen manager
//
//  Mac OSD by R. Belmont
//
//============================================================

#import "windowcontroller.h"
#import "mamefswindow.h"

@interface MAMEWindowController ()
{
	MAMEFSWindow *_fullscreenWindow;
	NSWindow* _standardWindow;
}
@end

@implementation MAMEWindowController

- (instancetype)initWithWindow:(NSWindow *)window
{
	self = [super initWithWindow:window];

	if (self)
	{
		_fullscreenWindow = nil;
		_standardWindow = window;
	}

	return self;
}

- (void) goFullscreen
{
	if(_fullscreenWindow)
	{
		return;
	}

	_fullscreenWindow = [[MAMEFSWindow alloc] init];

	NSRect viewRect = [_fullscreenWindow frame];
	[self.window.contentView setFrameSize: viewRect.size];
	[_fullscreenWindow setContentView:self.window.contentView];

	_standardWindow = [self window];
	[_standardWindow orderOut:self];

	[self setWindow:_fullscreenWindow];
	[_fullscreenWindow makeKeyAndOrderFront:self];
}

- (void) goWindow
{
	if(_fullscreenWindow == nil)
	{
		return;
	}

	NSRect viewRect = [_standardWindow frame];
	[self.window.contentView setFrame:viewRect];

	[self setWindow:_standardWindow];
	[[self window] setContentView:_fullscreenWindow.contentView];
	[[self window] makeKeyAndOrderFront:self];

	_fullscreenWindow = nil;
}


- (void) keyDown:(NSEvent *)event
{
//  unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];
	[super keyDown:event];
}

- (NSWindow *) getWindow
{
	if(_fullscreenWindow == nil)
	{
		return _standardWindow;
	}

	return _fullscreenWindow;
}
@end

void MacPollInputs()
{
	NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
			untilDate:[NSDate distantPast] // do not wait for event
			inMode:NSDefaultRunLoopMode
			dequeue:YES];

	if (event)
	{
		[NSApp sendEvent:event];
		[NSApp updateWindows];
	}
}

void *GetOSWindow(void *wincontroller)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;

	return [wc getWindow];
}

void *CreateMAMEWindow(char *title, int x, int y, int w, int h, bool isFullscreen)
{
	NSRect bounds = NSMakeRect(x, y, w, h);
	NSUInteger style = NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask;
	NSWindow *window = [NSWindow alloc];
	MAMEWindowController *controller = [MAMEWindowController alloc];

	[window initWithContentRect:bounds
		styleMask:style
		backing:NSBackingStoreBuffered
		defer:NO];
	[controller initWithWindow:window];
	NSString *nstitle = [[NSString alloc] initWithUTF8String:title];
	[window setTitle:nstitle];
	[nstitle release];
	if (isFullscreen)
	{
		[controller goFullscreen];
	}
	else
	{
		[window makeKeyAndOrderFront:nil];
	}

	return (void *)controller;
}
