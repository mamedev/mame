// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugosx.h - MacOS X Cocoa debug window handling
//
//============================================================

#ifndef __SDL_DEBUGOSX__
#define __SDL_DEBUGOSX__

// make sure this comes first
#include <AvailabilityMacros.h>


#ifdef __OBJC__

// standard Cocoa headers
#import <Cocoa/Cocoa.h>

// workarounds for 10.6 warnings
#ifdef MAC_OS_X_VERSION_MAX_ALLOWED

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1060

@protocol NSWindowDelegate <NSObject>
@end

@protocol NSSplitViewDelegate <NSObject>
@end

@protocol NSControlTextEditingDelegate <NSObject>
@end

@protocol NSTextFieldDelegate <NSControlTextEditingDelegate>
@end

@protocol NSOutlineViewDataSource <NSObject>
@end

#endif // MAC_OS_X_VERSION_MAX_ALLOWED < 1060

#endif // MAC_OS_X_VERSION_MAX_ALLOWED

#endif // __OBJC__

#endif // __SDL_DEBUGOSX__
