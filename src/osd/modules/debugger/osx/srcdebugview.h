// license:BSD-3-Clause
// copyright-holders:tim lindner
//============================================================
//
//  srcdebugview.h - MacOS X Cocoa source code debug view handling
//
//============================================================

#import "disassemblyview.h"
#include "debug/dvsourcecode.h"

@interface MAMESrcDebugView : MAMEDisassemblyView
{
}

- (id)initWithFrame:(NSRect)f machine:(running_machine &)m;
- (void)setSourceIndex:(int)index;
- (void)insertSubviewItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index;
- (void)update;
@end
