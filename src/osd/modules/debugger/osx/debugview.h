// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugview.h - MacOS X Cocoa debug window handling
//
//============================================================

#import "debugosx.h"

#include "debug/debugvw.h"

#import <Cocoa/Cocoa.h>


@interface MAMEDebugView : NSView
{
	int             type;
	running_machine *machine;
	debug_view      *view;
	BOOL            wholeLineScroll;

	int32_t         totalWidth, totalHeight, originTop;

	NSFont          *font;
	CGFloat         fontWidth, fontHeight, fontAscent;

	NSTextStorage   *text;
	NSTextContainer *textContainer;
	NSLayoutManager *layoutManager;
}

+ (NSFont *)defaultFontForMachine:(running_machine &)m;

- (id)initWithFrame:(NSRect)f type:(debug_view_type)t machine:(running_machine &)m wholeLineScroll:(BOOL)w;

- (void)update;
- (void)adjustSizeAndRecomputeVisible;

- (NSSize)maximumFrameSize;

- (NSFont *)font;
- (void)setFont:(NSFont *)f;

- (BOOL)cursorSupported;
- (BOOL)cursorVisible;
- (debug_view_xy)cursorPosition;

- (IBAction)copyVisible:(id)sender;
- (IBAction)paste:(id)sender;

- (void)viewBoundsDidChange:(NSNotification *)notification;
- (void)viewFrameDidChange:(NSNotification *)notification;

- (void)windowDidBecomeKey:(NSNotification *)notification;
- (void)windowDidResignKey:(NSNotification *)notification;

- (void)addContextMenuItemsToMenu:(NSMenu *)menu;

- (void)saveConfigurationToNode:(util::xml::data_node *)node;
- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node;

@end


@protocol MAMEDebugViewSubviewSupport <NSObject>

- (NSString *)selectedSubviewName;
- (int)selectedSubviewIndex;
- (void)selectSubviewAtIndex:(int)index;
- (BOOL)selectSubviewForDevice:(device_t *)device;

@end


@protocol MAMEDebugViewExpressionSupport <NSObject>

- (NSString *)expression;
- (void)setExpression:(NSString *)exp;

@end
