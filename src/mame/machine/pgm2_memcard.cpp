// license:BSD-3-Clause
// copyright-holders:David Haywood, Miodrag Milanovic
/*********************************************************************

    pgm2_memcard.cpp

    PGM2 Memory card functions.
	(based on ng_memcard.cpp)

*********************************************************************/

#include "emu.h"
#include "emuopts.h"

#include "pgm2_memcard.h"


// device type definition
DEFINE_DEVICE_TYPE(PGM2_MEMCARD, pgm2_memcard_device, "pgm2_memcard", "PGM2 Memory Card")

//-------------------------------------------------
//  pgm2_memcard_device - constructor
//-------------------------------------------------

pgm2_memcard_device::pgm2_memcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PGM2_MEMCARD, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pgm2_memcard_device::device_start()
{
	save_item(NAME(m_memcard_data));
}

/*-------------------------------------------------
    memcard_insert - insert an existing memory card
    with the given index
-------------------------------------------------*/

image_init_result pgm2_memcard_device::call_load()
{
	if(length() != 0x100)
		return image_init_result::FAIL;

	fseek(0, SEEK_SET);
	size_t ret = fread(m_memcard_data, 0x100);
	if(ret != 0x100)
		return image_init_result::FAIL;

	return image_init_result::PASS;
}

void pgm2_memcard_device::call_unload()
{
	fseek(0, SEEK_SET);
	fwrite(m_memcard_data, 0x100);
}

image_init_result pgm2_memcard_device::call_create(int format_type, util::option_resolution *format_options)
{
	// cards must contain valid defaults for each game / region or they don't work?
	memory_region *rgn = memregion("^default_card");

	if (!rgn)
		return image_init_result::FAIL;

	memcpy(m_memcard_data, rgn->base(), 0x100);

	size_t ret = fwrite(m_memcard_data, 0x100);
	if(ret != 0x100)
		return image_init_result::FAIL;

	return image_init_result::PASS;
}


READ8_MEMBER(pgm2_memcard_device::read)
{
	return m_memcard_data[offset];
}

WRITE8_MEMBER(pgm2_memcard_device::write)
{
	m_memcard_data[offset] = data;
}
