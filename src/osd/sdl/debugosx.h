// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugosx.h - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __SDL_DEBUGOSX__
#define __SDL_DEBUGOSX__

// make sure this comes first
#include <AvailabilityMacros.h>

// MAME headers
#include "emu.h"
#include "debug/debugvw.h"


#ifdef __OBJC__

// standard Cocoa headers
#import <Cocoa/Cocoa.h>

// workarounds for 10.6 warnings
#ifdef MAC_OS_X_VERSION_MAX_ALLOWED

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1050

typedef int NSInteger;
typedef unsigned NSUInteger;
typedef float CGFloat;

#endif // MAC_OS_X_VERSION_MAX_ALLOWED < 1050

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1060

@protocol NSWindowDelegate <NSObject>
@end

@protocol NSSplitViewDelegate <NSObject>
@end

@protocol NSControlTextEditingDelegate <NSObject>
@end

@protocol NSTextFieldDelegate <NSControlTextEditingDelegate>
@end

#endif // MAC_OS_X_VERSION_MAX_ALLOWED < 1060

#endif // MAC_OS_X_VERSION_MAX_ALLOWED


//============================================================
//  PROTOCOLS
//============================================================

@protocol MAMEDebugViewSubviewSupport <NSObject>

- (NSString *)selectedSubviewName;
- (int)selectedSubviewIndex;
- (void)selectSubviewAtIndex:(int)index;
- (void)selectSubviewForCPU:(device_t *)device;

@end


@protocol MAMEDebugViewExpressionSupport <NSObject>

- (NSString *)expression;
- (void)setExpression:(NSString *)exp;

@end



//============================================================
//  CLASSES
//============================================================

@interface MAMEDebugCommandHistory : NSObject
{
	NSInteger       length, position;
	NSString        *current;
	NSMutableArray  *history;
}

+ (NSInteger)defaultLength;

- (id)init;

- (NSInteger)length;
- (void)setLength:(NSInteger)l;

- (void)add:(NSString *)entry;
- (NSString *)previous:(NSString *)cur;
- (NSString *)next:(NSString *)cur;
- (void)reset;
- (void)clear;

@end


@interface MAMEDebugView : NSView
{
	int             type;
	running_machine *machine;
	debug_view      *view;

	debug_view_xy   *totalSize, *topLeft;

	NSFont          *font;
	CGFloat         fontWidth, fontHeight, fontAscent;
}

+ (NSFont *)defaultFont;

- (id)initWithFrame:(NSRect)f type:(debug_view_type)t machine:(running_machine &)m;

- (void)update;

- (NSSize)maximumFrameSize;

- (NSFont *)font;
- (void)setFont:(NSFont *)f;

- (void)windowDidBecomeKey:(NSNotification *)notification;
- (void)windowDidResignKey:(NSNotification *)notification;

@end


@interface MAMEMemoryView : MAMEDebugView <MAMEDebugViewSubviewSupport, MAMEDebugViewExpressionSupport>
{
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m;

- (NSSize)maximumFrameSize;

- (NSString *)selectedSubviewName;
- (int)selectedSubviewIndex;
- (void)selectSubviewAtIndex:(int)index;
- (void)selectSubviewForCPU:(device_t *)device;

- (NSString *)expression;
- (void)setExpression:(NSString *)exp;

- (IBAction)showChunkSize:(id)sender;
- (IBAction)showPhysicalAddresses:(id)sender;
- (IBAction)showReverseView:(id)sender;
- (IBAction)showReverseViewToggle:(id)sender;

- (void)insertActionItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index;
- (void)insertSubviewItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index;

@end


@interface MAMEDisassemblyView : MAMEDebugView <MAMEDebugViewSubviewSupport, MAMEDebugViewExpressionSupport>
{
	BOOL    useConsole;
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m useConsole:(BOOL)uc;

- (NSSize)maximumFrameSize;

- (NSString *)selectedSubviewName;
- (int)selectedSubviewIndex;
- (void)selectSubviewAtIndex:(int)index;
- (void)selectSubviewForCPU:(device_t *)device;

- (NSString *)expression;
- (void)setExpression:(NSString *)exp;

- (IBAction)debugToggleBreakpoint:(id)sender;
- (IBAction)debugToggleBreakpointEnable:(id)sender;
- (IBAction)debugRunToCursor:(id)sender;

- (IBAction)showRightColumn:(id)sender;

- (void)insertActionItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index;
- (void)insertSubviewItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index;

@end


@interface MAMERegistersView : MAMEDebugView <MAMEDebugViewSubviewSupport>
{
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m;

- (NSSize)maximumFrameSize;

- (NSString *)selectedSubviewName;
- (int)selectedSubviewIndex;
- (void)selectSubviewAtIndex:(int)index;
- (void)selectSubviewForCPU:(device_t *)device;

@end


@interface MAMEConsoleView : MAMEDebugView
{
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m;

@end


@interface MAMEErrorLogView : MAMEDebugView
{
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m;

@end


@interface MAMEDebugWindowHandler : NSObject <NSWindowDelegate>
{
	NSWindow        *window;
	running_machine *machine;
}

+ (void)addCommonActionItems:(NSMenu *)menu;
+ (NSPopUpButton *)newActionButtonWithFrame:(NSRect)frame;

- (id)initWithMachine:(running_machine &)m title:(NSString *)t;

- (void)activate;

- (IBAction)debugRun:(id)sender;
- (IBAction)debugRunAndHide:(id)sender;
- (IBAction)debugRunToNextCPU:(id)sender;
- (IBAction)debugRunToNextInterrupt:(id)sender;
- (IBAction)debugRunToNextVBLANK:(id)sender;

- (IBAction)debugStepInto:(id)sender;
- (IBAction)debugStepOver:(id)sender;
- (IBAction)debugStepOut:(id)sender;

- (IBAction)debugSoftReset:(id)sender;
- (IBAction)debugHardReset:(id)sender;

- (IBAction)debugExit:(id)sender;

- (void)showDebugger:(NSNotification *)notification;
- (void)hideDebugger:(NSNotification *)notification;

@end


@interface MAMEDebugConsole : MAMEDebugWindowHandler <NSTextFieldDelegate, NSSplitViewDelegate>
{
	MAMEDebugCommandHistory *history;
	NSMutableArray          *auxiliaryWindows;

