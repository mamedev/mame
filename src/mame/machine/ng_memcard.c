// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    ng_memcard.c

    NEOGEO Memory card functions.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "ng_memcard.h"

// device type definition
const device_type NG_MEMCARD = &device_creator<ng_memcard_device>;

//-------------------------------------------------
//  ng_memcard_device - constructor
//-------------------------------------------------

ng_memcard_device::ng_memcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NG_MEMCARD, "NeoGeo Memory Card", tag, owner, clock, "ng_memcard", __FILE__),
		device_image_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ng_memcard_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ng_memcard_device::device_start()
{
	save_item(NAME(m_memcard_data));
}

/*-------------------------------------------------
    memcard_insert - insert an existing memory card
    with the given index
-------------------------------------------------*/

bool ng_memcard_device::call_load()
{
	if(length() != 0x800)
		return IMAGE_INIT_FAIL;

	fseek(0, SEEK_SET);
	size_t ret = fread(m_memcard_data, 0x800);
	if(ret != 0x800)
		return IMAGE_INIT_FAIL;

	return IMAGE_INIT_PASS;
}

void ng_memcard_device::call_unload()
{
	fseek(0, SEEK_SET);
	fwrite(m_memcard_data, 0x800);
}

bool ng_memcard_device::call_create(int format_type, option_resolution *format_options)
{
	memset(m_memcard_data, 0, 0x800);

	size_t ret = fwrite(m_memcard_data, 0x800);
	if(ret != 0x800)
		return IMAGE_INIT_FAIL;

	return IMAGE_INIT_PASS;
}


READ8_MEMBER(ng_memcard_device::read)
{
	return m_memcard_data[offset];
}

WRITE8_MEMBER(ng_memcard_device::write)
{
	m_memcard_data[offset] = data;
}
