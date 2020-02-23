// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood


#include "emu.h"
#include "includes/spg2xx.h"


class spg2xx_game_mysprtch_state : public spg2xx_game_state
{
public:
	spg2xx_game_mysprtch_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_romregion(*this, "maincpu")
	{ }

	void mysprtch(machine_config& config);
	void mgt20in1(machine_config& config);

	void init_mysprtcp();
	void init_mgt20in1();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void mem_map_mysprtch(address_map& map);

	virtual DECLARE_WRITE16_MEMBER(porta_w) override;

	int m_romsize;

	int m_mysprtch_rombase;
	uint16_t m_prev_porta;
	int m_bank_enabled;

private:
	DECLARE_READ16_MEMBER(mysprtch_rom_r);
	required_region_ptr<uint16_t> m_romregion;
};

class spg2xx_game_mysprtch24_state : public spg2xx_game_mysprtch_state
{
public:
	spg2xx_game_mysprtch24_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_game_mysprtch_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;
	virtual DECLARE_WRITE16_MEMBER(porta_w) override;

private:
};



READ16_MEMBER(spg2xx_game_mysprtch_state::mysprtch_rom_r)
{
	// due to granularity of rom bank this manual method is safer
	return m_romregion[(offset + (m_mysprtch_rombase * 0x200000)) & (m_romsize-1)];
}

void spg2xx_game_mysprtch_state::mem_map_mysprtch(address_map &map)
{
	map(0x000000, 0x3fffff).r(FUNC(spg2xx_game_mysprtch_state::mysprtch_rom_r));
}

void spg2xx_game_mysprtch_state::machine_start()
{
	spg2xx_game_state::machine_start();

	m_romsize = (memregion("maincpu")->bytes()/2);

	save_item(NAME(m_mysprtch_rombase));
	save_item(NAME(m_prev_porta));
	save_item(NAME(m_bank_enabled));
}

void spg2xx_game_mysprtch_state::machine_reset()
{
	spg2xx_game_state::machine_reset();

	m_mysprtch_rombase = 2;
	m_prev_porta = 0x0000;
	m_bank_enabled = 0;

	m_maincpu->invalidate_cache();
	m_maincpu->reset();
}

void spg2xx_game_mysprtch24_state::machine_reset()
{
	spg2xx_game_state::machine_reset();

	m_mysprtch_rombase = 3;
	m_prev_porta = 0x0000;
	m_bank_enabled = 0;

	m_maincpu->invalidate_cache();
	m_maincpu->reset();
}

static INPUT_PORTS_START( mysprtch ) // Down + Button 1 and Button 2 for service mode
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 RF Key")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 LF Wave")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 LF Key") // doesn't show in test mode but still read at times
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 RF Wave")

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

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 RF Wave")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 RF Key")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 LF Wave")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 LF Key") // doesn't show in test mode but still read at times
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
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

	PORT_START("P3")
	PORT_DIPNAME( 0x0001, 0x0001, "P3" )
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

static INPUT_PORTS_START( mgt20in1 ) // this seems to expect rotated controls by default (although the bowling expects you to rotate the controller to match mysprtchl)
	PORT_INCLUDE(mysprtch)

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 RF Key")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 LF Wave")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 LF Key") // doesn't show in test mode but still read at times
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 RF Wave")
INPUT_PORTS_END


WRITE16_MEMBER(spg2xx_game_mysprtch_state::porta_w)
{
	logerror("%s: porta_w %04x\n", machine().describe_context(), data);

	// this is rather ugly guesswork based on use and testmode

	if ((m_prev_porta & 0x00ff) != (data & 0x00ff))
	{
		logerror("lower changed\n");

		if ((data & 0x00ff) == 0x0000)
		{
			m_bank_enabled = 1;
			logerror("bank enabled\n");
		}
		else
		{
			m_bank_enabled = 0;
			logerror("bank disabled\n");
		}
	}

	if ((data & 0xff00) == 0x1e00) // gets written in test mode and expects the default bank to be restored
	{
		m_mysprtch_rombase = 2;
	}
	else if (m_bank_enabled)
	{
		logerror("potential bank %02x\n", data >> 9);

		int bank = 0;
		bank |= (data & 0x0400) ? 1 : 0;
		bank |= (data & 0x0800) ? 2 : 0;
		bank |= (data & 0x0200) ? 0 : 4; // inverted

		m_mysprtch_rombase = bank;
	}

	m_prev_porta = data;
}

WRITE16_MEMBER(spg2xx_game_mysprtch24_state::porta_w)
{
	logerror("%s: porta_w %04x\n", machine().describe_context(), data);

	// this is rather ugly guesswork based on use and testmode
	// the game writes 0x0000 to the ports during startup, which would cause an incorrect bank
	// but this value can't just be ignored when we want to bank there
	// so only bank if running from RAM

	// probably should be the same logic for both games, as the test mode on this one proves
	// that the logic in spg2xx_game_mysprtch_state::porta_w is incorrect

	if (m_maincpu->pc() < 0x4000)
	{
		int bank = 0;
		bank |= (data & 0x0400) ? 1 : 0;
		bank |= (data & 0x0800) ? 2 : 0;
		bank |= (data & 0x0200) ? 0 : 4; // inverted

		m_mysprtch_rombase = bank;
	}

	m_prev_porta = data;
}


