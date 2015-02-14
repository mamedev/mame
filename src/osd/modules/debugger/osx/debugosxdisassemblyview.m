// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugosxdisassemblyview.m - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "debugosxdisassemblyview.h"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/debugvw.h"
#include "debug/dvdisasm.h"


@implementation MAMEDisassemblyView

- (device_debug::breakpoint *)findBreakpointAtAddress:(offs_t)address inAddressSpace:(address_space &)space {
	device_debug				*cpuinfo = space.device().debug();
	device_debug::breakpoint	*bp;
	for (bp = cpuinfo->breakpoint_first(); (bp != NULL) && (address != bp->address()); bp = bp->next()) {}
	return bp;
}

- (void)createContextMenu {
	NSMenu		*contextMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"Disassembly"];
	NSMenuItem	*item;

	item = [contextMenu addItemWithTitle:@"Toggle Breakpoint"
								  action:@selector(debugToggleBreakpoint:)
						   keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF9FunctionKey]];
	[item setKeyEquivalentModifierMask:0];
	[item setTarget:self];

	item = [contextMenu addItemWithTitle:@"Disable Breakpoint"
								  action:@selector(debugToggleBreakpointEnable:)
						   keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF9FunctionKey]];
	[item setKeyEquivalentModifierMask:NSShiftKeyMask];
	[item setTarget:self];

	[contextMenu addItem:[NSMenuItem separatorItem]];

	item = [contextMenu addItemWithTitle:@"Run to Cursor"
								  action:@selector(debugRunToCursor:)
						   keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF4FunctionKey]];
	[item setKeyEquivalentModifierMask:0];
	[item setTarget:self];

	[contextMenu addItem:[NSMenuItem separatorItem]];

	item = [contextMenu addItemWithTitle:@"Raw Opcodes"
								  action:@selector(showRightColumn:)
						   keyEquivalent:@"r"];
	[item setTarget:self];
	[item setTag:DASM_RIGHTCOL_RAW];

	item = [contextMenu addItemWithTitle:@"Encrypted Opcodes"
								  action:@selector(showRightColumn:)
						   keyEquivalent:@"e"];
	[item setTarget:self];
	[item setTag:DASM_RIGHTCOL_ENCRYPTED];

	item = [contextMenu addItemWithTitle:@"Comments"
								  action:@selector(showRightColumn:)
						   keyEquivalent:@"n"];
	[item setTarget:self];
	[item setTag:DASM_RIGHTCOL_COMMENTS];

	[self setMenu:contextMenu];
	[contextMenu release];
}


