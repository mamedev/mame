// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  oglview.mm - our OpenGL view
//
//  Mac OSD by R. Belmont
//
//============================================================

#import "oglview.h"

#define SUPPORT_RETINA_RESOLUTION 1

@interface MAMEGLView ()
{
}
@end

@implementation MAMEGLView

- (void) awakeFromNib
{
	NSOpenGLPixelFormatAttribute attrs[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 24,
		0
	};

	NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];

	[self setPixelFormat:pf];
	[self setOpenGLContext:context];

#if SUPPORT_RETINA_RESOLUTION
	// Opt-In to Retina resolution
	[self setWantsBestResolutionOpenGLSurface:YES];
#endif // SUPPORT_RETINA_RESOLUTION
}

- (void) prepareOpenGL
{
	[super prepareOpenGL];

	[self initGL];
}

- (void) windowWillClose:(NSNotification*)notification
{
}

- (void) initGL
{
	[[self openGLContext] makeCurrentContext];
	GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (void)reshape
{
	[super reshape];

	CGLLockContext([[self openGLContext] CGLContextObj]);

//  NSRect viewRectPoints = [self bounds];

//#if SUPPORT_RETINA_RESOLUTION
//    NSRect viewRectPixels = [self convertRectToBacking:viewRectPoints];
//#else
//    NSRect viewRectPixels = viewRectPoints;
//#endif

	CGLUnlockContext([[self openGLContext] CGLContextObj]);
}


- (void)renewGState
{
	[[self window] disableScreenUpdatesUntilFlush];
	[super renewGState];
}

- (void) drawRect: (NSRect) theRect
{
}

- (void) startDrawView
{
	[[self openGLContext] makeCurrentContext];

	CGLLockContext([[self openGLContext] CGLContextObj]);
}

- (void) completeDrawView
{
	CGLFlushDrawable([[self openGLContext] CGLContextObj]);
	CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

- (void) dealloc
{
	[super dealloc];
}
@end
