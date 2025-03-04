// license:BSD-3-Clause
// copyright-holders:tim lindner
//============================================================
//
//  srcdebugview.m - MacOS X Cocoa source code debug view handling
//
//============================================================

#include "emu.h"

#import "debugwindowhandler.h"
#import "srcdebugview.h"

#include "debug/debugvw.h"
#include "util/xmlfile.h"


@implementation MAMESrcDebugView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_SOURCE machine:m wholeLineScroll:NO]))
		return nil;
	return self;
}


- (void)insertSubviewItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index {
	const debug_view_sourcecode *dv_source = downcast<debug_view_sourcecode *>(view);
	const srcdbg_provider_base *debug_info = dv_source->srcdbg_provider();

	if (debug_info)
	{
		std::size_t num_files = debug_info->num_files();
		for (std::size_t i = 0; i < num_files; i++)
		{
			const char * entry_text = debug_info->file_index_to_path(i).built();
			NSString *title = [NSString stringWithUTF8String:entry_text];
			[[menu insertItemWithTitle:title
								action:@selector(sourceDebugBarChanged:)
						 keyEquivalent:@""
							   atIndex:index++] setTag:i];
		}
	}
}


- (void)setSourceIndex:(int)index {
 	debug_view_sourcecode *dv_source = downcast<debug_view_sourcecode *>(view);
	dv_source->set_src_index(index);
}


- (void)update {
	NSWindow *window = [self window];
	id delegate = [window delegate];

	const debug_view_sourcecode *dv_source = downcast<debug_view_sourcecode *>(view);

	[super update];
	[delegate setSourceButton:dv_source->cur_src_index()];
}

@end
