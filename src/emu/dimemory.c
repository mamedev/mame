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
//  ADDRESS SPACE CONFIG
//**************************************************************************

//-------------------------------------------------
//  address_space_config - constructors
//-------------------------------------------------

address_space_config::address_space_config()
	: m_name("unknown"),
	  m_endianness(ENDIANNESS_NATIVE),
	  m_databus_width(0),
	  m_addrbus_width(0),
	  m_addrbus_shift(0),
	  m_logaddr_width(0),
	  m_page_shift(0),
	  m_internal_map(NULL),
	  m_default_map(NULL)
{
}

address_space_config::address_space_config(const char *name, endianness_t endian, UINT8 datawidth, UINT8 addrwidth, INT8 addrshift, address_map_constructor internal, address_map_constructor defmap)
	: m_name(name),
	  m_endianness(endian),
	  m_databus_width(datawidth),
	  m_addrbus_width(addrwidth),
	  m_addrbus_shift(addrshift),
	  m_logaddr_width(addrwidth),
	  m_page_shift(0),
	  m_internal_map(internal),
	  m_default_map(defmap)
{
}

address_space_config::address_space_config(const char *name, endianness_t endian, UINT8 datawidth, UINT8 addrwidth, INT8 addrshift, UINT8 logwidth, UINT8 pageshift, address_map_constructor internal, address_map_constructor defmap)
	: m_name(name),
	  m_endianness(endian),
	  m_databus_width(datawidth),
	  m_addrbus_width(addrwidth),
	  m_addrbus_shift(addrshift),
	  m_logaddr_width(logwidth),
	  m_page_shift(pageshift),
	  m_internal_map(internal),
	  m_default_map(defmap)
{
}



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
//  static_set_vblank_int - configuration helper
//  to set up VBLANK interrupts on the device
//-------------------------------------------------

