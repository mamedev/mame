// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    vtlb.h

    Generic virtual TLB implementation.

***************************************************************************/

#pragma once

#ifndef __VTLB_H__
#define __VTLB_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define VTLB_FLAGS_MASK             0xff

#define VTLB_READ_ALLOWED           0x01        /* (1 << TRANSLATE_READ) */
#define VTLB_WRITE_ALLOWED          0x02        /* (1 << TRANSLATE_WRITE) */
#define VTLB_FETCH_ALLOWED          0x04        /* (1 << TRANSLATE_FETCH) */
#define VTLB_FLAG_VALID             0x08
#define VTLB_USER_READ_ALLOWED      0x10        /* (1 << TRANSLATE_READ_USER) */
#define VTLB_USER_WRITE_ALLOWED     0x20        /* (1 << TRANSLATE_WRITE_USER) */
#define VTLB_USER_FETCH_ALLOWED     0x40        /* (1 << TRANSLATE_FETCH_USER) */
#define VTLB_FLAG_FIXED             0x80



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* represents an entry in the VTLB */
typedef UINT32 vtlb_entry;


/* opaque structure describing VTLB state */
struct vtlb_state;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- initialization/teardown ----- */

/* allocate a new VTLB for the given CPU */
vtlb_state *vtlb_alloc(device_t *cpu, address_spacenum space, int fixed_entries, int dynamic_entries);

/* free an allocated VTLB */
void vtlb_free(vtlb_state *vtlb);


/* ----- filling ----- */

/* called by the CPU core in response to an unmapped access */
int vtlb_fill(vtlb_state *vtlb, offs_t address, int intention);

/* load a fixed VTLB entry */
void vtlb_load(vtlb_state *vtlb, int entrynum, int numpages, offs_t address, vtlb_entry value);

/* load a dynamic VTLB entry */
void vtlb_dynload(vtlb_state *vtlb, UINT32 index, offs_t address, vtlb_entry value);

/* ----- flushing ----- */

/* flush all knowledge from the dynamic part of the VTLB */
void vtlb_flush_dynamic(vtlb_state *vtlb);

/* flush knowledge of a particular address from the VTLB */
void vtlb_flush_address(vtlb_state *vtlb, offs_t address);


/* ----- accessors ----- */

/* return a pointer to the base of the linear VTLB lookup table */
const vtlb_entry *vtlb_table(vtlb_state *vtlb);


#endif /* __VTLB_H__ */
