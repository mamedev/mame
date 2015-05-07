// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugconsole.h - MacOS X Cocoa debug window handling
//
//============================================================

#import "debugosx.h"

#import "debugwindowhandler.h"

#include "emu.h"

#import <Cocoa/Cocoa.h>


@class MAMEDebugCommandHistory, MAMEDebugView, MAMEDisassemblyView, MAMERegistersView;

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

- (IBAction)debugToggleBreakpoint:(id)sender;
- (IBAction)debugToggleBreakpointEnable:(id)sender;
- (IBAction)debugRunToCursor:(id)sender;

- (IBAction)debugNewMemoryWindow:(id)sender;
- (IBAction)debugNewDisassemblyWindow:(id)sender;
- (IBAction)debugNewErrorLogWindow:(id)sender;
- (IBAction)debugNewPointsWindow:(id)sender;
- (IBAction)debugNewDevicesWindow:(id)sender;

- (void)debugNewMemoryWindowForSpace:(address_space *)space device:(device_t *)device expression:(NSString *)expression;
- (void)debugNewDisassemblyWindowForSpace:(address_space *)space device:(device_t *)device expression:(NSString *)expression;
- (void)debugNewInfoWindowForDevice:(device_t &)device;

- (void)showDebugger:(NSNotification *)notification;
- (void)auxiliaryWindowWillClose:(NSNotification *)notification;

- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor;
- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command;

- (void)windowWillClose:(NSNotification *)notification;

- (CGFloat)splitView:(NSSplitView *)sender constrainMinCoordinate:(CGFloat)min ofSubviewAt:(NSInteger)offs;
- (CGFloat)splitView:(NSSplitView *)sender constrainMaxCoordinate:(CGFloat)max ofSubviewAt:(NSInteger)offs;
- (BOOL)splitView:(NSSplitView *)sender canCollapseSubview:(NSView *)subview;
- (void)splitView:(NSSplitView *)sender resizeSubviewsWithOldSize:(NSSize)oldSize;

@end
