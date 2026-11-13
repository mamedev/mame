// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  windowcontroller.mm - our window/fullscreen manager
//
//  Mac OSD by R. Belmont
//
//============================================================

#import <Cocoa/Cocoa.h>

@interface MAMEWindowController : NSWindowController

// -video none: the window backs the render target but is never shown
@property (nonatomic, assign) BOOL headless;

- (NSWindow *) getStandardWindow;

@end
