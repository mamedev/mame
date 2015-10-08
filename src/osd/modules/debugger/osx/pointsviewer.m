// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  pointsviewer.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "pointsviewer.h"

#import "breakpointsview.h"
#import "watchpointsview.h"


@implementation MAMEPointsViewer

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c {
	MAMEDebugView	*breakView, *watchView;
	NSScrollView	*breakScroll, *watchScroll;
	NSTabViewItem	*breakTab, *watchTab;
	NSPopUpButton	*actionButton, *subviewButton;
	NSRect			subviewFrame;

	if (!(self = [super initWithMachine:m title:@"(Break|Watch)points" console:c]))
		return nil;
	NSRect const contentBounds = [[window contentView] bounds];
	NSFont *const defaultFont = [[MAMEDebugView class] defaultFontForMachine:m];

	// create the subview popup
	subviewButton = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 100, 19)];
	[subviewButton setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];
	[subviewButton setBezelStyle:NSShadowlessSquareBezelStyle];
	[subviewButton setFocusRingType:NSFocusRingTypeNone];
	[subviewButton setFont:defaultFont];
	[subviewButton setTarget:self];
	[subviewButton setAction:@selector(changeSubview:)];
	[[subviewButton cell] setArrowPosition:NSPopUpArrowAtBottom];
	[[[subviewButton menu] addItemWithTitle:@"All Breakpoints"
									 action:NULL
							  keyEquivalent:@""] setTag:0];
	[[[subviewButton menu] addItemWithTitle:@"All Watchpoints"
									 action:NULL
							  keyEquivalent:@""] setTag:1];
	[subviewButton sizeToFit];
	subviewFrame = [subviewButton frame];
	subviewFrame.origin.x = subviewFrame.size.height;
	subviewFrame.origin.y = contentBounds.size.height - subviewFrame.size.height;
	subviewFrame.size.width = contentBounds.size.width - subviewFrame.size.height;
	[subviewButton setFrame:subviewFrame];
	[[window contentView] addSubview:subviewButton];
	[subviewButton release];

	// create the action popup
	actionButton = [[self class] newActionButtonWithFrame:NSMakeRect(0,
																	 subviewFrame.origin.y,
																	 subviewFrame.size.height,
																	 subviewFrame.size.height)];
	[actionButton setAutoresizingMask:(NSViewMaxXMargin | NSViewMinYMargin)];
	[actionButton setFont:[NSFont systemFontOfSize:[defaultFont pointSize]]];
	[[window contentView] addSubview:actionButton];
	[actionButton release];

	// create the breakpoints view
	breakView = [[MAMEBreakpointsView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
												   machine:*machine];
	breakScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0,
																 0,
																 contentBounds.size.width,
																 contentBounds.size.height - subviewFrame.size.height)];
	[breakScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[breakScroll setHasHorizontalScroller:YES];
	[breakScroll setHasVerticalScroller:YES];
	[breakScroll setAutohidesScrollers:YES];
	[breakScroll setBorderType:NSNoBorder];
	[breakScroll setDocumentView:breakView];
	[breakView release];
	breakTab = [[NSTabViewItem alloc] initWithIdentifier:@""];
	[breakTab setView:breakScroll];
	[breakScroll release];

	// create the breakpoints view
	watchView = [[MAMEWatchpointsView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
												   machine:*machine];
	watchScroll = [[NSScrollView alloc] initWithFrame:[breakScroll frame]];
	[watchScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[watchScroll setHasHorizontalScroller:YES];
	[watchScroll setHasVerticalScroller:YES];
	[watchScroll setAutohidesScrollers:YES];
	[watchScroll setBorderType:NSNoBorder];
	[watchScroll setDocumentView:watchView];
	[watchView release];
	watchTab = [[NSTabViewItem alloc] initWithIdentifier:@""];
	[watchTab setView:watchScroll];
	[watchScroll release];

	// create a tabless tabview for the two subviews
	tabs = [[NSTabView alloc] initWithFrame:[breakScroll frame]];
	[tabs setTabViewType:NSNoTabsNoBorder];
	[tabs setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[tabs addTabViewItem:breakTab];
	[breakTab release];
	[tabs addTabViewItem:watchTab];
	[watchTab release];
	[[window contentView] addSubview:tabs];
	[tabs release];

	// set default state
	[subviewButton selectItemAtIndex:0];
	[tabs selectFirstTabViewItem:self];
	[window makeFirstResponder:subviewButton];
	[window setTitle:[[subviewButton selectedItem] title]];

	// calculate the optimal size for everything
	NSSize const breakDesired = [NSScrollView frameSizeForContentSize:[breakView maximumFrameSize]
												hasHorizontalScroller:YES
												  hasVerticalScroller:YES
														   borderType:[breakScroll borderType]];
	NSSize const watchDesired = [NSScrollView frameSizeForContentSize:[watchView maximumFrameSize]
												hasHorizontalScroller:YES
												  hasVerticalScroller:YES
														   borderType:[watchScroll borderType]];
	NSSize const desired = NSMakeSize(MAX(breakDesired.width, watchDesired.width),
									  MAX(breakDesired.height, watchDesired.height));
	[self cascadeWindowWithDesiredSize:desired forView:tabs];

	// don't forget the result
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (IBAction)changeSubview:(id)sender {
	[tabs selectTabViewItemAtIndex:[[sender selectedItem] tag]];
	[window setTitle:[[sender selectedItem] title]];
}

@end
