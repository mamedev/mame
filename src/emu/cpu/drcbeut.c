// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbeut.c

    Utility functions for dynamic recompiling backends.

***************************************************************************/

#include "emu.h"
#include "drcbeut.h"

using namespace uml;


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_RECOVER         (0)



//**************************************************************************
//  DRC HASH TABLE
//**************************************************************************

//-------------------------------------------------
//  drc_hash_table - constructor
//-------------------------------------------------

drc_hash_table::drc_hash_table(drc_cache &cache, UINT32 modes, UINT8 addrbits, UINT8 ignorebits)
	: m_cache(cache),
		m_modes(modes),
		m_nocodeptr(NULL),
		m_l1bits((addrbits - ignorebits) / 2),
		m_l2bits((addrbits - ignorebits) - m_l1bits),
		m_l1shift(ignorebits + m_l2bits),
		m_l2shift(ignorebits),
		m_l1mask((1 << m_l1bits) - 1),
		m_l2mask((1 << m_l2bits) - 1),
		m_base(reinterpret_cast<drccodeptr ***>(cache.alloc(modes * sizeof(**m_base)))),
		m_emptyl1(NULL),
		m_emptyl2(NULL)
{
	reset();
}


//-------------------------------------------------
//  reset - flush existing hash tables and create
//  new ones
//-------------------------------------------------

bool drc_hash_table::reset()
{
	// allocate an empty l2 hash table
	m_emptyl2 = (drccodeptr *)m_cache.alloc_temporary(sizeof(drccodeptr) << m_l2bits);
	if (m_emptyl2 == NULL)
		return false;

	// populate it with pointers to the recompile_exit code
	for (int entry = 0; entry < (1 << m_l2bits); entry++)
		m_emptyl2[entry] = m_nocodeptr;

	// allocate an empty l1 hash table
	m_emptyl1 = (drccodeptr **)m_cache.alloc_temporary(sizeof(drccodeptr *) << m_l1bits);
	if (m_emptyl1 == NULL)
		return false;

	// populate it with pointers to the empty l2 table
	for (int entry = 0; entry < (1 << m_l1bits); entry++)
		m_emptyl1[entry] = m_emptyl2;

	// reset the hash tables
	for (int modenum = 0; modenum < m_modes; modenum++)
		m_base[modenum] = m_emptyl1;

	return true;
}


//-------------------------------------------------
//  block_begin - note the beginning of a block
//-------------------------------------------------

void drc_hash_table::block_begin(drcuml_block &block, const uml::instruction *instlist, UINT32 numinst)
{
	// before generating code, pre-allocate any hash entries; we do this by setting dummy hash values
	for (int inum = 0; inum < numinst; inum++)
	{
		const uml::instruction &inst = instlist[inum];

		// if the opcode is a hash, verify that it makes sense and then set a NULL entry
		if (inst.opcode() == OP_HASH)
		{
			assert(inst.numparams() == 2);

			// if we fail to allocate, we must abort the block
			if (!set_codeptr(inst.param(0).immediate(), inst.param(1).immediate(), NULL))
				block.abort();
		}

		// if the opcode is a hashjmp to a fixed location, make sure we preallocate the tables
		if (inst.opcode() == OP_HASHJMP && inst.param(0).is_immediate() && inst.param(1).is_immediate())
		{
			// if we fail to allocate, we must abort the block
			drccodeptr code = get_codeptr(inst.param(0).immediate(), inst.param(1).immediate());
			if (!set_codeptr(inst.param(0).immediate(), inst.param(1).immediate(), code))
				block.abort();
		}
	}
}


//-------------------------------------------------
//  block_end - note the end of a block
//-------------------------------------------------

void drc_hash_table::block_end(drcuml_block &block)
{
	// nothing to do here, yet
}


//-------------------------------------------------
//  set_default_codeptr - change the default
//  codeptr
//-------------------------------------------------

void drc_hash_table::set_default_codeptr(drccodeptr nocodeptr)
{
	// nothing to do if the same
	drccodeptr old = m_nocodeptr;
	if (old == nocodeptr)
		return;
	m_nocodeptr = nocodeptr;

	// update the empty L2 table first
	for (int l2entry = 0; l2entry < (1 << m_l2bits); l2entry++)
		m_emptyl2[l2entry] = nocodeptr;

	// now scan all existing hashtables for entries
	for (int modenum = 0; modenum < m_modes; modenum++)
		if (m_base[modenum] != m_emptyl1)
			for (int l1entry = 0; l1entry < (1 << m_l1bits); l1entry++)
				if (m_base[modenum][l1entry] != m_emptyl2)
					for (int l2entry = 0; l2entry < (1 << m_l2bits); l2entry++)
						if (m_base[modenum][l1entry][l2entry] == old)
							m_base[modenum][l1entry][l2entry] = nocodeptr;
}


