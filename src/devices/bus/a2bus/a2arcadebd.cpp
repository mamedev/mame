// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2arcadeboard.c

    Implementation of the Third Millenium Engineering Arcade Board

    TODO:
        - VDPTEST program seems to want more than 16K of RAM, but docs/ads/press releases say 16k, period
        - MLDEMO program needs vsync IRQ from the TMS but doesn't program the registers the way our emulation
          wants to enable IRQs

*********************************************************************/

#include "emu.h"
#include "a2arcadebd.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define TMS_TAG "arcbd_tms"
#define AY_TAG "arcbd_ay"
#define SCREEN_TAG "screen"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_ARCADEBOARD = &device_creator<a2bus_arcboard_device>;

MACHINE_CONFIG_FRAGMENT( arcadeboard )
	MCFG_DEVICE_ADD( TMS_TAG, TMS9918A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000) // 16k of VRAM
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(a2bus_arcboard_device, tms_irq_w))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS_TAG, tms9918a_device, screen_update )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(AY_TAG, AY8910, 1022727)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_arcboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( arcadeboard );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_arcboard_device::a2bus_arcboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_ARCADEBOARD, "Third Millenium Engineering Arcade Board", tag, owner, clock, "a2arcbd", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG),
	m_ay(*this, AY_TAG)
{
}

a2bus_arcboard_device::a2bus_arcboard_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG),
	m_ay(*this, AY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_arcboard_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
}

void a2bus_arcboard_device::device_reset()
{
}

/*
    C0nx map:
    0 - TMS read vram
    1 - TMS read status
    2 - TMS write vram
    3 - TMS write register
    5 - AY register select
    6 - AY data
*/

UINT8 a2bus_arcboard_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 0:
			return m_tms->vram_read(space, 0);

		case 1:
			return m_tms->register_read(space, 0);

		case 6:
			return m_ay->data_r(space, 0);
	}

	return 0xff;
}

void a2bus_arcboard_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 2:
			m_tms->vram_write(space, 0, data);
			break;

		case 3:
			m_tms->register_write(space, 0, data);
			break;

		case 5:
			m_ay->address_w(space, 0, data);
			break;

		case 6:
			m_ay->data_w(space, 0, data);
			break;
	}
}

WRITE_LINE_MEMBER( a2bus_arcboard_device::tms_irq_w )
{
	if (state)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}
