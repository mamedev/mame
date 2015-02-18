// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugcommandhistory.h - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "debugosx.h"

#import <Cocoa/Cocoa.h>


@interface MAMEDebugCommandHistory : NSObject
{
	NSInteger		length, position;
	NSString		*current;
	NSMutableArray	*history;
}

+ (NSInteger)defaultLength;

- (id)init;

- (NSInteger)length;
- (void)setLength:(NSInteger)l;

- (void)add:(NSString *)entry;
- (NSString *)previous:(NSString *)cur;
- (NSString *)next:(NSString *)cur;
- (void)edit;
- (void)reset;
- (void)clear;

@end
