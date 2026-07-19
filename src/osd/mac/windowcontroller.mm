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

#import <Carbon/Carbon.h>
#import <IOKit/hidsystem/IOLLEvent.h>
#import <QuartzCore/QuartzCore.h>

// implemented in window.cpp
extern "C" int MacRequestMachineExit();

// implemented in modules/input/input_mac.cpp
extern "C" void MacKeyboardEvent(int vk, int down);
extern "C" void MacMouseButtonEvent(int button, int down);
extern "C" void MacMouseMotionEvent(float dx, float dy);
extern "C" void MacMouseWheelEvent(float dv, float dh);
extern "C" void MacCharEvent(unsigned int ch);
extern "C" void MacPointerUpdate(void *nswindow, int x, int y, unsigned int buttons, unsigned int pressed, unsigned int released, int clicks);
extern "C" void MacPointerLeave(void *nswindow, int x, int y, unsigned int released, int clicks);
extern "C" void MacMouseWheelUI(void *nswindow, int x, int y, int delta);

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

- (void) goFullscreen:(uint32_t)displayID
{
	if(_fullscreenWindow)
	{
		return;
	}

	// put the fullscreen window on the requested display if possible
	NSScreen *screen = nil;
	for (NSScreen *candidate in [NSScreen screens])
	{
		NSNumber *number = [[candidate deviceDescription] objectForKey:@"NSScreenNumber"];
		if ([number unsignedIntValue] == displayID)
		{
			screen = candidate;
			break;
		}
	}
	if (screen == nil)
	{
		screen = [_standardWindow screen];
	}
	_fullscreenWindow = [[MAMEFSWindow alloc] initWithScreen:screen];

	// migrate the content view (and the GL context or Metal layer living on
	// it) to the fullscreen window; setContentView resizes it to fit
	NSView *content = [_standardWindow contentView];
	[_standardWindow orderOut:self];
	[_fullscreenWindow setContentView:content];

	[self setWindow:_fullscreenWindow];
	[_fullscreenWindow makeKeyAndOrderFront:self];
}

- (void) goWindow
{
	if(_fullscreenWindow == nil)
	{
		return;
	}

	// migrate the content view back to the standard window
	NSView *content = [_fullscreenWindow contentView];
	[_fullscreenWindow orderOut:self];
	[_standardWindow setContentView:content];

	[self setWindow:_standardWindow];
	[_standardWindow makeKeyAndOrderFront:self];

	[_fullscreenWindow release];
	_fullscreenWindow = nil;
}


- (void) keyDown:(NSEvent *)event
{
//  unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];
	[super keyDown:event];
}

- (BOOL) windowShouldClose:(id)sender
{
	MacRequestMachineExit();
	return NO;
}

- (NSWindow *) getWindow
{
	if(_fullscreenWindow == nil)
	{
		return _standardWindow;
	}

	return _fullscreenWindow;
}

- (NSWindow *) getStandardWindow
{
	return _standardWindow;
}
@end

//============================================================
//  event pump and input event decoding
//============================================================

static uint32_t s_pointer_buttons = 0;
static bool s_pointer_inside = false;
static int s_pointer_x = 0;
static int s_pointer_y = 0;
static NSWindow *s_pointer_window = nil;    // which native window the pointer is in

// get the event location in content view backing pixels, top-left origin;
// returns whether the location is inside the view
static bool EventLocation(NSEvent *event, int *outx, int *outy)
{
	NSWindow *window = [event window];
	if (window == nil)
	{
		*outx = *outy = 0;
		return false;
	}

	NSView *view = [window contentView];
	NSPoint point = [view convertPoint:[event locationInWindow] fromView:nil];
	NSRect bounds = [view bounds];
	CGFloat scale = [window backingScaleFactor];

	*outx = (int)(point.x * scale);
	*outy = (int)((bounds.size.height - point.y) * scale);   // flip to top-left origin

	return (point.x >= 0) && (point.y >= 0) && (point.x < bounds.size.width) && (point.y < bounds.size.height);
}

