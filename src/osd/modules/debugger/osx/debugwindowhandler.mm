// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugwindowhandler.m - MacOS X Cocoa debug window handling
//
//============================================================

#include "emu.h"
#import "debugwindowhandler.h"

#import "debugconsole.h"
#import "debugcommandhistory.h"
#import "debugview.h"

#include "debugger.h"
#include "debug/debugcon.h"

#include "util/xmlfile.h"


//============================================================
//  NOTIFICATIONS
//============================================================

NSString *const MAMEHideDebuggerNotification = @"MAMEHideDebuggerNotification";
NSString *const MAMEShowDebuggerNotification = @"MAMEShowDebuggerNotification";
NSString *const MAMEAuxiliaryDebugWindowWillCloseNotification = @"MAMEAuxiliaryDebugWindowWillCloseNotification";
NSString *const MAMESaveDebuggerConfigurationNotification = @"MAMESaveDebuggerConfigurationNotification";


//============================================================
//  MAMEDebugWindowHandler class
//============================================================

@implementation MAMEDebugWindowHandler

+ (void)addCommonActionItems:(NSMenu *)menu {
	[menu addItemWithTitle:@"Break"
					action:@selector(debugBreak:)
			 keyEquivalent:@""];

	NSMenuItem *runParentItem = [menu addItemWithTitle:@"Run"
												action:@selector(debugRun:)
										 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF5FunctionKey]];
	NSMenu *runMenu = [[NSMenu alloc] initWithTitle:@"Run"];
	[runParentItem setSubmenu:runMenu];
	[runMenu release];
	[runParentItem setKeyEquivalentModifierMask:0];
	[[runMenu addItemWithTitle:@"and Hide Debugger"
						action:@selector(debugRunAndHide:)
				 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF12FunctionKey]]
	 setKeyEquivalentModifierMask:0];
	[[runMenu addItemWithTitle:@"to Next CPU"
						action:@selector(debugRunToNextCPU:)
				 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF6FunctionKey]]
	 setKeyEquivalentModifierMask:0];
	[[runMenu addItemWithTitle:@"until Next Interrupt on Current CPU"
						action:@selector(debugRunToNextInterrupt:)
				 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF7FunctionKey]]
	 setKeyEquivalentModifierMask:0];
	[[runMenu addItemWithTitle:@"until Next VBLANK"
						action:@selector(debugRunToNextVBLANK:)
				 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF8FunctionKey]]
	 setKeyEquivalentModifierMask:0];

	NSMenuItem *stepParentItem = [menu addItemWithTitle:@"Step" action:NULL keyEquivalent:@""];
	NSMenu *stepMenu = [[NSMenu alloc] initWithTitle:@"Step"];
	[stepParentItem setSubmenu:stepMenu];
	[stepMenu release];
	[[stepMenu addItemWithTitle:@"Into"
						 action:@selector(debugStepInto:)
				  keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF11FunctionKey]]
	 setKeyEquivalentModifierMask:0];
	[[stepMenu addItemWithTitle:@"Over"
						 action:@selector(debugStepOver:)
				  keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF10FunctionKey]]
	 setKeyEquivalentModifierMask:0];
	[[stepMenu addItemWithTitle:@"Out"
						 action:@selector(debugStepOut:)
				  keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF10FunctionKey]]
	 setKeyEquivalentModifierMask:NSEventModifierFlagShift];

	NSMenuItem *resetParentItem = [menu addItemWithTitle:@"Reset" action:NULL keyEquivalent:@""];
	NSMenu *resetMenu = [[NSMenu alloc] initWithTitle:@"Reset"];
	[resetParentItem setSubmenu:resetMenu];
	[resetMenu release];
	[[resetMenu addItemWithTitle:@"Soft"
						  action:@selector(debugSoftReset:)
				   keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF3FunctionKey]]
	 setKeyEquivalentModifierMask:0];
	[[resetMenu addItemWithTitle:@"Hard"
						  action:@selector(debugHardReset:)
				   keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF3FunctionKey]]
	 setKeyEquivalentModifierMask:NSEventModifierFlagShift];

	[menu addItem:[NSMenuItem separatorItem]];

	NSMenuItem *newParentItem = [menu addItemWithTitle:@"New" action:NULL keyEquivalent:@""];
	NSMenu *newMenu = [[NSMenu alloc] initWithTitle:@"New"];
	[newParentItem setSubmenu:newMenu];
	[newMenu release];
	[newMenu addItemWithTitle:@"Memory Window"
					   action:@selector(debugNewMemoryWindow:)
				keyEquivalent:@"d"];
	[newMenu addItemWithTitle:@"Disassembly Window"
					   action:@selector(debugNewDisassemblyWindow:)
				keyEquivalent:@"a"];
	[newMenu addItemWithTitle:@"Error Log Window"
					   action:@selector(debugNewErrorLogWindow:)
				keyEquivalent:@"l"];
	[newMenu addItemWithTitle:@"(Break|Watch)points Window"
					   action:@selector(debugNewPointsWindow:)
				keyEquivalent:@"b"];
	[newMenu addItemWithTitle:@"Devices Window"
					   action:@selector(debugNewDevicesWindow:)
				keyEquivalent:@""];

	[menu addItem:[NSMenuItem separatorItem]];

	[menu addItemWithTitle:@"Close Window" action:@selector(performClose:) keyEquivalent:@"w"];
	[menu addItemWithTitle:@"Quit" action:@selector(debugExit:) keyEquivalent:@"q"];
}


