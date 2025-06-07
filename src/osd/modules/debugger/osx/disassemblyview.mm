// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  disassemblyview.m - MacOS X Cocoa debug window handling
//
//============================================================

#include "emu.h"

#import "debugwindowhandler.h"
#import "disassemblyview.h"

#include "debug/debugvw.h"

#include "util/xmlfile.h"


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
	else if (action == @selector(sourceDebugChanged:))
	{
		NSWindow *window = [self window];
		id delegate = [window delegate];
		BOOL which = [delegate getDisasemblyView];
		if (which == [item tag])
		{
			[item setState:NSControlStateValueOn];
		}
		else
		{
			[item setState:NSControlStateValueOff];
		}

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
	NSMenuItem  *item;

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
	[item setKeyEquivalentModifierMask:NSEventModifierFlagShift];

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

	[menu addItem:[NSMenuItem separatorItem]];

	item = [menu addItemWithTitle:@"Show Source"
						   action:@selector(sourceDebugChanged:)
					keyEquivalent:@"u"];
	[item setTarget:self];
	[item setTag:MENU_SHOW_SOURCE];

	item = [menu addItemWithTitle:@"Show Disassembly"
						   action:@selector(sourceDebugChanged:)
					keyEquivalent:@"U"];
	[item setTarget:self];
	[item setTag:MENU_SHOW_DISASM];


}


- (NSString *)selectedSubviewName {
	const debug_view_source *source = view->source();
	if (source != nullptr)
		return [NSString stringWithUTF8String:source->name()];
	else
		return @"";
}


- (int)selectedSubviewIndex {
	const debug_view_source *source = view->source();
	if (source != nullptr)
		return view->source_index(*source);
	else
		return -1;
}


- (void)selectSubviewAtIndex:(int)index {
	const int   selected = [self selectedSubviewIndex];
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
		debug_view_disasm_source const *const source = downcast<debug_view_disasm_source const *>(ptr.get());
		if (&source->space() == space)
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
	return [NSString stringWithUTF8String:downcast<debug_view_disasm *>(view)->expression()];
}


- (void)setExpression:(NSString *)exp {
	downcast<debug_view_disasm *>(view)->set_expression([exp UTF8String]);
}


- (debug_view_disasm_source const *)source {
	return downcast<debug_view_disasm_source const *>(view->source());
}


- (NSNumber *)selectedAddress {
	std::optional<offs_t> address = downcast<debug_view_disasm *>(view)->selected_address();
	if (address.has_value())
	{
		return [NSNumber numberWithUnsignedInt:address.value()];
	}
	else
	{
		return nil;
	}
}


- (IBAction)showRightColumn:(id)sender {
	downcast<debug_view_disasm *>(view)->set_right_column((disasm_right_column) [sender tag]);
}


- (IBAction)sourceDebugChanged:(id)sender {
	if ([sender tag] == MENU_SHOW_SOURCE)
	{
//      m_codeDock->setWidget(m_srcdbgFrame);
		NSWindow *window = [self window];
		id del = [window delegate];
		[del setDisasemblyView:true];
		machine->debug_view().update_all(DVT_SOURCE);
	}
	else
	{
//      m_codeDock->setWidget(m_dasmFrame);
		NSWindow *window = [self window];
		id del = [window delegate];
		[del setDisasemblyView:false];
		machine->debug_view().update_all(DVT_DISASSEMBLY);
	}
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
	[disableItem setKeyEquivalentModifierMask:NSEventModifierFlagShift];

	NSMenu      *runMenu = [[menu itemWithTitle:@"Run"] submenu];
	NSMenuItem  *runItem;
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

	[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];

	NSMenuItem *showSource = [menu insertItemWithTitle:@"Show Source"
												action:@selector(sourceDebugChanged:)
										 keyEquivalent:@"u"
											   atIndex:index++ ];

	[showSource setTarget:self];
	[showSource setTag:MENU_SHOW_SOURCE];

	NSMenuItem *showDisasembly = [menu insertItemWithTitle:@"Show Disassembly"
													action:@selector(sourceDebugChanged:)
											 keyEquivalent:@"U"
												   atIndex:index++ ];
	[showDisasembly setTarget:self];
	[showDisasembly setTag:MENU_SHOW_DISASM];

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


- (void)saveConfigurationToNode:(util::xml::data_node *)node {
	[super saveConfigurationToNode:node];
	debug_view_disasm *const dasmView = downcast<debug_view_disasm *>(view);
	node->set_attribute_int(osd::debugger::ATTR_WINDOW_DISASSEMBLY_RIGHT_COLUMN, dasmView->right_column());
}


- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node {
	[super restoreConfigurationFromNode:node];
	debug_view_disasm *const dasmView = downcast<debug_view_disasm *>(view);
	dasmView->set_right_column((disasm_right_column)node->get_attribute_int(osd::debugger::ATTR_WINDOW_DISASSEMBLY_RIGHT_COLUMN, dasmView->right_column()));
}

@end
