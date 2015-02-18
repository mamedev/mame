// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  disassemblyview.m - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "disassemblyview.h"

#include "debugger.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/debugvw.h"


@implementation MAMEDisassemblyView

- (device_debug::breakpoint *)findBreakpointAtAddress:(offs_t)address forDevice:(device_t &)device {
	device_debug				*cpuinfo = device.debug();
	device_debug::breakpoint	*bp;
	for (bp = cpuinfo->breakpoint_first(); (bp != NULL) && (address != bp->address()); bp = bp->next()) { }
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
	SEL const action = [item action];
	BOOL const inContextMenu = ([item menu] == [self menu]);
	BOOL haveCursor = view->cursor_visible();
	BOOL const isCurrent = (debug_cpu_get_visible_cpu(*machine) == view->source()->device());

	device_debug::breakpoint *breakpoint = NULL;
	if (haveCursor)
	{
		offs_t const address = downcast<debug_view_disasm *>(view)->selected_address();
		breakpoint = [self findBreakpointAtAddress:address forDevice:[self source]->device()];
	}

	if (action == @selector(debugToggleBreakpoint:))
	{
		if (haveCursor)
		{
			if (breakpoint != NULL)
			{
				if (inContextMenu)
					[item setTitle:@"Clear Breakpoint"];
				else
					[item setTitle:@"Clear Breakpoint at Cursor"];
			}
			else
			{
				if (inContextMenu)
					[item setTitle:@"Set Breakpoint"];
				else
					[item setTitle:@"Set Breakpoint at Cursor"];
			}
		}
		else
		{
			if (inContextMenu)
				[item setTitle:@"Toggle Breakpoint"];
			else
				[item setTitle:@"Toggle Breakpoint at Cursor"];
		}
		return haveCursor && (!useConsole || isCurrent);
	}
	else if (action == @selector(debugToggleBreakpointEnable:))
	{
		if ((breakpoint != NULL) && !breakpoint->enabled())
		{
			if (inContextMenu)
				[item setTitle:@"Enable Breakpoint"];
			else
				[item setTitle:@"Enable Breakpoint at Cursor"];
		}
		else
		{
			if (inContextMenu)
				[item setTitle:@"Disable Breakpoint"];
			else
				[item setTitle:@"Disable Breakpoint at Cursor"];
		}
		return (breakpoint != NULL) && (!useConsole || isCurrent);
	}
	else if (action == @selector(debugRunToCursor:))
	{
		return !useConsole || isCurrent;
	}
	else if (action == @selector(showRightColumn:))
	{
		[item setState:((downcast<debug_view_disasm *>(view)->right_column() == [item tag]) ? NSOnState : NSOffState)];
		return YES;
	}
	else
	{
		return YES;
	}
}


- (NSSize)maximumFrameSize {
	debug_view_xy			max(0, 0);
	const debug_view_source	*source = view->source();

	for (const debug_view_source *source = view->source_list().first(); source != NULL; source = source->next())
	{
		debug_view_xy current;
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


- (IBAction)debugToggleBreakpoint:(id)sender {
	if (view->cursor_visible())
	{
		device_t &device = [self source]->device();
		if (!useConsole || (debug_cpu_get_visible_cpu(*machine) == &device))
		{
			offs_t const address = downcast<debug_view_disasm *>(view)->selected_address();
			device_debug::breakpoint *bp = [self findBreakpointAtAddress:address forDevice:device];

			// if it doesn't exist, add a new one
			if (useConsole)
			{
				NSString *command;
				if (bp == NULL)
					command = [NSString stringWithFormat:@"bpset %lX", (unsigned long)address];
				else
					command = [NSString stringWithFormat:@"bpclear %X", (unsigned)bp->index()];
				debug_console_execute_command(*machine, [command UTF8String], 1);
			}
			else
			{
				if (bp == NULL)
				{
					UINT32 const bpnum = device.debug()->breakpoint_set(address, NULL, NULL);
					debug_console_printf(*machine, "Breakpoint %X set\n", bpnum);
				}
				else
				{
					int const bpnum = bp->index();
					device.debug()->breakpoint_clear(bpnum);
					debug_console_printf(*machine, "Breakpoint %X cleared\n", (UINT32)bpnum);
				}
			}

			// fail to do this and the display doesn't update
			machine->debug_view().update_all();
			debugger_refresh_display(*machine);
		}
	}
}


- (IBAction)debugToggleBreakpointEnable:(id)sender {
	if (view->cursor_visible())
	{
		device_t &device = [self source]->device();
		if (!useConsole || (debug_cpu_get_visible_cpu(*machine) == &device))
		{
			offs_t const address = downcast<debug_view_disasm *>(view)->selected_address();
			device_debug::breakpoint *bp = [self findBreakpointAtAddress:address forDevice:device];
			if (bp != NULL)
			{
				if (useConsole)
				{
					NSString *command;
					if (bp->enabled())
						command = [NSString stringWithFormat:@"bpdisable %X", (unsigned)bp->index()];
					else
						command = [NSString stringWithFormat:@"bpenable %X", (unsigned)bp->index()];
					debug_console_execute_command(*machine, [command UTF8String], 1);
				}
				else
				{
					device.debug()->breakpoint_enable(bp->index(), !bp->enabled());
					debug_console_printf(*machine,
										 "Breakpoint %X %s\n",
										 (UINT32)bp->index(),
										 bp->enabled() ? "enabled" : "disabled");
				}
				machine->debug_view().update_all();
				debugger_refresh_display(*machine);
			}
		}
	}
}


- (IBAction)debugRunToCursor:(id)sender {
	if (view->cursor_visible())
	{
		device_t &device = [self source]->device();
		if (!useConsole || (debug_cpu_get_visible_cpu(*machine) == &device))
		{
			offs_t const address = downcast<debug_view_disasm *>(view)->selected_address();
			if (useConsole)
			{
				NSString *command = [NSString stringWithFormat:@"go 0x%lX", (unsigned long)address];
				debug_console_execute_command(*machine, [command UTF8String], 1);
			}
			else
			{
				device.debug()->go(address);
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
