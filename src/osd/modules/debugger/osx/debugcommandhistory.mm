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
		[history insertObject:entry atIndex:0];
		while ([history count] > length)
			[history removeLastObject];
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

@end