+ (NSPopUpButton *)newActionButtonWithFrame:(NSRect)frame {
	NSPopUpButton *actionButton = [[NSPopUpButton alloc] initWithFrame:frame pullsDown:YES];
	[actionButton setTitle:@""];
	[actionButton addItemWithTitle:@""];
	[actionButton setBezelStyle:NSBezelStyleShadowlessSquare];
	[actionButton setFocusRingType:NSFocusRingTypeNone];
	[[actionButton cell] setArrowPosition:NSPopUpArrowAtCenter];
	[[self class] addCommonActionItems:[actionButton menu]];
	return actionButton;
}


- (id)initWithMachine:(running_machine &)m title:(NSString *)t {
	if (!(self = [super init]))
		return nil;

	window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 320, 240)
										 styleMask:(NSWindowStyleMaskTitled |
													NSWindowStyleMaskClosable |
													NSWindowStyleMaskMiniaturizable |
													NSWindowStyleMaskResizable)
										   backing:NSBackingStoreBuffered
											 defer:YES];
	[window setReleasedWhenClosed:NO];
	[window setDelegate:self];
	[window setTitle:t];
	[window setContentMinSize:NSMakeSize(320, 240)];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(saveConfig:)
												 name:MAMESaveDebuggerConfigurationNotification
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(showDebugger:)
												 name:MAMEShowDebuggerNotification
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(hideDebugger:)
												 name:MAMEHideDebuggerNotification
											   object:nil];

	machine = &m;

	return self;
}


- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	if (window != nil)
	{
		[window orderOut:self];
		[window release];
	}

	[super dealloc];
}


- (void)activate {
	[window makeKeyAndOrderFront:self];
}


- (IBAction)debugBreak:(id)sender {
	if (machine->debug_flags & DEBUG_FLAG_ENABLED)
		machine->debugger().console().get_visible_cpu()->debug()->halt_on_next_instruction("User-initiated break\n");
}


- (IBAction)debugRun:(id)sender {
	machine->debugger().console().get_visible_cpu()->debug()->go();
}


- (IBAction)debugRunAndHide:(id)sender {
	[[NSNotificationCenter defaultCenter] postNotificationName:MAMEHideDebuggerNotification
														object:self
													  userInfo:[NSDictionary dictionaryWithObject:[NSValue valueWithPointer:machine]
																						   forKey:@"MAMEDebugMachine"]];
	machine->debugger().console().get_visible_cpu()->debug()->go();
}


- (IBAction)debugRunToNextCPU:(id)sender {
	machine->debugger().console().get_visible_cpu()->debug()->go_next_device();
}


- (IBAction)debugRunToNextInterrupt:(id)sender {
	machine->debugger().console().get_visible_cpu()->debug()->go_interrupt();
}


- (IBAction)debugRunToNextVBLANK:(id)sender {
	machine->debugger().console().get_visible_cpu()->debug()->go_vblank();
}


- (IBAction)debugStepInto:(id)sender {
	machine->debugger().console().get_visible_cpu()->debug()->single_step(1, [self sourceFrameActive]);
}


- (IBAction)debugStepOver:(id)sender {
	machine->debugger().console().get_visible_cpu()->debug()->single_step_over(1, [self sourceFrameActive]);
}


