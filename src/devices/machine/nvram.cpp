// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    nvram.cpp

    Generic non-volatile RAM.

***************************************************************************/

#include "emu.h"
#include "nvram.h"

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NVRAM, nvram_device, "nvram", "NVRAM")

//-------------------------------------------------
//  nvram_device - constructor
//-------------------------------------------------

nvram_device::nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NVRAM, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_region(*this, DEVICE_SELF),
	m_default_value(DEFAULT_ALL_1),
	m_custom_handler(*this),
	m_base(nullptr),
	m_length(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nvram_device::device_start()
{
	// bind our handler
	m_custom_handler.resolve();
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void nvram_device::nvram_default()
{
	// make sure we have a valid base pointer
	determine_final_base();

	// region always wins
	if (m_region.found())
	{
		memcpy(m_base, m_region->base(), m_length);
		return;
	}

	// default values for other cases
	switch (m_default_value)
	{
		// all-0's
		case DEFAULT_ALL_0:
			memset(m_base, 0, m_length);
			break;

		// all 1's
		default:
		case DEFAULT_ALL_1:
			memset(m_base, 0xff, m_length);
			break;

		// custom handler
		case DEFAULT_CUSTOM:
			m_custom_handler(*this, m_base, m_length);
			break;

		// none - do nothing
		case DEFAULT_NONE:
			break;
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool nvram_device::nvram_read(util::read_stream &file)
{
	// make sure we have a valid base pointer
	determine_final_base();

	// FIXME: consider width/Endianness
	auto const [err, actual] = read(file, m_base, m_length);
	return !err && (actual == m_length);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool nvram_device::nvram_write(util::write_stream &file)
{
	// FIXME: consider width/Endianness
	auto const [err, actual] = write(file, m_base, m_length);
	return !err;
}


//-------------------------------------------------
//  determine_final_base - get the final base
//  pointer by looking up the memory share, unless
//  a pointer was provided to us
//-------------------------------------------------

void nvram_device::determine_final_base()
{
	// find our shared pointer with the target RAM
	if (!m_base)
	{
		memory_share *const share = owner()->memshare(tag());
		if (!share)
			throw emu_fatalerror("NVRAM device '%s' has no corresponding share() region", tag());
		m_base = share->ptr();
		m_length = share->bytes();
	}

	// if we are region-backed for the default, find it now and make sure it's the right size
	if (m_region.found() && m_region->bytes() != m_length)
		throw emu_fatalerror("%s",string_format("NVRAM device '%s' has a default region, but it should be 0x%X bytes", tag(), m_length));
}