static void HandleMouseButton(NSEvent *event, int button, bool down)
{
	int x, y;
	const bool inside = EventLocation(event, &x, &y);
	NSWindow *window = [event window];

	// Press the button only when the click lands in the content view, but
	// always deliver releases to prevent stuck buttons.
	if (inside || !down)
	{
		MacMouseButtonEvent(button, down ? 1 : 0);
	}

	if (inside && s_pointer_inside && (window != s_pointer_window))
	{
		s_pointer_inside = false;
		MacPointerLeave(s_pointer_window, s_pointer_x, s_pointer_y, 0, 0);
	}

	const uint32_t mask = 1U << button;
	uint32_t pressed = 0, released = 0;

	if (down)
	{
		pressed = mask & ~s_pointer_buttons;
		s_pointer_buttons |= mask;
	}
	else
	{
		released = mask & s_pointer_buttons;
		s_pointer_buttons &= ~mask;
	}

	if (inside || s_pointer_inside)
	{
		if (inside)
		{
			s_pointer_window = window;
		}
		s_pointer_x = x;
		s_pointer_y = y;
		s_pointer_inside = true;
		MacPointerUpdate(s_pointer_window, x, y, s_pointer_buttons, pressed, released, (int)[event clickCount]);

		// once every button is released outside the view, the pointer is gone
		if (!inside && (s_pointer_buttons == 0))
		{
			s_pointer_inside = false;
			MacPointerLeave(s_pointer_window, x, y, 0, 0);
		}
	}
}

static void HandleMouseMotion(NSEvent *event)
{
	int x, y;
	const bool inside = EventLocation(event, &x, &y);
	NSWindow *window = [event window];

	// dragging keeps the pointer bound to the window it started in
	const bool dragging = s_pointer_inside && (s_pointer_buttons != 0);

	// leaving the previous window (or crossing into a different one)?
	if (s_pointer_inside && !dragging && (!inside || (window != s_pointer_window)))
	{
		s_pointer_inside = false;
		MacPointerLeave(s_pointer_window, s_pointer_x, s_pointer_y, 0, 0);
	}

	if (inside || dragging)
	{
		// Only feed relative motion to the input system while the pointer
		// is over a MAME window or dragging out of one.
		MacMouseMotionEvent([event deltaX], [event deltaY]);

		// update the UI pointer, following drags outside the view
		if (!dragging)
		{
			s_pointer_window = window;
		}
		s_pointer_x = x;
		s_pointer_y = y;
		s_pointer_inside = true;
		MacPointerUpdate(s_pointer_window, x, y, s_pointer_buttons, 0, 0, 0);
	}
}

// Modifier keys arrive as flag changes, with the device-dependent
// flags telling the left and right keys apart
static void HandleFlagsChanged(NSEvent *event)
{
	const NSUInteger flags = [event modifierFlags];
	int down;

	switch ([event keyCode])
	{
	case kVK_Shift:         down = (flags & NX_DEVICELSHIFTKEYMASK) != 0; break;
	case kVK_RightShift:    down = (flags & NX_DEVICERSHIFTKEYMASK) != 0; break;
	case kVK_Control:       down = (flags & NX_DEVICELCTLKEYMASK) != 0; break;
	case kVK_RightControl:  down = (flags & NX_DEVICERCTLKEYMASK) != 0; break;
	case kVK_Option:        down = (flags & NX_DEVICELALTKEYMASK) != 0; break;
	case kVK_RightOption:   down = (flags & NX_DEVICERALTKEYMASK) != 0; break;
	case kVK_Command:       down = (flags & NX_DEVICELCMDKEYMASK) != 0; break;
	case kVK_RightCommand:  down = (flags & NX_DEVICERCMDKEYMASK) != 0; break;
	case kVK_CapsLock:      down = (flags & NSEventModifierFlagCapsLock) != 0; break;
	default:                return;
	}

	MacKeyboardEvent([event keyCode], down);
}

