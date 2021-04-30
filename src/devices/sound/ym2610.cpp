// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym2610.h"


DEFINE_DEVICE_TYPE(YM2610, ym2610_device, "ym2610", "YM2610 OPNB")
DEFINE_DEVICE_TYPE(YM2610B, ym2610b_device, "ym2610b", "YM2610B OPNB2")


//*********************************************************
//  YM2610 DEVICE BASE
//*********************************************************

//-------------------------------------------------
//  ym2610_device_base - constructor
//-------------------------------------------------

template<typename ChipClass>
ym2610_device_base<ChipClass>::ym2610_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	ymfm_device_ssg_base<ChipClass>(mconfig, tag, owner, clock, type, 1, 0),
	device_memory_interface(mconfig, *this),
	m_adpcm_a_config("adpcm-a", ENDIANNESS_LITTLE, 8, 24, 0),
	m_adpcm_b_config("adpcm-b", ENDIANNESS_LITTLE, 8, 24, 0),
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
//  adpcm_a_read - callback to read data for the
//  ADPCM-A engine; in this case, from address
//  space 0
//-------------------------------------------------

template<typename ChipClass>
uint8_t ym2610_device_base<ChipClass>::adpcm_a_read(uint32_t offset)
{
	return space(0).read_byte(offset);
}


//-------------------------------------------------
//  adpcm_b_read - callback to read data for the
//  ADPCM-B engine; in this case, from address
//  space 1
//-------------------------------------------------

template<typename ChipClass>
uint8_t ym2610_device_base<ChipClass>::adpcm_b_read(uint32_t offset)
{
	return space(1).read_byte(offset);
}



//*********************************************************
//  YM2610 DEVICE
//*********************************************************

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

//-------------------------------------------------
//  ym2610b_device - constructor
//-------------------------------------------------

ym2610b_device::ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ym2610_device_base<ymfm::ym2610b>(mconfig, tag, owner, clock, YM2610B)
{
}


