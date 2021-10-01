// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymopn.h"


//*********************************************************
//  YM2203 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM2203, ym2203_device, "ym2203", "YM2203 OPN")

//-------------------------------------------------
//  ym2203_device - constructor
//-------------------------------------------------

ym2203_device::ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_ssg_device_base<ymfm::ym2203>(mconfig, tag, owner, clock, YM2203)
{
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ym2203_device::device_start()
{
	// set our target output fidelity
	m_chip.set_fidelity(SSG_FIDELITY);

	// call our parent
	parent::device_start();
}



//*********************************************************
//  YM2608 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM2608, ym2608_device, "ym2608", "YM2608 OPNA")

//-------------------------------------------------
//  ym2608_device - constructor
//-------------------------------------------------

ym2608_device::ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_ssg_device_base<ymfm::ym2608>(mconfig, tag, owner, clock, YM2608),
	device_rom_interface(mconfig, *this),
	m_internal(*this, "internal")
{
}


//-------------------------------------------------
//  ym2608_device - start of emulation
//-------------------------------------------------

void ym2608_device::device_start()
{
	// set our target output fidelity
	m_chip.set_fidelity(SSG_FIDELITY);

	// call our parent
	parent::device_start();
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
//  ymfm_external_read - callback to read data for
//  the ADPCM-A/B engines
//-------------------------------------------------

uint8_t ym2608_device::ymfm_external_read(ymfm::access_class type, uint32_t offset)
{
	if (type == ymfm::ACCESS_ADPCM_A)
		return m_internal->as_u8(offset % m_internal->bytes());
	else if (type == ymfm::ACCESS_ADPCM_B)
		return space(0).read_byte(offset);
	return parent::ymfm_external_read(type, offset);
}


//-------------------------------------------------
//  ymfm_external_write - callback to write data to
//  the ADPCM-B engine; in this case, to our
//  default address space
//-------------------------------------------------

void ym2608_device::ymfm_external_write(ymfm::access_class type, uint32_t offset, uint8_t data)
{
	if (type == ymfm::ACCESS_ADPCM_B)
		return space(0).write_byte(offset, data);
	parent::ymfm_external_write(type, offset, data);
}



//*********************************************************
//  YM2610 DEVICE BASE
//*********************************************************

//-------------------------------------------------
//  ym2610_device_base - constructor
//-------------------------------------------------

template<typename ChipClass>
ym2610_device_base<ChipClass>::ym2610_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	ymfm_ssg_device_base<ChipClass>(mconfig, tag, owner, clock, type),
	device_memory_interface(mconfig, *this),
	m_adpcm_a_config("adpcm_a", ENDIANNESS_LITTLE, 8, 24, 0),
	m_adpcm_b_config("adpcm_b", ENDIANNESS_LITTLE, 8, 24, 0),
	m_adpcm_a_region(*this, "adpcma"),
	m_adpcm_b_region(*this, "adpcmb")
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

template<typename ChipClass>
device_memory_interface::space_config_vector ym2610_device_base<ChipClass>::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_adpcm_a_config),
		std::make_pair(1, &m_adpcm_b_config)
	};
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

template<typename ChipClass>
void ym2610_device_base<ChipClass>::device_start()
{
	// set our target output fidelity
	parent::m_chip.set_fidelity(SSG_FIDELITY);

	// call our parent
	parent::device_start();

	// automatically map memory regions if not configured externally
	if (!has_configured_map(0) && !has_configured_map(1))
	{
		if (m_adpcm_a_region)
			space(0).install_rom(0, m_adpcm_a_region->bytes() - 1, m_adpcm_a_region->base());

		if (m_adpcm_b_region)
			space(1).install_rom(0, m_adpcm_b_region->bytes() - 1, m_adpcm_b_region->base());
		else if (m_adpcm_a_region)
			space(1).install_rom(0, m_adpcm_a_region->bytes() - 1, m_adpcm_a_region->base());
	}
}


//-------------------------------------------------
//  ymfm_external_read - callback to read data for
//  the ADPCM-A/B engines
//-------------------------------------------------

template<typename ChipClass>
uint8_t ym2610_device_base<ChipClass>::ymfm_external_read(ymfm::access_class type, uint32_t offset)
{
	if (type == ymfm::ACCESS_ADPCM_A)
		return space(0).read_byte(offset);
	else if (type == ymfm::ACCESS_ADPCM_B)
		return space(1).read_byte(offset);
	return 0;
}



//*********************************************************
//  YM2610 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM2610, ym2610_device, "ym2610", "YM2610 OPNB")

//-------------------------------------------------
//  ym2610_device - constructor
//-------------------------------------------------

ym2610_device::ym2610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ym2610_device_base<ymfm::ym2610>(mconfig, tag, owner, clock, YM2610)
{
}


//*********************************************************
//  YM2610B DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM2610B, ym2610b_device, "ym2610b", "YM2610B OPNB2")

//-------------------------------------------------
//  ym2610b_device - constructor
//-------------------------------------------------

ym2610b_device::ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ym2610_device_base<ymfm::ym2610b>(mconfig, tag, owner, clock, YM2610B)
{
}



//*********************************************************
//  YM2612 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM2612, ym2612_device, "ym2612", "YM2612 OPN2")

//-------------------------------------------------
//  ym2612_device - constructor
//-------------------------------------------------

ym2612_device::ym2612_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym2612>(mconfig, tag, owner, clock, YM2612)
{
}



//*********************************************************
//  YM3438 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM3438, ym3438_device, "ym3438", "YM3438 OPN2C")

//-------------------------------------------------
//  ym3438_device - constructor
//-------------------------------------------------

ym3438_device::ym3438_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym3438>(mconfig, tag, owner, clock, YM3438)
{
}



//*********************************************************
//  YMF276 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YMF276, ymf276_device, "ymf276", "YMF276 OPN2L")

//-------------------------------------------------
//  ymf276_device - constructor
//-------------------------------------------------

ymf276_device::ymf276_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ymf276>(mconfig, tag, owner, clock, YMF276)
{
}