	MAMERegistersView       *regView;
	MAMEDisassemblyView     *dasmView;
	MAMEDebugView           *consoleView;
	NSTextField             *commandField;
}

- (id)initWithMachine:(running_machine &)m;

- (void)setCPU:(device_t *)device;

- (IBAction)doCommand:(id)sender;

- (IBAction)debugNewMemoryWindow:(id)sender;
- (IBAction)debugNewDisassemblyWindow:(id)sender;
- (IBAction)debugNewErrorLogWindow:(id)sender;

- (void)showDebugger:(NSNotification *)notification;
- (void)auxiliaryWindowWillClose:(NSNotification *)notification;

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command;

- (void)windowWillClose:(NSNotification *)notification;

- (CGFloat)splitView:(NSSplitView *)sender constrainMinCoordinate:(CGFloat)min ofSubviewAt:(NSInteger)offs;
- (CGFloat)splitView:(NSSplitView *)sender constrainMaxCoordinate:(CGFloat)max ofSubviewAt:(NSInteger)offs;
- (BOOL)splitView:(NSSplitView *)sender canCollapseSubview:(NSView *)subview;
- (void)splitView:(NSSplitView *)sender resizeSubviewsWithOldSize:(NSSize)oldSize;

@end


@interface MAMEAuxiliaryDebugWindowHandler : MAMEDebugWindowHandler
{
	MAMEDebugConsole    *console;
}

+ (void)cascadeWindow:(NSWindow *)window;

- (id)initWithMachine:(running_machine &)m title:(NSString *)t console:(MAMEDebugConsole *)c;

- (IBAction)debugNewMemoryWindow:(id)sender;
- (IBAction)debugNewDisassemblyWindow:(id)sender;
- (IBAction)debugNewErrorLogWindow:(id)sender;

- (void)windowWillClose:(NSNotification *)notification;

- (void)cascadeWindowWithDesiredSize:(NSSize)desired forView:(NSView *)view;

@end


@interface MAMEExpressionAuxiliaryDebugWindowHandler : MAMEAuxiliaryDebugWindowHandler <NSTextFieldDelegate>
{
	MAMEDebugCommandHistory *history;
	NSTextField             *expressionField;
}

- (id)initWithMachine:(running_machine &)m title:(NSString *)t console:(MAMEDebugConsole *)c;

- (id <MAMEDebugViewExpressionSupport>)documentView;

- (IBAction)doExpression:(id)sender;

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command;

@end


@interface MAMEMemoryViewer : MAMEExpressionAuxiliaryDebugWindowHandler
{
	MAMEMemoryView  *memoryView;
}

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c;

- (IBAction)changeSubview:(id)sender;

@end


@interface MAMEDisassemblyViewer : MAMEExpressionAuxiliaryDebugWindowHandler
{
	MAMEDisassemblyView *dasmView;
}

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c;

- (IBAction)changeSubview:(id)sender;

@end


@interface MAMEErrorLogViewer : MAMEAuxiliaryDebugWindowHandler
{
	MAMEErrorLogView    *logView;
}

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c;

@end

#endif // __OBJC__



//============================================================
//  PROTOTYPES
//============================================================

void debugwin_update_during_game(running_machine &machine);


#endif // __SDL_DEBUGOSX__
