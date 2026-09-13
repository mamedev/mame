// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  macglcontext.mm - Cocoa side of the Mac GL context
//
//  Mac OSD by R. Belmont
//
//============================================================

#define GL_SILENCE_DEPRECATION (1)

#import <Cocoa/Cocoa.h>

#include <dlfcn.h>

extern void *GetOSWindow(void *wincontroller);

extern "C" void *MacGLContextCreate(void *wincontroller)
{
	NSWindow *window = (NSWindow *)GetOSWindow(wincontroller);
	if (window == nil)
	{
		return nullptr;
	}

	NSOpenGLPixelFormatAttribute attrs[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAAllowOfflineRenderers,
		0
	};

	NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	if (format == nil)
	{
		return nullptr;
	}

	NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
	[format release];
	if (context == nil)
	{
		return nullptr;
	}

	// make the GL surface track the backing store scale so we render at native Retina resolution
	NSView *view = [window contentView];
	[view setWantsBestResolutionOpenGLSurface:YES];

	[context setView:view];
	[context makeCurrentContext];

	return context;
}

extern "C" void MacGLContextDestroy(void *context)
{
	NSOpenGLContext *ctx = (NSOpenGLContext *)context;

	if ([NSOpenGLContext currentContext] == ctx)
	{
		[NSOpenGLContext clearCurrentContext];
	}
	[ctx clearDrawable];
	[ctx release];
}

extern "C" void MacGLContextMakeCurrent(void *context)
{
	NSOpenGLContext *ctx = (NSOpenGLContext *)context;

	[ctx makeCurrentContext];
	// pick up any window resize/move since the last frame
	[ctx update];
}

extern "C" void MacGLContextSwapBuffers(void *context)
{
	NSOpenGLContext *ctx = (NSOpenGLContext *)context;

	[ctx flushBuffer];
}

extern "C" bool MacGLContextSetSwapInterval(void *context, int interval)
{
	NSOpenGLContext *ctx = (NSOpenGLContext *)context;
	GLint value = interval;

	[ctx setValues:&value forParameter:NSOpenGLContextParameterSwapInterval];
	return true;
}

extern "C" void *MacGLGetProcAddress(const char *proc)
{
	static void *gl_handle = nullptr;

	if (gl_handle == nullptr)
	{
		gl_handle = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY | RTLD_GLOBAL);
	}

	if (gl_handle != nullptr)
	{
		return dlsym(gl_handle, proc);
	}

	return dlsym(RTLD_DEFAULT, proc);
}
