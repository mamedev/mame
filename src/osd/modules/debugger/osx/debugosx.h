// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugosx.h - MacOS X Cocoa debug window handling
//
//============================================================

#ifndef MAME_OSD_DEBUGGER_OSX_DEBUGOSX_H
#define MAME_OSD_DEBUGGER_OSX_DEBUGOSX_H

#pragma once

#include "../xmlconfig.h"


#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0

// make sure this comes first
#include <AvailabilityMacros.h>


#ifdef __OBJC__

// standard Cocoa headers
#import <Cocoa/Cocoa.h>


#endif // __OBJC__

#endif // MAME_OSD_DEBUGGER_OSX_DEBUGOSX_H
