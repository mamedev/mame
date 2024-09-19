// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

// JungleTac developed systems sold by Performance Designed Products, Lexibook etc.
// these are mostly handhelds with built in screens, and seem to be newer(?) revisions of the software
// of note, the way the audio is stored for Tiger Rescue has been updated and the core does not handle it properly at the moment

#include "emu.h"
#include "spg2xx.h"


namespace {

class spg2xx_lexizeus_game_state : public spg2xx_game_state
{
public:
	spg2xx_lexizeus_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void lexizeus(machine_config &config);

	void init_zeus();

protected:
	//virtual void machine_start() override ATTR_COLD;
	//virtual void machine_reset() override ATTR_COLD;
};


class spg2xx_lexiseal_game_state : public spg2xx_lexizeus_game_state
{
public:
	spg2xx_lexiseal_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_lexizeus_game_state(mconfig, type, tag)
	{ }

	void lexiseal(machine_config& config);

protected:
	//virtual void machine_start() override ATTR_COLD;
	//virtual void machine_reset() override ATTR_COLD;

	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

class spg2xx_vsplus_game_state : public spg2xx_lexizeus_game_state
{
public:
	spg2xx_vsplus_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_lexizeus_game_state(mconfig, type, tag)
	{ }

	void vsplus(machine_config &config);

	void init_vsplus();

protected:
	//virtual void machine_start() override ATTR_COLD;
	//virtual void machine_reset() override ATTR_COLD;

	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};



static INPUT_PORTS_START( lexizeus ) // how many buttons does this have?  I accidentally entered a secret test mode before that seemed to indicate 6, but can't get there again
	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "P1" )
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
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button 1") // shoot in Tiger Rescue & Deep
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Pause")

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "P2" )
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

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Button 1 Rapid") // same function as button 1 but with rapid toggle on/off
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Button 2 Rapid") // same function as button 2 but with rapid toggle on/off
	PORT_DIPNAME( 0x0004, 0x0004, "P3" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button 2") // toggles ball / number view in pool
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
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
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



static INPUT_PORTS_START( lexiseal )
	PORT_START("P1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED ) // doesn't respond as 'select'
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) // pause / start

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( vsplus )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Pause")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Rapid A")
	PORT_BIT( 0x0006, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Rapid B")
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void spg2xx_lexiseal_game_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: portb_w %04x %04x\n", machine().describe_context(), data, mem_mask);

	// mem_mask is set to fc when writing banks

	if ((data&mem_mask) == 0x5c)
		switch_bank(0);
	else if ((data&mem_mask) == 0xdc)
		switch_bank(1);
	else if ((data&mem_mask) == 0x7c)
		switch_bank(2);
}


void spg2xx_vsplus_game_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask & 0x0080)
	{
		if (data & 0x0080)
			switch_bank(1);
		else
			switch_bank(0);
	}
}

void spg2xx_lexiseal_game_state::lexiseal(machine_config &config)
{
	non_spg_base(config);
	m_maincpu->portb_out().set(FUNC(spg2xx_lexiseal_game_state::portb_w));
	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");
}

void spg2xx_lexizeus_game_state::lexizeus(machine_config &config)
{
	non_spg_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_lexiseal_game_state::mem_map_4m);
	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");
}

void spg2xx_vsplus_game_state::vsplus(machine_config &config)
{
	non_spg_base(config);
	m_maincpu->portb_out().set(FUNC(spg2xx_vsplus_game_state::portb_w));
	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");

	// output was PAL when connected to TV at least
	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);
}


void spg2xx_lexizeus_game_state::init_zeus()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0x8000 / 2; i < size / 2; i++)
	{
		// global 16-bit xor
		ROM[i] = ROM[i] ^ 0x8a1d;

		// 4 single bit conditional xors
		if (ROM[i] & 0x0020)
			ROM[i] ^= 0x0100;

		if (ROM[i] & 0x0040)
			ROM[i] ^= 0x1000;

		if (ROM[i] & 0x4000)
			ROM[i] ^= 0x0001;

		if (ROM[i] & 0x0080)
			ROM[i] ^= 0x0004;

		// global 16-bit bitswap
		ROM[i] = bitswap<16>(ROM[i], 7, 12, 9, 14, 4, 6, 0, 10, 15, 1, 3, 2, 5, 13, 8, 11);
	}
}

void spg2xx_vsplus_game_state::init_vsplus()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	decrypt_ac_ff(ROM, size);
}

ROM_START( lexizeus )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "lexibook1g900us.bin", 0x0000, 0x800000, CRC(c2370806) SHA1(cbb599c29c09b62b6a9951c724cd9fc496309cf9))
ROM_END