//-------------------------------------------------
//  set_codeptr - set the codeptr for the given
//  mode/pc
//-------------------------------------------------

bool drc_hash_table::set_codeptr(UINT32 mode, UINT32 pc, drccodeptr code)
{
	// copy-on-write for the l1 hash table
	assert(mode < m_modes);
	if (m_base[mode] == m_emptyl1)
	{
		drccodeptr **newtable = (drccodeptr **)m_cache.alloc_temporary(sizeof(drccodeptr *) << m_l1bits);
		if (newtable == NULL)
			return false;
		memcpy(newtable, m_emptyl1, sizeof(drccodeptr *) << m_l1bits);
		m_base[mode] = newtable;
	}

	// copy-on-write for the l2 hash table
	UINT32 l1 = (pc >> m_l1shift) & m_l1mask;
	if (m_base[mode][l1] == m_emptyl2)
	{
		drccodeptr *newtable = (drccodeptr *)m_cache.alloc_temporary(sizeof(drccodeptr) << m_l2bits);
		if (newtable == NULL)
			return false;
		memcpy(newtable, m_emptyl2, sizeof(drccodeptr) << m_l2bits);
		m_base[mode][l1] = newtable;
	}

	// set the new entry
	UINT32 l2 = (pc >> m_l2shift) & m_l2mask;
	m_base[mode][l1][l2] = code;
	return true;
}



//**************************************************************************
//  DRC MAP VARIABLES
//**************************************************************************

//-------------------------------------------------
//  drc_map_variables - constructor
//-------------------------------------------------

drc_map_variables::drc_map_variables(drc_cache &cache, UINT64 uniquevalue)
	: m_cache(cache),
		m_uniquevalue(uniquevalue)
{
	memset(m_mapvalue, 0, sizeof(m_mapvalue));
}


//-------------------------------------------------
//  ~drc_map_variables - destructor
//-------------------------------------------------

drc_map_variables::~drc_map_variables()
{
	// must detach all items from the entry list so that the list object
	// doesn't try to free them on exit
	m_entry_list.detach_all();
}


//-------------------------------------------------
//  block_begin - note the beginning of a block
//-------------------------------------------------

void drc_map_variables::block_begin(drcuml_block &block)
{
	// release any remaining live entries
	map_entry *entry;
	while ((entry = m_entry_list.detach_head()) != NULL)
		m_cache.dealloc(entry, sizeof(*entry));

	// reset the variable values
	memset(m_mapvalue, 0, sizeof(m_mapvalue));
}


//-------------------------------------------------
//  block_end - note the end of a block
//-------------------------------------------------

void drc_map_variables::block_end(drcuml_block &block)
{
	// only process if we have data
	if (m_entry_list.first() == NULL)
		return;

	// begin "code generation" aligned to an 8-byte boundary
	drccodeptr *top = m_cache.begin_codegen(sizeof(UINT64) + sizeof(UINT32) + 2 * sizeof(UINT32) * m_entry_list.count());
	if (top == NULL)
		block.abort();
	UINT32 *dest = (UINT32 *)(((FPTR)*top + 7) & ~7);

	// store the cookie first
	*(UINT64 *)dest = m_uniquevalue;
	dest += 2;

	// get the pointer to the first item and store an initial backwards offset
	drccodeptr lastptr = m_entry_list.first()->m_codeptr;
	*dest = (drccodeptr)dest - lastptr;
	dest++;

	// now iterate over entries and store them
	UINT32 curvalue[MAPVAR_COUNT] = { 0 };
	bool changed[MAPVAR_COUNT] = { false };
	for (map_entry *entry = m_entry_list.first(); entry != NULL; entry = entry->next())
	{
		// update the current value of the variable and detect changes
		if (curvalue[entry->m_mapvar] != entry->m_newval)
		{
			curvalue[entry->m_mapvar] = entry->m_newval;
			changed[entry->m_mapvar] = true;
		}

		// if the next code pointer is different, or if we're at the end, flush changes
		if (entry->next() == NULL || entry->next()->m_codeptr != entry->m_codeptr)
		{
			// build a mask of changed variables
			int numchanged = 0;
			UINT32 varmask = 0;
			for (int varnum = 0; varnum < ARRAY_LENGTH(changed); varnum++)
				if (changed[varnum])
				{
					changed[varnum] = false;
					varmask |= 1 << varnum;
					numchanged++;
				}

			// if nothing really changed, skip it
			if (numchanged == 0)
				continue;

			// first word is a code delta plus mask of changed variables
			UINT32 codedelta = entry->m_codeptr - lastptr;
			while (codedelta > 0xffff)
			{
				*dest++ = 0xffff << 16;
				codedelta -= 0xffff;
			}
			*dest++ = (codedelta << 16) | (varmask << 4) | numchanged;

			// now output updated variable values
			for (int varnum = 0; varnum < ARRAY_LENGTH(changed); varnum++)
				if ((varmask >> varnum) & 1)
					*dest++ = curvalue[varnum];

			// remember our lastptr
			lastptr = entry->m_codeptr;
		}
	}

	// add a terminator
	*dest++ = 0;

	// complete codegen
	*top = (drccodeptr)dest;
	m_cache.end_codegen();
}


