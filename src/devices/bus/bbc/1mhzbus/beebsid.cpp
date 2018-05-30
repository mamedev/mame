// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BeebSID emulation

**********************************************************************/


#include "emu.h"
#include "beebsid.h"
#include "speaker.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MOS8580_TAG     "mos8580"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_BEEBSID, bbc_beebsid_device, "beebsid", "BeebSID")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_beebsid_device::device_add_mconfig)
	SPEAKER(config, "speaker").front_center();
	MCFG_DEVICE_ADD(MOS8580_TAG, MOS8580, XTAL(16'000'000) / 16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 1.0)

	MCFG_BBC_PASSTHRU_1MHZBUS_SLOT_ADD()
MACHINE_CONFIG_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_beebsid_device - constructor
//-------------------------------------------------

bbc_beebsid_device::bbc_beebsid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_BEEBSID, tag, owner, clock),
	device_bbc_1mhzbus_interface(mconfig, *this),
	m_sid(*this, MOS8580_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_beebsid_device::device_start()
{
	address_space& space = machine().device("maincpu")->memory().space(AS_PROGRAM);

	space.install_readwrite_handler(0xfc20, 0xfc3f, read8_delegate(FUNC(mos8580_device::read), (mos8580_device*)m_sid), write8_delegate(FUNC(mos8580_device::write), (mos8580_device*)m_sid));
}
