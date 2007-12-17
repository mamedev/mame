//============================================================
//
//  minisync.c - Minimal core synchronization functions
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#include "osdcore.h"


//============================================================
//  osd_lock_alloc
//============================================================

osd_lock *osd_lock_alloc(void)
{
	// the minimal implementation does not support threading
	// just return a dummy value here
	return (osd_lock *)1;
}


//============================================================
//  osd_lock_acquire
//============================================================

void osd_lock_acquire(osd_lock *lock)
{
	// the minimal implementation does not support threading
	// the acquire always "succeeds"
}


//============================================================
//  osd_lock_try
//============================================================

int osd_lock_try(osd_lock *lock)
{
	// the minimal implementation does not support threading
	// the acquire always "succeeds"
	return TRUE;
}


//============================================================
//  osd_lock_release
//============================================================

void osd_lock_release(osd_lock *lock)
{
	// the minimal implementation does not support threading
	// do nothing here
}


//============================================================
//  osd_lock_free
//============================================================

void osd_lock_free(osd_lock *lock)
{
	// the minimal implementation does not support threading
	// do nothing here
}
