// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  registerpointsview.h - MacOS X Cocoa debug window handling
//
//============================================================

#import "debugosx.h"

#import "debugview.h"


#import <Cocoa/Cocoa.h>


@interface MAMERegisterpointsView : MAMEDebugView
{
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m;

@end
