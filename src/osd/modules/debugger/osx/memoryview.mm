// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  memoryview.m - MacOS X Cocoa debug window handling
//
//============================================================

#include "emu.h"
#import "memoryview.h"

#include "debug/debugvw.h"

#include "util/xmlfile.h"


@implementation MAMEMemoryView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_MEMORY machine:m wholeLineScroll:NO]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (BOOL)validateMenuItem:(NSMenuItem *)item {
	SEL                 action = [item action];
	NSInteger           tag = [item tag];
	debug_view_memory   *memview = downcast<debug_view_memory *>(view);

	if (action == @selector(showChunkSize:))
	{
		[item setState:((tag == int(memview->get_data_format())) ? NSOnState : NSOffState)];
		return YES;
	}
	else if (action == @selector(showPhysicalAddresses:))
	{
		[item setState:((tag == memview->physical()) ? NSOnState : NSOffState)];
		return YES;
	}
	else if (action == @selector(showReverseView:))
	{
		[item setState:((tag == memview->reverse()) ? NSOnState : NSOffState)];
		return YES;
	}
	else if (action == @selector(showReverseViewToggle:))
	{
		[item setState:(memview->reverse() ? NSOnState : NSOffState)];
		return YES;
	}
	else if (action == @selector(changeBytesPerLine:))
	{
		return (memview->chunks_per_row() + [item tag]) > 0;
	}
	else if (action == @selector(showAddressRadix:))
	{
		[item setState:((memview->address_radix() == [item tag]) ? NSOnState : NSOffState)];
		return YES;
	}
	else
	{
		return [super validateMenuItem:item];
	}
}


- (NSSize)maximumFrameSize {
	debug_view_xy           max(0, 0);
	debug_view_source const *source = view->source();
	for (auto &source : view->source_list())
	{
		view->set_source(*source);
		debug_view_xy const current = view->total_size();
		max.x = std::max(max.x, current.x);
		max.y = std::max(max.y, current.y);
	}
	view->set_source(*source);
	return NSMakeSize(ceil((max.x * fontWidth) + (2 * [textContainer lineFragmentPadding])),
					  ceil(max.y * fontHeight));
}


- (void)addContextMenuItemsToMenu:(NSMenu *)menu {
	[super addContextMenuItemsToMenu:menu];
	if ([menu numberOfItems] > 0)
		[menu addItem:[NSMenuItem separatorItem]];
	[self insertActionItemsInMenu:menu atIndex:[menu numberOfItems]];
}


- (NSString *)selectedSubviewName {
	debug_view_source const *source = view->source();
	if (source != nullptr)
		return [NSString stringWithUTF8String:source->name()];
	else
		return @"";
}


- (int)selectedSubviewIndex {
	debug_view_source const *source = view->source();
	if (source != nullptr)
		return view->source_index(*source);
	else
		return -1;
}


- (void)selectSubviewAtIndex:(int)index {
	int const   selected = [self selectedSubviewIndex];
	if (selected != index)
	{
		const debug_view_source *source = view->source(index);
		if (source != nullptr)
		{
			view->set_source(*source);
			if ([[self window] firstResponder] != self)
				view->set_cursor_visible(false);
		}
	}
}


- (BOOL)selectSubviewForDevice:(device_t *)device {
	debug_view_source const *const source = view->source_for_device(device);
	if (source != nullptr)
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
	if (space == nullptr) return NO;
	for (auto &ptr : view->source_list())
	{
		debug_view_memory_source const *const source = downcast<debug_view_memory_source const *>(ptr.get());
		auto [mintf, spacenum] = source->space();
		if (&mintf->space(spacenum) == space)
		{
			if (view->source() != source)
			{
				view->set_source(*source);
				if ([[self window] firstResponder] != self)
					view->set_cursor_visible(false);
			}
			return YES;
		}
	}
	return NO;
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
	downcast<debug_view_memory *>(view)->set_data_format(debug_view_memory::data_format([sender tag]));
}


- (IBAction)showPhysicalAddresses:(id)sender {
	downcast<debug_view_memory *>(view)->set_physical([sender tag]);
}

