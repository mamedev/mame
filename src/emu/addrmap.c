/***************************************************************************

    addrmap.c

    Macros and helper functions for handling address map definitions.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"


//**************************************************************************
//  MACROS
//**************************************************************************

// maps a full 64-bit mask down to an 8-bit byte mask
#define UNITMASK8(x) \
	((((UINT64)(x) >> (63-7)) & 0x80) | \
	 (((UINT64)(x) >> (55-6)) & 0x40) | \
	 (((UINT64)(x) >> (47-5)) & 0x20) | \
	 (((UINT64)(x) >> (39-4)) & 0x10) | \
	 (((UINT64)(x) >> (31-3)) & 0x08) | \
	 (((UINT64)(x) >> (23-2)) & 0x04) | \
	 (((UINT64)(x) >> (15-1)) & 0x02) | \
	 (((UINT64)(x) >> ( 7-0)) & 0x01))

// maps a full 64-bit mask down to a 4-bit word mask
#define UNITMASK16(x) \
	((((UINT64)(x) >> (63-3)) & 0x08) | \
	 (((UINT64)(x) >> (47-2)) & 0x04) | \
	 (((UINT64)(x) >> (31-1)) & 0x02) | \
	 (((UINT64)(x) >> (15-0)) & 0x01))

// maps a full 64-bit mask down to a 2-bit dword mask
#define UNITMASK32(x) \
	((((UINT64)(x) >> (63-1)) & 0x02) | \
	 (((UINT64)(x) >> (31-0)) & 0x01))



//**************************************************************************
//  ADDRESS MAP ENTRY
//**************************************************************************

//-------------------------------------------------
//  address_map_entry - constructor
//-------------------------------------------------

address_map_entry::address_map_entry(address_map &map, offs_t start, offs_t end)
	: m_next(NULL),
	  m_map(map),
	  m_addrstart((map.m_globalmask == 0) ? start : start & map.m_globalmask),
	  m_addrend((map.m_globalmask == 0) ? end : end & map.m_globalmask),
	  m_addrmirror(0),
	  m_addrmask(0),
	  m_share(NULL),
	  m_baseptr(NULL),
	  m_sizeptr(NULL),
	  m_baseptroffs_plus1(0),
	  m_sizeptroffs_plus1(0),
	  m_genbaseptroffs_plus1(0),
	  m_gensizeptroffs_plus1(0),
	  m_region(NULL),
	  m_rgnoffs(0),
	  m_memory(NULL),
	  m_bytestart(0),
	  m_byteend(0),
	  m_bytemirror(0),
	  m_bytemask(0)
{
}


//-------------------------------------------------
//  set_mask - set the mask value
//-------------------------------------------------

void address_map_entry::set_mask(offs_t _mask)
{
	m_addrmask = _mask;
	if (m_map.m_globalmask != 0)
		m_addrmask &= m_map.m_globalmask;
}


//-------------------------------------------------
//  set_read_port - set up a handler for reading
//  an I/O port
//-------------------------------------------------

void address_map_entry::set_read_port(const device_config &devconfig, const char *tag)
{
	m_read.type = AMH_PORT;
	m_read.tag = devconfig.siblingtag(m_read.derived_tag, tag);
}


//-------------------------------------------------
//  set_write_port - set up a handler for writing
//  an I/O port
//-------------------------------------------------

void address_map_entry::set_write_port(const device_config &devconfig, const char *tag)
{
	m_write.type = AMH_PORT;
	m_write.tag = devconfig.siblingtag(m_write.derived_tag, tag);
}


//-------------------------------------------------
//  set_readwrite_port - set up a handler for 
//  reading and writing an I/O port
//-------------------------------------------------

void address_map_entry::set_readwrite_port(const device_config &devconfig, const char *tag)
{
	m_read.type = AMH_PORT;
	m_read.tag = devconfig.siblingtag(m_read.derived_tag, tag);
	m_write.type = AMH_PORT;
	m_write.tag = devconfig.siblingtag(m_write.derived_tag, tag);
}


//-------------------------------------------------
//  set_read_bank - set up a handler for reading
//  from a memory bank
//-------------------------------------------------

void address_map_entry::set_read_bank(const device_config &devconfig, const char *tag)
{
	m_read.type = AMH_BANK;
	m_read.tag = devconfig.siblingtag(m_read.derived_tag, tag);
}


//-------------------------------------------------
//  set_write_bank - set up a handler for writing
//  to a memory bank
//-------------------------------------------------

void address_map_entry::set_write_bank(const device_config &devconfig, const char *tag)
{
	m_write.type = AMH_BANK; 
	m_write.tag = devconfig.siblingtag(m_write.derived_tag, tag);
}


//-------------------------------------------------
//  set_readwrite_bank - set up a handler for 
//  writing to a memory bank
//-------------------------------------------------

void address_map_entry::set_readwrite_bank(const device_config &devconfig, const char *tag)
{
	m_read.type = AMH_BANK; 
	m_read.tag = devconfig.siblingtag(m_read.derived_tag, tag);
	m_write.type = AMH_BANK; 
	m_write.tag = devconfig.siblingtag(m_write.derived_tag, tag);
}


//-------------------------------------------------
//  internal_set_handler - handler setters for
//  8-bit read/write handlers
//-------------------------------------------------

void address_map_entry::internal_set_handler(read8_space_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(8, unitmask, string));
	m_read.type = AMH_HANDLER;
	m_read.bits = (unitmask == 0) ? 0 : 8;
	m_read.mask = UNITMASK8(unitmask);
	m_read.handler.read.shandler8 = func;
	m_read.name = string;
}


void address_map_entry::internal_set_handler(write8_space_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(8, unitmask, string));
	m_write.type = AMH_HANDLER;
	m_write.bits = (unitmask == 0) ? 0 : 8;
	m_write.mask = UNITMASK8(unitmask);
	m_write.handler.write.shandler8 = func;
	m_write.name = string;
}


void address_map_entry::internal_set_handler(read8_space_func rfunc, const char *rstring, write8_space_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(rfunc, rstring, unitmask);
	internal_set_handler(wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read8_device_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(8, unitmask, string));
	m_read.type = AMH_DEVICE_HANDLER;
	m_read.bits = (unitmask == 0) ? 0 : 8;
	m_read.mask = UNITMASK8(unitmask);
	m_read.handler.read.dhandler8 = func;
	m_read.name = string;
	m_read.tag = devconfig.siblingtag(m_read.derived_tag, tag);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write8_device_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(8, unitmask, string));
	m_write.type = AMH_DEVICE_HANDLER;
	m_write.bits = (unitmask == 0) ? 0 : 8;
	m_write.mask = UNITMASK8(unitmask);
	m_write.handler.write.dhandler8 = func;
	m_write.name = string;
	m_write.tag = devconfig.siblingtag(m_write.derived_tag, tag);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read8_device_func rfunc, const char *rstring, write8_device_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, rstring, unitmask);
	internal_set_handler(devconfig, tag, wfunc, wstring, unitmask);
}


//-------------------------------------------------
//  internal_set_handler - handler setters for
//  16-bit read/write handlers
//-------------------------------------------------

void address_map_entry::internal_set_handler(read16_space_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(16, unitmask, string));
	m_read.type = AMH_HANDLER;
	m_read.bits = (unitmask == 0) ? 0 : 16;
	m_read.mask = UNITMASK16(unitmask);
	m_read.handler.read.shandler16 = func;
	m_read.name = string;
}


void address_map_entry::internal_set_handler(write16_space_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(16, unitmask, string));
	m_write.type = AMH_HANDLER;
	m_write.bits = (unitmask == 0) ? 0 : 16;
	m_write.mask = UNITMASK16(unitmask);
	m_write.handler.write.shandler16 = func;
	m_write.name = string;
}


void address_map_entry::internal_set_handler(read16_space_func rfunc, const char *rstring, write16_space_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(rfunc, rstring, unitmask);
	internal_set_handler(wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read16_device_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(16, unitmask, string));
	m_read.type = AMH_DEVICE_HANDLER;
	m_read.bits = (unitmask == 0) ? 0 : 16;
	m_read.mask = UNITMASK16(unitmask);
	m_read.handler.read.dhandler16 = func;
	m_read.name = string;
	m_read.tag = devconfig.siblingtag(m_read.derived_tag, tag);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write16_device_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(16, unitmask, string));
	m_write.type = AMH_DEVICE_HANDLER;
	m_write.bits = (unitmask == 0) ? 0 : 16;
	m_write.mask = UNITMASK16(unitmask);
	m_write.handler.write.dhandler16 = func;
	m_write.name = string;
	m_write.tag = devconfig.siblingtag(m_write.derived_tag, tag);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read16_device_func rfunc, const char *rstring, write16_device_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, rstring, unitmask);
	internal_set_handler(devconfig, tag, wfunc, wstring, unitmask);
}


//-------------------------------------------------
//  internal_set_handler - handler setters for
//  32-bit read/write handlers
//-------------------------------------------------

void address_map_entry::internal_set_handler(read32_space_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(32, unitmask, string));
	m_read.type = AMH_HANDLER;
	m_read.bits = (unitmask == 0) ? 0 : 32;
	m_read.mask = UNITMASK32(unitmask);
	m_read.handler.read.shandler32 = func;
	m_read.name = string;
}


void address_map_entry::internal_set_handler(write32_space_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(32, unitmask, string));
	m_write.type = AMH_HANDLER;
	m_write.bits = (unitmask == 0) ? 0 : 32;
	m_write.mask = UNITMASK32(unitmask);
	m_write.handler.write.shandler32 = func;
	m_write.name = string;
}


void address_map_entry::internal_set_handler(read32_space_func rfunc, const char *rstring, write32_space_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(rfunc, rstring, unitmask);
	internal_set_handler(wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read32_device_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(32, unitmask, string));
	m_read.type = AMH_DEVICE_HANDLER;
	m_read.bits = (unitmask == 0) ? 0 : 32;
	m_read.mask = UNITMASK32(unitmask);
	m_read.handler.read.dhandler32 = func;
	m_read.name = string;
	m_read.tag = devconfig.siblingtag(m_read.derived_tag, tag);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write32_device_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(32, unitmask, string));
	m_write.type = AMH_DEVICE_HANDLER;
	m_write.bits = (unitmask == 0) ? 0 : 32;
	m_write.mask = UNITMASK32(unitmask);
	m_write.handler.write.dhandler32 = func;
	m_write.name = string;
	m_write.tag = devconfig.siblingtag(m_write.derived_tag, tag);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read32_device_func rfunc, const char *rstring, write32_device_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, rstring, unitmask);
	internal_set_handler(devconfig, tag, wfunc, wstring, unitmask);
}


//-------------------------------------------------
//  internal_set_handler - handler setters for
//  64-bit read/write handlers
//-------------------------------------------------

void address_map_entry::internal_set_handler(read64_space_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(64, unitmask, string));
	m_read.type = AMH_HANDLER;
	m_read.bits = (unitmask == 0) ? 0 : 64;
	m_read.mask = 0;
	m_read.handler.read.shandler64 = func;
	m_read.name = string;
}


void address_map_entry::internal_set_handler(write64_space_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(64, unitmask, string));
	m_write.type = AMH_HANDLER;
	m_write.bits = (unitmask == 0) ? 0 : 64;
	m_write.mask = 0;
	m_write.handler.write.shandler64 = func;
	m_write.name = string;
}


void address_map_entry::internal_set_handler(read64_space_func rfunc, const char *rstring, write64_space_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(rfunc, rstring, unitmask);
	internal_set_handler(wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read64_device_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(64, unitmask, string));
	m_read.type = AMH_DEVICE_HANDLER;
	m_read.bits = (unitmask == 0) ? 0 : 64;
	m_read.mask = 0;
	m_read.handler.read.dhandler64 = func;
	m_read.name = string;
	m_read.tag = devconfig.siblingtag(m_read.derived_tag, tag);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write64_device_func func, const char *string, UINT64 unitmask)
{
	assert(unitmask_is_appropriate(64, unitmask, string));
	m_write.type = AMH_DEVICE_HANDLER;
	m_write.bits = (unitmask == 0) ? 0 : 64;
	m_write.mask = 0;
	m_write.handler.write.dhandler64 = func;
	m_write.name = string;
	m_write.tag = devconfig.siblingtag(m_write.derived_tag, tag);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read64_device_func rfunc, const char *rstring, write64_device_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, rstring, unitmask);
	internal_set_handler(devconfig, tag, wfunc, wstring, unitmask);
}


//-------------------------------------------------
//  unitmask_is_appropriate - verify that the
//  provided unitmask is valid and expected
//-------------------------------------------------

bool address_map_entry::unitmask_is_appropriate(UINT8 width, UINT64 unitmask, const char *string)
{
	// if no mask, this must match the default width of the map
	if (unitmask == 0)
	{
		if (m_map.m_databits != width)
			throw emu_fatalerror("Handler %s is a %d-bit handler but was specified in a %d-bit address map", string, width, m_map.m_databits);
		return true;
	}

	// if we have a mask, we must be smaller than the default width of the map
	if (m_map.m_databits <= width)
		throw emu_fatalerror("Handler %s is a %d-bit handler and is too wide to be used in a %d-bit address map", string, width, m_map.m_databits);

	// the mask must represent whole units of width
	UINT32 basemask = (width == 8) ? 0xff : (width == 16) ? 0xffff : 0xffffffff;
	UINT64 singlemask = basemask;
	while (singlemask != 0)
	{
		if ((unitmask & singlemask) != 0 && (unitmask & singlemask) != singlemask)
			throw emu_fatalerror("Handler %s specified a mask of %08X%08X; needs to be in even chunks of %X", string, (UINT32)(unitmask >> 32), (UINT32)unitmask, basemask);
		singlemask <<= width;
	}
	return true;
}



//**************************************************************************
//  WIDTH-SPECIFIC ADDRESS MAP ENTRY CONSTRUCTORS
//**************************************************************************

//-------------------------------------------------
//  address_map_entry8 - constructor
//-------------------------------------------------

address_map_entry8::address_map_entry8(address_map &map, offs_t start, offs_t end)
	: address_map_entry(map, start, end)
{
}


//-------------------------------------------------
//  address_map_entry16 - constructor
//-------------------------------------------------

address_map_entry16::address_map_entry16(address_map &map, offs_t start, offs_t end)
	: address_map_entry(map, start, end)
{
}


//-------------------------------------------------
//  address_map_entry32 - constructor
//-------------------------------------------------

address_map_entry32::address_map_entry32(address_map &map, offs_t start, offs_t end)
	: address_map_entry(map, start, end)
{
}


//-------------------------------------------------
//  address_map_entry64 - constructor
//-------------------------------------------------

address_map_entry64::address_map_entry64(address_map &map, offs_t start, offs_t end)
	: address_map_entry(map, start, end)
{
}



//**************************************************************************
//  ADDRESS MAP
//**************************************************************************

//-------------------------------------------------
//  address_map - constructor
//-------------------------------------------------

address_map::address_map(const device_config &devconfig, int spacenum)
	: m_spacenum(spacenum),
	  m_databits(0xff),
	  m_unmapval(0),
	  m_globalmask(0),
	  m_entrylist(NULL),
	  m_tailptr(&m_entrylist)
{
	// get our memory interface
	const device_config_memory_interface *memintf;
	if (!devconfig.interface(memintf))
		throw emu_fatalerror("No memory interface defined for device '%s'\n", devconfig.tag());

	// and then the configuration for the current address space
	const address_space_config *spaceconfig = memintf->space_config(spacenum);
	if (!devconfig.interface(memintf))
		throw emu_fatalerror("No memory address space configuration found for device '%s', space %d\n", devconfig.tag(), spacenum);

	// append the internal device map (first so it takes priority) */
	if (spaceconfig->m_internal_map != NULL)
		(*spaceconfig->m_internal_map)(*this, devconfig);

	// construct the standard map */
	if (memintf->address_map(spacenum) != NULL)
		(*memintf->address_map(spacenum))(*this, devconfig);

	// append the default device map (last so it can be overridden) */
	if (spaceconfig->m_default_map != NULL)
		(*spaceconfig->m_default_map)(*this, devconfig);
}


