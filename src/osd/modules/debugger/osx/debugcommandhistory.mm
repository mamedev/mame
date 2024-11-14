// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugcommandhistory.m - MacOS X Cocoa debug window handling
//
//============================================================

//============================================================
//  MAMEDebugView class
//============================================================

#import "debugcommandhistory.h"

#include "util/xmlfile.h"


@implementation MAMEDebugCommandHistory

+ (NSInteger)defaultLength {
	return 100;
}


- (id)init {
	if (!(self = [super init]))
		return nil;
	length = [[self class] defaultLength];
	position = -1;
	current = nil;
	history = [[NSMutableArray alloc] initWithCapacity:length];
	return self;
}


- (void)dealloc {
	if (current != nil)
		[current release];
	if (history != nil)
		[history release];
	[super dealloc];
}


- (NSInteger)length {
	return length;
}


- (void)setLength:(NSInteger)l {
	length = l;
	if ([history count] > length)
		[history removeObjectsInRange:NSMakeRange(length, [history count] - length)];
}


- (void)add:(NSString *)entry {
	if (([history count] == 0) || ![[history objectAtIndex:0] isEqualToString:entry]) {
		while ([history count] >= length)
			[history removeLastObject];
		[history insertObject:entry atIndex:0];
	}
	position = 0;
}


- (NSString *)previous:(NSString *)cur {
	if ((position + 1) < [history count]) {
		if (position < 0) {
			[current autorelease];
			current = [cur copy];
		}
		return [history objectAtIndex:++position];
	} else {
		return nil;
	}
}


- (NSString *)next:(NSString *)cur {
	if (position > 0) {
		return [history objectAtIndex:--position];
	} else if ((position == 0) && (current != nil) && ![current isEqualToString:[history objectAtIndex:0]]) {
		position--;
		return [[current retain] autorelease];
	} else {
		return nil;
	}
}


- (void)edit {
	if (position == 0)
		position--;
}

- (void)reset {
	position = -1;
	if (current != nil) {
		[current release];
		current = nil;
	}
}


- (void)clear {
	position = -1;
	if (current != nil) {
		[current release];
		current = nil;
	}
	[history removeAllObjects];
}


- (void)saveConfigurationToNode:(util::xml::data_node *)node {
	util::xml::data_node *const hist = node->add_child(osd::debugger::NODE_WINDOW_HISTORY, nullptr);
	if (hist) {
		for (NSInteger i = [history count]; 0 < i; --i)
			hist->add_child(osd::debugger::NODE_HISTORY_ITEM, [[history objectAtIndex:(i - 1)] UTF8String]);
	}
}


- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node {
	[self clear];
	util::xml::data_node const *const hist = node->get_child(osd::debugger::NODE_WINDOW_HISTORY);
	if (hist) {
		util::xml::data_node const *item = hist->get_child(osd::debugger::NODE_HISTORY_ITEM);
		while (item) {
			if (item->get_value() && *item->get_value()) {
				while ([history count] >= length)
					[history removeLastObject];
				[history insertObject:[NSString stringWithUTF8String:item->get_value()] atIndex:0];
			}
			item = item->get_next_sibling(osd::debugger::NODE_HISTORY_ITEM);
		}
	}
}

@end
