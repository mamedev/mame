// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    GPL16250 / GPAC800 / GMC384 / GCM420 related support

    GPL16250 is the GeneralPlus / SunPlus part number
    GPAC800 is the JAKKS Pacific codename
    GMC384 / GCM420 is what is printed on the die

    ----

    GPL16250 games using ROM + RAM configuration
*/

#include "emu.h"
#include "generalplus_gpl16250_romram.h"


static INPUT_PORTS_START( wrlshunt )
	PORT_START("IN0")
	PORT_START("IN1")
	PORT_START("IN2")
INPUT_PORTS_END


static INPUT_PORTS_START( jak_s500 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( paccon ) // for Test Mode hold buttons 1+2 until the screen starts changing colours (happens after the copyright display)
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) // PAL/NTSC flag
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) // 'A'
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) // '*C*  (doesn't exist?) (cheat)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) // 'B'
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) // '*MENU*' (doesn't exist?)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED ) // '***'

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_ths )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

uint16_t wrlshunt_game_state::cs0_r(offs_t offset)
{
	return m_romregion[offset & m_romwords_mask];
}

void wrlshunt_game_state::cs0_w(offs_t offset, uint16_t data)
{
	logerror("cs0_w write to ROM?\n");
	//m_romregion[offset & 0x3ffffff] = data;
}

uint16_t wrlshunt_game_state::cs1_r(offs_t offset)
{
	return m_sdram[offset & 0x3fffff];
}

void wrlshunt_game_state::cs1_w(offs_t offset, uint16_t data)
{
	m_sdram[offset & 0x3fffff] = data;
}


void wrlshunt_game_state::machine_start()
{
	m_romwords_mask = (memregion("maincpu")->bytes()/2)-1;
	save_item(NAME(m_sdram));
}

void wrlshunt_game_state::machine_reset()
{
	cs_callback(0x00, 0x00, 0x00, 0x00, 0x00);
	m_maincpu->set_cs_space(m_memory->get_program());
	m_maincpu->reset(); // reset CPU so vector gets read etc.

	//m_maincpu->set_paldisplaybank_high_hack(1);
}

void wrlshunt_game_state::init_wrlshunt()
{
	m_sdram.resize(0x400000); // 0x400000 words, 0x800000 bytes
}

void wrlshunt_game_state::init_ths()
{
	m_sdram.resize(0x400000); // 0x400000 words, 0x800000 bytes (verify)
}

void wrlshunt_game_state::gpl16250_romram(machine_config &config)
{
	gcm394_game_state::base(config);
}

uint16_t wrlshunt_game_state::porta_r()
{
	uint16_t data = m_io[0]->read();
	logerror("%s: Port A Read: %04x\n",  machine().describe_context(), data);
	return data;
}

void wrlshunt_game_state::porta_w(uint16_t data)
{
	logerror("%s: Port A:WRITE %04x\n", machine().describe_context(), data);

	// HACK
	address_space& mem = m_maincpu->space(AS_PROGRAM);
	if (mem.read_word(0x5b354) == 0xafd0)   // wrlshubt - skip check (EEPROM?)
		mem.write_word(0x5b354, 0xB403);
}




uint16_t jak_s500_game_state::porta_r()
{
	uint16_t data = m_io[0]->read();
	logerror("%s: Port A Read: %04x\n", machine().describe_context(), data);

	// these are debug helpers to access the test modes while we don't have the
	// secret codes / controls mapped properly

	//address_space& mem = m_maincpu->space(AS_PROGRAM);

	//if (mem.read_word(0x22b408) == 0x4846)
	//  mem.write_word(0x22b408, 0x4840);    // jak_s500 force service mode

	//if (mem.read_word(0x236271) == 0x4846)
	//  mem.write_word(0x236271, 0x4840);    // jak_totm force service mode

	//if (mem.read_word(0x22d6f7) == 0x4846)
	//  mem.write_word(0x22d6f7, 0x4840);    // jak_pf force service mode

	//if (mem.read_word(0x23e295) == 0x4846)
	//  mem.write_word(0x23e295, 0x4840);    // jak_smwm force service mode

	//if (mem.read_word(0x22e92e) == 0x4646)
	//  mem.write_word(0x22e92e, 0x4640);    // jak_swcl force service mode

	return data;
}

