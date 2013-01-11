/**********************************************************************

    cartslot.c

    Cartridge device

**********************************************************************/

#include <ctype.h>
#include "emu.h"
#include "cartslot.h"


// device type definition
const device_type CARTSLOT = &device_creator<cartslot_image_device>;

//-------------------------------------------------
//  cartslot_image_device - constructor
//-------------------------------------------------

cartslot_image_device::cartslot_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CARTSLOT, "Cartslot", tag, owner, clock),
		device_image_interface(mconfig, *this),
		m_extensions("bin"),
		m_interface(NULL),
		m_must_be_loaded(0),
		m_device_start(NULL),
		m_device_load(NULL),
		m_device_unload(NULL),
		m_device_partialhash(NULL),
		m_device_displayinfo(NULL)
{

}

//-------------------------------------------------
//  cartslot_image_device - destructor
//-------------------------------------------------

cartslot_image_device::~cartslot_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cartslot_image_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    load_cartridge
-------------------------------------------------*/

int cartslot_image_device::load_cartridge(const rom_entry *romrgn, const rom_entry *roment, bool load)
{
	const char *region;
	const char *type;
	UINT32 flags;
	offs_t offset, size, read_length, pos = 0, len;
	UINT8 *ptr;
	UINT8 clear_val;
	int datawidth, littleendian, i, j;
	device_t *cpu;

	astring regiontag;
	device().siblingtag(regiontag, ROMREGION_GETTAG(romrgn));
	region = regiontag.cstr();
	offset = ROM_GETOFFSET(roment);
	size = ROM_GETLENGTH(roment);
	flags = ROM_GETFLAGS(roment);
	ptr = ((UINT8 *) device().machine().root_device().memregion(region)->base()) + offset;

	if (load)
	{
		if (software_entry() == NULL)
		{
			/* must this be full size */
			if (flags & ROM_FULLSIZE)
			{
				if (length() != size)
					return IMAGE_INIT_FAIL;
			}

			/* read the ROM */
			pos = read_length = fread(ptr, size);

			/* reset the ROM to the initial point. */
			/* eventually, we could add a flag to allow the ROM to continue instead of restarting whenever a new cart region is present */
			fseek(0, SEEK_SET);
		}
		else
		{
			/* must this be full size */
			if (flags & ROM_FULLSIZE)
			{
				if (get_software_region_length("rom") != size)
					return IMAGE_INIT_FAIL;
			}

			/* read the ROM */
			pos = read_length = get_software_region_length("rom");
			memcpy(ptr, get_software_region("rom"), read_length);
		}

		/* do we need to mirror the ROM? */
		if (flags & ROM_MIRROR)
		{
			while(pos < size)
			{
				len = MIN(read_length, size - pos);
				memcpy(ptr + pos, ptr, len);
				pos += len;
			}
		}

		/* postprocess this region */
		type = regiontag.cstr();
		littleendian = ROMREGION_ISLITTLEENDIAN(romrgn);
		datawidth = ROMREGION_GETWIDTH(romrgn) / 8;

		/* if the region is inverted, do that now */
		device_memory_interface *memory;
		cpu = device().machine().device(type);
		if (cpu!=NULL && cpu->interface(memory))
		{
			datawidth = cpu->memory().space_config(AS_PROGRAM)->m_databus_width / 8;
			littleendian = (cpu->memory().space_config()->m_endianness == ENDIANNESS_LITTLE);
		}

		/* swap the endianness if we need to */
#ifdef LSB_FIRST
		if (datawidth > 1 && !littleendian)
#else
		if (datawidth > 1 && littleendian)
#endif
		{
			for (i = 0; i < size; i += datawidth)
			{
				UINT8 temp[8];
				memcpy(temp, &ptr[i], datawidth);
				for (j = datawidth - 1; j >= 0; j--)
					ptr[i + j] = temp[datawidth - 1 - j];
			}
		}
	}

	/* clear out anything that remains */
	if (!(flags & ROM_NOCLEAR))
	{
		clear_val = (flags & ROM_FILL_FF) ? 0xFF : 0x00;
		memset(ptr + pos, clear_val, size - pos);
	}
	return IMAGE_INIT_PASS;
}



/*-------------------------------------------------
    process_cartridge
-------------------------------------------------*/

int cartslot_image_device::process_cartridge(bool load)
{
	const rom_entry *romrgn, *roment;
	int result = 0;

	device_iterator deviter(device().mconfig().root_device());
	for (device_t *device = deviter.first(); device != NULL; device = deviter.next())
		for (romrgn = rom_first_region(*device); romrgn != NULL; romrgn = rom_next_region(romrgn))
		{
			roment = romrgn + 1;
			while(!ROMENTRY_ISREGIONEND(roment))
			{
				if (ROMENTRY_GETTYPE(roment) == ROMENTRYTYPE_CARTRIDGE)
				{
					astring regiontag;
					this->device().siblingtag(regiontag, roment->_hashdata);

					if (strcmp(regiontag.cstr(),this->device().tag())==0)
					{
						result |= load_cartridge(romrgn, roment, load);

						/* if loading failed in any cart region, stop loading */
						if (result)
							return result;
					}
				}
				roment++;
			}
		}

	return IMAGE_INIT_PASS;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cartslot_image_device::device_start()
{

	/* if this cartridge has a custom DEVICE_START, use it */
	if (m_device_start != NULL)
	{
		(*m_device_start)(this);
	}
}


/*-------------------------------------------------
    call_load
-------------------------------------------------*/

bool cartslot_image_device::call_load()
{
	/* if this cartridge has a custom DEVICE_IMAGE_LOAD, use it */
	if (m_device_load != NULL)
		return (*m_device_load)(*this);

	/* otherwise try the normal route */
	return process_cartridge(true);
}


/*-------------------------------------------------
    call_unload
-------------------------------------------------*/
void cartslot_image_device::call_unload()
{
	/* if this cartridge has a custom DEVICE_IMAGE_UNLOAD, use it */
	if (m_device_unload != NULL)
	{
		(*m_device_unload)(*this);
		return;
	}
	process_cartridge(false);
}
