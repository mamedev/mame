// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  disassemblyview.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "disassemblyview.h"

#include "debug/debugvw.h"


@implementation MAMEDisassemblyView

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f type:DVT_DISASSEMBLY machine:m wholeLineScroll:NO]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (BOOL)validateMenuItem:(NSMenuItem *)item {
	SEL const action = [item action];

	if (action == @selector(showRightColumn:))
	{
		[item setState:((downcast<debug_view_disasm *>(view)->right_column() == [item tag]) ? NSOnState : NSOffState)];
		return YES;
	}
	else
	{
		return [super validateMenuItem:item];
	}
}


- (NSSize)maximumFrameSize {
	debug_view_xy			max(0, 0);
	debug_view_source const	*source = view->source();
	for (debug_view_source const *source = view->first_source(); source != NULL; source = source->next())
	{
		view->set_source(*source);
		debug_view_xy const current = view->total_size();
		max.x = MAX(max.x, current.x);
		max.y = MAX(max.y, current.y);
	}
	view->set_source(*source);
	return NSMakeSize(ceil((max.x * fontWidth) + (2 * [textContainer lineFragmentPadding])),
					  ceil(max.y * fontHeight));
}


- (void)addContextMenuItemsToMenu:(NSMenu *)menu {
	NSMenuItem	*item;

	[super addContextMenuItemsToMenu:menu];

	if ([menu numberOfItems] > 0)
		[menu addItem:[NSMenuItem separatorItem]];

	item = [menu addItemWithTitle:@"Toggle Breakpoint"
						   action:@selector(debugToggleBreakpoint:)
					keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF9FunctionKey]];
	[item setKeyEquivalentModifierMask:0];

	item = [menu addItemWithTitle:@"Disable Breakpoint"
						   action:@selector(debugToggleBreakpointEnable:)
					keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF9FunctionKey]];
	[item setKeyEquivalentModifierMask:NSShiftKeyMask];

	[menu addItem:[NSMenuItem separatorItem]];

	item = [menu addItemWithTitle:@"Run to Cursor"
						   action:@selector(debugRunToCursor:)
					keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF4FunctionKey]];
	[item setKeyEquivalentModifierMask:0];

	[menu addItem:[NSMenuItem separatorItem]];

	item = [menu addItemWithTitle:@"Raw Opcodes"
						   action:@selector(showRightColumn:)
					keyEquivalent:@"r"];
	[item setTarget:self];
	[item setTag:DASM_RIGHTCOL_RAW];

	item = [menu addItemWithTitle:@"Encrypted Opcodes"
						   action:@selector(showRightColumn:)
					keyEquivalent:@"e"];
	[item setTarget:self];
	[item setTag:DASM_RIGHTCOL_ENCRYPTED];

	item = [menu addItemWithTitle:@"Comments"
						   action:@selector(showRightColumn:)
					keyEquivalent:@"n"];
	[item setTarget:self];
	[item setTag:DASM_RIGHTCOL_COMMENTS];
}


- (NSString *)selectedSubviewName {
	const debug_view_source *source = view->source();
	if (source != NULL)
		return [NSString stringWithUTF8String:source->name()];
	else
		return @"";
}


- (int)selectedSubviewIndex {
	const debug_view_source *source = view->source();
	if (source != NULL)
		return view->source_list().indexof(*source);
	else
		return -1;
}


- (void)selectSubviewAtIndex:(int)index {
	const int	selected = view->source_list().indexof(*view->source());
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
	debug_view_disasm_source const *source = downcast<debug_view_disasm_source const *>(view->first_source());
	while ((source != NULL) && (&source->space() != space))
		source = downcast<debug_view_disasm_source *>(source->next());
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
	return [NSString stringWithUTF8String:downcast<debug_view_disasm *>(view)->expression()];
}


- (void)setExpression:(NSString *)exp {
	downcast<debug_view_disasm *>(view)->set_expression([exp UTF8String]);
}


- (debug_view_disasm_source const *)source {
	return downcast<debug_view_disasm_source const *>(view->source());
}


- (offs_t)selectedAddress {
	return downcast<debug_view_disasm *>(view)->selected_address();
}


- (IBAction)showRightColumn:(id)sender {
	downcast<debug_view_disasm *>(view)->set_right_column((disasm_right_column) [sender tag]);
}


- (void)insertActionItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index {
	NSMenuItem *breakItem = [menu insertItemWithTitle:@"Toggle Breakpoint at Cursor"
											   action:@selector(debugToggleBreakpoint:)
										keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF9FunctionKey]
											  atIndex:index++];
	[breakItem setKeyEquivalentModifierMask:0];

	NSMenuItem *disableItem = [menu insertItemWithTitle:@"Disable Breakpoint at Cursor"
												 action:@selector(debugToggleBreakpointEnable:)
										  keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF9FunctionKey]
												atIndex:index++];
	[disableItem setKeyEquivalentModifierMask:NSShiftKeyMask];

	NSMenu		*runMenu = [[menu itemWithTitle:@"Run"] submenu];
	NSMenuItem	*runItem;
	if (runMenu != nil) {
		runItem = [runMenu addItemWithTitle:@"to Cursor"
									 action:@selector(debugRunToCursor:)
							  keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF4FunctionKey]];
	} else {
		runItem = [menu insertItemWithTitle:@"Run to Cursor"
									 action:@selector(debugRunToCursor:)
							  keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF4FunctionKey]
									atIndex:index++];
	}
	[runItem setKeyEquivalentModifierMask:0];

	[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];

	NSMenuItem *rawItem = [menu insertItemWithTitle:@"Show Raw Opcodes"
											 action:@selector(showRightColumn:)
									  keyEquivalent:@"r"
											atIndex:index++];
	[rawItem setTarget:self];
	[rawItem setTag:DASM_RIGHTCOL_RAW];

	NSMenuItem *encItem = [menu insertItemWithTitle:@"Show Encrypted Opcodes"
											 action:@selector(showRightColumn:)
									  keyEquivalent:@"e"
											atIndex:index++];
	[encItem setTarget:self];
	[encItem setTag:DASM_RIGHTCOL_ENCRYPTED];

	NSMenuItem *commentsItem = [menu insertItemWithTitle:@"Show Comments"
												  action:@selector(showRightColumn:)
										   keyEquivalent:@"n"
												 atIndex:index++];
	[commentsItem setTarget:self];
	[commentsItem setTag:DASM_RIGHTCOL_COMMENTS];

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
