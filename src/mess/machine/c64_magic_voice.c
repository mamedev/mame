/**********************************************************************

    Commodore Magic Voice cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_magic_voice.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define T6721A_TAG  "u5"
#define MOS6525_TAG "u2"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_MAGIC_VOICE = &device_creator<c64_magic_voice_cartridge_device>;


//-------------------------------------------------
//  tpi6525_interface tpi_intf
//-------------------------------------------------

static const tpi6525_interface tpi_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_magic_voice )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_magic_voice )
	MCFG_TPI6525_ADD(MOS6525_TAG, tpi_intf)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(T6721A_TAG, T6721A, XTAL_640kHz)
	MCFG_T6721A_EOS_HANDLER(DEVWRITELINE(MOS6525_TAG, tpi6525_device, i2_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_C64_PASSTHRU_EXPANSION_SLOT_ADD()
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_magic_voice_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_magic_voice );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_magic_voice_cartridge_device - constructor
//-------------------------------------------------

c64_magic_voice_cartridge_device::c64_magic_voice_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_MAGIC_VOICE, "C64 Magic Voice cartridge", tag, owner, clock, "c64_magic_voice", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_vslsi(*this, T6721A_TAG),
	m_tpi(*this, MOS6525_TAG),
	m_exp(*this, C64_EXPANSION_SLOT_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_magic_voice_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_magic_voice_cartridge_device::device_reset()
{
	m_tpi->reset();
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_magic_voice_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	data = m_exp->cd_r(space, offset, data, sphi2, ba, roml, romh, io1, io2);

	if (!io2 && BIT(offset, 7))
	{
		m_tpi->read(space, offset & 0x07);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_magic_voice_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2 && BIT(offset, 7))
	{
		m_tpi->write(space, offset & 0x07, data);
	}

	m_exp->cd_w(space, offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_magic_voice_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	return m_exp->game_r(offset, sphi2, ba, rw, hiram);
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_magic_voice_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	return m_exp->exrom_r(offset, sphi2, ba, rw, hiram);
}
