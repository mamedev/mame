//============================================================
//
//  miniwork.c - Minimal core work item functions
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
