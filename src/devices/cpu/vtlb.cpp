// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    vtlb.c

    Generic virtual TLB implementation.

***************************************************************************/

#include "emu.h"
#include "vtlb.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define PRINTF_TLB          (0)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* VTLB state */
struct vtlb_state
{
	cpu_device *        cpudevice;          /* CPU device */
	address_spacenum    space;              /* address space */
	int                 dynamic;            /* number of dynamic entries */
	int                 fixed;              /* number of fixed entries */
	int                 dynindex;           /* index of next dynamic entry */
	int                 pageshift;          /* bits to shift to get page index */
	int                 addrwidth;          /* logical address bus width */
	std::vector<offs_t> live;             /* array of live entries by table index */
	std::vector<int> fixedpages;          /* number of pages each fixed entry covers */
	std::vector<vtlb_entry> table;        /* table of entries by address */
};



/***************************************************************************
    INITIALIZATION/TEARDOWN
***************************************************************************/

/*-------------------------------------------------
    vtlb_alloc - allocate a new VTLB for the
    given CPU
-------------------------------------------------*/

vtlb_state *vtlb_alloc(device_t *cpu, address_spacenum space, int fixed_entries, int dynamic_entries)
{
	vtlb_state *vtlb;

	/* allocate memory for the core structure */
	vtlb = auto_alloc_clear(cpu->machine(), <vtlb_state>());

	/* fill in CPU information */
	vtlb->cpudevice = downcast<cpu_device *>(cpu);
	vtlb->space = space;
	vtlb->dynamic = dynamic_entries;
	vtlb->fixed = fixed_entries;
	const address_space_config *spaceconfig = device_get_space_config(*cpu, space);
	assert(spaceconfig != nullptr);
	vtlb->pageshift = spaceconfig->m_page_shift;
	vtlb->addrwidth = spaceconfig->m_logaddr_width;

	/* validate CPU information */
	assert((1 << vtlb->pageshift) > VTLB_FLAGS_MASK);
	assert(vtlb->addrwidth > vtlb->pageshift);

	/* allocate the entry array */
	vtlb->live.resize(fixed_entries + dynamic_entries);
	memset(&vtlb->live[0], 0, vtlb->live.size()*sizeof(vtlb->live[0]));
	cpu->save_item(NAME(vtlb->live));

	/* allocate the lookup table */
	vtlb->table.resize((size_t) 1 << (vtlb->addrwidth - vtlb->pageshift));
	memset(&vtlb->table[0], 0, vtlb->table.size()*sizeof(vtlb->table[0]));
	cpu->save_item(NAME(vtlb->table));

	/* allocate the fixed page count array */
	if (fixed_entries > 0)
	{
		vtlb->fixedpages.resize(fixed_entries);
		memset(&vtlb->fixedpages[0], 0, fixed_entries*sizeof(vtlb->fixedpages[0]));
		cpu->save_item(NAME(vtlb->fixedpages));
	}
	return vtlb;
}


/*-------------------------------------------------
    vtlb_free - free an allocated VTLB
-------------------------------------------------*/

void vtlb_free(vtlb_state *vtlb)
{
	auto_free(vtlb->cpudevice->machine(), vtlb);
}



/***************************************************************************
    FILLING
***************************************************************************/

/*-------------------------------------------------
    vtlb_fill - rcalled by the CPU core in
    response to an unmapped access
-------------------------------------------------*/

int vtlb_fill(vtlb_state *vtlb, offs_t address, int intention)
{
	offs_t tableindex = address >> vtlb->pageshift;
	vtlb_entry entry = vtlb->table[tableindex];
	offs_t taddress;

	if (PRINTF_TLB)
		printf("vtlb_fill: %08X(%X) ... ", address, intention);

	/* should not be called here if the entry is in the table already */
//  assert((entry & (1 << intention)) == 0);

	/* if we have no dynamic entries, we always fail */
	if (vtlb->dynamic == 0)
	{
		if (PRINTF_TLB)
			printf("failed: no dynamic entries\n");
		return FALSE;
	}

	/* ask the CPU core to translate for us */
	taddress = address;
	if (!vtlb->cpudevice->translate(vtlb->space, intention, taddress))
	{
		if (PRINTF_TLB)
			printf("failed: no translation\n");
		return FALSE;
	}

	/* if this is the first successful translation for this address, allocate a new entry */
	if ((entry & VTLB_FLAGS_MASK) == 0)
	{
		int liveindex = vtlb->dynindex++ % vtlb->dynamic;

		/* if an entry already exists at this index, free it */
		if (vtlb->live[liveindex] != 0)
			vtlb->table[vtlb->live[liveindex] - 1] = 0;

		/* claim this new entry */
		vtlb->live[liveindex] = tableindex + 1;

		/* form a new blank entry */
		entry = (taddress >> vtlb->pageshift) << vtlb->pageshift;
		entry |= VTLB_FLAG_VALID;

		if (PRINTF_TLB)
			printf("success (%08X), new entry\n", taddress);
	}

	/* otherwise, ensure that different intentions do not produce different addresses */
	else
	{
		assert((entry >> vtlb->pageshift) == (taddress >> vtlb->pageshift));
		assert(entry & VTLB_FLAG_VALID);

		if (PRINTF_TLB)
			printf("success (%08X), existing entry\n", taddress);
	}

	/* add the intention to the list of valid intentions and store */
	entry |= 1 << (intention & (TRANSLATE_TYPE_MASK | TRANSLATE_USER_MASK));
	vtlb->table[tableindex] = entry;
	return TRUE;
}