void device_config_memory_interface::static_set_addrmap(device_config *device, int spacenum, address_map_constructor map)
{
	device_config_memory_interface *memory = dynamic_cast<device_config_memory_interface *>(device);
	if (memory == NULL)
		throw emu_fatalerror("MCFG_DEVICE_ADDRESS_MAP called on device '%s' with no memory interface", device->tag());
	if (spacenum >= ARRAY_LENGTH(memory->m_address_map))
		throw emu_fatalerror("MCFG_DEVICE_ADDRESS_MAP called with out-of-range space number %d", device->tag(), spacenum);
	memory->m_address_map[spacenum] = map;
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
			::address_map *map = global_alloc(::address_map(*devconfig, spacenum));

			// if this is an empty map, just skip it
			if (map->m_entrylist.first() == NULL)
			{
				global_free(map);
				continue;
			}

			// validate the global map parameters
			if (map->m_spacenum != spacenum)
			{
				mame_printf_error("%s: %s device '%s' space %d has address space %d handlers!\n", driver.source_file, driver.name, devconfig->tag(), spacenum, map->m_spacenum);
				error = true;
			}
			if (map->m_databits != datawidth)
			{
				mame_printf_error("%s: %s device '%s' uses wrong memory handlers for %s space! (width = %d, memory = %08x)\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, datawidth, map->m_databits);
				error = true;
			}

			// loop over entries and look for errors
			for (address_map_entry *entry = map->m_entrylist.first(); entry != NULL; entry = entry->next())
			{
				UINT32 bytestart = spaceconfig->addr2byte(entry->m_addrstart);
				UINT32 byteend = spaceconfig->addr2byte_end(entry->m_addrend);

				// look for overlapping entries
				if (!detected_overlap)
				{
					address_map_entry *scan;
					for (scan = map->m_entrylist.first(); scan != entry; scan = scan->next())
						if (entry->m_addrstart <= scan->m_addrend && entry->m_addrend >= scan->m_addrstart &&
							((entry->m_read.m_type != AMH_NONE && scan->m_read.m_type != AMH_NONE) ||
							 (entry->m_write.m_type != AMH_NONE && scan->m_write.m_type != AMH_NONE)))
						{
							mame_printf_warning("%s: %s '%s' %s space has overlapping memory (%X-%X,%d,%d) vs (%X-%X,%d,%d)\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, entry->m_addrstart, entry->m_addrend, entry->m_read.m_type, entry->m_write.m_type, scan->m_addrstart, scan->m_addrend, scan->m_read.m_type, scan->m_write.m_type);
							detected_overlap = true;
							break;
						}
				}

				// look for inverted start/end pairs
				if (byteend < bytestart)
				{
					mame_printf_error("%s: %s wrong %s memory read handler start = %08x > end = %08x\n", driver.source_file, driver.name, spaceconfig->m_name, entry->m_addrstart, entry->m_addrend);
					error = true;
				}

				// look for misaligned entries
				if ((bytestart & (alignunit - 1)) != 0 || (byteend & (alignunit - 1)) != (alignunit - 1))
				{
					mame_printf_error("%s: %s wrong %s memory read handler start = %08x, end = %08x ALIGN = %d\n", driver.source_file, driver.name, spaceconfig->m_name, entry->m_addrstart, entry->m_addrend, alignunit);
					error = true;
				}

				// if this is a program space, auto-assign implicit ROM entries
				if (entry->m_read.m_type == AMH_ROM && entry->m_region == NULL)
				{
					entry->m_region = devconfig->tag();
					entry->m_rgnoffs = entry->m_addrstart;
				}

				// if this entry references a memory region, validate it
				if (entry->m_region != NULL && entry->m_share == 0)
				{
					// look for the region
					bool found = false;
					for (const rom_source *source = rom_first_source(m_machine_config); source != NULL && !found; source = rom_next_source(*source))
						for (const rom_entry *romp = rom_first_region(*source); !ROMENTRY_ISEND(romp) && !found; romp++)
						{
							const char *regiontag = ROMREGION_GETTAG(romp);
							if (regiontag != NULL)
							{
								astring fulltag;
								rom_region_name(fulltag, &driver, source, romp);
								if (fulltag.cmp(entry->m_region) == 0)
								{
									// verify the address range is within the region's bounds
									offs_t length = ROMREGION_GETLENGTH(romp);
									if (entry->m_rgnoffs + (byteend - bytestart + 1) > length)
									{
										mame_printf_error("%s: %s device '%s' %s space memory map entry %X-%X extends beyond region '%s' size (%X)\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, entry->m_addrstart, entry->m_addrend, entry->m_region, length);
										error = true;
									}
									found = true;
								}
							}
						}

					// error if not found
					if (!found)
					{
						mame_printf_error("%s: %s device '%s' %s space memory map entry %X-%X references non-existant region '%s'\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, entry->m_addrstart, entry->m_addrend, entry->m_region);
						error = true;
					}
				}

				// make sure all devices exist
				if ((entry->m_read.m_type == AMH_LEGACY_DEVICE_HANDLER && entry->m_read.m_tag != NULL && m_machine_config.m_devicelist.find(entry->m_read.m_tag) == NULL) ||
					(entry->m_write.m_type == AMH_LEGACY_DEVICE_HANDLER && entry->m_write.m_tag != NULL && m_machine_config.m_devicelist.find(entry->m_write.m_tag) == NULL))
				{
					mame_printf_error("%s: %s device '%s' %s space memory map entry references nonexistant device '%s'\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, entry->m_write.m_tag);
					error = true;
				}

				// make sure ports exist
//              if ((entry->m_read.m_type == AMH_PORT && entry->m_read.m_tag != NULL && portlist.find(entry->m_read.m_tag) == NULL) ||
//                  (entry->m_write.m_type == AMH_PORT && entry->m_write.m_tag != NULL && portlist.find(entry->m_write.m_tag) == NULL))
//              {
//                  mame_printf_error("%s: %s device '%s' %s space memory map entry references nonexistant port tag '%s'\n", driver.source_file, driver.name, devconfig->tag(), spaceconfig->m_name, entry->m_read.tag);
//                  error = true;
//              }

				// validate bank and share tags
				if (entry->m_read.m_type == AMH_BANK && !validate_tag(driver, "bank", entry->m_read.m_tag))
					error = true ;
				if (entry->m_write.m_type == AMH_BANK && !validate_tag(driver, "bank", entry->m_write.m_tag))
					error = true;
				if (entry->m_share != NULL && !validate_tag(driver, "share", entry->m_share))
					error = true;
			}

			// release the address map
			global_free(map);
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

void device_memory_interface::set_address_space(int spacenum, address_space &space)
{
	assert(spacenum < ARRAY_LENGTH(m_addrspace));
	m_addrspace[spacenum] = &space;
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
