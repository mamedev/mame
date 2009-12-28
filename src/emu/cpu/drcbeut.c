/***************************************************************************

    drcbeut.c

    Utility functions for dynamic recompiling backends.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "drcbeut.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_RECOVER			(0)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* structure holding information about a single label */
typedef struct _drcmap_entry drcmap_entry;
struct _drcmap_entry
{
	drcmap_entry *		next;				/* pointer to next map entry */
	drccodeptr			codeptr;			/* pointer to the relevant code */
	UINT32				mapvar;				/* map variable id */
	UINT32				newval;				/* value of the variable starting at codeptr */
};


/* structure describing the state of the code map */
struct _drcmap_state
{
	drccache *			cache;				/* pointer to the cache */
	UINT64				uniquevalue;		/* unique value used to find the table */
	drcmap_entry *		head;				/* head of the live list */
	drcmap_entry **		tailptr;			/* pointer to tail of the live list */
	UINT32				numvalues;			/* number of values  in the list */
	UINT32				mapvalue[DRCUML_MAPVAR_END - DRCUML_MAPVAR_M0]; /* array of current values */
};


/* structure holding information about a single label */
typedef struct _drclabel drclabel;
struct _drclabel
{
	drclabel *			next;				/* pointer to next label */
	drcuml_codelabel	label;				/* the label specified */
	drccodeptr			codeptr;			/* pointer to the relevant code */
};