//-------------------------------------------------
//  set_value - set a map value for the given
//  code pointer
//-------------------------------------------------

void drc_map_variables::set_value(drccodeptr codebase, UINT32 mapvar, UINT32 newvalue)
{
	assert(mapvar >= MAPVAR_M0 && mapvar < MAPVAR_END);

	// if this value isn't different, skip it
	if (m_mapvalue[mapvar - MAPVAR_M0] == newvalue)
		return;

	// allocate a new entry and fill it in
	map_entry *entry = (map_entry *)m_cache.alloc(sizeof(*entry));
	entry->m_next = NULL;
	entry->m_codeptr = codebase;
	entry->m_mapvar = mapvar - MAPVAR_M0;
	entry->m_newval = newvalue;

	// hook us into the end of the list
	m_entry_list.append(*entry);

	// update our state in the table as well
	m_mapvalue[mapvar - MAPVAR_M0] = newvalue;
}


//-------------------------------------------------
//  get_value - return a map value for the given
//  code pointer
//-------------------------------------------------

UINT32 drc_map_variables::get_value(drccodeptr codebase, UINT32 mapvar) const
{
	assert(mapvar >= MAPVAR_M0 && mapvar < MAPVAR_END);
	mapvar -= MAPVAR_M0;

	// get an aligned pointer to start scanning
	UINT64 *curscan = (UINT64 *)(((FPTR)codebase | 7) + 1);
	UINT64 *endscan = (UINT64 *)m_cache.top();

	// look for the signature
	while (curscan < endscan && *curscan++ != m_uniquevalue) ;
	if (curscan >= endscan)
		return 0;

	// switch to 32-bit pointers for processing the rest
	UINT32 *data = (UINT32 *)curscan;

	// first get the 32-bit starting offset to the code
	drccodeptr curcode = (drccodeptr)data - *data;
	data++;

	// now loop until we advance past our target
	UINT32 varmask = 0x10 << mapvar;
	UINT32 result = 0;
	while (true)
	{
		// a 0 is a terminator
		UINT32 controlword = *data++;
		if (controlword == 0)
			break;

		// update the codeptr; if this puts us past the end, we're done
		curcode += (controlword >> 16) & 0xffff;
		if (curcode > codebase)
			break;

		// if our mapvar has changed, process this word
		if ((controlword & varmask) != 0)
		{
			// count how many words precede the one we care about
			int dataoffs = 0;
			for (UINT32 skipmask = (controlword & (varmask - 1)) >> 4; skipmask != 0; skipmask = skipmask & (skipmask - 1))
				dataoffs++;

			// fetch the one we want
			result = data[dataoffs];
		}

		// low 4 bits contain the total number of words of data
		data += controlword & 0x0f;
	}
	if (LOG_RECOVER)
		printf("recover %d @ %p = %08X\n", mapvar, codebase, result);
	return result;
}

UINT32 drc_map_variables::static_get_value(drc_map_variables &map, drccodeptr codebase, UINT32 mapvar)
{
	return map.get_value(codebase, mapvar);
}



//-------------------------------------------------
//  get_last_value - return the most recently set
//  map value
//-------------------------------------------------

