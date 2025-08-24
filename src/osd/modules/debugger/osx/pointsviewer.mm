// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  pointsviewer.m - MacOS X Cocoa debug window handling
//
//============================================================

#include "emu.h"
#import "pointsviewer.h"

#import "breakpointsview.h"
#import "exceptionpointsview.h"
#import "registerpointsview.h"
#import "watchpointsview.h"

#include "util/xmlfile.h"


@implementation MAMEPointsViewer

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c {
	MAMEDebugView   *breakView, *watchView, *registerView, *exceptionView;
	NSScrollView    *breakScroll, *watchScroll, *registerScroll, *exceptionScroll;
	NSTabViewItem   *breakTab, *watchTab, *registerTab, *exceptionTab;
	NSPopUpButton   *actionButton;
	NSRect          subviewFrame;

	if (!(self = [super initWithMachine:m title:@"(Break|Watch)points" console:c]))
		return nil;
	NSRect const contentBounds = [[window contentView] bounds];
	NSFont *const defaultFont = [[MAMEDebugView class] defaultFontForMachine:m];

	// create the subview popup
	subviewButton = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 100, 19)];
	[subviewButton setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];
	[subviewButton setBezelStyle:NSBezelStyleShadowlessSquare];
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
	[[[subviewButton menu] addItemWithTitle:@"All Registerpoints"
									 action:NULL
							  keyEquivalent:@""] setTag:2];
	[[[subviewButton menu] addItemWithTitle:@"All Exceptionpoints"
									 action:NULL
							  keyEquivalent:@""] setTag:3];
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
	[breakScroll setDrawsBackground:NO];
	[breakScroll setDocumentView:breakView];
	[breakView release];
	breakTab = [[NSTabViewItem alloc] initWithIdentifier:@""];
	[breakTab setView:breakScroll];
	[breakScroll release];

	// create the watchpoints view
	watchView = [[MAMEWatchpointsView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
												   machine:*machine];
	watchScroll = [[NSScrollView alloc] initWithFrame:[breakScroll frame]];
	[watchScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[watchScroll setHasHorizontalScroller:YES];
	[watchScroll setHasVerticalScroller:YES];
	[watchScroll setAutohidesScrollers:YES];
	[watchScroll setBorderType:NSNoBorder];
	[watchScroll setDrawsBackground:NO];
	[watchScroll setDocumentView:watchView];
	[watchView release];
	watchTab = [[NSTabViewItem alloc] initWithIdentifier:@""];
	[watchTab setView:watchScroll];
	[watchScroll release];

	// create the registerpoints view
	registerView = [[MAMERegisterpointsView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
														 machine:*machine];
	registerScroll = [[NSScrollView alloc] initWithFrame:[breakScroll frame]];
	[registerScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[registerScroll setHasHorizontalScroller:YES];
	[registerScroll setHasVerticalScroller:YES];
	[registerScroll setAutohidesScrollers:YES];
	[registerScroll setBorderType:NSNoBorder];
	[registerScroll setDrawsBackground:NO];
	[registerScroll setDocumentView:registerView];
	[registerView release];
	registerTab = [[NSTabViewItem alloc] initWithIdentifier:@""];
	[registerTab setView:registerScroll];
	[registerScroll release];

	// create the exceptionpoints view
	exceptionView = [[MAMEExceptionpointsView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
														   machine:*machine];
	exceptionScroll = [[NSScrollView alloc] initWithFrame:[breakScroll frame]];
	[exceptionScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[exceptionScroll setHasHorizontalScroller:YES];
	[exceptionScroll setHasVerticalScroller:YES];
	[exceptionScroll setAutohidesScrollers:YES];
	[exceptionScroll setBorderType:NSNoBorder];
	[exceptionScroll setDrawsBackground:NO];
	[exceptionScroll setDocumentView:exceptionView];
	[exceptionView release];
	exceptionTab = [[NSTabViewItem alloc] initWithIdentifier:@""];
	[exceptionTab setView:exceptionScroll];
	[exceptionScroll release];

	// create a tabless tabview for the four subviews
	tabs = [[NSTabView alloc] initWithFrame:[breakScroll frame]];
	[tabs setTabViewType:NSNoTabsNoBorder];
	[tabs setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[tabs addTabViewItem:breakTab];
	[breakTab release];
	[tabs addTabViewItem:watchTab];
	[watchTab release];
	[tabs addTabViewItem:registerTab];
	[registerTab release];
	[tabs addTabViewItem:exceptionTab];
	[exceptionTab release];
	[[window contentView] addSubview:tabs];
	[tabs release];

	// set default state
	[subviewButton selectItemAtIndex:0];
	[tabs selectFirstTabViewItem:self];
	[window makeFirstResponder:subviewButton];
	[window setTitle:[[subviewButton selectedItem] title]];

	// calculate the optimal size for everything
	NSSize const breakDesired = [NSScrollView frameSizeForContentSize:[breakView maximumFrameSize]
											  horizontalScrollerClass:[NSScroller class]
												verticalScrollerClass:[NSScroller class]
														   borderType:[breakScroll borderType]
														  controlSize:NSControlSizeRegular
														scrollerStyle:NSScrollerStyleOverlay];
	NSSize const watchDesired = [NSScrollView frameSizeForContentSize:[watchView maximumFrameSize]
											  horizontalScrollerClass:[NSScroller class]
												verticalScrollerClass:[NSScroller class]
														   borderType:[watchScroll borderType]
														  controlSize:NSControlSizeRegular
														scrollerStyle:NSScrollerStyleOverlay];
	NSSize const registerDesired = [NSScrollView frameSizeForContentSize:[registerView maximumFrameSize]
												 horizontalScrollerClass:[NSScroller class]
												   verticalScrollerClass:[NSScroller class]
															  borderType:[registerScroll borderType]
															 controlSize:NSControlSizeRegular
														   scrollerStyle:NSScrollerStyleOverlay];
	NSSize const exceptionDesired = [NSScrollView frameSizeForContentSize:[exceptionView maximumFrameSize]
												  horizontalScrollerClass:[NSScroller class]
													verticalScrollerClass:[NSScroller class]
															   borderType:[exceptionScroll borderType]
															  controlSize:NSControlSizeRegular
															scrollerStyle:NSScrollerStyleOverlay];
	NSSize const desired = NSMakeSize(std::max({ breakDesired.width, watchDesired.width, registerDesired.width, exceptionDesired.width }),
									  std::max({ breakDesired.height, watchDesired.height, registerDesired.height, exceptionDesired.height }));
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


- (void)saveConfigurationToNode:(util::xml::data_node *)node {
	[super saveConfigurationToNode:node];
	node->set_attribute_int(osd::debugger::ATTR_WINDOW_TYPE, osd::debugger::WINDOW_TYPE_POINTS_VIEWER);
	node->set_attribute_int(osd::debugger::ATTR_WINDOW_POINTS_TYPE, [tabs indexOfTabViewItem:[tabs selectedTabViewItem]]);
}


- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node {
	[super restoreConfigurationFromNode:node];
	int const tab = node->get_attribute_int(osd::debugger::ATTR_WINDOW_POINTS_TYPE, [tabs indexOfTabViewItem:[tabs selectedTabViewItem]]);
	if ((0 <= tab) && ([tabs numberOfTabViewItems] > tab))
	{
		[subviewButton selectItemAtIndex:tab];
		[self changeSubview:subviewButton];
	}
}

@end
