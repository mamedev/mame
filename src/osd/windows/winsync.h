//============================================================
//
//  winsync.h - Windows core synchronization functions
//
//  Copyright (c) 1996-2014, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __WINSYNC__
#define __WINSYNC__

//============================================================
//  Scalable Locks
//============================================================

struct osd_scalable_lock;

osd_scalable_lock *osd_scalable_lock_alloc(void);

INT32 osd_scalable_lock_acquire(osd_scalable_lock *lock);

void osd_scalable_lock_release(osd_scalable_lock *lock, INT32 myslot);

void osd_scalable_lock_free(osd_scalable_lock *lock);

#endif  /* __WINSYNC__ */