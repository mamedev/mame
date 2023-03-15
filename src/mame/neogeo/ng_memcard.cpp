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
DEFINE_DEVICE_TYPE(NG_MEMCARD, ng_memcard_device, "ng_memcard", "NeoGeo Memory Card")

//-------------------------------------------------
//  ng_memcard_device - constructor
//-------------------------------------------------

ng_memcard_device::ng_memcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NG_MEMCARD, tag, owner, clock)
	, device_memcard_image_interface(mconfig, *this)
{
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

image_init_result ng_memcard_device::call_load()
{
	if(length() != 0x800)
		return image_init_result::FAIL;

	fseek(0, SEEK_SET);
	size_t ret = fread(m_memcard_data, 0x800);
	if(ret != 0x800)
		return image_init_result::FAIL;

	return image_init_result::PASS;
}

void ng_memcard_device::call_unload()
{
	fseek(0, SEEK_SET);
	fwrite(m_memcard_data, 0x800);
}

image_init_result ng_memcard_device::call_create(int format_type, util::option_resolution *format_options)
{
	memset(m_memcard_data, 0, 0x800);

	size_t ret = fwrite(m_memcard_data, 0x800);
	if(ret != 0x800)
		return image_init_result::FAIL;

	return image_init_result::PASS;
}


uint8_t ng_memcard_device::read(offs_t offset)
{
	return m_memcard_data[offset];
}

void ng_memcard_device::write(offs_t offset, uint8_t data)
{
	m_memcard_data[offset] = data;
}