- (IBAction)debugStepOut:(id)sender {
	machine->debugger().console().get_visible_cpu()->debug()->single_step_out();
}


- (IBAction)debugSoftReset:(id)sender {
	machine->schedule_soft_reset();
	machine->debugger().console().get_visible_cpu()->debug()->go();
}


- (IBAction)debugHardReset:(id)sender {
	machine->schedule_hard_reset();
}


- (IBAction)debugExit:(id)sender {
	machine->schedule_exit();
}

- (BOOL)sourceFrameActive
{
	return NO;
}

- (void)showDebugger:(NSNotification *)notification {
	running_machine *m = (running_machine *)[[[notification userInfo] objectForKey:@"MAMEDebugMachine"] pointerValue];
	if (m == machine)
	{
		if (![window isVisible] && ![window isMiniaturized])
			[window orderFront:self];
	}
}


- (void)hideDebugger:(NSNotification *)notification {
	running_machine *m = (running_machine *)[[[notification userInfo] objectForKey:@"MAMEDebugMachine"] pointerValue];
	if (m == machine)
		[window orderOut:self];
}


- (void)saveConfig:(NSNotification *)notification {
	running_machine *m = (running_machine *)[[[notification userInfo] objectForKey:@"MAMEDebugMachine"] pointerValue];
	if (m == machine)
	{
		util::xml::data_node *parentnode = (util::xml::data_node *)[[[notification userInfo] objectForKey:@"MAMEDebugParentNode"] pointerValue];
		util::xml::data_node *node = parentnode->add_child(osd::debugger::NODE_WINDOW, nullptr);
		if (node)
			[self saveConfigurationToNode:node];
	}
}


- (void)saveConfigurationToNode:(util::xml::data_node *)node {
	NSRect frame = [window frame];
	node->set_attribute_float(osd::debugger::ATTR_WINDOW_POSITION_X, frame.origin.x);
	node->set_attribute_float(osd::debugger::ATTR_WINDOW_POSITION_Y, frame.origin.y);
	node->set_attribute_float(osd::debugger::ATTR_WINDOW_WIDTH, frame.size.width);
	node->set_attribute_float(osd::debugger::ATTR_WINDOW_HEIGHT, frame.size.height);
}


- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node {
	NSRect frame = [window frame];
	frame.origin.x = node->get_attribute_float(osd::debugger::ATTR_WINDOW_POSITION_X, frame.origin.x);
	frame.origin.y = node->get_attribute_float(osd::debugger::ATTR_WINDOW_POSITION_Y, frame.origin.y);
	frame.size.width = node->get_attribute_float(osd::debugger::ATTR_WINDOW_WIDTH, frame.size.width);
	frame.size.height = node->get_attribute_float(osd::debugger::ATTR_WINDOW_HEIGHT, frame.size.height);

	NSSize min = [window minSize];
	frame.size.width = std::max(frame.size.width, min.width);
	frame.size.height = std::max(frame.size.height, min.height);

	NSSize max = [window maxSize];
	frame.size.width = std::min(frame.size.width, max.width);
	frame.size.height = std::min(frame.size.height, max.height);

	[window setFrame:frame display:YES];
}

@end


//============================================================
//  MAMEAuxiliaryDebugWindowHandler class
//============================================================

@implementation MAMEAuxiliaryDebugWindowHandler

+ (void)cascadeWindow:(NSWindow *)window {
	static NSPoint lastPosition = { 0, 0 };
	if (NSEqualPoints(lastPosition, NSZeroPoint)) {
		NSRect available = [[NSScreen mainScreen] visibleFrame];
		lastPosition = NSMakePoint(available.origin.x + 12, available.origin.y + available.size.height - 8);
	}
	lastPosition = [window cascadeTopLeftFromPoint:lastPosition];
}


