// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  disassemblyviewer.m - MacOS X Cocoa debug window handling
//
//============================================================

#include "emu.h"
#import "disassemblyviewer.h"

#import "debugconsole.h"
#import "debugview.h"
#import "disassemblyview.h"
#import "srcdebugview.h"

#include "debugger.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/points.h"

#include "util/xmlfile.h"


@implementation MAMEDisassemblyViewer

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c {
	NSScrollView    *dasmScroll, *srcdbgScroll;
	NSView          *expressionContainer;
	NSPopUpButton   *actionButton;
	NSRect          expressionFrame;

	if (!(self = [super initWithMachine:m title:@"Disassembly" console:c]))
		return nil;
	NSRect const contentBounds = [[window contentView] bounds];
	NSFont *const defaultFont = [[MAMEDebugView class] defaultFontForMachine:m];

	// create the expression field
	expressionField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 100, 19)];
	[expressionField setAutoresizingMask:(NSViewWidthSizable | NSViewMaxXMargin | NSViewMinYMargin)];
	[expressionField setFont:defaultFont];
	[expressionField setFocusRingType:NSFocusRingTypeNone];
	[expressionField setTarget:self];
	[expressionField setAction:@selector(doExpression:)];
	[expressionField setDelegate:self];
	[expressionField sizeToFit];

	// create the subview popup
	subviewButton = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 100, 19)];
	[subviewButton setAutoresizingMask:(NSViewWidthSizable | NSViewMinXMargin | NSViewMinYMargin)];
	[subviewButton setBezelStyle:NSBezelStyleShadowlessSquare];
	[subviewButton setFocusRingType:NSFocusRingTypeNone];
	[subviewButton setFont:defaultFont];
	[subviewButton setTarget:self];
	[subviewButton setAction:@selector(changeSubview:)];
	[[subviewButton cell] setArrowPosition:NSPopUpArrowAtBottom];
	[subviewButton sizeToFit];

	// adjust sizes to make it fit nicely
	expressionFrame = [expressionField frame];
	expressionFrame.size.height = std::max(expressionFrame.size.height, [subviewButton frame].size.height);
	expressionFrame.size.width = (contentBounds.size.width - expressionFrame.size.height) / 2;
	[expressionField setFrame:expressionFrame];
	expressionFrame.origin.x = expressionFrame.size.width;
	expressionFrame.size.width = contentBounds.size.width - expressionFrame.size.height - expressionFrame.origin.x;
	[subviewButton setFrame:expressionFrame];

	// create a container for the expression field and subview popup
	expressionFrame = NSMakeRect(expressionFrame.size.height,
								 contentBounds.size.height - expressionFrame.size.height,
								 contentBounds.size.width - expressionFrame.size.height,
								 expressionFrame.size.height);
	expressionContainer = [[NSView alloc] initWithFrame:expressionFrame];
	[expressionContainer setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];
	[expressionContainer addSubview:expressionField];
	[expressionField release];
	[expressionContainer addSubview:subviewButton];
	[subviewButton release];

	// create the disassembly view
	dasmView = [[MAMEDisassemblyView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100) machine:*machine];
	[dasmView insertSubviewItemsInMenu:[subviewButton menu] atIndex:0];
	dasmScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0,
																0,
																contentBounds.size.width,
																expressionFrame.origin.y)];
	[dasmScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[dasmScroll setHasHorizontalScroller:YES];
	[dasmScroll setHasVerticalScroller:YES];
	[dasmScroll setAutohidesScrollers:YES];
	[dasmScroll setBorderType:NSNoBorder];
	[dasmScroll setDrawsBackground:NO];
	[dasmScroll setDocumentView:dasmView];
	[dasmView release];

	// create the action popup
	actionButton = [[self class] newActionButtonWithFrame:NSMakeRect(0,
																	 expressionFrame.origin.y,
																	 expressionFrame.size.height,
																	 expressionFrame.size.height)];
	[actionButton setAutoresizingMask:(NSViewMaxXMargin | NSViewMinYMargin)];
	[actionButton setFont:[NSFont systemFontOfSize:[defaultFont pointSize]]];
	[dasmView insertActionItemsInMenu:[actionButton menu] atIndex:1];

	// set default state
	[dasmView selectSubviewForDevice:machine->debugger().console().get_visible_cpu()];
	[dasmView setExpression:@"curpc"];
	[expressionField setStringValue:@"curpc"];
	[expressionField selectText:self];
	[subviewButton selectItemAtIndex:[subviewButton indexOfItemWithTag:[dasmView selectedSubviewIndex]]];
	[window makeFirstResponder:expressionField];
	[window setTitle:[NSString stringWithFormat:@"Disassembly: %@", [dasmView selectedSubviewName]]];

	// calculate the optimal size for everything
	NSSize const desired = [NSScrollView frameSizeForContentSize:[dasmView maximumFrameSize]
										 horizontalScrollerClass:[NSScroller class]
										   verticalScrollerClass:[NSScroller class]
													  borderType:[dasmScroll borderType]
													 controlSize:NSControlSizeRegular
												   scrollerStyle:NSScrollerStyleOverlay];

	// create enclosing disasembly group
	NSRect groupRect = NSUnionRect(NSUnionRect([expressionContainer frame], [dasmScroll frame]), [actionButton frame]);
	dissasemblyGroupView = [[NSView alloc] initWithFrame:groupRect];
	[dissasemblyGroupView addSubview:expressionContainer];
	[dissasemblyGroupView addSubview:dasmScroll];
	[dissasemblyGroupView addSubview:actionButton];
	[dissasemblyGroupView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[expressionContainer release];
	[dasmScroll release];
	[actionButton release];
	[[window contentView] addSubview:dissasemblyGroupView];
	[dissasemblyGroupView release];

	[self cascadeWindowWithDesiredSize:desired forView:dasmScroll];

	NSRect bounds = [dissasemblyGroupView bounds];

	// create the source debug view
	srcdbgView = [[MAMESrcDebugView alloc] initWithFrame:NSMakeRect(bounds.origin.x,
																	bounds.origin.y,
																	bounds.size.width,
																	bounds.size.height-22) machine:*machine];
	[srcdbgView selectSubviewForDevice:machine->debugger().console().get_visible_cpu()];
	[srcdbgView setExpression:@"curpc"];
	[srcdbgView maximumFrameSize]; // called to correctly setup source
	[srcdbgView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	srcdbgScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(bounds.origin.x,
																  bounds.origin.y,
																  bounds.size.width,
																  bounds.size.height-22)];
	[srcdbgScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[srcdbgScroll setHasHorizontalScroller:YES];
	[srcdbgScroll setHasVerticalScroller:YES];
	[srcdbgScroll setAutohidesScrollers:YES];
	[srcdbgScroll setBorderType:NSBezelBorder];
	[srcdbgScroll setDrawsBackground:NO];
	[srcdbgScroll setDocumentView:srcdbgView];
	[srcdbgView release];

	// create the source popup button
	sourceButton = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(bounds.origin.x+23,
																   bounds.origin.y+bounds.size.height-22,
																   bounds.size.width-23,
																   19)];
	[sourceButton setAutoresizingMask:(NSViewWidthSizable | NSViewMaxXMargin | NSViewMinYMargin)];
	[sourceButton setBezelStyle:NSBezelStyleShadowlessSquare];
	[sourceButton setFocusRingType:NSFocusRingTypeNone];
	[sourceButton setFont:defaultFont];
	[sourceButton setTarget:self];
	[sourceButton setAction:@selector(changeSubview:)];
	[[sourceButton cell] setArrowPosition:NSPopUpArrowAtBottom];
	[srcdbgView insertSubviewItemsInMenu:[sourceButton menu] atIndex:0];

	// create action button for source group
	actionButton = [[self class] newActionButtonWithFrame:NSMakeRect(bounds.origin.x,
																	 bounds.origin.y+bounds.size.height-22,
																	 22,
																	 22)];
	[actionButton setAutoresizingMask:(NSViewMaxXMargin | NSViewMinYMargin)];
	[actionButton setFont:[NSFont systemFontOfSize:[defaultFont pointSize]]];
	[srcdbgView insertActionItemsInMenu:[actionButton menu] atIndex:1];

	// create source container to group together the popup and debug view
	srcdbgGroupView = [[NSView alloc] initWithFrame:bounds];
	[srcdbgGroupView addSubview:srcdbgScroll];
	[srcdbgGroupView addSubview:sourceButton];
	[srcdbgGroupView addSubview:actionButton];
	[srcdbgGroupView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[srcdbgScroll release];
	[sourceButton release];
	[actionButton release];
	[srcdbgGroupView setHidden:YES];
	[[window contentView] addSubview:srcdbgGroupView];
	[srcdbgGroupView release];

	// don't forget the result
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (void)setDisasemblyView:(BOOL)value {
	[dissasemblyGroupView setHidden:value];
	[srcdbgGroupView setHidden:!value];
}


- (BOOL) getDisasemblyView {
	return ![dissasemblyGroupView isHidden];
}


- (id <MAMEDebugViewExpressionSupport>)documentView {
	return dasmView;
}


- (void) setSourceButton:(int)index {
	[sourceButton selectItemAtIndex:index];
}


- (IBAction)sourceDebugBarChanged:(id)sender {
	[srcdbgView setSourceIndex:[sender tag]];
}


- (IBAction)debugNewMemoryWindow:(id)sender {
	debug_view_disasm_source const *source = [dasmView source];
	[console debugNewMemoryWindowForSpace:&source->space()
								   device:source->device()
							   expression:nil];
}


- (IBAction)debugNewDisassemblyWindow:(id)sender {
	debug_view_disasm_source const *source = [dasmView source];
	[console debugNewDisassemblyWindowForSpace:&source->space()
										device:source->device()
									expression:[dasmView expression]];
}


- (BOOL)selectSubviewForDevice:(device_t *)device {
	BOOL const result = [dasmView selectSubviewForDevice:device];
	[subviewButton selectItemAtIndex:[subviewButton indexOfItemWithTag:[dasmView selectedSubviewIndex]]];
	[window setTitle:[NSString stringWithFormat:@"Disassembly: %@", [dasmView selectedSubviewName]]];
	return result;
}


- (BOOL)selectSubviewForSpace:(address_space *)space {
	BOOL const result = [dasmView selectSubviewForSpace:space];
	[subviewButton selectItemAtIndex:[subviewButton indexOfItemWithTag:[dasmView selectedSubviewIndex]]];
	[window setTitle:[NSString stringWithFormat:@"Disassembly: %@", [dasmView selectedSubviewName]]];
	return result;
}


- (IBAction)debugToggleBreakpoint:(id)sender {
	MAMEDisassemblyView *visibleView = [dissasemblyGroupView isHidden] ? (MAMEDisassemblyView *)srcdbgView : dasmView;
	NSNumber *num = [visibleView selectedAddress];

	if (num)
	{
		if ([visibleView cursorVisible])
		{
			device_t &device = *[visibleView source]->device();

			offs_t const address = [num unsignedIntValue];
			const debug_breakpoint *bp = device.debug()->breakpoint_find(address);

			// if it doesn't exist, add a new one
			if (bp == nullptr)
			{
				uint32_t const bpnum = device.debug()->breakpoint_set(address);
				machine->debugger().console().printf("Breakpoint %X set\n", bpnum);
			}
			else
			{
				int const bpnum = bp->index();
				device.debug()->breakpoint_clear(bpnum);
				machine->debugger().console().printf("Breakpoint %X cleared\n", (uint32_t)bpnum);
			}

			// fail to do this and the display doesn't update
			machine->debug_view().update_all();
			machine->debugger().refresh_display();
		}
	}
}


- (IBAction)debugToggleBreakpointEnable:(id)sender {
	MAMEDisassemblyView *visibleView = [dissasemblyGroupView isHidden] ? (MAMEDisassemblyView *)srcdbgView : dasmView;
	NSNumber *num = [visibleView selectedAddress];

	if (num)
	{
		if ([visibleView cursorVisible])
		{
			device_t &device = *[visibleView source]->device();

			offs_t const address = [num unsignedIntValue];
			const debug_breakpoint *bp = device.debug()->breakpoint_find(address);
			if (bp != nullptr)
			{
				device.debug()->breakpoint_enable(bp->index(), !bp->enabled());
				machine->debugger().console().printf("Breakpoint %X %s\n",
													 (uint32_t)bp->index(),
													 bp->enabled() ? "enabled" : "disabled");
				machine->debug_view().update_all();
				machine->debugger().refresh_display();
			}
		}
	}
}


- (IBAction)debugRunToCursor:(id)sender {
	MAMEDisassemblyView *visibleView = [dissasemblyGroupView isHidden] ? (MAMEDisassemblyView *)srcdbgView : dasmView;
	NSNumber *num = [visibleView selectedAddress];

	if (num)
	{
		if ([dasmView cursorVisible])
		{
			[dasmView source]->device()->debug()->go([num unsignedIntValue]);
		}
	}
}


- (IBAction)changeSubview:(id)sender {
	[dasmView selectSubviewAtIndex:[[sender selectedItem] tag]];
	[window setTitle:[NSString stringWithFormat:@"Disassembly: %@", [dasmView selectedSubviewName]]];
}


- (void)saveConfigurationToNode:(util::xml::data_node *)node {
	[super saveConfigurationToNode:node];
	node->set_attribute_int(osd::debugger::ATTR_WINDOW_TYPE, osd::debugger::WINDOW_TYPE_DISASSEMBLY_VIEWER);
	node->set_attribute_int(osd::debugger::ATTR_WINDOW_DISASSEMBLY_CPU, [dasmView selectedSubviewIndex]);
	[dasmView saveConfigurationToNode:node];
}


- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node {
	[super restoreConfigurationFromNode:node];
	int const region = node->get_attribute_int(osd::debugger::ATTR_WINDOW_DISASSEMBLY_CPU, [dasmView selectedSubviewIndex]);
	[dasmView selectSubviewAtIndex:region];
	[window setTitle:[NSString stringWithFormat:@"Disassembly: %@", [dasmView selectedSubviewName]]];
	[subviewButton selectItemAtIndex:[subviewButton indexOfItemWithTag:[dasmView selectedSubviewIndex]]];
	[dasmView restoreConfigurationFromNode:node];
}


- (BOOL)validateMenuItem:(NSMenuItem *)item {
	MAMEDisassemblyView *visibleView = [dissasemblyGroupView isHidden] ? (MAMEDisassemblyView *)srcdbgView : dasmView;
	SEL const action = [item action];
	BOOL const inContextMenu = ([item menu] == [dasmView menu]);
	BOOL const haveCursor = [visibleView cursorVisible];

	const debug_breakpoint *breakpoint = nullptr;
	if (haveCursor)
	{
		NSNumber *num = [visibleView selectedAddress];
		if (num)
		{
			breakpoint = [visibleView source]->device()->debug()->breakpoint_find([num unsignedIntValue]);
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
		return haveCursor;
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
		return breakpoint != nullptr;
	}
	else
	{
		return YES;
	}
}

@end
