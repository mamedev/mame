// license:BSD-3-Clause
// copyright-holders:R. Belmont, Golden Child
/*********************************************************************

    ssprite.cpp

    Implementation of the Synetix SuperSprite

*********************************************************************/

#include "emu.h"
#include "ssprite.h"
#include "sound/tms5220.h"
#include "speaker.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define TMS_TAG "ssprite_tms"
#define TMS5220_TAG "ssprite_tms5220"
#define AY_TAG "ssprite_ay"
#define SCREEN_TAG "screen"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_SSPRITE, a2bus_ssprite_device, "a2ssprite", "Synetix SuperSprite")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(a2bus_ssprite_device::device_add_mconfig)
	MCFG_DEVICE_ADD( TMS_TAG, TMS9918A, XTAL(10'738'635) / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000) // 16k of VRAM
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(*this, a2bus_ssprite_device, tms_irq_w))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS_TAG, tms9918a_device, screen_update )

	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(AY_TAG, AY8912, 1022727)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_DEVICE_ADD(TMS5220_TAG, TMS5220, 640000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ssprite_device::a2bus_ssprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ssprite_device(mconfig, A2BUS_SSPRITE, tag, owner, clock)
{
}

a2bus_ssprite_device::a2bus_ssprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG),
	m_ay(*this, AY_TAG),
	m_tms5220(*this, TMS5220_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ssprite_device::device_start()
{
}

void a2bus_ssprite_device::device_reset()
{
}

/*
    C0nx map: (info from Synetix SuperSprite Owners manual.pdf page 33 of 266)
    0 - TMS9918 VDP vram read/write
    1 - TMS9918 VDP register write
    2 - TMS5220 Speech read/write
    3 - Video Switch APPLE VIDEO IN OFF
    4 - Video Switch APPLE VIDEO IN ON
    5 - Video Switch APPLE ONLY OUT
    6 - Video Switch MIX VDP/EXTERNAL VIDEO
    7 - TMS 9918 WRITE ONLY/FRAME RESET
    C - AY Sound data write
    D - AY Sound data write
    E - AY Sound register write or data read
    F - AY Sound register write or data read
*/


uint8_t a2bus_ssprite_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return m_tms->vram_read();
		case 1:
			return m_tms->register_read();
		case 2:
			return 0x1f | m_tms5220->status_r(); // copied this line from a2echoii.cpp
		case 14:
		case 15:
			return m_ay->data_r();
	}

	return 0xff;
}

void a2bus_ssprite_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_tms->vram_write(data);
			break;
		case 1:
			m_tms->register_write(data);
			break;
		case 2:
			m_tms5220->data_w(data);
			break;
		case 12:
		case 13:
			m_ay->data_w(data);
			break;
		case 14:
		case 15:
			m_ay->address_w(data);
			break;
	}
}

WRITE_LINE_MEMBER( a2bus_ssprite_device::tms_irq_w )
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