void spg2xx_game_mysprtch_state::mysprtch(machine_config& config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_mysprtch_state::mem_map_mysprtch);
	m_maincpu->set_force_no_drc(true); // uses JVS opcode, not implemented in recompiler

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_mysprtch_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_mysprtch_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_mysprtch_state::base_portc_r));

	m_maincpu->porta_out().set(FUNC(spg2xx_game_mysprtch_state::porta_w));
}

void spg2xx_game_mysprtch_state::mgt20in1(machine_config& config)
{
	mysprtch(config);

	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);
}

void spg2xx_game_mysprtch_state::init_mysprtcp()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0; i < size / 2; i++)
	{
		ROM[i] = bitswap<16>(ROM[i], 15, 13, 14, 12,
									 7,  6,  5,  4,
									 11, 10, 9,  8,
									 3,  1,  2,  0);

		ROM[i] = ROM[i] ^ 0xfafa;
	}
}



void spg2xx_game_mysprtch_state::init_mgt20in1()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0; i < size/2; i++)
	{
		ROM[i] = ROM[i] ^ 0x4ec4;

		uint16_t res = 0;

		if (ROM[i] & 0x0001) res ^= 0x0040;
		if (ROM[i] & 0x0002) res ^= 0x0002;
		if (ROM[i] & 0x0004) res ^= 0x0020;
		if (ROM[i] & 0x0008) res ^= 0x1200; // 2 bits changed

		if (ROM[i] & 0x0010) res ^= 0x0100;
		if (ROM[i] & 0x0020) res ^= 0x4000;
		if (ROM[i] & 0x0040) res ^= 0x0010;
		if (ROM[i] & 0x0080) res ^= 0x0800;

		if (ROM[i] & 0x0100) res ^= 0x4400; // 2 bits changed
		if (ROM[i] & 0x0200) res ^= 0x0080;
		if (ROM[i] & 0x0400) res ^= 0x0001;
		if (ROM[i] & 0x0800) res ^= 0x2000;

		if (ROM[i] & 0x1000) res ^= 0x0004;
		if (ROM[i] & 0x2000) res ^= 0x000a; // 2 bits changed
		if (ROM[i] & 0x4000) res ^= 0x0200;
		if (ROM[i] & 0x8000) res ^= 0x8010; // 2 bits changed

		ROM[i] = res;
	}
}


ROM_START( mysprtcp )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mysportschallengeplus.bin", 0x0000, 0x2000000, CRC(6911d19c) SHA1(c71bc38595e5505434395b6d59320caabfc7bce3) )
ROM_END

ROM_START( mysptqvc )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "qvcmysportschallenge.bin", 0x0000000, 0x2000000, CRC(04783adc) SHA1(a173145ec307fc12f231d3e3f6efa60f8c2f0c89) ) // last 8MB is unused
ROM_END

ROM_START( mysprtch )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 ) // SOP64 M6MLT947, has two /CE lines so internally this '24MByte / 192Mbit' chip is likely 2 ROM dies in a single package
	ROM_LOAD16_WORD_SWAP( "senariomysportschallengesop64h.bin", 0x0000000, 0x1000000, CRC(3714df21) SHA1(f725dad48b9dfeba188879a6fd28652a7330d3e5) )
	ROM_LOAD16_WORD_SWAP( "senariomysportschallengesop64l.bin", 0x1000000, 0x0800000, CRC(0f71099f) SHA1(6e4b9ce329edbb6f0b962cb5669e04c6bd209596) )
ROM_END



ROM_START( mgt20in1 )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "m29gl128.u2", 0x000000, 0x1000000, CRC(41d594e3) SHA1(351890455bed28bcaf173d8fd9a4cc997c404d94) )
ROM_END

// Unit with Blue surround to power button. Box shows 'Wireless Sports Plus' but title screen shots "My Sports Challenge Plus"  Appears to be V-Tac developed as it has the common V-Tac test mode.
CONS( 200?, mysprtcp,  0, 0, mysprtch, mysprtch, spg2xx_game_mysprtch_state,  init_mysprtcp, "Senario / V-Tac Technology Co Ltd.",  "My Sports Challenge Plus / Wireless Sports Plus",  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// from a QVC licensed unit with a different physical shape etc. uses a 32MByte rom with only 24MByte used
CONS( 200?, mysptqvc,  0, 0, mysprtch, mysprtch, spg2xx_game_mysprtch24_state,  init_mysprtcp, "Senario / V-Tac Technology Co Ltd. (QVC license)",  "My Sports Challenge (6-in-1 version, QVC license)",  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, mysprtch,  0, 0, mysprtch, mysprtch, spg2xx_game_mysprtch24_state,  init_mysprtcp, "Senario / V-Tac Technology Co Ltd.",                "My Sports Challenge (5-in-1 version)",  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// 2009 date on PCB, not actually in German, so maybe sold under different brands?
CONS( 2009, mgt20in1,  0, 0, mgt20in1, mgt20in1, spg2xx_game_mysprtch_state,  init_mgt20in1, "MGT",                                 "MGT 20-in-1 TV-Spielekonsole (Germany)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