- (id)initWithMachine:(running_machine &)m title:(NSString *)t console:(MAMEDebugConsole *)c {
	if (!(self = [super initWithMachine:m title:t]))
		return nil;
	console = c;
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (IBAction)debugNewMemoryWindow:(id)sender {
	[console debugNewMemoryWindow:sender];
}


- (IBAction)debugNewDisassemblyWindow:(id)sender {
	[console debugNewDisassemblyWindow:sender];
}


- (IBAction)debugNewErrorLogWindow:(id)sender {
	[console debugNewErrorLogWindow:sender];
}


- (IBAction)debugNewPointsWindow:(id)sender {
	[console debugNewPointsWindow:sender];
}


- (IBAction)debugNewDevicesWindow:(id)sender {
	[console debugNewDevicesWindow:sender];
}


- (void)windowWillClose:(NSNotification *)notification {
	[[NSNotificationCenter defaultCenter] postNotificationName:MAMEAuxiliaryDebugWindowWillCloseNotification
														object:self];
}

- (void)cascadeWindowWithDesiredSize:(NSSize)desired forView:(NSView *)view {
	// convert desired size to an adjustment and apply it to the current window frame
	NSSize const current = [view frame].size;
	desired.width -= current.width;
	desired.height -= current.height;
	NSRect windowFrame = [window frame];
	windowFrame.size.width += desired.width;
	windowFrame.size.height += desired.height;

	// limit the size to the minimum size
	NSSize const minimum = [window minSize];
	windowFrame.size.width = std::max(windowFrame.size.width, minimum.width);
	windowFrame.size.height = std::max(windowFrame.size.height, minimum.height);

	// limit the size to the main screen size
	NSRect const available = [[NSScreen mainScreen] visibleFrame];
	windowFrame.size.width = std::min(windowFrame.size.width, available.size.width);
	windowFrame.size.height = std::min(windowFrame.size.height, available.size.height);

	// arbitrary additional height limit
	windowFrame.size.height = std::min(windowFrame.size.height, CGFloat(320));

	// place it in the bottom right corner and apply
	windowFrame.origin.x = available.origin.x + available.size.width - windowFrame.size.width;
	windowFrame.origin.y = available.origin.y;
	[window setFrame:windowFrame display:YES];
	[[self class] cascadeWindow:window];
}

@end


//============================================================
//  MAMEExpreesionAuxiliaryDebugWindowHandler class
//============================================================

@implementation MAMEExpressionAuxiliaryDebugWindowHandler

- (id)initWithMachine:(running_machine &)m title:(NSString *)t console:(MAMEDebugConsole *)c {
	if (!(self = [super initWithMachine:m title:t console:c]))
		return nil;
	history = [[MAMEDebugCommandHistory alloc] init];
	return self;
}


- (void)dealloc {
	if (history != nil)
		[history release];
	[super dealloc];
}


- (id <MAMEDebugViewExpressionSupport>)documentView {
	return nil;
}


- (NSString *)expression {
	return [[self documentView] expression];
}

- (void)setExpression:(NSString *)expression {
	[history add:expression];
	[[self documentView] setExpression:expression];
	[expressionField setStringValue:expression];
	[expressionField selectText:self];
}


- (IBAction)doExpression:(id)sender {
	NSString *expr = [sender stringValue];
	if ([expr length] > 0) {
		[history add:expr];
		[[self documentView] setExpression:expr];
	} else {
		[sender setStringValue:[[self documentView] expression]];
		[history reset];
	}
	[sender selectText:self];
}


- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor
{
	if (control == expressionField)
		[history edit];

	return YES;
}


- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command {
	if (control == expressionField) {
		if (command == @selector(cancelOperation:)) {
			[history reset];
			[expressionField setStringValue:[[self documentView] expression]];
			[expressionField selectText:self];
			return YES;
		} else if (command == @selector(moveUp:)) {
			NSString *hist = [history previous:[expressionField stringValue]];
			if (hist != nil) {
				[expressionField setStringValue:hist];
				[expressionField selectText:self];
			}
			return YES;
		} else if (command == @selector(moveDown:)) {
			NSString *hist = [history next:[expressionField stringValue]];
			if (hist != nil) {
				[expressionField setStringValue:hist];
				[expressionField selectText:self];
			}
			return YES;
		}
	}
	return NO;
}


- (void)saveConfigurationToNode:(util::xml::data_node *)node {
	[super saveConfigurationToNode:node];
	node->add_child(osd::debugger::NODE_WINDOW_EXPRESSION, [[self expression] UTF8String]);
	[history saveConfigurationToNode:node];
}


- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node {
	[super restoreConfigurationFromNode:node];
	util::xml::data_node const *const expr = node->get_child(osd::debugger::NODE_WINDOW_EXPRESSION);
	if (expr && expr->get_value())
		[self setExpression:[NSString stringWithUTF8String:expr->get_value()]];
	[history restoreConfigurationFromNode:node];
}

@end
