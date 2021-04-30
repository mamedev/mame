// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym2608.h"


DEFINE_DEVICE_TYPE(YM2608, ym2608_device, "ym2608", "YM2608 OPNA")


//*********************************************************
//  YM2608 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2608_device - constructor
//-------------------------------------------------

ym2608_device::ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_ssg_base<ymfm::ym2608>(mconfig, tag, owner, clock, YM2608, 1, 2),
	device_rom_interface(mconfig, *this),
	m_internal(*this, "internal")
{
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region
//-------------------------------------------------

ROM_START( ym2608 )
	ROM_REGION( 0x2000, "internal", 0 )
	//
	// While this rom was dumped by output analysis, not decap, it was tested
	// by playing it back into the chip as an external adpcm sample and produced
	// an identical dac result. a decap would be nice to verify things 100%,
	// but there is currently no reason to think this rom dump is incorrect.
	//
	// offset 0x0000: Source: 01BD.ROM  Length:  448 / 0x000001C0
	// offset 0x01C0: Source: 02SD.ROM  Length:  640 / 0x00000280
	// offset 0x0440: Source: 04TOP.ROM Length: 5952 / 0x00001740
	// offset 0x1B80: Source: 08HH.ROM  Length:  384 / 0x00000180
	// offset 0x1D00: Source: 10TOM.ROM Length:  640 / 0x00000280
	// offset 0x1F80: Source: 20RIM.ROM Length:  128 / 0x00000080
	//
	ROM_LOAD16_WORD( "ym2608_adpcm_rom.bin", 0x0000, 0x2000, CRC(23c9e0d8) SHA1(50b6c3e288eaa12ad275d4f323267bb72b0445df) )
ROM_END

const tiny_rom_entry *ym2608_device::device_rom_region() const
{
	return ROM_NAME( ym2608 );
}


//-------------------------------------------------
//  rom_bank_updated - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void ym2608_device::rom_bank_updated()
{
	m_stream->update();
}


//-------------------------------------------------
//  adpcm_a_read - callback to read data for the
//  ADPCM-A engine; in this case, from the internal
//  ROM containing drum samples
//-------------------------------------------------

uint8_t ym2608_device::adpcm_a_read(uint32_t offset)
{
	return m_internal->as_u8(offset % m_internal->bytes());
}


//-------------------------------------------------
//  adpcm_b_read - callback to read data for the
//  ADPCM-B engine; in this case, from our default
//  address space
//-------------------------------------------------

uint8_t ym2608_device::adpcm_b_read(uint32_t offset)
{
	return space(0).read_byte(offset);
}


//-------------------------------------------------
//  adpcm_b_write - callback to write data to the
//  ADPCM-B engine; in this case, to our default
//  address space
//-------------------------------------------------

void ym2608_device::adpcm_b_write(uint32_t offset, uint8_t data)
{
	space(0).write_byte(offset, data);
}
