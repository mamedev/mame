// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "y8950.h"


DEFINE_DEVICE_TYPE(Y8950, y8950_device, "y8950", "Y8950 OPL MSX-Audio")


//*********************************************************
//  Y8950 DEVICE
//*********************************************************

//-------------------------------------------------
//  y8950_device - constructor
//-------------------------------------------------

y8950_device::y8950_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::y8950>(mconfig, tag, owner, clock, Y8950),
	device_rom_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  rom_bank_updated - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void y8950_device::rom_bank_updated()
{
	m_stream->update();
}


//-------------------------------------------------
//  ymfm_adpcm_b_read - callback to read data for
//  the ADPCM-B engine; in this case, from our
//  default address space
//-------------------------------------------------

uint8_t y8950_device::ymfm_adpcm_b_read(uint32_t offset)
{
	return read_byte(offset);
}


//-------------------------------------------------
//  ymfm_adpcm_b_write - callback to write data to
//  the ADPCM-B engine; in this case, to our
//  default address space
//-------------------------------------------------

void y8950_device::ymfm_adpcm_b_write(uint32_t offset, uint8_t data)
{
	space().write_byte(offset, data);
}
