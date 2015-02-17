// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  pointsviewer.m - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
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
	NSRect			contentBounds;

	if (!(self = [super initWithMachine:m title:@"(Break|Watch)points" console:c]))
		return nil;
	contentBounds = [[window contentView] bounds];

	// create the subview popup
	subviewButton = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(19,
																	contentBounds.size.height - 19,
																	contentBounds.size.width - 19,
																	19)];
	[subviewButton setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];
	[subviewButton setBezelStyle:NSShadowlessSquareBezelStyle];
	[subviewButton setFocusRingType:NSFocusRingTypeNone];
	[subviewButton setFont:[[MAMEDebugView class] defaultFont]];
	[subviewButton setTarget:self];
	[subviewButton setAction:@selector(changeSubview:)];
	[[subviewButton cell] setArrowPosition:NSPopUpArrowAtBottom];
	[[[subviewButton menu] addItemWithTitle:@"All Breakpoints"
									 action:NULL
							  keyEquivalent:@""] setTag:0];
	[[[subviewButton menu] addItemWithTitle:@"All Watchpoints"
									 action:NULL
							  keyEquivalent:@""] setTag:1];
	[[window contentView] addSubview:subviewButton];
	[subviewButton release];

	// create the action popup
	actionButton = [[self class] newActionButtonWithFrame:NSMakeRect(0,
																	 contentBounds.size.height - 19,
																	 19,
																	 19)];
	[actionButton setAutoresizingMask:(NSViewMaxXMargin | NSViewMinYMargin)];
	[[window contentView] addSubview:actionButton];
	[actionButton release];

	// create the breakpoints view
	breakView = [[MAMEBreakpointsView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
												   machine:*machine];
	breakScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0,
																 0,
																 contentBounds.size.width,
																 contentBounds.size.height - 19)];
	[breakScroll setDrawsBackground:YES];
	[breakScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[breakScroll setHasHorizontalScroller:YES];
	[breakScroll setHasVerticalScroller:YES];
	[breakScroll setAutohidesScrollers:YES];
	[breakScroll setBorderType:NSNoBorder];
	[breakScroll setDocumentView:breakView];
	[breakView release];
	breakTab = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[breakTab setView:breakScroll];
	[breakScroll release];

	// create the breakpoints view
	watchView = [[MAMEWatchpointsView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
												   machine:*machine];
	watchScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0,
																 0,
																 contentBounds.size.width,
																 contentBounds.size.height - 19)];
	[watchScroll setDrawsBackground:YES];
	[watchScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[watchScroll setHasHorizontalScroller:YES];
	[watchScroll setHasVerticalScroller:YES];
	[watchScroll setAutohidesScrollers:YES];
	[watchScroll setBorderType:NSNoBorder];
	[watchScroll setDocumentView:watchView];
	[watchView release];
	watchTab = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[watchTab setView:watchScroll];
	[watchScroll release];

	// create a tabless tabview for the two subviews
	tabs = [[NSTabView alloc] initWithFrame:NSMakeRect(0,
													   0,
													   contentBounds.size.width,
													   contentBounds.size.height - 19)];
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
