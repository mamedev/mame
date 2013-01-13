/*
    osxsupport.m - Cocoa glue to emulated deprecated old Carbon path finder functions
*/

#import <Cocoa/Cocoa.h>
#import <AvailabilityMacros.h>
#include "osxsupport.h"

// convert an NSString to a C string
#ifndef OSX_PPC
static char *StringToChar(NSString *str)
{
	const char *charstr = [str UTF8String];
	char *resstr = (char *)malloc(strlen(charstr)+1);

	strcpy(resstr, charstr);
	return resstr;
}

char *FindPrefsDir(void)
{
	char *resstr = NULL;
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSPreferencePanesDirectory, NSUserDomainMask, YES);

	if ([paths count] > 0)
	{
		resstr = StringToChar([paths objectAtIndex:0]) ;
	}
	return resstr;
}
#endif