/*-------------------------------------------------
    vtlb_load - load a fixed VTLB entry
-------------------------------------------------*/

void vtlb_load(vtlb_state *vtlb, int entrynum, int numpages, offs_t address, vtlb_entry value)
{
	offs_t tableindex = address >> vtlb->pageshift;
	int liveindex = vtlb->dynamic + entrynum;
	int pagenum;

	/* must be in range */
	assert(entrynum >= 0 && entrynum < vtlb->fixed);

	if (PRINTF_TLB)
		printf("vtlb_load %d for %d pages at %08X == %08X\n", entrynum, numpages, address, value);

	/* if an entry already exists at this index, free it */
	if (vtlb->live[liveindex] != 0)
	{
		int pagecount = vtlb->fixedpages[entrynum];
		int oldtableindex = vtlb->live[liveindex] - 1;
		for (pagenum = 0; pagenum < pagecount; pagenum++)
			vtlb->table[oldtableindex + pagenum] = 0;
	}

	/* claim this new entry */
	vtlb->live[liveindex] = tableindex + 1;

	/* store the raw value, making sure the "fixed" flag is set */
	value |= VTLB_FLAG_FIXED;
	vtlb->fixedpages[entrynum] = numpages;
	for (pagenum = 0; pagenum < numpages; pagenum++)
		vtlb->table[tableindex + pagenum] = value + (pagenum << vtlb->pageshift);
}

/*-------------------------------------------------
    vtlb_dynload - load a dynamic VTLB entry
-------------------------------------------------*/

void vtlb_dynload(vtlb_state *vtlb, UINT32 index, offs_t address, vtlb_entry value)
{
	vtlb_entry entry = vtlb->table[index];

	if (vtlb->dynamic == 0)
	{
		if (PRINTF_TLB)
			printf("failed: no dynamic entries\n");
		return;
	}

	int liveindex = vtlb->dynindex++ % vtlb->dynamic;
	/* is entry already live? */
	if (!(entry & VTLB_FLAG_VALID))
	{
		/* if an entry already exists at this index, free it */
		if (vtlb->live[liveindex] != 0)
			vtlb->table[vtlb->live[liveindex] - 1] = 0;

		/* claim this new entry */
		vtlb->live[liveindex] = index + 1;
	}
	/* form a new blank entry */
	entry = (address >> vtlb->pageshift) << vtlb->pageshift;
	entry |= VTLB_FLAG_VALID | value;

	if (PRINTF_TLB)
		printf("success (%08X), new entry\n", address);

	vtlb->table[index] = entry;
}

/***************************************************************************
    FLUSHING
***************************************************************************/

/*-------------------------------------------------
    vtlb_flush_dynamic - flush all knowledge
    from the dynamic part of the VTLB
-------------------------------------------------*/

void vtlb_flush_dynamic(vtlb_state *vtlb)
{
	int liveindex;

	if (PRINTF_TLB)
		printf("vtlb_flush_dynamic\n");

	/* loop over live entries and release them from the table */
	for (liveindex = 0; liveindex < vtlb->dynamic; liveindex++)
		if (vtlb->live[liveindex] != 0)
		{
			offs_t tableindex = vtlb->live[liveindex] - 1;
			vtlb->table[tableindex] = 0;
			vtlb->live[liveindex] = 0;
		}
}


/*-------------------------------------------------
    vtlb_flush_address - flush knowledge of a
    particular address from the VTLB
-------------------------------------------------*/

void vtlb_flush_address(vtlb_state *vtlb, offs_t address)
{
	offs_t tableindex = address >> vtlb->pageshift;

	if (PRINTF_TLB)
		printf("vtlb_flush_address %08X\n", address);

	/* free the entry in the table; for speed, we leave the entry in the live array */
	vtlb->table[tableindex] = 0;
}



/***************************************************************************
    ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    vtlb_table - return a pointer to the base of
    the linear VTLB lookup table
-------------------------------------------------*/

const vtlb_entry *vtlb_table(vtlb_state *vtlb)
{
	return &vtlb->table[0];
}
