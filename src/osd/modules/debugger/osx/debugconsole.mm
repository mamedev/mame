// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugconsole.m - MacOS X Cocoa debug window handling
//
//============================================================

#include "emu.h"
#import "debugconsole.h"

#import "debugcommandhistory.h"
#import "consoleview.h"
#import "debugview.h"
#import "deviceinfoviewer.h"
#import "devicesviewer.h"
#import "disassemblyview.h"
#import "disassemblyviewer.h"
#import "errorlogviewer.h"
#import "memoryviewer.h"
#import "pointsviewer.h"
#import "registersview.h"
#import "srcdebugview.h"

#include "debugger.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/points.h"

#include "util/xmlfile.h"


@implementation MAMEDebugConsole

- (id)initWithMachine:(running_machine &)m {
	NSScrollView    *regScroll, *srcdbgScroll, *consoleScroll;
	NSView          *consoleContainer, *dasmContainer;
	NSPopUpButton   *actionButton;
	NSRect          rct;

	// initialise superclass
	if (!(self = [super initWithMachine:m title:@"Debug"]))
		return nil;
	history = [[MAMEDebugCommandHistory alloc] init];
	auxiliaryWindows = [[NSMutableArray alloc] init];
	NSFont *const defaultFont = [[MAMEDebugView class] defaultFontForMachine:m];

	// create the register view
	regView = [[MAMERegistersView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100) machine:*machine];
	regScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[regScroll setHasHorizontalScroller:YES];
	[regScroll setHasVerticalScroller:YES];
	[regScroll setAutohidesScrollers:YES];
	[regScroll setBorderType:NSBezelBorder];
	[regScroll setDrawsBackground:NO];
	[regScroll setDocumentView:regView];
	[regView release];

	// create the disassembly view
	dasmView = [[MAMEDisassemblyView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100) machine:*machine];
	[dasmView setExpression:@"curpc"];
	[dasmView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	dasmScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[dasmScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[dasmScroll setHasHorizontalScroller:YES];
	[dasmScroll setHasVerticalScroller:YES];
	[dasmScroll setAutohidesScrollers:YES];
	[dasmScroll setBorderType:NSBezelBorder];
	[dasmScroll setDrawsBackground:NO];
	[dasmScroll setDocumentView:dasmView];
	[dasmView release];

	// create the source debug view
	srcdbgView = [[MAMESrcDebugView alloc] initWithFrame:NSMakeRect(0, 0, 100, 78) machine:*machine];
	[srcdbgView setExpression:@"curpc"];
	[srcdbgView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	srcdbgScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 100, 78)];
	[srcdbgScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[srcdbgScroll setHasHorizontalScroller:YES];
	[srcdbgScroll setHasVerticalScroller:YES];
	[srcdbgScroll setAutohidesScrollers:YES];
	[srcdbgScroll setBorderType:NSBezelBorder];
	[srcdbgScroll setDrawsBackground:NO];
	[srcdbgScroll setDocumentView:srcdbgView];
	[srcdbgView release];

	// create the source popup button
	sourceButton = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 78, 100, 19)];
	[sourceButton setAutoresizingMask:(NSViewWidthSizable | NSViewMinXMargin | NSViewMinYMargin)];
	[sourceButton setBezelStyle:NSBezelStyleShadowlessSquare];
	[sourceButton setFocusRingType:NSFocusRingTypeNone];
	[sourceButton setFont:defaultFont];
	[sourceButton setTarget:self];
	[sourceButton setAction:@selector(changeSubview:)];
	[[sourceButton cell] setArrowPosition:NSPopUpArrowAtBottom];
	[srcdbgView insertSubviewItemsInMenu:[sourceButton menu] atIndex:0];

	// create source container to group together the popup and debug view
	srcdbgContainerView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[srcdbgContainerView addSubview:srcdbgScroll];
	[srcdbgContainerView addSubview:sourceButton];
	[srcdbgContainerView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[srcdbgScroll release];
	[sourceButton release];

	// Create disassembly container
	dasmContainer = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[dasmContainer addSubview:dasmScroll];
	[dasmContainer addSubview:srcdbgContainerView];
	[dasmContainer setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[dasmScroll release];
	[srcdbgContainerView release];

	[srcdbgContainerView setHidden:true];

	// create the console view
	consoleView = [[MAMEConsoleView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100) machine:*machine];
	consoleScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[consoleScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[consoleScroll setHasHorizontalScroller:YES];
	[consoleScroll setHasVerticalScroller:YES];
	[consoleScroll setAutohidesScrollers:YES];
	[consoleScroll setBorderType:NSBezelBorder];
	[consoleScroll setDrawsBackground:NO];
	[consoleScroll setDocumentView:consoleView];
	[consoleView release];

	// create the command field
	commandField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 100, 19)];
	[commandField setAutoresizingMask:(NSViewWidthSizable | NSViewMaxYMargin)];
	[commandField setFont:defaultFont];
	[commandField setFocusRingType:NSFocusRingTypeNone];
	[commandField setTarget:self];
	[commandField setAction:@selector(doCommand:)];
	[commandField setDelegate:self];
	[commandField sizeToFit];
	rct = [commandField frame];
	[commandField setFrame:NSMakeRect(rct.size.height, 0, 100 - rct.size.height, rct.size.height)];

	// create the action pull-down button
	actionButton = [[self class] newActionButtonWithFrame:NSMakeRect(0, 0, rct.size.height, rct.size.height)];
	[actionButton setAutoresizingMask:(NSViewMaxXMargin | NSViewMaxYMargin)];
	[actionButton setFont:[NSFont systemFontOfSize:[defaultFont pointSize]]];
	[dasmView insertActionItemsInMenu:[actionButton menu] atIndex:1];

	// create the container for the console and command input field
	consoleContainer = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[consoleScroll setFrame:NSMakeRect(0,
									   rct.size.height,
									   100,
									   [consoleContainer bounds].size.height - rct.size.height)];
	[consoleContainer addSubview:consoleScroll];
	[consoleContainer addSubview:commandField];
	[consoleContainer addSubview:actionButton];
	[consoleScroll release];
	[commandField release];
	[actionButton release];

	// create the split between the disassembly and the console
	dasmSplit = [[NSSplitView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[dasmSplit setDelegate:self];
	[dasmSplit setVertical:NO];
	[dasmSplit addSubview:dasmContainer];
	[dasmSplit addSubview:consoleContainer];
	[dasmSplit setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[dasmContainer release];
	[consoleContainer release];

	// create the split between the registers and the console
	regSplit = [[NSSplitView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[regSplit setDelegate:self];
	[regSplit setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[regSplit setVertical:YES];
	[regSplit addSubview:regScroll];
	[regSplit addSubview:dasmSplit];
	[regScroll release];
	[dasmSplit release];

	// put the split views in the window and get them into a half-reasonable state
	[window setContentView:regSplit];
	[regSplit release];
	[regSplit adjustSubviews];
	[dasmSplit adjustSubviews];

	// keyboard focus should start on the command field
	[window makeFirstResponder:commandField];

	// calculate the optimal size for everything
	NSRect const    available = [[NSScreen mainScreen] visibleFrame];
	NSSize const    regCurrent = [regScroll frame].size;
	NSSize const    regSize = [NSScrollView frameSizeForContentSize:[regView maximumFrameSize]
											horizontalScrollerClass:[NSScroller class]
											  verticalScrollerClass:[NSScroller class]
														 borderType:[regScroll borderType]
														controlSize:NSControlSizeRegular
													  scrollerStyle:NSScrollerStyleOverlay];
	NSSize const    dasmCurrent = [dasmScroll frame].size;
	NSSize const    dasmSize = [NSScrollView frameSizeForContentSize:[dasmView maximumFrameSize]
											 horizontalScrollerClass:[NSScroller class]
											   verticalScrollerClass:[NSScroller class]
														  borderType:[dasmScroll borderType]
														 controlSize:NSControlSizeRegular
													   scrollerStyle:NSScrollerStyleOverlay];
	[srcdbgView maximumFrameSize]; // called to correctly setup source
	NSSize const    consoleCurrent = [consoleContainer frame].size;
	NSSize          consoleSize = [NSScrollView frameSizeForContentSize:[consoleView maximumFrameSize]
												horizontalScrollerClass:[NSScroller class]
												  verticalScrollerClass:[NSScroller class]
															 borderType:[consoleScroll borderType]
															controlSize:NSControlSizeRegular
														  scrollerStyle:NSScrollerStyleOverlay];
	NSRect          windowFrame = [window frame];
	NSSize          adjustment;

	consoleSize.width += consoleCurrent.width - [consoleScroll frame].size.width;
	consoleSize.height += consoleCurrent.height - [consoleScroll frame].size.height;
	adjustment.width = regSize.width - regCurrent.width;
	adjustment.height = regSize.height - regCurrent.height;
	adjustment.width += std::max(dasmSize.width - dasmCurrent.width, consoleSize.width - consoleCurrent.width);

	windowFrame.size.width += adjustment.width;
	windowFrame.size.height += adjustment.height; // not used - better to go for fixed height
	windowFrame.size.height = std::min(CGFloat(512.0), available.size.height);
	windowFrame.size.width = std::min(windowFrame.size.width, available.size.width);
	windowFrame.origin.x = available.origin.x + available.size.width - windowFrame.size.width;
	windowFrame.origin.y = available.origin.y;
	[window setFrame:windowFrame display:YES];

	NSRect lhsFrame = [regScroll frame];
	NSRect rhsFrame = [dasmSplit frame];
	adjustment.width = std::min(regSize.width, ([regSplit frame].size.width - [regSplit dividerThickness]) / 2);
	rhsFrame.origin.x -= lhsFrame.size.width - adjustment.width;
	rhsFrame.size.width += lhsFrame.size.width - adjustment.width;
	lhsFrame.size.width = adjustment.width;
	[regScroll setFrame:lhsFrame];
	[dasmSplit setFrame:rhsFrame];

	// select the current processor
	[self setCPU:machine->debugger().console().get_visible_cpu()];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(auxiliaryWindowWillClose:)
												 name:MAMEAuxiliaryDebugWindowWillCloseNotification
											   object:nil];

	// don't forget the return value
	return self;
}


- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	if (history != nil)
		[history release];
	if (auxiliaryWindows != nil)
		[auxiliaryWindows release];

	[super dealloc];
}


- (BOOL)sourceFrameActive {
	return ![dasmScroll isHidden];
}


- (void)setCPU:(device_t *)device {
	[regView selectSubviewForDevice:device];
	[dasmView selectSubviewForDevice:device];
	[srcdbgView selectSubviewForDevice:device];
	[window setTitle:[NSString stringWithFormat:@"Debug: %s - %s '%s'",
												device->machine().system().name,
												device->name(),
												device->tag()]];
}


- (IBAction)doCommand:(id)sender {
	NSString *command = [sender stringValue];
	if ([command length] == 0)
	{
		machine->debugger().console().get_visible_cpu()->debug()->single_step();
		[history reset];
	}
	else
	{
		machine->debugger().console().execute_command([command UTF8String], 1);
		[history add:command];
		[history edit];
	}
	[sender setStringValue:@""];
}


- (IBAction)debugToggleBreakpoint:(id)sender {
	MAMEDisassemblyView *visibleView = [dasmScroll isHidden] ? (MAMEDisassemblyView *)srcdbgView : dasmView;
	NSNumber *num = [visibleView selectedAddress];

	if (num)
	{
		offs_t const address = [num unsignedIntValue];
		device_t &device = *[visibleView source]->device();
		if ([visibleView cursorVisible] && (machine->debugger().console().get_visible_cpu() == &device))
		{
			const debug_breakpoint *bp = [visibleView source]->device()->debug()->breakpoint_find(address);

			// if it doesn't exist, add a new one
			NSString *command;
			if (bp == nullptr)
				command = [NSString stringWithFormat:@"bpset 0x%lX", (unsigned long)address];
			else
				command = [NSString stringWithFormat:@"bpclear 0x%X", (unsigned)bp->index()];
			machine->debugger().console().execute_command([command UTF8String], 1);
		}
	}
}

- (IBAction)debugToggleBreakpointEnable:(id)sender {
	MAMEDisassemblyView *visibleView = [dasmScroll isHidden] ? (MAMEDisassemblyView *)srcdbgView : dasmView;
	NSNumber *num = [visibleView selectedAddress];

	if (num)
	{
		device_t &device = *[visibleView source]->device();
		if ([visibleView cursorVisible] && (machine->debugger().console().get_visible_cpu() == &device))
		{
			offs_t const address = [num unsignedIntValue];
			const debug_breakpoint *bp = [visibleView source]->device()->debug()->breakpoint_find(address);
			if (bp != nullptr)
			{
				NSString *command;
				if (bp->enabled())
					command = [NSString stringWithFormat:@"bpdisable 0x%X", (unsigned)bp->index()];
				else
					command = [NSString stringWithFormat:@"bpenable 0x%X", (unsigned)bp->index()];
				machine->debugger().console().execute_command([command UTF8String], 1);
			}
		}
	}
}


- (IBAction)debugRunToCursor:(id)sender {
	MAMEDisassemblyView *visibleView = [dasmScroll isHidden] ? (MAMEDisassemblyView *)srcdbgView : dasmView;
	NSNumber *num = [visibleView selectedAddress];

	if (num)
	{
		device_t &device = *[visibleView source]->device();
		if ([visibleView cursorVisible] && (machine->debugger().console().get_visible_cpu() == &device))
		{
			offs_t const address = [num unsignedIntValue];
			NSString *command = [NSString stringWithFormat:@"go 0x%lX", (unsigned long)address];
			machine->debugger().console().execute_command([command UTF8String], 1);
		}
	}
}


- (IBAction)debugNewMemoryWindow:(id)sender {
	MAMEMemoryViewer *win = [[MAMEMemoryViewer alloc] initWithMachine:*machine console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	[win activate];
}


- (IBAction)debugNewDisassemblyWindow:(id)sender {
	MAMEDisassemblyViewer *win = [[MAMEDisassemblyViewer alloc] initWithMachine:*machine console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	[win activate];
}


- (IBAction)debugNewErrorLogWindow:(id)sender {
	MAMEErrorLogViewer *win = [[MAMEErrorLogViewer alloc] initWithMachine:*machine console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	[win activate];
}


- (IBAction)debugNewPointsWindow:(id)sender{
	MAMEPointsViewer *win = [[MAMEPointsViewer alloc] initWithMachine:*machine console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	[win activate];
}


- (IBAction)debugNewDevicesWindow:(id)sender {
	MAMEDevicesViewer *win = [[MAMEDevicesViewer alloc] initWithMachine:*machine console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	[win activate];
}


- (void)debugNewMemoryWindowForSpace:(address_space *)space device:(device_t *)device expression:(NSString *)expression {
	MAMEMemoryViewer *win = [[MAMEMemoryViewer alloc] initWithMachine:*machine console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	if ([win selectSubviewForSpace:space])
	{
		if (expression != nil)
			[win setExpression:expression];
	}
	else
	{
		[win selectSubviewForDevice:device];
	}
	[win activate];
}


- (void)debugNewDisassemblyWindowForSpace:(address_space *)space device:(device_t *)device expression:(NSString *)expression {
	MAMEDisassemblyViewer *win = [[MAMEDisassemblyViewer alloc] initWithMachine:*machine console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	if ([win selectSubviewForSpace:space])
	{
		if (expression != nil)
			[win setExpression:expression];
	}
	else
	{
		[win selectSubviewForDevice:device];
	}
	[win activate];
}


- (void)debugNewInfoWindowForDevice:(device_t &)device {
	MAMEDeviceInfoViewer *win = [[MAMEDeviceInfoViewer alloc] initWithDevice:device
																	 machine:*machine
																	 console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	[win activate];
}


- (void)showDebugger:(NSNotification *)notification {
	device_t *device = (device_t * )[[[notification userInfo] objectForKey:@"MAMEDebugDevice"] pointerValue];
	if (&device->machine() == machine)
	{
		[self setCPU:device];
		[window makeKeyAndOrderFront:self];
	}
}


- (void)auxiliaryWindowWillClose:(NSNotification *)notification {
	[auxiliaryWindows removeObjectIdenticalTo:[notification object]];
}


- (void)loadConfiguration:(util::xml::data_node const *)parentnode {
	util::xml::data_node const *node = nullptr;
	for (node = parentnode->get_child(osd::debugger::NODE_WINDOW); node; node = node->get_next_sibling(osd::debugger::NODE_WINDOW))
	{
		MAMEDebugWindowHandler *win = nil;
		switch (node->get_attribute_int(osd::debugger::ATTR_WINDOW_TYPE, -1))
		{
		case osd::debugger::WINDOW_TYPE_CONSOLE:
			[self restoreConfigurationFromNode:node];
			break;
		case osd::debugger::WINDOW_TYPE_MEMORY_VIEWER:
			win = [[MAMEMemoryViewer alloc] initWithMachine:*machine console:self];
			break;
		case osd::debugger::WINDOW_TYPE_DISASSEMBLY_VIEWER:
			win = [[MAMEDisassemblyViewer alloc] initWithMachine:*machine console:self];
			break;
		case osd::debugger::WINDOW_TYPE_ERROR_LOG_VIEWER:
			win = [[MAMEErrorLogViewer alloc] initWithMachine:*machine console:self];
			break;
		case osd::debugger::WINDOW_TYPE_POINTS_VIEWER:
			win = [[MAMEPointsViewer alloc] initWithMachine:*machine console:self];
			break;
		case osd::debugger::WINDOW_TYPE_DEVICES_VIEWER:
			win = [[MAMEDevicesViewer alloc] initWithMachine:*machine console:self];
			break;
		case osd::debugger::WINDOW_TYPE_DEVICE_INFO_VIEWER:
			{
				// FIXME: feels like a leaky abstraction, but device is needed for init
				device_t *const device = machine->root_device().subdevice(node->get_attribute_string(osd::debugger::ATTR_WINDOW_DEVICE_TAG, ":"));
				win = [[MAMEDeviceInfoViewer alloc] initWithDevice:(device ? *device : machine->root_device()) machine:*machine console:self];
			}
			break;
		default:
			break;
		}
		if (win)
		{
			[auxiliaryWindows addObject:win];
			[win restoreConfigurationFromNode:node];
			[win release];
			[win activate];
		}
	}
}


- (void)saveConfigurationToNode:(util::xml::data_node *)node {
	[super saveConfigurationToNode:node];
	node->set_attribute_int(osd::debugger::ATTR_WINDOW_TYPE, osd::debugger::WINDOW_TYPE_CONSOLE);
	util::xml::data_node *const splits = node->add_child(osd::debugger::NODE_WINDOW_SPLITS, nullptr);
	if (splits)
	{
		splits->set_attribute_float(osd::debugger::ATTR_SPLITS_CONSOLE_STATE,
									[regSplit isSubviewCollapsed:[[regSplit subviews] objectAtIndex:0]]
								  ? 0.0
								  : NSMaxX([[[regSplit subviews] objectAtIndex:0] frame]));
		splits->set_attribute_float(osd::debugger::ATTR_SPLITS_CONSOLE_DISASSEMBLY,
									[dasmSplit isSubviewCollapsed:[[dasmSplit subviews] objectAtIndex:0]]
								  ? 0.0
								  : NSMaxY([[[dasmSplit subviews] objectAtIndex:0] frame]));
	}
	[dasmView saveConfigurationToNode:node];
	[srcdbgView saveConfigurationToNode:node];
	[history saveConfigurationToNode:node];
}


- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node {
	[super restoreConfigurationFromNode:node];
	util::xml::data_node const *const splits = node->get_child(osd::debugger::NODE_WINDOW_SPLITS);
	if (splits)
	{
		[regSplit setPosition:splits->get_attribute_float(osd::debugger::ATTR_SPLITS_CONSOLE_STATE, NSMaxX([[[regSplit subviews] objectAtIndex:0] frame]))
			 ofDividerAtIndex:0];
		[dasmSplit setPosition:splits->get_attribute_float(osd::debugger::ATTR_SPLITS_CONSOLE_DISASSEMBLY, NSMaxY([[[dasmSplit subviews] objectAtIndex:0] frame]))
			  ofDividerAtIndex:0];
	}
	[dasmView restoreConfigurationFromNode:node];
	[srcdbgView restoreConfigurationFromNode:node];
	[history restoreConfigurationFromNode:node];
}



- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor {
	if (control == commandField)
		[history edit];

	return YES;
}


- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command {
	if (control == commandField) {
		if (command == @selector(cancelOperation:)) {
			[commandField setStringValue:@""];
			[history reset];
			return YES;
		} else if (command == @selector(moveUp:)) {
			NSString *hist = [history previous:[commandField stringValue]];
			if (hist != nil) {
				[commandField setStringValue:hist];
				[commandField selectText:self];
				[(NSText *)[window firstResponder] setSelectedRange:NSMakeRange([hist length], 0)];
			}
			return YES;
		} else if (command == @selector(moveDown:)) {
			NSString *hist = [history next:[commandField stringValue]];
			if (hist != nil) {
				[commandField setStringValue:hist];
				[commandField selectText:self];
				[(NSText *)[window firstResponder] setSelectedRange:NSMakeRange([hist length], 0)];
			}
			return YES;
		}
	}
	return NO;
}


- (void)windowWillClose:(NSNotification *)notification {
	if ([notification object] == window)
	{
		NSDictionary *info = [NSDictionary dictionaryWithObjectsAndKeys:[NSValue valueWithPointer:machine],
																		@"MAMEDebugMachine",
																		nil];
		[[NSNotificationCenter defaultCenter] postNotificationName:MAMEHideDebuggerNotification
															object:self
														  userInfo:info];
		machine->debugger().console().get_visible_cpu()->debug()->go();
	}
}


- (void)setDisasemblyView:(BOOL)value {
	[dasmScroll setHidden:value];
	[srcdbgContainerView setHidden:!value];
}


- (BOOL) getDisasemblyView {
	return ![dasmScroll isHidden];
}


- (void) setSourceButton:(int)index {
	[sourceButton selectItemAtIndex:index];
}


- (CGFloat)splitView:(NSSplitView *)sender constrainMinCoordinate:(CGFloat)min ofSubviewAt:(NSInteger)offs {
	return (min < 100) ? 100 : min;
}


- (CGFloat)splitView:(NSSplitView *)sender constrainMaxCoordinate:(CGFloat)max ofSubviewAt:(NSInteger)offs {
	NSSize  sz = [sender bounds].size;
	CGFloat allowed = ([sender isVertical] ? sz.width : sz.height) - 100 - [sender dividerThickness];
	return (max > allowed) ? allowed : max;
}


- (BOOL)splitView:(NSSplitView *)sender canCollapseSubview:(NSView *)subview {
	// allow registers or disassembly to be collapsed, but not console
	return [[sender subviews] indexOfObjectIdenticalTo:subview] == 0;
}


- (void)splitView:(NSSplitView *)sender resizeSubviewsWithOldSize:(NSSize)oldSize {
	// This can only deal with a single split, but that's all we use, anyway
	NSRect first, second;
	[sender adjustSubviews];
	first = [[[sender subviews] objectAtIndex:0] frame];
	second = [[[sender subviews] objectAtIndex:1] frame];
	if ([sender isVertical]) {
		if (first.size.width < 100) {
			CGFloat diff = 100 - first.size.width;
			first.size.width = 100;
			second.origin.x += diff;
			second.size.width -= diff;
		} else if (second.size.width < 100) {
			CGFloat diff = 100 - second.size.width;
			second.size.width = 100;
			second.origin.x -= diff;
			first.size.width -= diff;
		}
	} else {
		if (first.size.height < 100) {
			CGFloat diff = 100 - first.size.height;
			first.size.height = 100;
			second.origin.y += diff;
			second.size.height -= diff;
		} else if (second.size.height < 100) {
			CGFloat diff = 100 - second.size.height;
			second.size.height = 100;
			second.origin.y -= diff;
			first.size.height -= diff;
		}
	}
	[[[sender subviews] objectAtIndex:0] setFrame:first];
	[[[sender subviews] objectAtIndex:1] setFrame:second];
}


- (IBAction)sourceDebugBarChanged:(id)sender {
	[srcdbgView setSourceIndex:[sender tag]];
}


- (BOOL)validateMenuItem:(NSMenuItem *)item {
	MAMEDisassemblyView *visibleView = [dasmScroll isHidden] ? (MAMEDisassemblyView *)srcdbgView : dasmView;
	SEL const action = [item action];
	BOOL const inContextMenu = ([item menu] == [dasmView menu]);
	BOOL const haveCursor = [dasmView cursorVisible];
	BOOL const isCurrent = (machine->debugger().console().get_visible_cpu() == [dasmView source]->device());

	const debug_breakpoint *breakpoint = nullptr;
	if (haveCursor)
	{
		NSNumber *num = [visibleView selectedAddress];
		if (num)
		{
			offs_t const address = [num unsignedIntValue];
			breakpoint = [visibleView source]->device()->debug()->breakpoint_find(address);
		}
	}

	if (action == @selector(debugToggleBreakpoint:))
	{
		if (haveCursor)
		{
			if (breakpoint != nullptr)
			{
				if (inContextMenu)
					[item setTitle:@"Clear Breakpoint"];
				else
					[item setTitle:@"Clear Breakpoint at Cursor"];
			}
			else
			{
				if (inContextMenu)
					[item setTitle:@"Set Breakpoint"];
				else
					[item setTitle:@"Set Breakpoint at Cursor"];
			}
		}
		else
		{
			if (inContextMenu)
				[item setTitle:@"Toggle Breakpoint"];
			else
				[item setTitle:@"Toggle Breakpoint at Cursor"];
		}
		return haveCursor && isCurrent;
	}
	else if (action == @selector(debugToggleBreakpointEnable:))
	{
		if ((breakpoint != nullptr) && !breakpoint->enabled())
		{
			if (inContextMenu)
				[item setTitle:@"Enable Breakpoint"];
			else
				[item setTitle:@"Enable Breakpoint at Cursor"];
		}
		else
		{
			if (inContextMenu)
				[item setTitle:@"Disable Breakpoint"];
			else
				[item setTitle:@"Disable Breakpoint at Cursor"];
		}
		return (breakpoint != nullptr) && isCurrent;
	}
	else if (action == @selector(debugRunToCursor:))
	{
		return isCurrent;
	}
	else
	{
		return YES;
	}
}

@end