uint16_t jak_s500_game_state::portb_r()
{
	uint16_t data = m_io[1]->read();
	logerror("%s: Port B Read: %04x\n", machine().describe_context(), data);
	return data;
}


void jak_s500_game_state::machine_reset()
{
	cs_callback(0x00, 0x00, 0x00, 0x00, 0x00);
	m_maincpu->set_cs_space(m_memory->get_program());
	m_maincpu->reset(); // reset CPU so vector gets read etc.

	//m_maincpu->set_paldisplaybank_high_hack(0);
}


void lazertag_game_state::machine_reset()
{
	jak_s500_game_state::machine_reset();
	//m_maincpu->set_pal_sprites_hack(0x800);
}

void paccon_game_state::machine_reset()
{
	jak_s500_game_state::machine_reset();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x6593, 0x6593, read16smo_delegate(*this, FUNC(paccon_game_state::paccon_speedup_hack_r)));
//  install_speedup_hack(0x6593, 0x30033);
}

void jak_pf_game_state::machine_reset()
{
	jak_s500_game_state::machine_reset();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0001, 0x0001, read16smo_delegate(*this, FUNC(jak_pf_game_state::jak_pf_speedup_hack_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x13dd, 0x13dd, read16smo_delegate(*this, FUNC(jak_pf_game_state::jak_pf_speedup_hack2_r)));
}

void jak_prft_game_state::machine_reset()
{
	jak_s500_game_state::machine_reset();
}


uint16_t paccon_game_state::paccon_speedup_hack_r()
{
	u32 const pc = m_maincpu->pc();
	if (pc == 0x30033)
		m_maincpu->spin_until_time(m_maincpu->cycles_to_attotime(2000));
	return m_maincpu->get_ram_addr(0x6593);
}

uint16_t jak_pf_game_state::jak_pf_speedup_hack_r()
{
	u32 const pc = m_maincpu->pc();
	if (pc == 0x30010)
		m_maincpu->spin_until_time(m_maincpu->cycles_to_attotime(2000));
	return m_maincpu->get_ram_addr(0x0001);
}

uint16_t jak_pf_game_state::jak_pf_speedup_hack2_r()
{
	u32 const pc = m_maincpu->pc();
	if (pc == 0x2611b4)
		m_maincpu->spin_until_time(m_maincpu->cycles_to_attotime(2000));
	return m_maincpu->get_ram_addr(0x13dd);
}

ROM_START( paccon )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("pacmanconnect.bin", 0x000000, 0x400000, CRC(8567cdc7) SHA1(cef4e003142e479169e4438ab33558436ee9ee68) )
ROM_END



ROM_START(lazertag)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("lazertag.bin", 0x000000, 0x1000000, CRC(8bf16a28) SHA1(90d05e1876332324b074e4845e28b90fcb007122) )
ROM_END

ROM_START(jak_sinv)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("jakkspaceinvaders.u3", 0x000000, 0x800000, CRC(2ccb1fc9) SHA1(21d92829de4b03b92894d92853bb5ec360dfad3c) )
ROM_END

ROM_START(jak_s500)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("spbwheel.bin", 0x000000, 0x800000, CRC(6ba1d335) SHA1(1bb3e4d02c7b35dd4d336971c6a9f82071cc6ce1) )
ROM_END

ROM_START(jak_swcl)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("jakksclonewars.bin", 0x000000, 0x800000, CRC(549bb326) SHA1(992a60321580a4e014801d401b3a7ee000d2b465) )
ROM_END

ROM_START(jak_smwm)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("spidersense.bin", 0x000000, 0x800000, CRC(e0676d0e) SHA1(01c01852fe4aea799c09ebbb6870b2f6e92085c4) )
ROM_END


ROM_START(jak_pf)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("phineas.bin", 0x000000, 0x800000, CRC(bb18f70d) SHA1(4e3c204e44efe9186809404521ebeac348c45fac))
ROM_END

ROM_START(jak_prft)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("powerrangerssword.bin", 0x000000, 0x800000, CRC(77bc8aea) SHA1(a2efaa718d8ecece46cebb9f0f13a8fa10fc2826) )
ROM_END

ROM_START(jak_tink)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("disneyfairies.bin", 0x000000, 0x800000, CRC(566dae87) SHA1(3abe1b7d578ed9255101bfec0e4bb4d6dc0aa0b7) )
ROM_END





ROM_START(jak_totm)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("toysonthemove.bin", 0x000000, 0x800000, CRC(d08fb72a) SHA1(1fea98542ef7c65eef31afb70fd50952b4cef1c1) )
ROM_END

ROM_START(jak_ths)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("tripleheadersports.bin", 0x000000, 0x800000, CRC(2b5f8734) SHA1(57bccaa70f0efbf3da3259b74f3082d1a14c9908) )
ROM_END


ROM_START(wrlshunt)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x8000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("wireless.bin", 0x0000, 0x8000000, CRC(a6ecc20e) SHA1(3645f23ba2bb218e92d4560a8ae29dddbaabf796))
ROM_END

/*
Wireless Hunting Video Game System
(info provided with dump)

System: Wireless Hunting Video Game System
Publisher: Hamy / Kids Station Toys Inc
Year: 2011
ROM: FDI MSP55LV100G
RAM: Micron Technology 48LC8M16A2

Games:

Secret Mission
Predator
Delta Force
Toy Land
Dream Forest
Trophy Season
Freedom Force
Be Careful
Net Power
Open Training
Super Archer
Ultimate Frisbee
UFO Shooting
Happy Darts
Balloon Shoot
Avatair
Angry Pirate
Penguin War
Ghost Shooter
Duck Hunt


ROM Board:

Package: SO44
Spacing: 1.27 mm
Width: 16.14 mm
Length: 27.78 mm
Voltage: 3V
Pinout:

          A25  A24
            |  |
      +--------------------------+
A21 --|==   #  # `.__.'        ==|-- A20
A18 --|==                      ==|-- A19
A17 --|==                      ==|-- A8
 A7 --|==                      ==|-- A9
 A6 --|==                  o   ==|-- A10
 A5 --|==  +----------------+  ==|-- A11
 A4 --|==  |                |  ==|-- A12
 A3 --|==  |  MSP55LV100G   |  ==|-- A13
 A2 --|==  |  0834 M02H     |  ==|-- A14
 A1 --|==  |  JAPAN         |  ==|-- A15
 A0 --|==  |                |  ==|-- A16
#CE --|==  |                |  ==|-- A23
GND --|==  |                |  ==|-- A22
#OE --|==  |                |  ==|-- Q15
 Q0 --|==  |                |  ==|-- Q7
 Q8 --|==  |                |  ==|-- Q14
 Q1 --|==  +----------------+  ==|-- Q6
 Q9 --|==                      ==|-- Q13
 Q2 --|==       M55L100G       ==|-- Q5
Q10 --|==                      ==|-- Q12
 Q3 --|==                      ==|-- Q4
Q11 --|==                      ==|-- VCC
      +--------------------------+


The only interesting string in this ROM is SPF2ALP,
which is also found in the Wireless Air 60 ROM.

*/

// Bokudora Ver1.4 2014-09-20 on PCB
ROM_START(tomycar)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x2000000, "maincpu", ROMREGION_ERASE00)
	// this loading gives correct sprites (must be swapped somewhere on PCB because this was otherwise a standard pinout?)
	// but backgrounds are still broken (different issue maybe? there are writes to CS0 ROM area)
	ROM_LOAD16_WORD_SWAP( "tomycar.bin", 0x000000, 0x800000, CRC(bd98a198) SHA1(117aba55bf98bf76cbc9ed169e2a968bfdd9ed1a) )
	ROM_CONTINUE(0x1000000, 0x800000)
	ROM_CONTINUE(0x0800000, 0x800000)
	ROM_CONTINUE(0x1800000, 0x800000)