UINT32 drc_map_variables::get_last_value(UINT32 mapvar)
{
	assert(mapvar >= MAPVAR_M0 && mapvar < MAPVAR_END);
	return m_mapvalue[mapvar - MAPVAR_M0];
}



//**************************************************************************
//  DRC LABEL LIST
//**************************************************************************

//-------------------------------------------------
//  drc_label_list - constructor
//-------------------------------------------------

drc_label_list::drc_label_list(drc_cache &cache)
	: m_cache(cache),
		m_oob_callback_delegate(FUNC(drc_label_list::oob_callback), this)
{
}


//-------------------------------------------------
//  ~drc_label_list - destructor
//-------------------------------------------------

drc_label_list::~drc_label_list()
{
	// must detach all items from the entry list so that the list object
	// doesn't try to free them on exit
	m_list.detach_all();
}


//-------------------------------------------------
//  block_begin - note the beginning of a block
//-------------------------------------------------

void drc_label_list::block_begin(drcuml_block &block)
{
	// make sure the label list is clear, but don't fatalerror
	reset(false);
}


//-------------------------------------------------
//  block_end - note the end of a block
//-------------------------------------------------

void drc_label_list::block_end(drcuml_block &block)
{
	// can't free until the cache is clean of our OOB requests
	assert(!m_cache.generating_code());

	// free all of the pending fixup requests
	label_fixup *fixup;
	while ((fixup = m_fixup_list.detach_head()) != NULL)
		m_cache.dealloc(fixup, sizeof(*fixup));

	// make sure the label list is clear, and fatalerror if we missed anything
	reset(true);
}


//-------------------------------------------------
//  get_codeptr - find or allocate a new label;
//  returns NULL and requests an OOB callback if
//  undefined
//-------------------------------------------------

drccodeptr drc_label_list::get_codeptr(uml::code_label label, drc_label_fixup_delegate callback, void *param)
{
	label_entry *curlabel = find_or_allocate(label);

	// if no code pointer, request an OOB callback
	if (curlabel->m_codeptr == NULL && !callback.isnull())
	{
		label_fixup *fixup = reinterpret_cast<label_fixup *>(m_cache.alloc(sizeof(*fixup)));
		fixup->m_callback = callback;
		fixup->m_label = curlabel;
		m_fixup_list.append(*fixup);
		m_cache.request_oob_codegen(m_oob_callback_delegate, fixup, param);
	}

	return curlabel->m_codeptr;
}


//-------------------------------------------------
//  set_codeptr - set the pointer to a new label
//-------------------------------------------------

void drc_label_list::set_codeptr(uml::code_label label, drccodeptr codeptr)
{
	// set the code pointer
	label_entry *curlabel = find_or_allocate(label);
	assert(curlabel->m_codeptr == NULL);
	curlabel->m_codeptr = codeptr;
}


//-------------------------------------------------
//  reset - reset a label list (add all entries to
//  the free list)
//-------------------------------------------------

void drc_label_list::reset(bool fatal_on_leftovers)
{
	// loop until out of labels
	label_entry *curlabel;
	while ((curlabel = m_list.detach_head()) != NULL)
	{
		// fatal if we were a leftover
		if (fatal_on_leftovers && curlabel->m_codeptr == NULL)
			fatalerror("Label %08X never defined!\n", curlabel->m_label.label());

		// free the label
		m_cache.dealloc(curlabel, sizeof(*curlabel));
	}
}


//-------------------------------------------------
//  find_or_allocate - look up a label and
//  allocate a new one if not found
//-------------------------------------------------

drc_label_list::label_entry *drc_label_list::find_or_allocate(uml::code_label label)
{
	// find the label, or else allocate a new one
	label_entry *curlabel;
	for (curlabel = m_list.first(); curlabel != NULL; curlabel = curlabel->next())
		if (curlabel->m_label == label)
			break;

	// if none found, allocate
	if (curlabel == NULL)
	{
		curlabel = (label_entry *)m_cache.alloc(sizeof(*curlabel));
		curlabel->m_label = label;
		curlabel->m_codeptr = NULL;
		m_list.append(*curlabel);
	}
	return curlabel;
}


//-------------------------------------------------
//  oob_callback - out-of-band codegen callback
//  for labels
//-------------------------------------------------

void drc_label_list::oob_callback(drccodeptr *codeptr, void *param1, void *param2)
{
	label_fixup *fixup = reinterpret_cast<label_fixup *>(param1);
	fixup->m_callback(param2, fixup->m_label->m_codeptr);
}
