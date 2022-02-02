// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    bitbngr.c

*********************************************************************/

#include "emu.h"
#include "bitbngr.h"

#include "softlist_dev.h"

#include <cstring>



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

DEFINE_DEVICE_TYPE(BITBANGER, bitbanger_device, "bitbanger", "Bitbanger")

/*-------------------------------------------------
    ctor
-------------------------------------------------*/

bitbanger_device::bitbanger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BITBANGER, tag, owner, clock),
	device_image_interface(mconfig, *this),
	m_next(nullptr),
	m_end(nullptr),
	m_interface(nullptr),
	m_is_readonly(false)
{
}



/*-------------------------------------------------
    native_output - outputs data to a file
-------------------------------------------------*/

void bitbanger_device::output(uint8_t data)
{
	if (!loaded_through_softlist() && exists())
		fwrite(&data, 1);
}


/*-------------------------------------------------
    native_input - inputs data from a file
-------------------------------------------------*/

uint32_t bitbanger_device::input(void *buffer, uint32_t length)
{
	if (loaded_through_softlist())
	{
		size_t const result = std::min<size_t>(length, m_end - m_next);
		memcpy(buffer, m_next, result);
		m_next += result;
		return uint32_t(result);
	}
	else if (exists())
	{
		return fread(buffer, length);
	}
	else
	{
		return 0;
	}
}



/*-------------------------------------------------
    device_start
-------------------------------------------------*/

void bitbanger_device::device_start()
{
}



/*-------------------------------------------------
    get_software_list_loader
-------------------------------------------------*/

const software_list_loader &bitbanger_device::get_software_list_loader() const
{
	return rom_software_list_loader::instance();
}



/*-------------------------------------------------
    call_load
-------------------------------------------------*/

image_init_result bitbanger_device::call_load()
{
	if (loaded_through_softlist())
	{
		auto const length = get_software_region_length("input");
		m_next = get_software_region("input");
		m_end = m_next + length;
	}
	else
	{
		m_next = m_end = nullptr;
	}
	return image_init_result::PASS;
}

image_init_result bitbanger_device::call_create(int format_type, util::option_resolution *format_options)
{
	// we don't need to do anything special
	return image_init_result::PASS;
}

/*-------------------------------------------------
    call_unload
-------------------------------------------------*/

void bitbanger_device::call_unload()
{
	m_next = m_end = nullptr;
}
