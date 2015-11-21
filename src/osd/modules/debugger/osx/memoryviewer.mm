// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  memoryviewer.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "memoryviewer.h"

#import "debugconsole.h"
#import "debugview.h"
#import "memoryview.h"

#include "debug/debugcpu.h"
#include "debug/dvmemory.h"


@implementation MAMEMemoryViewer

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c {
	NSScrollView	*memoryScroll;
	NSView			*expressionContainer;
	NSPopUpButton	*actionButton;
	NSRect			expressionFrame;

	if (!(self = [super initWithMachine:m title:@"Memory" console:c]))
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
	[subviewButton setBezelStyle:NSShadowlessSquareBezelStyle];
	[subviewButton setFocusRingType:NSFocusRingTypeNone];
	[subviewButton setFont:defaultFont];
	[subviewButton setTarget:self];
	[subviewButton setAction:@selector(changeSubview:)];
	[[subviewButton cell] setArrowPosition:NSPopUpArrowAtBottom];
	[subviewButton sizeToFit];

	// adjust sizes to make it fit nicely
	expressionFrame = [expressionField frame];
	expressionFrame.size.height = MAX(expressionFrame.size.height, [subviewButton frame].size.height);
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
	[[window contentView] addSubview:expressionContainer];
	[expressionContainer release];

	// create the memory view
	memoryView = [[MAMEMemoryView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
											   machine:*machine];
	[memoryView insertSubviewItemsInMenu:[subviewButton menu] atIndex:0];
	memoryScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0,
																  0,
																  contentBounds.size.width,
																  expressionFrame.origin.y)];
	[memoryScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[memoryScroll setHasHorizontalScroller:YES];
	[memoryScroll setHasVerticalScroller:YES];
	[memoryScroll setAutohidesScrollers:YES];
	[memoryScroll setBorderType:NSNoBorder];
	[memoryScroll setDocumentView:memoryView];
	[memoryView release];
	[[window contentView] addSubview:memoryScroll];
	[memoryScroll release];

	// create the action popup
	actionButton = [[self class] newActionButtonWithFrame:NSMakeRect(0,
																	 expressionFrame.origin.y,
																	 expressionFrame.size.height,
																	 expressionFrame.size.height)];
	[actionButton setAutoresizingMask:(NSViewMaxXMargin | NSViewMinYMargin)];
	[actionButton setFont:[NSFont systemFontOfSize:[defaultFont pointSize]]];
	[memoryView insertActionItemsInMenu:[actionButton menu] atIndex:1];
	[[window contentView] addSubview:actionButton];
	[actionButton release];

	// set default state
	[memoryView selectSubviewForDevice:debug_cpu_get_visible_cpu(*machine)];
	[memoryView setExpression:@"0"];
	[expressionField setStringValue:@"0"];
	[expressionField selectText:self];
	[subviewButton selectItemAtIndex:[subviewButton indexOfItemWithTag:[memoryView selectedSubviewIndex]]];
	[window makeFirstResponder:expressionField];
	[window setTitle:[NSString stringWithFormat:@"Memory: %@", [memoryView selectedSubviewName]]];

	// calculate the optimal size for everything
	NSSize const desired = [NSScrollView frameSizeForContentSize:[memoryView maximumFrameSize]
										   hasHorizontalScroller:YES
											 hasVerticalScroller:YES
													  borderType:[memoryScroll borderType]];
	[self cascadeWindowWithDesiredSize:desired forView:memoryScroll];

	// don't forget the result
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (id <MAMEDebugViewExpressionSupport>)documentView {
	return memoryView;
}


- (IBAction)debugNewMemoryWindow:(id)sender {
	debug_view_memory_source const *source = [memoryView source];
	[console debugNewMemoryWindowForSpace:source->space()
								   device:source->device()
							   expression:[memoryView expression]];
}


- (IBAction)debugNewDisassemblyWindow:(id)sender {
	debug_view_memory_source const *source = [memoryView source];
	[console debugNewDisassemblyWindowForSpace:source->space()
										device:source->device()
									expression:[memoryView expression]];
}


- (BOOL)selectSubviewForDevice:(device_t *)device {
	BOOL const result = [memoryView selectSubviewForDevice:device];
	[subviewButton selectItemAtIndex:[subviewButton indexOfItemWithTag:[memoryView selectedSubviewIndex]]];
	[window setTitle:[NSString stringWithFormat:@"Memory: %@", [memoryView selectedSubviewName]]];
	return result;
}


- (BOOL)selectSubviewForSpace:(address_space *)space {
	BOOL const result = [memoryView selectSubviewForSpace:space];
	[subviewButton selectItemAtIndex:[subviewButton indexOfItemWithTag:[memoryView selectedSubviewIndex]]];
	[window setTitle:[NSString stringWithFormat:@"Memory: %@", [memoryView selectedSubviewName]]];
	return result;
}


- (IBAction)changeSubview:(id)sender {
	[memoryView selectSubviewAtIndex:[[sender selectedItem] tag]];
	[window setTitle:[NSString stringWithFormat:@"Memory: %@", [memoryView selectedSubviewName]]];
}

@end
