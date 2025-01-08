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

#include "coco_pak.h"
#include "sound/sn76496.h"
#include "speaker.h"

#define SN76489AN_TAG   "gmc_psg"

//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

namespace
{
	// ======================> coco_pak_banked_device

	class coco_pak_gmc_device :
		public coco_pak_banked_device
	{
	public:
		// construction/destruction
		coco_pak_gmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	protected:
		// device-level overrides
		virtual void scs_write(offs_t offset, u8 data) override;

	private:
		required_device<sn76489a_device> m_psg;
	};
};


void coco_pak_gmc_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "gmc_speaker").front_center();
	SN76489A(config, m_psg, 4_MHz_XTAL).add_route(ALL_OUTPUTS, "gmc_speaker", 1.0);
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_PAK_GMC, device_cococart_interface, coco_pak_gmc_device, "cocopakgmc", "CoCo Games Master Cartridge")

//-------------------------------------------------
//  coco_pak_device - constructor
//-------------------------------------------------

coco_pak_gmc_device::coco_pak_gmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: coco_pak_banked_device(mconfig, COCO_PAK_GMC, tag, owner, clock)
	, m_psg(*this, SN76489AN_TAG)
{
}


//-------------------------------------------------
//    scs_write
//-------------------------------------------------

void coco_pak_gmc_device::scs_write(offs_t offset, u8 data)
{
	switch(offset)
	{
		case 0:
			/* set the bank */
			coco_pak_banked_device::scs_write(offset, data);
			break;
		case 1:
			m_psg->write(data);
			break;
	}
}
