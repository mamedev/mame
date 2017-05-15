// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    coco_gmc.cpp

    Code for emulating the Games Master Cartridge. A banked switched ROM
    cartridge with a SN76489AN programmable sound generator.

    The ROM bank switching is exactly like the circuit developed for RoboCop
    and Predator.

    The SN76489AN is tied to address $FF41.

    Cartridge by John Linville.

***************************************************************************/

#include "emu.h"
#include "coco_gmc.h"
#include "speaker.h"

#define SN76489AN_TAG   "gmc_psg"

static MACHINE_CONFIG_FRAGMENT(cocopak_gmc)
	MCFG_SPEAKER_STANDARD_MONO("gmc_speaker")
	MCFG_SOUND_ADD(SN76489AN_TAG, SN76489A, XTAL_4MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "gmc_speaker", 1.0)
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_PAK_GMC, coco_pak_gmc_device, "cocopakgmc", "CoCo Games Master Cartridge")

//-------------------------------------------------
//  coco_pak_device - constructor
//-------------------------------------------------

coco_pak_gmc_device::coco_pak_gmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: coco_pak_banked_device(mconfig, COCO_PAK_GMC, tag, owner, clock),
		m_psg(*this, SN76489AN_TAG)
{
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor coco_pak_gmc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cocopak_gmc );
}

//-------------------------------------------------
//    write
//-------------------------------------------------

WRITE8_MEMBER(coco_pak_gmc_device::write)
{
	switch(offset)
	{
		case 0:
			/* set the bank */
			coco_pak_banked_device::write(space,offset,data,mem_mask);
			break;
		case 1:
			m_psg->write(data);
			break;
	}
}
