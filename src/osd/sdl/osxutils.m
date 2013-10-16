// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  osxutils.m - Mac OS X utilities for SDLMAME
//
//  Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
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