- (id)initWithFrame:(NSRect)f machine:(running_machine &)m useConsole:(BOOL)uc {
	if (!(self = [super initWithFrame:f type:DVT_DISASSEMBLY machine:m]))
		return nil;
	useConsole = uc;
	[self createContextMenu];
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (BOOL)validateMenuItem:(NSMenuItem *)item {
	SEL							action = [item action];
	BOOL						inContextMenu = ([item menu] == [self menu]);
	BOOL						haveCursor = NO, isCurrent = NO;
	device_debug::breakpoint	*breakpoint = NULL;

	if (view->cursor_visible()) {
		if (debug_cpu_get_visible_cpu(*machine) == view->source()->device()) {
			isCurrent = YES;
			if (!useConsole || isCurrent) {
				offs_t address = downcast<debug_view_disasm *>(view)->selected_address();
				haveCursor = YES;
				breakpoint = [self findBreakpointAtAddress:address inAddressSpace:downcast<const debug_view_disasm_source *>(view->source())->space()];
			}
		}
	}

	if (action == @selector(debugToggleBreakpoint:)) {
		if (haveCursor) {
			if (breakpoint != NULL) {
				if (inContextMenu)
					[item setTitle:@"Clear Breakpoint"];
				else
					[item setTitle:@"Clear Breakpoint at Cursor"];
			} else {
				if (inContextMenu)
					[item setTitle:@"Set Breakpoint"];
				else
					[item setTitle:@"Set Breakpoint at Cursor"];
			}
		} else {
			if (inContextMenu)
				[item setTitle:@"Toggle Breakpoint"];
			else
				[item setTitle:@"Toggle Breakpoint at Cursor"];
		}
		return haveCursor;
	} else if (action == @selector(debugToggleBreakpointEnable:)) {
		if ((breakpoint != NULL) && !breakpoint->enabled()) {
			if (inContextMenu)
				[item setTitle:@"Enable Breakpoint"];
			else
				[item setTitle:@"Enable Breakpoint at Cursor"];
		} else {
			if (inContextMenu)
				[item setTitle:@"Disable Breakpoint"];
			else
				[item setTitle:@"Disable Breakpoint at Cursor"];
		}
		return (breakpoint != NULL);
	} else if (action == @selector(debugRunToCursor:)) {
		return isCurrent;
	} else if (action == @selector(showRightColumn:)) {
		[item setState:((downcast<debug_view_disasm *>(view)->right_column() == [item tag]) ? NSOnState : NSOffState)];
		return YES;
	} else {
		return YES;
	}
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


- (void)selectSubviewForCPU:(device_t *)device {
	const debug_view_source     *selected = view->source();
	const debug_view_source     *source = view->source_for_device(device);
	if ( selected != source ) {
		view->set_source(*source);
		if ([[self window] firstResponder] != self)
			view->set_cursor_visible(false);
	}
}


- (NSString *)expression {
	return [NSString stringWithUTF8String:downcast<debug_view_disasm *>(view)->expression()];
}


- (void)setExpression:(NSString *)exp {
	downcast<debug_view_disasm *>(view)->set_expression([exp UTF8String]);
}


- (IBAction)debugToggleBreakpoint:(id)sender {
	if (view->cursor_visible()) {
		address_space &space = downcast<const debug_view_disasm_source *>(view->source())->space();
		if (!useConsole || (debug_cpu_get_visible_cpu(*machine) == &space.device())) {
			offs_t				address = downcast<debug_view_disasm *>(view)->selected_address();
			device_debug::breakpoint *bp = [self findBreakpointAtAddress:address inAddressSpace:space];

			// if it doesn't exist, add a new one
			if (useConsole) {
				NSString *command;
				if (bp == NULL)
					command = [NSString stringWithFormat:@"bpset %lX", (unsigned long)address];
				else
					command = [NSString stringWithFormat:@"bpclear %X", (unsigned)bp->index()];
				debug_console_execute_command(*machine, [command UTF8String], 1);
			} else {
				if (bp == NULL)
					space.device().debug()->breakpoint_set(address, NULL, NULL);
				else
					space.device().debug()->breakpoint_clear(bp->index());
			}
		}
	}
}


- (IBAction)debugToggleBreakpointEnable:(id)sender {
	if (view->cursor_visible()) {
		address_space &space = downcast<const debug_view_disasm_source *>(view->source())->space();
		if (!useConsole || (debug_cpu_get_visible_cpu(*machine) == &space.device())) {
			offs_t				address = downcast<debug_view_disasm *>(view)->selected_address();
			device_debug::breakpoint *bp = [self findBreakpointAtAddress:address inAddressSpace:space];

			if (bp != NULL) {
				NSString *command;
				if (useConsole) {
					if (bp->enabled())
						command = [NSString stringWithFormat:@"bpdisable %X", (unsigned)bp->index()];
					else
						command = [NSString stringWithFormat:@"bpenable %X", (unsigned)bp->index()];
					debug_console_execute_command(*machine, [command UTF8String], 1);
				} else {
					space.device().debug()->breakpoint_enable(bp->index(), !bp->enabled());
				}
			}
		}
	}
}


- (IBAction)debugRunToCursor:(id)sender {
	if (view->cursor_visible()) {
		address_space &space = downcast<const debug_view_disasm_source *>(view->source())->space();
		if (debug_cpu_get_visible_cpu(*machine) == &space.device()) {
			offs_t address = downcast<debug_view_disasm *>(view)->selected_address();
			if (useConsole) {
				NSString *command = [NSString stringWithFormat:@"go 0x%lX", (unsigned long)address];
				debug_console_execute_command(*machine, [command UTF8String], 1);
			} else {
				debug_cpu_get_visible_cpu(*machine)->debug()->go(address);
			}
		}
	}
}


- (IBAction)showRightColumn:(id)sender {
	downcast<debug_view_disasm *>(view)->set_right_column((disasm_right_column) [sender tag]);
}


- (void)insertActionItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index {
	{
		NSMenuItem *breakItem = [menu insertItemWithTitle:@"Toggle Breakpoint at Cursor"
												   action:@selector(debugToggleBreakpoint:)
											keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF9FunctionKey]
												  atIndex:index++];
		[breakItem setKeyEquivalentModifierMask:0];
		[breakItem setTarget:self];
	}
	{
		NSMenuItem *disableItem = [menu insertItemWithTitle:@"Disable Breakpoint at Cursor"
													 action:@selector(debugToggleBreakpointEnable:)
											  keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF9FunctionKey]
													atIndex:index++];
		[disableItem setKeyEquivalentModifierMask:NSShiftKeyMask];
		[disableItem setAlternate:YES];
		[disableItem setTarget:self];
	}
	{
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
		[runItem setTarget:self];
	}
	[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
	{
		NSMenuItem *rawItem = [menu insertItemWithTitle:@"Show Raw Opcodes"
												 action:@selector(showRightColumn:)
										  keyEquivalent:@"r"
												atIndex:index++];
		[rawItem setTarget:self];
		[rawItem setTag:DASM_RIGHTCOL_RAW];
	}
	{
		NSMenuItem *encItem = [menu insertItemWithTitle:@"Show Encrypted Opcodes"
												 action:@selector(showRightColumn:)
										  keyEquivalent:@"e"
												atIndex:index++];
		[encItem setTarget:self];
		[encItem setTag:DASM_RIGHTCOL_ENCRYPTED];
	}
	{
		NSMenuItem *commentsItem = [menu insertItemWithTitle:@"Show Comments"
													  action:@selector(showRightColumn:)
											   keyEquivalent:@"n"
													 atIndex:index++];
		[commentsItem setTarget:self];
		[commentsItem setTag:DASM_RIGHTCOL_COMMENTS];
	}
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