/* structure holding a live list of labels */
struct _drclabel_list
{
	drccache *			cache;				/* pointer to the cache */
	drclabel *			head;				/* head of the live list */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void label_list_reset(drclabel_list *list, int fatal_on_leftovers);
static drclabel *label_find_or_allocate(drclabel_list *list, drcuml_codelabel label);
static void label_oob_callback(drccodeptr *codeptr, void *param1, void *param2, void *param3);



/***************************************************************************
    HASH TABLE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    drchash_alloc - allocate memory in the cache
    for the hash table tracker (it auto-frees
    with the cache)
-------------------------------------------------*/

drchash_state *drchash_alloc(drccache *cache, int modes, int addrbits, int ignorebits)
{
	int effaddrbits = addrbits - ignorebits;
	drchash_state *drchash;

	/* allocate permanent state from the cache */
	drchash = (drchash_state *)drccache_memory_alloc(cache, sizeof(*drchash) + modes * sizeof(drchash->base[0]));
	if (drchash == NULL)
		return NULL;
	memset(drchash, 0, sizeof(*drchash) + modes * sizeof(drchash->base[0]));

	/* copy in parameters */
	drchash->cache = cache;
	drchash->modes = modes;

	/* compute the sizes of the tables */
	drchash->l1bits = effaddrbits / 2;
	drchash->l2bits = effaddrbits - drchash->l1bits;
	drchash->l1shift = ignorebits + drchash->l2bits;
	drchash->l2shift = ignorebits;
	drchash->l1mask = (1 << drchash->l1bits) - 1;
	drchash->l2mask = (1 << drchash->l2bits) - 1;

	/* reset the hash table, which allocates any subsequent tables */
	if (!drchash_reset(drchash))
		return NULL;

	return drchash;
}


/*-------------------------------------------------
    drchash_reset - flush existing hash tables and
    create new ones
-------------------------------------------------*/

int drchash_reset(drchash_state *drchash)
{
	int modenum, entry;

	/* allocate an empty l2 hash table */
	drchash->emptyl2 = (drccodeptr *)drccache_memory_alloc_temporary(drchash->cache, sizeof(drccodeptr) << drchash->l2bits);
	if (drchash->emptyl2 == NULL)
		return FALSE;

	/* populate it with pointers to the recompile_exit code */
	for (entry = 0; entry < (1 << drchash->l2bits); entry++)
		drchash->emptyl2[entry] = drchash->nocodeptr;

	/* allocate an empty l1 hash table */
	drchash->emptyl1 = (drccodeptr **)drccache_memory_alloc_temporary(drchash->cache, sizeof(drccodeptr *) << drchash->l1bits);
	if (drchash->emptyl1 == NULL)
		return FALSE;

	/* populate it with pointers to the empty l2 table */
	for (entry = 0; entry < (1 << drchash->l1bits); entry++)
		drchash->emptyl1[entry] = drchash->emptyl2;

	/* reset the hash tables */
	for (modenum = 0; modenum < drchash->modes; modenum++)
		drchash->base[modenum] = drchash->emptyl1;

	return TRUE;
}


/*-------------------------------------------------
    drchash_block_begin - note the beginning of a
    block
-------------------------------------------------*/

void drchash_block_begin(drchash_state *drchash, drcuml_block *block, const drcuml_instruction *instlist, UINT32 numinst)
{
	int inum;

	/* before generating code, pre-allocate any hash entries; we do this by setting dummy hash values */
	for (inum = 0; inum < numinst; inum++)
	{
		const drcuml_instruction *inst = &instlist[inum];

		/* if the opcode is a hash, verify that it makes sense and then set a NULL entry */
		if (inst->opcode == DRCUML_OP_HASH)
		{
			assert(inst->numparams == 2);
			assert(inst->param[0].type == DRCUML_PTYPE_IMMEDIATE);
			assert(inst->param[1].type == DRCUML_PTYPE_IMMEDIATE);

			/* if we fail to allocate, we must abort the block */
			if (!drchash_set_codeptr(drchash, inst->param[0].value, inst->param[1].value, NULL))
				drcuml_block_abort(block);
		}

		/* if the opcode is a hashjmp to a fixed location, make sure we preallocate the tables */
		if (inst->opcode == DRCUML_OP_HASHJMP && inst->param[0].type == DRCUML_PTYPE_IMMEDIATE && inst->param[1].type == DRCUML_PTYPE_IMMEDIATE)
		{
			/* if we fail to allocate, we must abort the block */
			drccodeptr code = drchash_get_codeptr(drchash, inst->param[0].value, inst->param[1].value);
			if (!drchash_set_codeptr(drchash, inst->param[0].value, inst->param[1].value, code))
				drcuml_block_abort(block);
		}
	}
}


/*-------------------------------------------------
    drchash_block_end - note the end of a block
-------------------------------------------------*/

void drchash_block_end(drchash_state *drchash, drcuml_block *block)
{
	/* nothing to do here, yet */
}


/*-------------------------------------------------
    drchash_set_default_codeptr - change the
    default codeptr
-------------------------------------------------*/

void drchash_set_default_codeptr(drchash_state *drchash, drccodeptr nocodeptr)
{
	drccodeptr old = drchash->nocodeptr;
	int modenum, l1entry, l2entry;

	/* nothing to do if the same */
	if (old == nocodeptr)
		return;
	drchash->nocodeptr = nocodeptr;

	/* update the empty L2 table first */
	for (l2entry = 0; l2entry < (1 << drchash->l2bits); l2entry++)
		drchash->emptyl2[l2entry] = nocodeptr;

	/* now scan all existing hashtables for entries */
	for (modenum = 0; modenum < drchash->modes; modenum++)
		if (drchash->base[modenum] != drchash->emptyl1)
			for (l1entry = 0; l1entry < (1 << drchash->l1bits); l1entry++)
				if (drchash->base[modenum][l1entry] != drchash->emptyl2)
					for (l2entry = 0; l2entry < (1 << drchash->l2bits); l2entry++)
						if (drchash->base[modenum][l1entry][l2entry] == old)
							drchash->base[modenum][l1entry][l2entry] = nocodeptr;
}


/*-------------------------------------------------
    drchash_set_codeptr - set the codeptr for the
    given mode/pc
-------------------------------------------------*/

int drchash_set_codeptr(drchash_state *drchash, UINT32 mode, UINT32 pc, drccodeptr code)
{
	UINT32 l1 = (pc >> drchash->l1shift) & drchash->l1mask;
	UINT32 l2 = (pc >> drchash->l2shift) & drchash->l2mask;

	assert(mode < drchash->modes);

	/* copy-on-write for the l1 hash table */
	if (drchash->base[mode] == drchash->emptyl1)
	{
		drccodeptr **newtable = (drccodeptr **)drccache_memory_alloc_temporary(drchash->cache, sizeof(drccodeptr *) << drchash->l1bits);
		if (newtable == NULL)
			return FALSE;
		memcpy(newtable, drchash->emptyl1, sizeof(drccodeptr *) << drchash->l1bits);
		drchash->base[mode] = newtable;
	}

	/* copy-on-write for the l2 hash table */
	if (drchash->base[mode][l1] == drchash->emptyl2)
	{
		drccodeptr *newtable = (drccodeptr *)drccache_memory_alloc_temporary(drchash->cache, sizeof(drccodeptr) << drchash->l2bits);
		if (newtable == NULL)
			return FALSE;
		memcpy(newtable, drchash->emptyl2, sizeof(drccodeptr) << drchash->l2bits);
		drchash->base[mode][l1] = newtable;
	}

	/* set the new entry */
	drchash->base[mode][l1][l2] = code;
	return TRUE;
}



/***************************************************************************
    CODE MAP MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    drcmap_alloc - allocate memory in the cache
    for the code mapper (it auto-frees with the
    cache)
-------------------------------------------------*/

drcmap_state *drcmap_alloc(drccache *cache, UINT64 uniquevalue)
{
	drcmap_state *drcmap;

	/* allocate permanent state from the cache */
	drcmap = (drcmap_state *)drccache_memory_alloc(cache, sizeof(*drcmap));
	if (drcmap == NULL)
		return NULL;
	memset(drcmap, 0, sizeof(*drcmap));

	/* remember the cache */
	drcmap->cache = cache;
	drcmap->tailptr = &drcmap->head;

	return drcmap;
}


/*-------------------------------------------------
    drcmap_block_begin - note the beginning of a
    block
-------------------------------------------------*/

void drcmap_block_begin(drcmap_state *drcmap, drcuml_block *block)
{
	/* release any remaining live entries */
	while (drcmap->head != NULL)
	{
		drcmap_entry *entry = drcmap->head;
		drcmap->head = entry->next;
		drccache_memory_free(drcmap->cache, entry, sizeof(*entry));
	}

	/* reset the tailptr and count */
	drcmap->tailptr = &drcmap->head;
	drcmap->numvalues = 0;

	/* reset the variable values */
	memset(drcmap->mapvalue, 0, sizeof(drcmap->mapvalue));
}


/*-------------------------------------------------
    drcmap_block_end - note the end of a block
-------------------------------------------------*/

void drcmap_block_end(drcmap_state *drcmap, drcuml_block *block)
{
	UINT32 curvalue[DRCUML_MAPVAR_END - DRCUML_MAPVAR_M0] = { 0 };
	UINT8 changed[DRCUML_MAPVAR_END - DRCUML_MAPVAR_M0] = { 0 };
	drcmap_entry *entry;
	drccodeptr lastptr;
	drccodeptr *top;
	UINT32 *dest;

	/* only process if we have data */
	if (drcmap->head == NULL)
		return;

	/* begin "code generation" aligned to an 8-byte boundary */
	top = drccache_begin_codegen(drcmap->cache, sizeof(UINT64) + sizeof(UINT32) + 2 * sizeof(UINT32) * drcmap->numvalues);
	if (top == NULL)
		drcuml_block_abort(block);
	dest = (UINT32 *)(((FPTR)*top + 7) & ~7);

	/* store the cookie first */
	*(UINT64 *)dest = drcmap->uniquevalue;
	dest += 2;

	/* get the pointer to the first item and store an initial backwards offset */
	lastptr = drcmap->head->codeptr;
	*dest = (drccodeptr)dest - lastptr;
	dest++;

	/* now iterate over entries and store them */
	for (entry = drcmap->head; entry != NULL; entry = entry->next)
	{
		/* update the current value of the variable and detect changes */
		if (curvalue[entry->mapvar] != entry->newval)
		{
			curvalue[entry->mapvar] = entry->newval;
			changed[entry->mapvar] = TRUE;
		}

		/* if the next code pointer is different, or if we're at the end, flush changes */
		if (entry->next == NULL || entry->next->codeptr != entry->codeptr)
		{
			UINT32 codedelta = entry->codeptr - lastptr;
			UINT32 varmask = 0;
			int numchanged;
			int varnum;

			/* build a mask of changed variables */
			for (numchanged = varnum = 0; varnum < ARRAY_LENGTH(changed); varnum++)
				if (changed[varnum])
				{
					changed[varnum] = FALSE;
					varmask |= 1 << varnum;
					numchanged++;
				}

			/* if nothing really changed, skip it */
			if (numchanged == 0)
				continue;

			/* first word is a code delta plus mask of changed variables */
			while (codedelta > 0xffff)
			{
				*dest++ = 0xffff << 16;
				codedelta -= 0xffff;
			}
			*dest++ = (codedelta << 16) | (varmask << 4) | numchanged;

			/* now output updated variable values */
			for (varnum = 0; varnum < ARRAY_LENGTH(changed); varnum++)
				if ((varmask >> varnum) & 1)
					*dest++ = curvalue[varnum];

			/* remember our lastptr */
			lastptr = entry->codeptr;
		}
	}

	/* add a terminator */
	*dest++ = 0;

	/* complete codegen */
	*top = (drccodeptr)dest;
	drccache_end_codegen(drcmap->cache);
}


/*-------------------------------------------------
    drcmap_set_value - set a map value for the
    given code pointer
-------------------------------------------------*/

void drcmap_set_value(drcmap_state *drcmap, drccodeptr codebase, UINT32 mapvar, UINT32 newvalue)
{
	drcmap_entry *entry;

	assert(mapvar >= DRCUML_MAPVAR_M0 && mapvar < DRCUML_MAPVAR_END);

	/* if this value isn't different, skip it */
	if (drcmap->mapvalue[mapvar - DRCUML_MAPVAR_M0] == newvalue)
		return;

	/* allocate a new entry and fill it in */
	entry = (drcmap_entry *)drccache_memory_alloc(drcmap->cache, sizeof(*entry));
	entry->next = NULL;
	entry->codeptr = codebase;
	entry->mapvar = mapvar - DRCUML_MAPVAR_M0;
	entry->newval = newvalue;

	/* hook us into the end of the list */
	*drcmap->tailptr = entry;
	drcmap->tailptr = &entry->next;

	/* update our state in the table as well */
	drcmap->mapvalue[mapvar - DRCUML_MAPVAR_M0] = newvalue;

	/* and increment the count */
	drcmap->numvalues++;
}


/*-------------------------------------------------
    drcmap_get_value - return a map value for the
    given code pointer
-------------------------------------------------*/

UINT32 drcmap_get_value(drcmap_state *drcmap, drccodeptr codebase, UINT32 mapvar)
{
	UINT64 *endscan = (UINT64 *)drccache_top(drcmap->cache);
	UINT32 varmask = 0x10 << mapvar;
	drccodeptr curcode;
	UINT32 result = 0;
	UINT64 *curscan;
	UINT32 *data;

	assert(mapvar >= DRCUML_MAPVAR_M0 && mapvar < DRCUML_MAPVAR_END);
	mapvar -= DRCUML_MAPVAR_M0;

	/* get an aligned pointer to start scanning */
	curscan = (UINT64 *)(((FPTR)codebase | 7) + 1);

	/* look for the signature */
	while (curscan < endscan && *curscan++ != drcmap->uniquevalue) ;
	if (curscan >= endscan)
		return 0;

	/* switch to 32-bit pointers for processing the rest */
	data = (UINT32 *)curscan;

	/* first get the 32-bit starting offset to the code */
	curcode = (drccodeptr)data - *data;
	data++;

	/* now loop until we advance past our target */
	while (TRUE)
	{
		UINT32 controlword = *data++;

		/* a 0 is a terminator */
		if (controlword == 0)
			break;

		/* update the codeptr; if this puts us past the end, we're done */
		curcode += (controlword >> 16) & 0xffff;
		if (curcode > codebase)
			break;

		/* if our mapvar has changed, process this word */
		if ((controlword & varmask) != 0)
		{
			int dataoffs = 0;
			UINT32 skipmask;

			/* count how many words precede the one we care about */
			for (skipmask = (controlword & (varmask - 1)) >> 4; skipmask != 0; skipmask = skipmask & (skipmask - 1))
				dataoffs++;

			/* fetch the one we want */
			result = data[dataoffs];
		}

		/* low 4 bits contain the total number of words of data */
		data += controlword & 0x0f;
	}
	if (LOG_RECOVER)
		printf("recover %d @ %p = %08X\n", mapvar, codebase, result);
	return result;
}


/*-------------------------------------------------
    drcmap_get_last_value - return the most
    recently set map value
-------------------------------------------------*/

UINT32 drcmap_get_last_value(drcmap_state *drcmap, UINT32 mapvar)
{
	assert(mapvar >= DRCUML_MAPVAR_M0 && mapvar < DRCUML_MAPVAR_END);
	return drcmap->mapvalue[mapvar - DRCUML_MAPVAR_M0];
}



/***************************************************************************
    LABEL MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    drclabel_list_alloc - allocate a label
    list within the cache (it auto-frees with the
    cache)
-------------------------------------------------*/

drclabel_list *drclabel_list_alloc(drccache *cache)
{
	drclabel_list *list;

	/* allocate permanent state from the cache */
	list = (drclabel_list *)drccache_memory_alloc(cache, sizeof(*list));
	if (list == NULL)
		return NULL;
	memset(list, 0, sizeof(*list));

	/* remember the cache */
	list->cache = cache;

	return list;
}


/*-------------------------------------------------
    drclabel_block_begin - note the beginning of
    a block
-------------------------------------------------*/

void drclabel_block_begin(drclabel_list *list, drcuml_block *block)
{
	/* make sure the label list is clear, but don't fatalerror */
	label_list_reset(list, FALSE);
}


/*-------------------------------------------------
    drclabel_block_end - note the end of a block
-------------------------------------------------*/

void drclabel_block_end(drclabel_list *list, drcuml_block *block)
{
	/* make sure the label list is clear, and fatalerror if we missed anything */
	label_list_reset(list, TRUE);
}


/*-------------------------------------------------
    drclabel_get_codeptr - find or allocate a new
    label; returns NULL and requests an OOB
    callback if undefined
-------------------------------------------------*/

drccodeptr drclabel_get_codeptr(drclabel_list *list, drcuml_codelabel label, drclabel_fixup_func fixup, void *param)
{
	drclabel *curlabel = label_find_or_allocate(list, label);

	/* if no code pointer, request an OOB callback */
	if (curlabel->codeptr == NULL && fixup != NULL)
		drccache_request_oob_codegen(list->cache, label_oob_callback, curlabel, (void *)fixup, param);

	return curlabel->codeptr;
}


/*-------------------------------------------------
    drclabel_set_codeptr - set the pointer to a new
    label
-------------------------------------------------*/

void drclabel_set_codeptr(drclabel_list *list, drcuml_codelabel label, drccodeptr codeptr)
{
	/* set the code pointer */
	drclabel *curlabel = label_find_or_allocate(list, label);
	assert(curlabel->codeptr == NULL);
	curlabel->codeptr = codeptr;
}



/***************************************************************************
    LABEL MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    label_list_reset - reset a label
    list (add all entries to the free list)
-------------------------------------------------*/

static void label_list_reset(drclabel_list *list, int fatal_on_leftovers)
{
	/* loop until out of labels */
	while (list->head != NULL)
	{
		/* remove from the list */
		drclabel *label = list->head;
		list->head = label->next;

		/* fatal if we were a leftover */
		if (fatal_on_leftovers && label->codeptr == NULL)
			fatalerror("Label %08X never defined!", label->label);

		/* free the label */
		drccache_memory_free(list->cache, label, sizeof(*label));
	}
}


/*-------------------------------------------------
    label_find_or_allocate - look up a label and
    allocate a new one if not found
-------------------------------------------------*/

static drclabel *label_find_or_allocate(drclabel_list *list, drcuml_codelabel label)
{
	drclabel *curlabel;

	/* find the label, or else allocate a new one */
	for (curlabel = list->head; curlabel != NULL; curlabel = curlabel->next)
		if (curlabel->label == label)
			break;

	/* if none found, allocate */
	if (curlabel == NULL)
	{
		curlabel = (drclabel *)drccache_memory_alloc(list->cache, sizeof(*curlabel));
		curlabel->next = list->head;
		curlabel->label = label;
		curlabel->codeptr = NULL;
		list->head = curlabel;
	}

	return curlabel;
}


/*-------------------------------------------------
    label_oob_callback - out-of-band codegen
    callback for labels
-------------------------------------------------*/

static void label_oob_callback(drccodeptr *codeptr, void *param1, void *param2, void *param3)
{
	drclabel *label = (drclabel *)param1;
	drclabel_fixup_func callback = (drclabel_fixup_func)param2;

	(*callback)(param3, label->codeptr);
}
