// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  memoryview.m - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "memoryview.h"

#include "debug/debugcpu.h"
#include "debug/debugvw.h"


@implementation MAMEMemoryView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	NSMenu	*contextMenu;

	if (!(self = [super initWithFrame:f type:DVT_MEMORY machine:m]))
		return nil;

	contextMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"Memory"];
	[self insertActionItemsInMenu:contextMenu atIndex:0];
	[self setMenu:contextMenu];
	[contextMenu release];

	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (BOOL)validateMenuItem:(NSMenuItem *)item {
	SEL					action = [item action];
	NSInteger			tag = [item tag];
	debug_view_memory	*memview = downcast<debug_view_memory *>(view);

	if (action == @selector(showChunkSize:)) {
		[item setState:((tag == memview->bytes_per_chunk()) ? NSOnState : NSOffState)];
	} else if (action == @selector(showPhysicalAddresses:)) {
		[item setState:((tag == memview->physical()) ? NSOnState : NSOffState)];
	} else if (action == @selector(showReverseView:)) {
		[item setState:((tag == memview->reverse()) ? NSOnState : NSOffState)];
	} else if (action == @selector(showReverseViewToggle:)) {
		[item setState:(memview->reverse() ? NSOnState : NSOffState)];
	}
	return YES;
}


- (NSSize)maximumFrameSize {
	debug_view_xy			max;
	device_t				*curcpu = debug_cpu_get_visible_cpu(*machine);
	debug_view_source const	*source = view->source_for_device(curcpu);

	max.x = max.y = 0;
	for (const debug_view_source *source = view->source_list().first();
		 source != NULL;
		 source = source->next())
	{
		debug_view_xy	current;
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
	debug_view_source const *source = view->source();
	if (source != NULL)
		return [NSString stringWithUTF8String:source->name()];
	else
		return @"";
}


- (int)selectedSubviewIndex {
	debug_view_source const *source = view->source();
	if (source != NULL)
		return view->source_list().indexof(*source);
	else
		return -1;
}


- (void)selectSubviewAtIndex:(int)index {
	int const	selected = view->source_list().indexof(*view->source());
	if (selected != index) {
		view->set_source(*view->source_list().find(index));
		if ([[self window] firstResponder] != self)
			view->set_cursor_visible(false);
	}
}


- (BOOL)selectSubviewForDevice:(device_t *)device {
	debug_view_source const *const source = view->source_for_device(device);
	if (source != NULL)
	{
		if (view->source() != source)
		{
			view->set_source(*source);
			if ([[self window] firstResponder] != self)
				view->set_cursor_visible(false);
		}
		return YES;
	}
	else
	{
		return NO;
	}
}


- (BOOL)selectSubviewForSpace:(address_space *)space {
	if (space == NULL) return NO;
	debug_view_memory_source const *source = downcast<debug_view_memory_source const *>(view->first_source());
	while ((source != NULL) && (source->space() != space))
		source = downcast<debug_view_memory_source *>(source->next());
	if (source != NULL)
	{
		if (view->source() != source)
		{
			view->set_source(*source);
			if ([[self window] firstResponder] != self)
				view->set_cursor_visible(false);
		}
		return YES;
	}
	else
	{
		return NO;
	}
}


- (NSString *)expression {
	return [NSString stringWithUTF8String:downcast<debug_view_memory *>(view)->expression()];
}


- (void)setExpression:(NSString *)exp {
	downcast<debug_view_memory *>(view)->set_expression([exp UTF8String]);
}


- (debug_view_memory_source const *)source {
	return downcast<debug_view_memory_source const *>(view->source());
}


- (IBAction)showChunkSize:(id)sender {
	downcast<debug_view_memory *>(view)->set_bytes_per_chunk([sender tag]);
}


- (IBAction)showPhysicalAddresses:(id)sender {
	downcast<debug_view_memory *>(view)->set_physical([sender tag]);
}


- (IBAction)showReverseView:(id)sender {
	downcast<debug_view_memory *>(view)->set_reverse([sender tag]);
}


- (IBAction)showReverseViewToggle:(id)sender {
	downcast<debug_view_memory *>(view)->set_reverse(!downcast<debug_view_memory *>(view)->reverse());
}


- (IBAction)changeBytesPerLine:(id)sender {
	debug_view_memory *const memView = downcast<debug_view_memory *>(view);
	memView->set_chunks_per_row(memView->chunks_per_row() + [sender tag]);
}


- (void)insertActionItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index {
	NSInteger tag;
	for (tag = 1; tag <= 8; tag <<= 1) {
		NSString	*title = [NSString stringWithFormat:@"%ld-byte Chunks", (long)tag];
		NSMenuItem	*chunkItem = [menu insertItemWithTitle:title
													action:@selector(showChunkSize:)
											 keyEquivalent:[NSString stringWithFormat:@"%ld", (long)tag]
												   atIndex:index++];
		[chunkItem setTarget:self];
		[chunkItem setTag:tag];
	}

	[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];

	NSMenuItem *logicalItem = [menu insertItemWithTitle:@"Logical Addresses"
												 action:@selector(showPhysicalAddresses:)
										  keyEquivalent:@"v"
												atIndex:index++];
	[logicalItem setTarget:self];
	[logicalItem setTag:FALSE];

	NSMenuItem *physicalItem = [menu insertItemWithTitle:@"Physical Addresses"
												  action:@selector(showPhysicalAddresses:)
										   keyEquivalent:@"y"
												 atIndex:index++];
	[physicalItem setTarget:self];
	[physicalItem setTag:TRUE];

	[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];

	NSMenuItem *reverseItem = [menu insertItemWithTitle:@"Reverse View"
												 action:@selector(showReverseViewToggle:)
										  keyEquivalent:@"r"
												atIndex:index++];
	[reverseItem setTarget:self];

	[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];

	NSMenuItem *increaseItem = [menu insertItemWithTitle:@"Increase Bytes Per Line"
												  action:@selector(changeBytesPerLine:)
										   keyEquivalent:@"p"
												 atIndex:index++];
	[increaseItem setTarget:self];
	[increaseItem setTag:1];

	NSMenuItem *decreaseItem = [menu insertItemWithTitle:@"Decrease Bytes Per Line"
												  action:@selector(changeBytesPerLine:)
										   keyEquivalent:@"o"
												 atIndex:index++];
	[decreaseItem setTarget:self];
	[decreaseItem setTag:-1];

	if (index < [menu numberOfItems])
		[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
}


- (void)insertSubviewItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index {
	for (const debug_view_source *source = view->source_list().first(); source != NULL; source = source->next())
	{
		[[menu insertItemWithTitle:[NSString stringWithUTF8String:source->name()]
							action:NULL
					 keyEquivalent:@""
						   atIndex:index++] setTag:view->source_list().indexof(*source)];
	}
	if (index < [menu numberOfItems])
		[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
}

@end
