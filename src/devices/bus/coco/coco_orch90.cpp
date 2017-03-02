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

#include "sound/volt_reg.h"
#include "speaker.h"


static MACHINE_CONFIG_FRAGMENT(coco_orch90)
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ldac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.5) // ls374.ic5 + r7 (8x20k) + r9 (8x10k)
	MCFG_SOUND_ADD("rdac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.5) // ls374.ic4 + r6 (8x20k) + r8 (8x10k)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "ldac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "ldac", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "rdac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "rdac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type COCO_ORCH90 = device_creator<coco_orch90_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_orch90_device - constructor
//-------------------------------------------------

coco_orch90_device::coco_orch90_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, COCO_ORCH90, "CoCo Orch-90 PAK", tag, owner, clock, "coco_orch90", __FILE__),
		device_cococart_interface(mconfig, *this ),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_orch90_device::device_start()
{
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
			m_ldac->write(data);
			break;

		case 0x3B:
			m_rdac->write(data);
			break;
	}
}
