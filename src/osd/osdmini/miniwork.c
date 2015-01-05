// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  miniwork.c - Minimal core work item functions
//
//============================================================

#include "osdcore.h"
#include <stdlib.h>


//============================================================
//  TYPE DEFINITIONS
//============================================================

struct osd_work_item
{
	void *result;
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

int osd_num_processors = 0;

//============================================================
//  osd_work_queue_alloc
//============================================================

osd_work_queue *osd_work_queue_alloc(int flags)
{
	// this minimal implementation doesn't need to keep any state
	// so we just return a non-NULL pointer
	return (osd_work_queue *)1;
}


//============================================================
//  osd_work_queue_items
//============================================================

int osd_work_queue_items(osd_work_queue *queue)
{
	// we never have pending items
	return 0;
}


//============================================================
//  osd_work_queue_wait
//============================================================

int osd_work_queue_wait(osd_work_queue *queue, osd_ticks_t timeout)
{
	// never anything to wait for, so do nothing
	return TRUE;
}


//============================================================
//  osd_work_queue_free
//============================================================

void osd_work_queue_free(osd_work_queue *queue)
{
	// never allocated anything, so nothing to do
}


//============================================================
//  osd_work_item_queue
//============================================================

osd_work_item *osd_work_item_queue_multiple(osd_work_queue *queue, osd_work_callback callback, INT32 numitems, void *parambase, INT32 paramstep, UINT32 flags)
{
	osd_work_item *item;
	int itemnum;

	// allocate memory to hold the result
	item = (osd_work_item *)malloc(sizeof(*item));
	if (item == NULL)
		return NULL;

	// loop over all requested items
	for (itemnum = 0; itemnum < numitems; itemnum++)
	{
		// execute the call directly
		item->result = (*callback)(parambase, 0);

		// advance the param
		parambase = (UINT8 *)parambase + paramstep;
	}

	// free the item if requested
	if (flags & WORK_ITEM_FLAG_AUTO_RELEASE)
	{
		free(item);
		item = NULL;
	}
	return item;
}


//============================================================
//  osd_work_item_wait
//============================================================

int osd_work_item_wait(osd_work_item *item, osd_ticks_t timeout)
{
	// never anything to wait for, so do nothing
	return TRUE;
}


//============================================================
//  osd_work_item_result
//============================================================

void *osd_work_item_result(osd_work_item *item)
{
	return item->result;
}


//============================================================
//  osd_work_item_release
//============================================================

void osd_work_item_release(osd_work_item *item)
{
	free(item);
}
