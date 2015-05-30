// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  errorlogviewer.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "errorlogviewer.h"

#import "errorlogview.h"


@implementation MAMEErrorLogViewer

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c {
	NSScrollView	*logScroll;
	NSString		*title;

	title = [NSString stringWithFormat:@"Error Log: %@ [%@]",
									   [NSString stringWithUTF8String:m.system().description],
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
	[logScroll setDocumentView:logView];
	[logView release];
	[window setContentView:logScroll];
	[logScroll release];

	// calculate the optimal size for everything
	{
		NSSize	desired = [NSScrollView frameSizeForContentSize:[logView maximumFrameSize]
										  hasHorizontalScroller:YES
											hasVerticalScroller:YES
													 borderType:[logScroll borderType]];

		// this thing starts with no content, so its prefered height may be very small
		desired.height = MAX(desired.height, 240);
		[self cascadeWindowWithDesiredSize:desired forView:logScroll];
	}

	// don't forget the result
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end
