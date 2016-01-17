// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore SFX Sound Expander cartridge emulation

**********************************************************************/

#include "sfx_sound_expander.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define YM3526_TAG  "ic3"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_SFX_SOUND_EXPANDER = &device_creator<c64_sfx_sound_expander_cartridge_device>;


//-------------------------------------------------
//  ym3526_interface ym3526_config
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_sfx_sound_expander_cartridge_device::opl_irq_w )
{
	m_slot->irq_w(state);
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_sfx_sound_expander )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_sfx_sound_expander )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(YM3526_TAG, YM3526, XTAL_3_579545MHz)
	MCFG_YM3526_IRQ_HANDLER(WRITELINE(c64_sfx_sound_expander_cartridge_device, opl_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)

	MCFG_C64_PASSTHRU_EXPANSION_SLOT_ADD()
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_sfx_sound_expander_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_sfx_sound_expander );
}


//-------------------------------------------------
//  INPUT_PORTS( c64_sfx_sound_expander )
//-------------------------------------------------

static INPUT_PORTS_START( c64_sfx_sound_expander )
	PORT_START("KB0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C#2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D#2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F#2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G2")

	PORT_START("KB1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G#2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A#3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C#3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D#3")

	PORT_START("KB2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F#3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G#3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A#4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B4")

	PORT_START("KB3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C#4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D#4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F#4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G4")

	PORT_START("KB4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G#4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A#5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C#5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D#5")

	PORT_START("KB5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F#5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G#5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A#6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B6")

	PORT_START("KB6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C#6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D#6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F#6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G6")

	PORT_START("KB7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G#6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A7")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A#7")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B7")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C7")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_sfx_sound_expander_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_sfx_sound_expander );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  get_offset - get passthru expansion port offset
//-------------------------------------------------

inline offs_t c64_sfx_sound_expander_cartridge_device::get_offset(offs_t offset, int rw)
{
	// assimilate the 3 different MIDI cartridge 6850 ACIA register mappings?
	return (offset & 0xfffc) | (rw << 1) | BIT(offset, 1);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_sfx_sound_expander_cartridge_device - constructor
//-------------------------------------------------

c64_sfx_sound_expander_cartridge_device::c64_sfx_sound_expander_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_SFX_SOUND_EXPANDER, "C64 SFX Sound Expander cartridge", tag, owner, clock, "c64_sfxse", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_opl(*this, YM3526_TAG),
	m_exp(*this, C64_EXPANSION_SLOT_TAG),
	m_kb0(*this, "KB0"),
	m_kb1(*this, "KB1"),
	m_kb2(*this, "KB2"),
	m_kb3(*this, "KB3"),
	m_kb4(*this, "KB4"),
	m_kb5(*this, "KB5"),
	m_kb6(*this, "KB6"),
	m_kb7(*this, "KB7")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_sfx_sound_expander_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_sfx_sound_expander_cartridge_device::device_reset()
{
	m_opl->reset();
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_sfx_sound_expander_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	data = m_exp->cd_r(space, get_offset(offset, 1), data, sphi2, ba, roml, romh, io1, io2);

	if (!io2 && sphi2)
	{
		if (BIT(offset, 3))
		{
			switch (offset & 0x07)
			{
			case 0: data = m_kb0->read(); break;
			case 1: data = m_kb1->read(); break;
			case 2: data = m_kb2->read(); break;
			case 3: data = m_kb3->read(); break;
			case 4: data = m_kb4->read(); break;
			case 5: data = m_kb5->read(); break;
			case 6: data = m_kb6->read(); break;
			case 7: data = m_kb7->read(); break;
			}
		}

		if (BIT(offset, 5))
		{
			data = m_opl->read(space, BIT(offset, 4));
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_sfx_sound_expander_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2 && sphi2)
	{
		m_opl->write(space, BIT(offset, 4), data);
	}

	m_exp->cd_w(space, get_offset(offset, 0), data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_sfx_sound_expander_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->game_r(get_offset(offset, rw), sphi2, ba, rw, m_slot->hiram());
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_sfx_sound_expander_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->exrom_r(get_offset(offset, rw), sphi2, ba, rw, m_slot->hiram());
}
