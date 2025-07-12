// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  exceptionpointsview.h - MacOS X Cocoa debug window handling
//
//============================================================

#import "debugosx.h"

#import "debugview.h"


#import <Cocoa/Cocoa.h>


@interface MAMEExceptionpointsView : MAMEDebugView
{
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m;

@end