//-------------------------------------------------
//  ~address_map - destructor
//-------------------------------------------------

address_map::~address_map()
{
	// free all entries */
	while (m_entrylist != NULL)
	{
		address_map_entry *entry = m_entrylist;
		m_entrylist = entry->m_next;
		global_free(entry);
	}
}


//-------------------------------------------------
//  configure - either configure the space and
//  databits, or verify they match previously-set
//  values
//-------------------------------------------------

void address_map::configure(UINT8 spacenum, UINT8 databits)
{
	assert(m_spacenum == spacenum);
	if (m_databits == 0xff)
		m_databits = databits;
	else
		assert(m_databits == databits);
}


//-------------------------------------------------
//  append - append an entry to the end of the
//  list
//-------------------------------------------------

void address_map::set_global_mask(offs_t mask)
{
//	if (m_entrylist != NULL)
//		throw emu_fatalerror("AM_GLOBALMASK must be specified before any entries");
	m_globalmask = mask;
}



//-------------------------------------------------
//  add - add a new entry of the appropriate type
//-------------------------------------------------

address_map_entry8 *address_map::add(offs_t start, offs_t end, address_map_entry8 *ptr)
{
	ptr = global_alloc(address_map_entry8(*this, start, end));
	*m_tailptr = ptr;
	m_tailptr = &ptr->m_next;
	return ptr;
}


