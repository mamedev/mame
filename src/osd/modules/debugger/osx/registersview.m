// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  registersview.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "registersview.h"

#include "debug/debugcpu.h"
#include "debug/debugvw.h"


@implementation MAMERegistersView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_STATE machine:m wholeLineScroll:NO]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (NSSize)maximumFrameSize {
	debug_view_xy			max;
	device_t				*curcpu = debug_cpu_get_visible_cpu(*machine);
	const debug_view_source	*source = view->source_for_device(curcpu);

	max.x = max.y = 0;
	for (const debug_view_source *source = view->source_list().first(); source != NULL; source = source->next())
	{
		debug_view_xy   current;
		view->set_source(*source);
		current = view->total_size();
		if (current.x > max.x)
			max.x = current.x;
		if (current.y > max.y)
			max.y = current.y;
	}
	view->set_source(*source);
	return NSMakeSize(max.x * fontWidth, max.y * fontHeight);
}


- (NSString *)selectedSubviewName {
	return @"";
}


- (int)selectedSubviewIndex {
	return -1;
}


- (void)selectSubviewAtIndex:(int)index {
}


- (void)selectSubviewForDevice:(device_t *)device {
	view->set_source(*view->source_for_device(device));
}

@end
