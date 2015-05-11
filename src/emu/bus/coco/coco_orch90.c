// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    orch90.c

    Code for emulating the CoCo Orch-90 (Orchestra 90) sound cartridge

    The Orch-90 was a simple sound cartridge; it had two 8-bit DACs
    supporting stereo sound.  The left channel was at $FF7A, and the right
    channel was at $FF7B

***************************************************************************/

#include "emu.h"
#include "coco_orch90.h"
#include "sound/dac.h"

static MACHINE_CONFIG_FRAGMENT(coco_orch90)
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("dac_left", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ADD("dac_right", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type COCO_ORCH90 = &device_creator<coco_orch90_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_orch90_device - constructor
//-------------------------------------------------

coco_orch90_device::coco_orch90_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, COCO_ORCH90, "CoCo Orch-90 PAK", tag, owner, clock, "coco_orch90", __FILE__),
		device_cococart_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_orch90_device::device_start()
{
	m_left_dac = subdevice<dac_device>("dac_left");
	m_right_dac = subdevice<dac_device>("dac_right");
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor coco_orch90_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( coco_orch90 );
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(coco_orch90_device::write)
{
	switch(offset)
	{
		case 0x3A:
			/* left channel write */
			m_left_dac->write_unsigned8(data);
			break;

		case 0x3B:
			/* right channel write */
			m_right_dac->write_unsigned8(data);
			break;
	}
}