void MacPollInputs()
{
	NSEvent *event;

	while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
			untilDate:[NSDate distantPast] // do not wait for event
			inMode:NSDefaultRunLoopMode
			dequeue:YES]) != nil)
	{
		bool dispatch = true;

		switch ([event type])
		{
		case NSEventTypeKeyDown:
			if (![event isARepeat])
			{
				MacKeyboardEvent([event keyCode], 1);
			}
			// swallow non-Command keys so they don't beep, but let Command
			// shortcuts through to the menus
			if (!([event modifierFlags] & NSEventModifierFlagCommand))
			{
				// forward characters for UI text entry
				NSString *chars = [event characters];
				for (NSUInteger i = 0; i < [chars length]; i++)
				{
					unichar c = [chars characterAtIndex:i];
					if (c == 0x7f)  // Delete key produces DEL; the UI wants backspace
					{
						c = 0x08;
					}
					if (c < 0xf700) // skip the function key range
					{
						MacCharEvent(c);
					}
				}
				dispatch = false;
			}
			break;

		case NSEventTypeKeyUp:
			MacKeyboardEvent([event keyCode], 0);
			dispatch = ([event modifierFlags] & NSEventModifierFlagCommand) != 0;
			break;

		case NSEventTypeFlagsChanged:
			HandleFlagsChanged(event);
			break;

		case NSEventTypeLeftMouseDown:
			HandleMouseButton(event, 0, true);
			break;

		case NSEventTypeLeftMouseUp:
			HandleMouseButton(event, 0, false);
			break;

		case NSEventTypeRightMouseDown:
			HandleMouseButton(event, 1, true);
			break;

		case NSEventTypeRightMouseUp:
			HandleMouseButton(event, 1, false);
			break;

		case NSEventTypeOtherMouseDown:
			HandleMouseButton(event, (int)[event buttonNumber], true);
			break;

		case NSEventTypeOtherMouseUp:
			HandleMouseButton(event, (int)[event buttonNumber], false);
			break;

		case NSEventTypeMouseMoved:
		case NSEventTypeLeftMouseDragged:
		case NSEventTypeRightMouseDragged:
		case NSEventTypeOtherMouseDragged:
			HandleMouseMotion(event);
			break;

		case NSEventTypeScrollWheel:
			{
				int x, y;
				if (EventLocation(event, &x, &y))
				{
					MacMouseWheelEvent([event deltaY], [event deltaX]);
					MacMouseWheelUI([event window], x, y, (int)([event deltaY] * 120));
				}
			}
			break;

		default:
			break;
		}

		if (dispatch)
		{
			[NSApp sendEvent:event];
		}
	}

	[NSApp updateWindows];
}

void *GetOSWindow(void *wincontroller)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;

	return [wc getWindow];
}

void *GetOSStandardWindow(void *wincontroller)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;

	return [wc getStandardWindow];
}

void MacOrderWindowFront(void *wincontroller)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;

	[[wc getWindow] orderFront:nil];
}

// display a BGRX software-rendered frame on the window's backing layer
void MacBlitVideoFrame(void *wincontroller, const void *bits, int width, int height, int pitch)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;
	NSWindow *window = [wc getWindow];
	NSView *view = [window contentView];

	static CGColorSpaceRef s_colorspace = nullptr;
	if (s_colorspace == nullptr)
	{
		s_colorspace = CGColorSpaceCreateDeviceRGB();
	}

	// the layer may hold onto the image after we return, so give it a copy
	const size_t stride = size_t(pitch) * 4;
	CFDataRef data = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)bits, stride * height);
	CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
	CGImageRef image = CGImageCreate(
			width, height,
			8, 32, stride,
			s_colorspace,
			CGBitmapInfo(kCGBitmapByteOrder32Little) | CGBitmapInfo(kCGImageAlphaNoneSkipFirst),
			provider, nullptr, false, kCGRenderingIntentDefault);

	if (![view wantsLayer])
	{
		[view setWantsLayer:YES];
	}

	// suppress the implicit contents-change animation
	[CATransaction begin];
	[CATransaction setDisableActions:YES];
	CALayer *layer = [view layer];
	[layer setContentsGravity:kCAGravityResize];
	[layer setContentsScale:[window backingScaleFactor]];
	[layer setContents:(id)image];
	[CATransaction commit];

	CGImageRelease(image);
	CGDataProviderRelease(provider);
	CFRelease(data);
}

