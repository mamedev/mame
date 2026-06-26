// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Datel Electronics Action Replay emulation

**********************************************************************/

#include "emu.h"
#include "action_replay.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_ACTION_REPLAY, c64_action_replay_cartridge_device, "c64_ar4x", "C64 Action Replay")


//-------------------------------------------------
//  ROM( c64_action_replay )
//-------------------------------------------------

ROM_START( c64_action_replay )
	ROM_REGION( 0x100, "pla", 0 )
	ROM_LOAD( "82s129.u3", 0x000, 0x100, CRC(f19e6686) SHA1(f39975d24eb009325fa5fac86b475137cac04813) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_action_replay_cartridge_device::device_rom_region() const
{
	return ROM_NAME( c64_action_replay );
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( freeze )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c64_action_replay_cartridge_device::freeze )
{
	if (!newval)
	{
		m_freeze = true;
	}
}


//-------------------------------------------------
//  INPUT_PORTS( c64_action_replay )
//-------------------------------------------------

static INPUT_PORTS_START( c64_action_replay )
	PORT_START("SW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::reset_w))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_action_replay_cartridge_device::freeze), 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_action_replay_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_action_replay );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_action_replay_cartridge_device - constructor
//-------------------------------------------------

c64_action_replay_cartridge_device::c64_action_replay_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_ACTION_REPLAY, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_ram(*this, "ram", 0x1000, ENDIANNESS_LITTLE),
	m_pla(*this, "pla"),
	m_freeze_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_action_replay_cartridge_device::device_start()
{
	m_freeze_timer = timer_alloc(FUNC(c64_action_replay_cartridge_device::unfreeze), this);

	// state saving
	save_item(NAME(m_bank));
	save_item(NAME(m_pla_a7));
	save_item(NAME(m_freeze));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_action_replay_cartridge_device::device_reset()
{
	m_bank = 0;
	m_pla_a7 = 1;
	m_freeze = false;

	m_slot->nmi_w(CLEAR_LINE);
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_action_replay_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	update_freeze(ba);

	if (pla_enabled())
	{
		u8 pla = read_pla(offset, io2);

		if ((!roml || !romh || !io2) && !BIT(pla, 0))
		{
			offs_t addr = (BIT(m_bank, 4) << 14) | (BIT(m_bank, 3) << 13) | (offset & 0x1fff);
			data = m_roml[addr];
		}

		if (!BIT(pla, 1))
		{
			data = m_ram[offset & 0xfff];
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_action_replay_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	update_freeze(ba);

	if (!io1 && pla_enabled())
	{
		/*

			bit		description

			0		PLA A2
			1		PLA A1
			2		PLA CE1
			3		ROM A13
			4		ROM A14
			5		PLA A0
			6		U6B CLR
			7
	
		*/

		m_bank = data & 0x7f;

		if (BIT(m_bank, 6)) 
		{
			m_slot->nmi_w(CLEAR_LINE);

			m_pla_a7 = 1;
		}
	}

	if (pla_enabled())
	{
		u8 pla = read_pla(offset, io2);

		if (!BIT(pla, 1))
		{
			m_ram[offset & 0xfff] = data;
		}
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_action_replay_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	update_freeze(ba);

	if (pla_enabled())
	{
		u8 pla = read_pla(offset, 1);
		return BIT(pla, 2);
	}

	return 1;
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_action_replay_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	update_freeze(ba);

	if (pla_enabled())
	{
		u8 pla = read_pla(offset, 1);
		return BIT(pla, 3);
	}

	return 1;
}


u8 c64_action_replay_cartridge_device::read_pla(offs_t offset, int io2)
{
	/*

		bit		description

		0	  	ROM CE
		1       RAM CS1
		2       GAME
		3       EXROM

 	*/

	offs_t addr = (m_pla_a7 << 7) | (BIT(offset, 14) << 6) | (BIT(offset, 15) << 5) | (BIT(offset, 13) << 4) | (io2 << 3) | (BIT(m_bank, 0) << 2) | (BIT(m_bank, 1) << 1) | BIT(m_bank, 5);

	return m_pla->base()[addr];
}


void c64_action_replay_cartridge_device::update_freeze(int ba)
{
	if (m_freeze && ba)
	{
		m_freeze = false;

		m_bank = 0;

		m_slot->nmi_w(ASSERT_LINE);

		m_freeze_timer->adjust(attotime::from_ticks(5, clock()));
	}
}


TIMER_CALLBACK_MEMBER( c64_action_replay_cartridge_device::unfreeze )
{
	m_slot->nmi_w(CLEAR_LINE);

	m_pla_a7 = 0;
}
