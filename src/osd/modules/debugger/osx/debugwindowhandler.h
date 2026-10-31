// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugwindowhandler.h - MacOS X Cocoa debug window handling
//
//============================================================

#import "debugosx.h"

#include "debug/debugcpu.h"

#import <Cocoa/Cocoa.h>


@protocol MAMEDebugViewExpressionSupport;
@class MAMEDebugCommandHistory, MAMEDebugConsole;


extern NSString *const MAMEHideDebuggerNotification;
extern NSString *const MAMEShowDebuggerNotification;
extern NSString *const MAMEAuxiliaryDebugWindowWillCloseNotification;
extern NSString *const MAMESaveDebuggerConfigurationNotification;


@interface MAMEDebugWindowHandler : NSObject <NSWindowDelegate>
{
	NSWindow        *window;
	running_machine *machine;
}

+ (void)addCommonActionItems:(NSMenu *)menu;
+ (NSPopUpButton *)newActionButtonWithFrame:(NSRect)frame;

- (id)initWithMachine:(running_machine &)m title:(NSString *)t;

- (void)activate;
- (BOOL)sourceFrameActive;

- (IBAction)debugBreak:(id)sender;
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
- (void)saveConfig:(NSNotification *)notification;

- (void)saveConfigurationToNode:(util::xml::data_node *)node;
- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node;

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
- (IBAction)debugNewPointsWindow:(id)sender;
- (IBAction)debugNewDevicesWindow:(id)sender;

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

- (NSString *)expression;
- (void)setExpression:(NSString *)expression;

- (IBAction)doExpression:(id)sender;

- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor;
- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command;

- (void)saveConfigurationToNode:(util::xml::data_node *)node;
- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node;


@end


@protocol MAMEDebugViewDisasemblyContainer <NSObject>

- (void) setDisasemblyView:(BOOL)value;
- (BOOL) getDisasemblyView;
- (void) setSourceButton:(int)index;
- (void) populateSourceButton;

@end
