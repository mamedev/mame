/**********************************************************************

    Commodore SFX Sound Expander cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_sfx_sound_expander.h"



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

static const ym3526_interface ym3526_config =
{
	DEVCB_LINE_MEMBER(c64_sfx_sound_expander_cartridge_device, irq_w)
};


//-------------------------------------------------
//  C64_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

READ8_MEMBER( c64_sfx_sound_expander_cartridge_device::dma_cd_r )
{
	return m_slot->dma_cd_r(offset);
}

WRITE8_MEMBER( c64_sfx_sound_expander_cartridge_device::dma_cd_w )
{
	m_slot->dma_cd_w(offset, data);
}

WRITE_LINE_MEMBER( c64_sfx_sound_expander_cartridge_device::irq_w )
{
	m_slot->irq_w(state);
}

WRITE_LINE_MEMBER( c64_sfx_sound_expander_cartridge_device::nmi_w )
{
	m_slot->nmi_w(state);
}

WRITE_LINE_MEMBER( c64_sfx_sound_expander_cartridge_device::dma_w )
{
	m_slot->dma_w(state);
}

WRITE_LINE_MEMBER( c64_sfx_sound_expander_cartridge_device::reset_w )
{
	m_slot->reset_w(state);
}

static C64_EXPANSION_INTERFACE( expansion_intf )
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_sfx_sound_expander_cartridge_device, dma_cd_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_sfx_sound_expander_cartridge_device, dma_cd_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_sfx_sound_expander_cartridge_device, irq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_sfx_sound_expander_cartridge_device, nmi_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_sfx_sound_expander_cartridge_device, dma_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_sfx_sound_expander_cartridge_device, reset_w)
};


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_sfx_sound_expander )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_sfx_sound_expander )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(YM3526_TAG, YM3526, XTAL_3_579545MHz)
	MCFG_SOUND_CONFIG(ym3526_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)

	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, 0, expansion_intf, c64_expansion_cards, NULL, NULL)
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("KB1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("KB2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("KB3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("KB4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("KB5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("KB6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("KB7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_sfx_sound_expander_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_sfx_sound_expander );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_sfx_sound_expander_cartridge_device - constructor
//-------------------------------------------------

c64_sfx_sound_expander_cartridge_device::c64_sfx_sound_expander_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_SFX_SOUND_EXPANDER, "C64 SFX Sound Expander cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_opl(*this, YM3526_TAG),
	m_exp(*this, C64_EXPANSION_SLOT_TAG)
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
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_sfx_sound_expander_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	data = m_exp->cd_r(space, offset, data, sphi2, ba, roml, romh, io1, io2);

	if (!io2)
	{
		if (BIT(offset, 3))
		{
			switch (offset & 0x07)
			{
			case 0: data = ioport("KB0")->read(); break;
			case 1: data = ioport("KB1")->read(); break;
			case 2: data = ioport("KB2")->read(); break;
			case 3: data = ioport("KB3")->read(); break;
			case 4: data = ioport("KB4")->read(); break;
			case 5: data = ioport("KB5")->read(); break;
			case 6: data = ioport("KB6")->read(); break;
			case 7: data = ioport("KB7")->read(); break;
			}
		}
		else if (BIT(offset, 5))
		{
			data = ym3526_r(m_opl, space, BIT(offset, 4));
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_sfx_sound_expander_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2 && BIT(offset, 5))
	{
		ym3526_w(m_opl, space, BIT(offset, 4), data);
	}

	m_exp->cd_w(space, offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_sfx_sound_expander_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	return m_exp->game_r(offset, sphi2, ba, rw, hiram);
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_sfx_sound_expander_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	return m_exp->exrom_r(offset, sphi2, ba, rw, hiram);
}
