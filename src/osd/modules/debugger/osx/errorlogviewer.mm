// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  errorlogviewer.m - MacOS X Cocoa debug window handling
//
//============================================================

#include "emu.h"
#import "errorlogviewer.h"

#import "errorlogview.h"

#include "util/xmlfile.h"


@implementation MAMEErrorLogViewer

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c {
	NSScrollView    *logScroll;
	NSString        *title;

	title = [NSString stringWithFormat:@"Error Log: %@ [%@]",
									   [NSString stringWithUTF8String:m.system().type.fullname()],
									   [NSString stringWithUTF8String:m.system().name]];
	if (!(self = [super initWithMachine:m title:title console:c]))
		return nil;

	// create the error log view
	logView = [[MAMEErrorLogView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100) machine:*machine];
	logScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[logScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[logScroll setHasHorizontalScroller:YES];
	[logScroll setHasVerticalScroller:YES];
	[logScroll setAutohidesScrollers:YES];
	[logScroll setBorderType:NSNoBorder];
	[logScroll setDrawsBackground:NO];
	[logScroll setDocumentView:logView];
	[logView release];
	[window setContentView:logScroll];
	[logScroll release];

	// calculate the optimal size for everything
	{
		NSSize  desired = [NSScrollView frameSizeForContentSize:[logView maximumFrameSize]
										  hasHorizontalScroller:YES
											hasVerticalScroller:YES
													 borderType:[logScroll borderType]];

		// this thing starts with no content, so its prefered height may be very small
		desired.height = std::max(desired.height, CGFloat(240));
		[self cascadeWindowWithDesiredSize:desired forView:logScroll];
	}

	// don't forget the result
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (void)saveConfigurationToNode:(util::xml::data_node *)node {
	[super saveConfigurationToNode:node];
	node->set_attribute_int("type", MAME_DEBUGGER_WINDOW_TYPE_ERROR_LOG_VIEWER);
}

@end
