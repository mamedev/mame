// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugconsole.h - MacOS X Cocoa debug window handling
//
//============================================================

#import "debugosx.h"

#import "debugwindowhandler.h"


#import <Cocoa/Cocoa.h>


@class MAMEDebugCommandHistory, MAMEDebugView, MAMEDisassemblyView, MAMERegistersView, MAMESrcDebugView;

@interface MAMEDebugConsole : MAMEDebugWindowHandler <NSTextFieldDelegate, NSSplitViewDelegate>
{
	MAMEDebugCommandHistory *history;
	NSMutableArray          *auxiliaryWindows;

	NSSplitView             *regSplit, *dasmSplit;
	MAMERegistersView       *regView;
	MAMEDisassemblyView     *dasmView;
	NSScrollView            *dasmScroll;
	MAMESrcDebugView        *srcdbgView;
	NSPopUpButton           *sourceButton;
	NSView                  *srcdbgContainerView;
	MAMEDebugView           *consoleView;
	NSTextField             *commandField;
}

- (id)initWithMachine:(running_machine &)m;

- (void)setCPU:(device_t *)device;
- (BOOL)sourceFrameActive;

- (IBAction)sourceDebugBarChanged:(id)sender;

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

- (void)loadConfiguration:(util::xml::data_node const *)parentnode;

- (void)saveConfigurationToNode:(util::xml::data_node *)node;
- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node;

- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor;
- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command;

- (void)windowWillClose:(NSNotification *)notification;

- (CGFloat)splitView:(NSSplitView *)sender constrainMinCoordinate:(CGFloat)min ofSubviewAt:(NSInteger)offs;
- (CGFloat)splitView:(NSSplitView *)sender constrainMaxCoordinate:(CGFloat)max ofSubviewAt:(NSInteger)offs;
- (BOOL)splitView:(NSSplitView *)sender canCollapseSubview:(NSView *)subview;
- (void)splitView:(NSSplitView *)sender resizeSubviewsWithOldSize:(NSSize)oldSize;

- (void)setDisasemblyView:(BOOL)value;
- (BOOL) getDisasemblyView;
- (void) setSourceButton:(int)index;

@end