// find the NSScreen for a display ID
static NSScreen *ScreenForDisplayID(uint32_t displayID)
{
	for (NSScreen *screen in [NSScreen screens])
	{
		NSNumber *number = [[screen deviceDescription] objectForKey:@"NSScreenNumber"];
		if ([number unsignedIntValue] == displayID)
		{
			return screen;
		}
	}
	return nil;
}

// Cocoa global coordinates are bottom-left origin, y-up; MAME and CoreGraphics
// use top-left origin, y-down.  the spaces flip about the primary display.
static CGFloat PrimaryScreenHeight()
{
	return NSMaxY([[[NSScreen screens] firstObject] frame]);
}

uint32_t MacDisplayIDForWindow(void *wincontroller)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;
	NSScreen *screen = [[wc getWindow] screen];

	if (screen == nil)
	{
		return 0;
	}

	return [[[screen deviceDescription] objectForKey:@"NSScreenNumber"] unsignedIntValue];
}

// get a display's work area (sans menu bar and Dock) in top-left-origin global coordinates
bool MacGetDisplayWorkArea(uint32_t displayID, int *x, int *y, int *width, int *height)
{
	NSScreen *screen = ScreenForDisplayID(displayID);

	if (screen == nil)
	{
		return false;
	}

	NSRect visible = [screen visibleFrame];
	*x = (int)visible.origin.x;
	*y = (int)(PrimaryScreenHeight() - NSMaxY(visible));
	*width = (int)visible.size.width;
	*height = (int)visible.size.height;

	return true;
}

void MacSetWindowAspectRatio(void *wincontroller, int width, int height)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;
	NSWindow *window = [wc getStandardWindow];

	if ((width > 0) && (height > 0))
	{
		// AppKit constrains user resizing natively once this is set
		[window setContentAspectRatio:NSMakeSize(width, height)];
	}
	else
	{
		// aspectRatio and resizeIncrements share the same constraint slot,
		// so unit increments clear the ratio; a zero aspect ratio instead
		// poisons AppKit's resize math (NaN frame -> trap)
		[window setResizeIncrements:NSMakeSize(1.0, 1.0)];
	}
}

void MacSetWindowMinSize(void *wincontroller, int width, int height)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;
	NSWindow *window = [wc getStandardWindow];

	[window setContentMinSize:NSMakeSize(width, height)];
}

bool MacAppHasFocus()
{
	return [NSApp isActive];
}

int MacPointerInWindow()
{
	return s_pointer_inside ? 1 : 0;
}

void MacShowPointer(bool show)
{
	// NSCursor hide/unhide calls must balance
	static bool s_cursor_hidden = false;

	if (!show && !s_cursor_hidden)
	{
		[NSCursor hide];
		s_cursor_hidden = true;
	}
	else if (show && s_cursor_hidden)
	{
		[NSCursor unhide];
		s_cursor_hidden = false;
	}
}

