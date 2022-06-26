// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

  	Ad Lib MCA
	MCA ID @70D7

    - Not configurable. Always at 388h.
    pos[0]=0000000Xb 
    pos[1]=11000XXXb

***************************************************************************/

#include "emu.h"
#include "adlib.h"

#include "sound/spkrdev.h"
#include "speaker.h"

#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_ADLIB, mca16_adlib_device, "mca16_adlib_device", "Ad Lib Music Synthesis Card (@70D7)");

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_adlib_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	YM3812(config, m_ym3812, 3'579'545_Hz_XTAL).add_route(ALL_OUTPUTS, "mono", 3.00);
}

//-------------------------------------------------
//  mca16_adlib_device - constructor
//-------------------------------------------------

mca16_adlib_device::mca16_adlib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_adlib_device(mconfig, MCA16_ADLIB, tag, owner, clock)
{
}

mca16_adlib_device::mca16_adlib_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0x70d7),
    m_ym3812(*this, "ym3812")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_adlib_device::device_start()
{
	set_mca_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_adlib_device::device_reset()
{
	reset_option_select();
	if(m_is_mapped) unmap();
    m_is_mapped = 0;
}

void mca16_adlib_device::remap()
{
    // The only valid setting is I/O base 0388h.
	m_mca->install_device(0x388, 0x389,
		read8sm_delegate(*this, FUNC(mca16_adlib_device::io8_r)), write8sm_delegate(*this, FUNC(mca16_adlib_device::io8_w)));
    m_is_mapped = true;
}

void mca16_adlib_device::unmap()
{
    // The only valid setting is I/O base 0388h.
	m_mca->unmap_device(0x388, 0x389);
    m_is_mapped = false;
}

uint8_t mca16_adlib_device::io8_r(offs_t offset)
{
    assert_card_feedback();

	switch(offset)
	{
		case 0: 	return m_ym3812->status_r();
		default: 	return 0xff;
	}
}

void mca16_adlib_device::io8_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();

    switch(offset)
	{
		case 0 : m_ym3812->address_w(data); break;
		case 1 : m_ym3812->data_w(data); break;
	}
}

bool mca16_adlib_device::map_has_changed()
{
	// This is the only valid setting.
    if(((m_option_select[0] & 0b11111110) == 0) && ((m_option_select[1] & 0b11111000) == 0b11000000))
    {
		// Needs to be mapped if not already mapped.
        return (m_is_mapped ? false : true);
	}
	else
	{
		// If mapped, we need to unmap it.
		return (m_is_mapped ? true : false);
	}
}

void mca16_adlib_device::update_pos_data_1(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] = data;
	if(map_has_changed())
	{
		if(m_is_mapped) unmap();
		remap();
	}
}

void mca16_adlib_device::update_pos_data_2(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] = data;
	if(map_has_changed())
	{
		if(m_is_mapped) unmap();
		remap();
	}
}