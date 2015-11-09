// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  osxutils.m - Mac OS X utilities for SDLMAME
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#import <Foundation/Foundation.h>

// MAMEOS headers
#import "osxutils.h"


//============================================================
//  NewAutoreleasePool
//============================================================

void * NewAutoreleasePool(void)
{
	return [[NSAutoreleasePool alloc] init];
}


//============================================================
//  ReleaseAutoreleasePool
//============================================================

void ReleaseAutoreleasePool(void *pool)
{
	[(NSAutoreleasePool *)pool release];
}
