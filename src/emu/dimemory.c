/***************************************************************************

    dimemory.c

    Device memory interfaces.

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
#include "validity.h"


//**************************************************************************
//  PARAMETERS
//**************************************************************************

#define DETECT_OVERLAPPING_MEMORY	(0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int TRIGGER_SUSPENDTIME = -4000;



//**************************************************************************
//  MEMORY DEVICE CONFIG
//**************************************************************************

//-------------------------------------------------
//  device_config_memory_interface - constructor
//-------------------------------------------------

device_config_memory_interface::device_config_memory_interface(const machine_config &mconfig, device_config &devconfig)
	: device_config_interface(mconfig, devconfig)
{
	// initialize remaining members
	memset(m_address_map, 0, sizeof(m_address_map));
}


//-------------------------------------------------
//  device_config_memory_interface - destructor
//-------------------------------------------------

device_config_memory_interface::~device_config_memory_interface()
{
}


//-------------------------------------------------
//  memory_space_config - return configuration for
//  the given space by index, or NULL if the space
//  does not exist
//-------------------------------------------------

const address_space_config *device_config_memory_interface::memory_space_config(int spacenum) const
{
	return NULL;
}


//-------------------------------------------------
//  interface_process_token - token processing for
//  the memory interface
//-------------------------------------------------

bool device_config_memory_interface::interface_process_token(UINT32 entrytype, const machine_config_token *&tokens)
{
	UINT32 data32;

	switch (entrytype)
	{
		// specify device address map
		case MCONFIG_TOKEN_DIMEMORY_MAP:
			TOKEN_UNGET_UINT32(tokens);
			TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, data32, 8);
			assert(data32 < ARRAY_LENGTH(m_address_map));
			m_address_map[data32] = TOKEN_GET_PTR(tokens, addrmap);
			return true;
	}

	return false;
}


//-------------------------------------------------
//  interface_validity_check - perform validity
//  checks on the memory configuration
//-------------------------------------------------

bool device_config_memory_interface::interface_validity_check(const game_driver &driver) const
{
	const device_config *devconfig = crosscast<const device_config *>(this);
	bool detected_overlap = DETECT_OVERLAPPING_MEMORY ? false : true;
	bool error = false;

	// loop over all address spaces
	for (int spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
	{
		const address_space_config *spaceconfig = space_config(spacenum);
		if (spaceconfig != NULL)
		{
			int datawidth = spaceconfig->m_databus_width;
			int alignunit = datawidth / 8;

			// construct the maps
			::address_map *map = address_map_alloc(devconfig, &driver, spacenum, NULL);

			// if this is an empty map, just skip it
			if (map->entrylist == NULL)
			{
				address_map_free(map);
				continue;
			}

			// validate the global map parameters
			if (map->spacenum != spacenum)
			{
				mame_printf_error("%s: %s device '%s' space %d has address space %d handlers!\n", driver.source_file, driver.name, devconfig->tag(), spacenum, map->spacenum);
				error = true;
			}
			if (map->databits != datawidth)
			{
				mame_printf_error("%s: %s device '%s' uses wrong memory handlers for %s space! (width = %d, memory = %08x)\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, datawidth, map->databits);
				error = true;
			}

			// loop over entries and look for errors
			for (address_map_entry *entry = map->entrylist; entry != NULL; entry = entry->next)
			{
				UINT32 bytestart = spaceconfig->addr2byte(entry->addrstart);
				UINT32 byteend = spaceconfig->addr2byte_end(entry->addrend);

				// look for overlapping entries
				if (!detected_overlap)
				{
					address_map_entry *scan;
					for (scan = map->entrylist; scan != entry; scan = scan->next)
						if (entry->addrstart <= scan->addrend && entry->addrend >= scan->addrstart &&
							((entry->read.type != AMH_NONE && scan->read.type != AMH_NONE) ||
							 (entry->write.type != AMH_NONE && scan->write.type != AMH_NONE)))
						{
							mame_printf_warning("%s: %s '%s' %s space has overlapping memory (%X-%X,%d,%d) vs (%X-%X,%d,%d)\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, entry->addrstart, entry->addrend, entry->read.type, entry->write.type, scan->addrstart, scan->addrend, scan->read.type, scan->write.type);
							detected_overlap = true;
							break;
						}
				}

				// look for inverted start/end pairs
				if (byteend < bytestart)
				{
					mame_printf_error("%s: %s wrong %s memory read handler start = %08x > end = %08x\n", driver.source_file, driver.name, spaceconfig->m_name, entry->addrstart, entry->addrend);
					error = true;
				}

				// look for misaligned entries
				if ((bytestart & (alignunit - 1)) != 0 || (byteend & (alignunit - 1)) != (alignunit - 1))
				{
					mame_printf_error("%s: %s wrong %s memory read handler start = %08x, end = %08x ALIGN = %d\n", driver.source_file, driver.name, spaceconfig->m_name, entry->addrstart, entry->addrend, alignunit);
					error = true;
				}

				// if this is a program space, auto-assign implicit ROM entries
				if (entry->read.type == AMH_ROM && entry->region == NULL)
				{
					entry->region = devconfig->tag();
					entry->rgnoffs = entry->addrstart;
				}

				// if this entry references a memory region, validate it
				if (entry->region != NULL && entry->share == 0)
				{
					// look for the region
					bool found = false;
					for (const rom_source *source = rom_first_source(&driver, &m_machine_config); source != NULL && !found; source = rom_next_source(&driver, &m_machine_config, source))
						for (const rom_entry *romp = rom_first_region(&driver, source); !ROMENTRY_ISEND(romp) && !found; romp++)
						{
							const char *regiontag = ROMREGION_GETTAG(romp);
							if (regiontag != NULL)
							{
								astring fulltag;
								rom_region_name(fulltag, &driver, source, romp);
								if (fulltag.cmp(entry->region) == 0)
								{
									// verify the address range is within the region's bounds
									offs_t length = ROMREGION_GETLENGTH(romp);
									if (entry->rgnoffs + (byteend - bytestart + 1) > length)
									{
										mame_printf_error("%s: %s device '%s' %s space memory map entry %X-%X extends beyond region '%s' size (%X)\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, entry->addrstart, entry->addrend, entry->region, length);
										error = true;
									}
									found = true;
								}
							}
						}

					// error if not found
					if (!found)
					{
						mame_printf_error("%s: %s device '%s' %s space memory map entry %X-%X references non-existant region '%s'\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, entry->addrstart, entry->addrend, entry->region);
						error = true;
					}
				}

				// make sure all devices exist
				if ((entry->read.type == AMH_DEVICE_HANDLER && entry->read.tag != NULL && m_machine_config.devicelist.find(entry->read.tag) == NULL) ||
					(entry->write.type == AMH_DEVICE_HANDLER && entry->write.tag != NULL && m_machine_config.devicelist.find(entry->write.tag) == NULL))
				{
					mame_printf_error("%s: %s device '%s' %s space memory map entry references nonexistant device '%s'\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, entry->write.tag);
					error = true;
				}

				// make sure ports exist
//              if ((entry->read.type == AMH_PORT && entry->read.tag != NULL && portlist.find(entry->read.tag) == NULL) ||
//                  (entry->write.type == AMH_PORT && entry->write.tag != NULL && portlist.find(entry->write.tag) == NULL))
//              {
//                  mame_printf_error("%s: %s device '%s' %s space memory map entry references nonexistant port tag '%s'\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, entry->read.tag);
//                  error = true;
//              }

				// validate bank and share tags
				if (entry->read.type == AMH_BANK && !validate_tag(&driver, "bank", entry->read.tag))
					error = true ;
				if (entry->write.type == AMH_BANK && !validate_tag(&driver, "bank", entry->write.tag))
					error = true;
				if (entry->share != NULL && !validate_tag(&driver, "share", entry->share))
					error = true;
			}

			// release the address map
			address_map_free(map);
		}
	}
	return error;
}



//**************************************************************************
//  MEMORY DEVICE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_memory_interface - constructor
//-------------------------------------------------

device_memory_interface::device_memory_interface(running_machine &machine, const device_config &config, device_t &device)
	: device_interface(machine, config, device),
	  m_memory_config(dynamic_cast<const device_config_memory_interface &>(config))
{
	memset(m_addrspace, 0, sizeof(m_addrspace));
}


//-------------------------------------------------
//  ~device_memory_interface - destructor
//-------------------------------------------------

device_memory_interface::~device_memory_interface()
{
}


//-------------------------------------------------
//  set_address_space - connect an address space
//  to a device
//-------------------------------------------------

void device_memory_interface::set_address_space(int spacenum, const address_space *space)
{
	assert(spacenum < ARRAY_LENGTH(m_addrspace));
	m_addrspace[spacenum] = space;
}


//-------------------------------------------------
//  memory_translate - translate from logical to
//  phyiscal addresses; designed to be overridden
//  by the actual device implementation if address
//  translation is supported
//-------------------------------------------------

bool device_memory_interface::memory_translate(int spacenum, int intention, offs_t &address)
{
	// by default it maps directly
	return true;
}


//-------------------------------------------------
//  memory_read - perform internal memory
//  operations that bypass the memory system;
//  designed to be overridden by the actual device
//  implementation if internal read operations are
//  handled by bypassing the memory system
//-------------------------------------------------

bool device_memory_interface::memory_read(int spacenum, offs_t offset, int size, UINT64 &value)
{
	// by default, we don't do anything
	return false;
}


//-------------------------------------------------
//  memory_write - perform internal memory
//  operations that bypass the memory system;
//  designed to be overridden by the actual device
//  implementation if internal write operations are
//  handled by bypassing the memory system
//-------------------------------------------------

bool device_memory_interface::memory_write(int spacenum, offs_t offset, int size, UINT64 value)
{
	// by default, we don't do anything
	return false;
}


//-------------------------------------------------
//  memory_readop - perform internal memory
//  operations that bypass the memory system;
//  designed to be overridden by the actual device
//  implementation if internal opcode fetching
//  operations are handled by bypassing the memory
//  system
//-------------------------------------------------

bool device_memory_interface::memory_readop(offs_t offset, int size, UINT64 &value)
{
	// by default, we don't do anything
	return false;
}