ROM_START( vsplus )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "vsplus.bin", 0x0000, 0x1000000, CRC(2b13d2cc) SHA1(accae7606d83a313b8ec0232d2d67b63c9c617af) )
ROM_END

ROM_START( lexiseal )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "lexibook_seal.bin", 0x0000, 0x1000000, CRC(3529f154) SHA1(f5f142600c6b2d037b97e007364ea2fa228e0163) )
ROM_END

ROM_START( discpal )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "disneyhh.bin", 0x0000, 0x400000, CRC(5fb7f32e) SHA1(795c992826ad4ac66d5438207f1c9b48f9fadc44) )
ROM_END

/*

VG Caplet ROM pinout from Sean Riddle (2 ROMs in single package)

BSIC M6MLT947 pinout
wide SOP64 footprint
end with 2 circles denotes pin 1 end
24MB with 2 chip enables
/CE1 L, CE2 H is 16MB, /CE1 H, CE2 L is 8MB
do not drive /CE1 and /CE2 both L at the same time

1  N/C          64 N/C
2  A15          63 A16
3  A14          62 /CE1
4  A13          61 VCC
5  A12          60 N/C
6  A11          59 GND
7  A10          58 N/C
8  A9           57 D15
9  A8           56 D7
10 A21          55 D14
11 A20          54 D6
12 N/C          53 D13
13 N/C          52 D5
14 VCC          51 D12
15 VCC          50 D4
16 N/C          49 GND
17 N/C          48 VCC
18 N/C          47 D11
19 N/C          46 D3
20 A22          45 D10
21 also A21     44 D2
22 A19          43 D9
23 A18          42 D1
24 A17          41 D8
25 A7           40 D0
26 A6           39 /OE
27 A5           38 GND
28 A4           37 N/C
29 A3           36 /CE2
30 A2           35 A0
31 A1           34 N/C
32 N/C          33 N/C

*/

ROM_START( vgcaplet )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "capleth.bin", 0x0000000, 0x1000000, CRC(f6cad3a7) SHA1(54167ab7651cf7a758ae36c443e74d3919e7fd6d) )
	ROM_LOAD16_WORD_SWAP( "capletl.bin", 0x1000000, 0x0800000, CRC(1ae2fa49) SHA1(62f41d45da011c1dfb35c1111a478798c2b33aaf) )
ROM_END

ROM_START( vgcap35 )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "vgpocket.bin", 0x0000000, 0x1000000, CRC(b4dd781b) SHA1(b060c83c2f96a2b78f075d1c8143f654016ff0ec) )
	ROM_RELOAD(0x1000000,0x1000000)
ROM_END

} // anonymous namespace


// these all have the same ROM scrambling

CONS( 200?, lexizeus,    0,     0,        lexizeus,     lexizeus, spg2xx_lexizeus_game_state, init_zeus, "Lexibook / JungleTac", "Zeus IG900 20-in-1 (US?)",          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // bad sound and some corrupt bg tilemap entries in Tiger Rescue, verify ROM data (same game runs in Zone 60 without issue)

CONS( 200?, vsplus,      0,     0,        vsplus,     vsplus, spg2xx_vsplus_game_state, init_vsplus, "<unknown> / JungleTac", "Vs Power Plus 30-in-1",          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )


CONS( 200?, lexiseal,    0,     0,        lexiseal,     lexiseal, spg2xx_lexiseal_game_state, init_zeus, "Lexibook / Sit Up Limited / JungleTac", "Seal 50-in-1",          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // also has bad sound in Tiger Rescue, but no corrupt tilemap
// There are versions of the Seal 50-in-1 that actually show Lexibook on the boot screen rather than it just being on the unit.  The Seal name was also used for some VT systems

CONS( 200?, discpal,     0,     0,        lexizeus,     lexiseal, spg2xx_lexizeus_game_state, init_zeus, "Performance Designed Products / Disney / Jungle Soft", "Disney Game It! Classic Pals",          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
// There was also a Game It! Princess Pals

CONS( 2006, vgcaplet,    0,     0,        lexiseal,     lexiseal, spg2xx_lexiseal_game_state, init_zeus, "Performance Designed Products (licensed by Taito / Data East) / JungleTac", "VG Pocket Caplet Fast Acting 50-in-1",          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2006, vgcap35,     0,     0,        lexiseal,     lexiseal, spg2xx_lexiseal_game_state, init_zeus, "Performance Designed Products (licensed by Taito / Data East) / JungleTac", "VG Pocket Caplet Fast Acting 35-in-1",          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