void MacCapturePointer(void *wincontroller, bool capture)
{
	if (capture)
	{
		MAMEWindowController *wc = (MAMEWindowController *)wincontroller;
		NSWindow *window = [wc getWindow];
		NSView *view = [window contentView];

		// warp the cursor to the middle of the content view so it's inside
		// the window when it reappears
		NSRect bounds = [view bounds];
		NSRect centerRect = NSMakeRect(NSMidX(bounds), NSMidY(bounds), 0, 0);
		NSRect screenRect = [window convertRectToScreen:[view convertRect:centerRect toView:nil]];

		// CG global coordinates are top-left origin on the primary display
		NSScreen *primary = [[NSScreen screens] firstObject];
		CGPoint warp = CGPointMake(screenRect.origin.x, NSMaxY([primary frame]) - screenRect.origin.y);
		CGWarpMouseCursorPosition(warp);

		// decouple the cursor from mouse movement; deltas keep flowing
		CGAssociateMouseAndMouseCursorPosition(false);
	}
	else
	{
		CGAssociateMouseAndMouseCursorPosition(true);
	}
}

void MacSetWindowContentSize(void *wincontroller, int width, int height)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;
	NSWindow *window = [wc getStandardWindow];

	// resize keeping the top-left corner anchored, like other platforms
	NSRect content = [window contentRectForFrameRect:[window frame]];
	content.origin.y += content.size.height - height;
	content.size.width = width;
	content.size.height = height;
	[window setFrame:[window frameRectForContentRect:content] display:YES];
}

void MacSetFullscreen(void *wincontroller, bool fullscreen, uint32_t displayID)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;

	if (fullscreen)
	{
		[wc goFullscreen:displayID];
	}
	else
	{
		[wc goWindow];
	}
}

void MacDestroyWindow(void *wincontroller)
{
	MAMEWindowController *wc = (MAMEWindowController *)wincontroller;

	// don't leave the pointer tracking aimed at a dead window
	if ((s_pointer_window == [wc getWindow]) || (s_pointer_window == [wc getStandardWindow]))
	{
		s_pointer_window = nil;
		s_pointer_inside = false;
	}

	// drop back to windowed mode first so the fullscreen window is released
	[wc goWindow];

	NSWindow *window = [wc getWindow];
	[window setDelegate:nil];
	[window orderOut:nil];
	[window release];       // alloc'd in CreateMAMEWindow with releasedWhenClosed:NO
	[wc release];
}

void MacWindowGetSize(void *wincontroller, int *width, int *height)
{
	NSWindow *window = (NSWindow *)GetOSWindow(wincontroller);
	NSRect bounds = [[window contentView] bounds];

	*width = (int)bounds.size.width;
	*height = (int)bounds.size.height;
}

void MacWindowGetSizePixels(void *wincontroller, int *width, int *height)
{
	NSWindow *window = (NSWindow *)GetOSWindow(wincontroller);
	NSView *view = [window contentView];
	NSRect backing = [view convertRectToBacking:[view bounds]];

	*width = (int)backing.size.width;
	*height = (int)backing.size.height;
}

void *CreateMAMEWindow(const char *title, int x, int y, int w, int h, bool isFullscreen, uint32_t displayID)
{
	// incoming coordinates are top-left-origin global space; flip to Cocoa's
	// bottom-left-origin space so windows land on the intended display
	NSRect bounds = NSMakeRect(x, PrimaryScreenHeight() - y - h, w, h);
	NSUInteger style = NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask;
	NSWindow *window = [NSWindow alloc];
	MAMEWindowController *controller = [MAMEWindowController alloc];

	[window initWithContentRect:bounds
		styleMask:style
		backing:NSBackingStoreBuffered
		defer:NO];
	// initWithContentRect relocates windows placed on secondary displays;
	// programmatic setFrame is not constrained, so force the intended frame
	[window setFrame:[window frameRectForContentRect:bounds] display:NO];
	[window setReleasedWhenClosed:NO];
	[controller initWithWindow:window];
	[controller setShouldCascadeWindows:NO];
	[window setDelegate:(id<NSWindowDelegate>)controller];
	NSString *nstitle = [[NSString alloc] initWithUTF8String:title];
	[window setTitle:nstitle];
	[nstitle release];
	if (isFullscreen)
	{
		[controller goFullscreen:displayID];
	}
	else
	{
		[window makeKeyAndOrderFront:nil];
	}

	return (void *)controller;
}
