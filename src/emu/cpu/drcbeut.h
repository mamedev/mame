/***************************************************************************

    drcbeut.h

    Utility functions for dynamic recompiling backends.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __DRCBEUT_H__
#define __DRCBEUT_H__

#include "drcuml.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* callback function for forward-referenced labels */
typedef void (*drclabel_fixup_func)(void *parameter, drccodeptr labelcodeptr);


/* opaque structure representing a managed list of labels */
typedef struct _drclabel_list drclabel_list;


/* opaque structure representing a managed code map */
typedef struct _drcmap_state drcmap_state;


/* information about the hash tables used by the UML and backend */
typedef struct _drchash_state drchash_state;
struct _drchash_state
{
	drccache *		cache;				/* cache where allocations come from */
	int				modes;				/* number of modes supported */

	drccodeptr		nocodeptr;			/* pointer to code which will handle missing entries */

	UINT8			l1bits;				/* bits worth of entries in l1 hash tables */
	UINT8			l1shift;			/* shift to apply to the PC to get the l1 hash entry */
	offs_t			l1mask;				/* mask to apply after shifting */
	UINT8			l2bits;				/* bits worth of entries in l2 hash tables */
	UINT8			l2shift;			/* shift to apply to the PC to get the l2 hash entry */
	offs_t			l2mask;				/* mask to apply after shifting */

	drccodeptr **	emptyl1;			/* pointer to empty l1 hash table */
	drccodeptr *	emptyl2;			/* pointer to empty l2 hash table */

	drccodeptr **	base[1];			/* pointer to the l1 table for each mode */
};


/* an integer register, with low/high parts */
typedef union _drcuml_ireg drcuml_ireg;
union _drcuml_ireg
{
#ifdef LSB_FIRST
	UINT32			l,h;				/* 32-bit low, high parts of the register */
#else
	UINT32			h,l;				/* 32-bit low, high parts of the register */
#endif
	UINT64			d;					/* 64-bit full register */
};


/* an floating-point register, with low/high parts */
typedef union _drcuml_freg drcuml_freg;
union _drcuml_freg
{
#ifdef LSB_FIRST
	float			l,unused;			/* 32-bit low, high parts of the register */
#else
	float			unused,l;			/* 32-bit low, high parts of the register */
#endif
	double			d;					/* 64-bit full register */
};


/* the collected machine state of a system */
typedef struct _drcuml_machine_state drcuml_machine_state;
struct _drcuml_machine_state
{
	drcuml_ireg		r[DRCUML_REG_I_END - DRCUML_REG_I0];	/* integer registers */
	drcuml_freg		f[DRCUML_REG_F_END - DRCUML_REG_F0];	/* floating-point registers */
	drcuml_ireg		exp;									/* exception parameter register */
	UINT8			fmod;									/* fmod (floating-point mode) register */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- hash table management ----- */

/* allocate memory in the cache for the hash table tracker (it auto-frees with the cache) */
drchash_state *drchash_alloc(drccache *cache, int modes, int addrbits, int ignorebits);

/* flush existing hash tables and create new ones */
int drchash_reset(drchash_state *drchash);

/* note the beginning of a block */
void drchash_block_begin(drchash_state *drchash, drcuml_block *block, const drcuml_instruction *instlist, UINT32 numinst);

/* note the end of a block */
void drchash_block_end(drchash_state *drchash, drcuml_block *block);

/* set the default codeptr for any empty hash entries (defaults to NULL) */
void drchash_set_default_codeptr(drchash_state *drchash, drccodeptr code);

/* set the codeptr for the given mode/pc */
int drchash_set_codeptr(drchash_state *drchash, UINT32 mode, UINT32 pc, drccodeptr code);



/* ----- code map management ----- */

/* allocate memory in the cache for the code mapper (it auto-frees with the cache) */
drcmap_state *drcmap_alloc(drccache *cache, UINT64 uniquevalue);

/* note the beginning of a block */
void drcmap_block_begin(drcmap_state *drcmap, drcuml_block *block);

/* note the end of a block */
void drcmap_block_end(drcmap_state *drcmap, drcuml_block *block);

/* set a map value for the given code pointer */
void drcmap_set_value(drcmap_state *drcmap, drccodeptr codebase, UINT32 mapvar, UINT32 newvalue);

/* return a map value for the given code pointer */
UINT32 drcmap_get_value(drcmap_state *drcmap, drccodeptr codebase, UINT32 mapvar);

/* return the most recently set map value */
UINT32 drcmap_get_last_value(drcmap_state *drcmap, UINT32 mapvar);



/* ----- label management ----- */

/* allocate a label list within the cache (it auto-frees with the cache) */
drclabel_list *drclabel_list_alloc(drccache *cache);

/* note the beginning of a block */
void drclabel_block_begin(drclabel_list *drcmap, drcuml_block *block);

/* note the end of a block */
void drclabel_block_end(drclabel_list *drcmap, drcuml_block *block);

/* find or allocate a new label; returns NULL and requests an OOB callback if undefined */
drccodeptr drclabel_get_codeptr(drclabel_list *list, drcuml_codelabel label, drclabel_fixup_func fixup, void *param);

/* set the pointer to a new label */
void drclabel_set_codeptr(drclabel_list *list, drcuml_codelabel label, drccodeptr codeptr);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    drchash_get_codeptr - return the codeptr
    allocated for the given mode/pc
-------------------------------------------------*/

INLINE drccodeptr drchash_get_codeptr(drchash_state *drchash, UINT32 mode, UINT32 pc)
{
	assert(mode < drchash->modes);
	return drchash->base[mode][(pc >> drchash->l1shift) & drchash->l1mask][(pc >> drchash->l2shift) & drchash->l2mask];
}


/*-------------------------------------------------
    drchash_code_exists - return TRUE if there is
    a matching hash entry for the given mode/pc
-------------------------------------------------*/

INLINE int drchash_code_exists(drchash_state *drchash, UINT32 mode, UINT32 pc)
{
	return (drchash_get_codeptr(drchash, mode, pc) != drchash->nocodeptr);
}


#endif