ROM_END



// also sold as "Pac-Man Connect & Play 35th Anniversary" (same ROM?)
CONS(2012, paccon,   0, 0, gpl16250_romram, paccon, paccon_game_state, init_wrlshunt, "Bandai", "Pac-Man Connect & Play (Feb 14 2012 10:46:23)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2008, lazertag, 0, 0, gpl16250_romram, jak_s500, lazertag_game_state, init_wrlshunt, "Tiger Electronics", "Lazer Tag Video Game Module", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2009, jak_swcl, 0, 0, gpl16250_romram, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc / HotGen Ltd",          "Star Wars: The Clone Wars - Republic Squadron (JAKKS Pacific TV Motion Game) (May 6 2009 12:53:31)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2009, jak_s500, 0, 0, gpl16250_romram, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc / HotGen Ltd",          "SpongeBob SquarePants Bikini Bottom 500 (JAKKS Pacific TV Motion Game) (Apr 16 2009 15:11:17)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2009, jak_smwm, 0, 0, gpl16250_romram, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc / HotGen Ltd",          "Spider-Man Web Master (JAKKS Pacific TV Motion Game) (Apr 23 2009 17:10:04)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2010, jak_pf,   0, 0, gpl16250_romram, jak_s500, jak_pf_game_state,   init_wrlshunt, "JAKKS Pacific Inc / HotGen Ltd",          "Phineas and Ferb: Best Game Ever! (JAKKS Pacific TV Motion Game) (Sep 16 2009 17:36:00)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // build date is 2009, but onscreen display is 2010
CONS(2009, jak_totm, 0, 0, gpl16250_romram, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc / HotGen Ltd",          "Toy Story - Toys on the Move (JAKKS Pacific TV Motion Game) (Dec 24 2009 17:34:29)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // Toys on the Move has ISSI 404A

CONS(2009, jak_prft, 0, 0, gpl16250_romram, jak_s500, jak_prft_game_state, init_wrlshunt, "JAKKS Pacific Inc / Santa Cruz Games",    "Power Rangers Force In Time (JAKKS Pacific TV Motion Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2009, jak_tink, 0, 0, gpl16250_romram, jak_s500, jak_prft_game_state, init_wrlshunt, "JAKKS Pacific Inc / Santa Cruz Games",    "Tinker Bell and the Lost Treasure (JAKKS Pacific TV Motion Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2009, jak_ths,  0, 0, gpl16250_romram, jak_ths,  jak_s500_game_state, init_ths,      "JAKKS Pacific Inc / Super Happy Fun Fun", "Triple Header Sports (JAKKS Pacific TV Motion Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2011, jak_sinv, 0, 0, gpl16250_romram, jak_s500, jak_s500_game_state, init_wrlshunt, "JAKKS Pacific Inc / Code Mystics",        "Retro Arcade featuring Space Invaders (JAKKS Pacific TV Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2011, wrlshunt, 0, 0, gpl16250_romram, wrlshunt, wrlshunt_game_state, init_wrlshunt, "Hamy / Kids Station Toys Inc", "Wireless Hunting Video Game System", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// ぼくはトミカドライバー はたらくのりもの大集合！
CONS(2014, tomycar,  0, 0, gpl16250_romram, paccon, jak_prft_game_state, init_wrlshunt, "Tomy Takara", "Boku wa Tomica Driver - Hataraku Norimono Daishuugou! (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