- (IBAction)showAddressRadix:(id)sender {
	downcast<debug_view_memory *>(view)->set_address_radix([sender tag]);
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


- (void)saveConfigurationToNode:(util::xml::data_node *)node {
	[super saveConfigurationToNode:node];
	debug_view_memory *const memView = downcast<debug_view_memory *>(view);
	node->set_attribute_int(osd::debugger::ATTR_WINDOW_MEMORY_REVERSE_COLUMNS, memView->reverse() ? 1 : 0);
	node->set_attribute_int(osd::debugger::ATTR_WINDOW_MEMORY_ADDRESS_MODE, memView->physical() ? 1 : 0);
	node->get_attribute_int(osd::debugger::ATTR_WINDOW_MEMORY_ADDRESS_RADIX, memView->address_radix());
	node->set_attribute_int(osd::debugger::ATTR_WINDOW_MEMORY_DATA_FORMAT, int(memView->get_data_format()));
	node->set_attribute_int(osd::debugger::ATTR_WINDOW_MEMORY_ROW_CHUNKS, memView->chunks_per_row());
}


- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node {
	[super restoreConfigurationFromNode:node];
	debug_view_memory *const memView = downcast<debug_view_memory *>(view);
	memView->set_reverse(0 != node->get_attribute_int(osd::debugger::ATTR_WINDOW_MEMORY_REVERSE_COLUMNS, memView->reverse() ? 1 : 0));
	memView->set_physical(0 != node->get_attribute_int(osd::debugger::ATTR_WINDOW_MEMORY_ADDRESS_MODE, memView->physical() ? 1 : 0));
	memView->set_address_radix(node->get_attribute_int(osd::debugger::ATTR_WINDOW_MEMORY_ADDRESS_RADIX, memView->address_radix()));
	memView->set_data_format(debug_view_memory::data_format(node->get_attribute_int(osd::debugger::ATTR_WINDOW_MEMORY_DATA_FORMAT, int(memView->get_data_format()))));
	memView->set_chunks_per_row(node->get_attribute_int(osd::debugger::ATTR_WINDOW_MEMORY_ROW_CHUNKS, memView->chunks_per_row()));
}


- (void)insertActionItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index {
	NSMenuItem  *chunkItem1 = [menu insertItemWithTitle:@"1-byte Chunks (Hex)"
												 action:@selector(showChunkSize:)
										  keyEquivalent:@"1"
												atIndex:index++];
	[chunkItem1 setTarget:self];
	[chunkItem1 setTag:int(debug_view_memory::data_format::HEX_8BIT)];

	NSMenuItem  *chunkItem2 = [menu insertItemWithTitle:@"2-byte Chunks (Hex)"
												 action:@selector(showChunkSize:)
										  keyEquivalent:@"2"
												atIndex:index++];
	[chunkItem2 setTarget:self];
	[chunkItem2 setTag:int(debug_view_memory::data_format::HEX_16BIT)];

	NSMenuItem  *chunkItem4 = [menu insertItemWithTitle:@"4-byte Chunks (Hex)"
												 action:@selector(showChunkSize:)
										  keyEquivalent:@"4"
												atIndex:index++];
	[chunkItem4 setTarget:self];
	[chunkItem4 setTag:int(debug_view_memory::data_format::HEX_32BIT)];

	NSMenuItem  *chunkItem8 = [menu insertItemWithTitle:@"8-byte Chunks (Hex)"
												 action:@selector(showChunkSize:)
										  keyEquivalent:@"8"
												atIndex:index++];
	[chunkItem8 setTarget:self];
	[chunkItem8 setTag:int(debug_view_memory::data_format::HEX_64BIT)];

	NSMenuItem  *chunkItem12 = [menu insertItemWithTitle:@"1-byte Chunks (Octal)"
												  action:@selector(showChunkSize:)
										   keyEquivalent:@"3"
												 atIndex:index++];
	[chunkItem12 setTarget:self];
	[chunkItem12 setTag:int(debug_view_memory::data_format::OCTAL_8BIT)];

	NSMenuItem  *chunkItem13 = [menu insertItemWithTitle:@"2-byte Chunks (Octal)"
												  action:@selector(showChunkSize:)
										   keyEquivalent:@"5"
												 atIndex:index++];
	[chunkItem13 setTarget:self];
	[chunkItem13 setTag:int(debug_view_memory::data_format::OCTAL_16BIT)];

	NSMenuItem  *chunkItem14 = [menu insertItemWithTitle:@"4-byte Chunks (Octal)"
												  action:@selector(showChunkSize:)
										   keyEquivalent:@"7"
												 atIndex:index++];
	[chunkItem14 setTarget:self];
	[chunkItem14 setTag:int(debug_view_memory::data_format::OCTAL_32BIT)];

	NSMenuItem  *chunkItem15 = [menu insertItemWithTitle:@"8-byte Chunks (Octal)"
												  action:@selector(showChunkSize:)
										   keyEquivalent:@"9"
												 atIndex:index++];
	[chunkItem15 setTarget:self];
	[chunkItem15 setTag:int(debug_view_memory::data_format::OCTAL_64BIT)];

	NSMenuItem  *chunkItem9 = [menu insertItemWithTitle:@"32-bit Floating Point"
												 action:@selector(showChunkSize:)
										  keyEquivalent:@"F"
												atIndex:index++];
	[chunkItem9 setTarget:self];
	[chunkItem9 setTag:int(debug_view_memory::data_format::FLOAT_32BIT)];

	NSMenuItem *chunkItem10 = [menu insertItemWithTitle:@"64-bit Floating Point"
												 action:@selector(showChunkSize:)
										  keyEquivalent:@"D"
												atIndex:index++];
	[chunkItem10 setTarget:self];
	[chunkItem10 setTag:int(debug_view_memory::data_format::FLOAT_64BIT)];

	NSMenuItem *chunkItem11 = [menu insertItemWithTitle:@"80-bit Floating Point"
												 action:@selector(showChunkSize:)
										  keyEquivalent:@"E"
												atIndex:index++];
	[chunkItem11 setTarget:self];
	[chunkItem11 setTag:int(debug_view_memory::data_format::FLOAT_80BIT)];

	[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];

	NSMenuItem *hexadecimalItem = [menu insertItemWithTitle:@"Hexadecimal Addresses"
													 action:@selector(showAddressRadix:)
											  keyEquivalent:@"H"
													atIndex:index++];
	[hexadecimalItem setTarget:self];
	[hexadecimalItem setTag:16];

	NSMenuItem *decimalItem = [menu insertItemWithTitle:@"Decimal Addresses"
												 action:@selector(showAddressRadix:)
										  keyEquivalent:@""
												atIndex:index++];
	[decimalItem setTarget:self];
	[decimalItem setTag:10];

	NSMenuItem *octalItem = [menu insertItemWithTitle:@"Octal Addresses"
											   action:@selector(showAddressRadix:)
										keyEquivalent:@"O"
											  atIndex:index++];
	[octalItem setTarget:self];
	[octalItem setTag:8];

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
	for (auto &source : view->source_list())
	{
		[[menu insertItemWithTitle:[NSString stringWithUTF8String:source->name()]
							action:NULL
					 keyEquivalent:@""
						   atIndex:index++] setTag:view->source_index(*source)];
	}
	if (index < [menu numberOfItems])
		[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
}

@end
