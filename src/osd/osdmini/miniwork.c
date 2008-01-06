//============================================================
//
//  miniwork.c - Minimal core work item functions
//
//  Copyright Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#include "osdcore.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================

struct _osd_work_item
{
	void *result;
};



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

osd_work_item *osd_work_item_queue(osd_work_queue *queue, osd_work_callback callback, void *param, UINT32 flags)
{
	osd_work_item *item;

	// allocate memory to hold the result
	item = malloc(sizeof(*item));
	if (item == NULL)
		return NULL;

	// execute the call directly
	item->result = (*callback)(param, 0);
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