address_map_entry16 *address_map::add(offs_t start, offs_t end, address_map_entry16 *ptr)
{
	ptr = global_alloc(address_map_entry16(*this, start, end));
	*m_tailptr = ptr;
	m_tailptr = &ptr->m_next;
	return ptr;
}


address_map_entry32 *address_map::add(offs_t start, offs_t end, address_map_entry32 *ptr)
{
	ptr = global_alloc(address_map_entry32(*this, start, end));
	*m_tailptr = ptr;
	m_tailptr = &ptr->m_next;
	return ptr;
}


address_map_entry64 *address_map::add(offs_t start, offs_t end, address_map_entry64 *ptr)
{
	ptr = global_alloc(address_map_entry64(*this, start, end));
	*m_tailptr = ptr;
	m_tailptr = &ptr->m_next;
	return ptr;
}



#if 0

// old code for reference

/***************************************************************************
    ADDRESS MAP HELPERS
***************************************************************************/

/*-------------------------------------------------
    map_detokenize - detokenize an array of
    address map tokens
-------------------------------------------------*/

#define check_map(field) do { \
	if (map->field != 0 && map->field != tmap.field) \
		fatalerror("%s: %s included a mismatched address map (%s %d) for an existing map with %s %d!\n", driver->source_file, driver->name, #field, tmap.field, #field, map->field); \
	} while (0)

#define check_entry_handler(row) do { \
	if (entry->row.type != AMH_NONE) \
		fatalerror("%s: %s AM_RANGE(0x%x, 0x%x) %s handler already set!\n", driver->source_file, driver->name, entry->addrstart, entry->addrend, #row); \
	} while (0)

#define check_entry_field(field) do { \
	if (entry->field != 0) \
		fatalerror("%s: %s AM_RANGE(0x%x, 0x%x) setting %s already set!\n", driver->source_file, driver->name, entry->addrstart, entry->addrend, #field); \
	} while (0)

static void map_detokenize(memory_private *memdata, address_map *map, const game_driver *driver, const device_config *devconfig, const addrmap_token *tokens)
{
	address_map_entry **firstentryptr;
	address_map_entry **entryptr;
	address_map_entry *entry;
	address_map tmap = {0};
	UINT32 entrytype;
	int maptype;

	/* check the first token */
	TOKEN_GET_UINT32_UNPACK3(tokens, entrytype, 8, tmap.spacenum, 8, tmap.databits, 8);
	if (entrytype != ADDRMAP_TOKEN_START)
		fatalerror("%s: %s Address map missing ADDRMAP_TOKEN_START!\n", driver->source_file, driver->name);
	if (tmap.spacenum >= ADDRESS_SPACES)
		fatalerror("%s: %s Invalid address space %d for memory map!\n", driver->source_file, driver->name, tmap.spacenum);
	if (tmap.databits != 8 && tmap.databits != 16 && tmap.databits != 32 && tmap.databits != 64)
		fatalerror("%s: %s Invalid data bits %d for memory map!\n", driver->source_file, driver->name, tmap.databits);
	check_map(spacenum);
	check_map(databits);

	/* fill in the map values */
	map->spacenum = tmap.spacenum;
	map->databits = tmap.databits;

	/* find the end of the list */
	for (entryptr = &map->entrylist; *entryptr != NULL; entryptr = &(*entryptr)->next) ;
	firstentryptr = entryptr;
	entry = NULL;

	/* loop over tokens until we hit the end */
	while (entrytype != ADDRMAP_TOKEN_END)
	{
		/* unpack the token from the first entry */
		TOKEN_GET_UINT32_UNPACK1(tokens, entrytype, 8);
		switch (entrytype)
		{
			/* end */
			case ADDRMAP_TOKEN_END:
				break;

			/* including */
			case ADDRMAP_TOKEN_INCLUDE:
				map_detokenize(memdata, map, driver, devconfig, TOKEN_GET_PTR(tokens, tokenptr));
				for (entryptr = &map->entrylist; *entryptr != NULL; entryptr = &(*entryptr)->next) ;
				entry = NULL;
				break;

			/* global flags */
			case ADDRMAP_TOKEN_GLOBAL_MASK:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, tmap.globalmask, 32);
				check_map(globalmask);
				map->globalmask = tmap.globalmask;
				break;

			case ADDRMAP_TOKEN_UNMAP_VALUE:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, tmap.unmapval, 1);
				check_map(unmapval);
				map->unmapval = tmap.unmapval;
				break;

			/* start a new range */
			case ADDRMAP_TOKEN_RANGE:
				entry = *entryptr = global_alloc_clear(address_map_entry);
				entryptr = &entry->next;
				TOKEN_GET_UINT64_UNPACK2(tokens, entry->addrstart, 32, entry->addrend, 32);
				break;

			case ADDRMAP_TOKEN_MASK:
				check_entry_field(addrmask);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, entry->addrmask, 32);
				break;

			case ADDRMAP_TOKEN_MIRROR:
				check_entry_field(addrmirror);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, entry->addrmirror, 32);
				if (entry->addrmirror != 0)
				{
					entry->addrstart &= ~entry->addrmirror;
					entry->addrend &= ~entry->addrmirror;
				}
				break;

			case ADDRMAP_TOKEN_READ:
				check_entry_handler(read);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK4(tokens, entrytype, 8, maptype, 8, entry->read.bits, 8, entry->read.mask, 8);
				entry->read.type = (map_handler_type)maptype;
				if (entry->read.type == AMH_HANDLER || entry->read.type == AMH_DEVICE_HANDLER)
				{
					entry->read.handler.read = TOKEN_GET_PTR(tokens, read);
					entry->read.name = TOKEN_GET_STRING(tokens);
				}
				if (entry->read.type == AMH_DEVICE_HANDLER || entry->read.type == AMH_PORT || entry->read.type == AMH_BANK)
					entry->read.tag = devconfig->siblingtag(entry->read.derived_tag, TOKEN_GET_STRING(tokens));
				break;

			case ADDRMAP_TOKEN_WRITE:
				check_entry_handler(write);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK4(tokens, entrytype, 8, maptype, 8, entry->write.bits, 8, entry->write.mask, 8);
				entry->write.type = (map_handler_type)maptype;
				if (entry->write.type == AMH_HANDLER || entry->write.type == AMH_DEVICE_HANDLER)
				{
					entry->write.handler.write = TOKEN_GET_PTR(tokens, write);
					entry->write.name = TOKEN_GET_STRING(tokens);
				}
				if (entry->write.type == AMH_DEVICE_HANDLER || entry->write.type == AMH_PORT || entry->write.type == AMH_BANK)
					entry->write.tag = devconfig->siblingtag(entry->write.derived_tag, TOKEN_GET_STRING(tokens));
				break;

			case ADDRMAP_TOKEN_READWRITE:
				check_entry_handler(read);
				check_entry_handler(write);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK4(tokens, entrytype, 8, maptype, 8, entry->read.bits, 8, entry->read.mask, 8);
				entry->write.type = entry->read.type = (map_handler_type)maptype;
				entry->write.bits = entry->read.bits;
				entry->write.mask = entry->read.mask;
				if (entry->read.type == AMH_HANDLER || entry->read.type == AMH_DEVICE_HANDLER)
				{
					entry->read.handler.read = TOKEN_GET_PTR(tokens, read);
					entry->read.name = TOKEN_GET_STRING(tokens);
					entry->write.handler.write = TOKEN_GET_PTR(tokens, write);
					entry->write.name = TOKEN_GET_STRING(tokens);
				}
				if (entry->read.type == AMH_DEVICE_HANDLER || entry->read.type == AMH_PORT || entry->read.type == AMH_BANK)
				{
					const char *basetag = TOKEN_GET_STRING(tokens);
					entry->read.tag = devconfig->siblingtag(entry->read.derived_tag, basetag);
					entry->write.tag = devconfig->siblingtag(entry->write.derived_tag, basetag);
				}
				break;

			case ADDRMAP_TOKEN_REGION:
				check_entry_field(region);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, entry->rgnoffs, 32);
				entry->region = devconfig->siblingtag(entry->region_string, TOKEN_GET_STRING(tokens));
				break;

			case ADDRMAP_TOKEN_SHARE:
				check_entry_field(share);
				entry->share = TOKEN_GET_STRING(tokens);
				if (memdata != NULL)
					memdata->sharemap.add(entry->share, UNMAPPED_SHARE_PTR, FALSE);
				break;

			case ADDRMAP_TOKEN_BASEPTR:
				check_entry_field(baseptr);
				entry->baseptr = (void **)TOKEN_GET_PTR(tokens, voidptr);
				break;

			case ADDRMAP_TOKEN_BASE_MEMBER:
				check_entry_field(baseptroffs_plus1);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, entry->baseptroffs_plus1, 24);
				entry->baseptroffs_plus1++;
				break;

			case ADDRMAP_TOKEN_BASE_GENERIC:
				check_entry_field(genbaseptroffs_plus1);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, entry->genbaseptroffs_plus1, 24);
				entry->genbaseptroffs_plus1++;
				break;

			case ADDRMAP_TOKEN_SIZEPTR:
				check_entry_field(sizeptr);
				entry->sizeptr = TOKEN_GET_PTR(tokens, sizeptr);
				break;

			case ADDRMAP_TOKEN_SIZE_MEMBER:
				check_entry_field(sizeptroffs_plus1);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, entry->sizeptroffs_plus1, 24);
				entry->sizeptroffs_plus1++;
				break;

			case ADDRMAP_TOKEN_SIZE_GENERIC:
				check_entry_field(gensizeptroffs_plus1);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, entry->gensizeptroffs_plus1, 24);
				entry->gensizeptroffs_plus1++;
				break;

			default:
				fatalerror("Invalid token %d in address map\n", entrytype);
				break;
		}
	}

	/* post-process to apply the global mask */
	if (map->globalmask != 0)
		for (entry = map->entrylist; entry != NULL; entry = entry->next)
		{
			entry->addrstart &= map->globalmask;
			entry->addrend &= map->globalmask;
			entry->addrmask &= map->globalmask;
		}
}

#endif
