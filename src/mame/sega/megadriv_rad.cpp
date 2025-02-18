// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Radica 'Mega Drive' and 'Genesis' clones
    these were mini battery operated "TV Game" consoles with wired in controller and no cartslot
    fully licensed by Sega

    reproduction 'System on a Chip' hardware, not perfect, flaws will need emulating eventually.

    not dumped

    Genesis Volume 2
    Genesis SF2 / GnG (PAL one is locked to PAL)

    Outrun 2019 (probably identical ROM to MD version, just custom controller)

    more?

*/

#include "emu.h"
#include "megadriv.h"


namespace {

class megadriv_radica_state_base : public md_ctrl_state
{
public:
	megadriv_radica_state_base(const machine_config &mconfig, device_type type, const char *tag) :
		md_ctrl_state(mconfig, type, tag),
		m_bank(0),
		m_romsize(0x400000),
		m_rom(*this, "maincpu")
	{ }

protected:
	uint16_t read(offs_t offset);
	uint16_t read_a13(offs_t offset);

	void megadriv_radica_map(address_map &map) ATTR_COLD;

	void radica_base_map(address_map &map) ATTR_COLD;

	int m_bank;
	int m_romsize;

private:
	required_region_ptr<uint16_t> m_rom;
};


class megadriv_radica_state : public megadriv_radica_state_base
{
public:
	megadriv_radica_state(const machine_config& mconfig, device_type type, const char* tag) :
		megadriv_radica_state_base(mconfig, type, tag)
	{ }

	void megadriv_radica_3button_ntsc(machine_config &config);
	void megadriv_radica_3button_pal(machine_config &config);

	void megadriv_radica_6button_ntsc(machine_config &config);
	void megadriv_radica_6button_pal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
};


class megadriv_dgunl_state : public megadriv_radica_state
{
public:
	megadriv_dgunl_state(const machine_config& mconfig, device_type type, const char* tag) :
		megadriv_radica_state(mconfig, type, tag)
	{ }

	void megadriv_dgunl_ntsc(machine_config &config);

	void init_dgunl3227();

protected:
	virtual void machine_start() override ATTR_COLD;

	uint16_t m_a1630a = 0;

private:
	uint16_t read_a16300(offs_t offset, uint16_t mem_mask);
	uint16_t read_a16302(offs_t offset, uint16_t mem_mask);
	virtual void write_a1630a(offs_t offset, uint16_t data, uint16_t mem_mask);

	void megadriv_dgunl_map(address_map &map) ATTR_COLD;
};


class megadriv_ra145_state : public megadriv_dgunl_state
{
public:
	megadriv_ra145_state(const machine_config& mconfig, device_type type, const char* tag) :
		megadriv_dgunl_state(mconfig, type, tag)
	{ }

	void megadriv_ra145_ntsc(machine_config &config);

	void init_ra145();

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	virtual void write_a1630a(offs_t offset, uint16_t data, uint16_t mem_mask) override;
};



void megadriv_radica_state_base::radica_base_map(address_map &map)
{
	megadriv_68k_base_map(map);

	map(0x000000, 0x3fffff).r(FUNC(megadriv_radica_state_base::read)); // Cartridge Program ROM
}

void megadriv_radica_state_base::megadriv_radica_map(address_map &map)
{
	radica_base_map(map);

	map(0xa13000, 0xa130ff).r(FUNC(megadriv_radica_state_base::read_a13));
}

uint16_t megadriv_dgunl_state::read_a16300(offs_t offset, uint16_t mem_mask)
{
	return 0x5a5a;
}

uint16_t megadriv_dgunl_state::read_a16302(offs_t offset, uint16_t mem_mask)
{
	return m_a1630a;
}

void megadriv_dgunl_state::write_a1630a(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_a1630a = data;
	m_bank = (data & 0x07) * 8;
}


void megadriv_ra145_state::write_a1630a(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// the banking must know how big each game is to get the base location, as it only writes a game number?!
	// it must also know to skip the unused copy of Golden Axe
	// there must be a table somewhere, or, if this is actually an emulation based system, it must be scanning
	// for headers somehow?

	m_a1630a = data;

	switch (data & 0xff)
	{


	//                  0x0000000           = Block of unknown data/code
	//                  0x0040000           = 145 in 1 menu
	//                  (includes unused Columns, Fatal Labyrinth, Blockout, Flicky, Shove It, Space Invaders 90 as part of menu data)
	case 0x00: m_bank = 0x02c0000; break;// = Fantasia                               00
	case 0x01: m_bank = 0x0340000; break;// = Fire Shark                             01
	case 0x02: m_bank = 0x03c0000; break;// = James Bond 007 - The Duel              02  (BAD ROM)
	case 0x03: m_bank = 0x0440000; break;// = Sunset Riders                          03
	case 0x04: m_bank = 0x04c0000; break;// = Robocop 3                              04
	case 0x05: m_bank = 0x0540000; break;// = Hokuto no Ken                          05  (BAD ROM)
	case 0x06: m_bank = 0x05c0000; break;// = Alien 3                                06
	case 0x07: m_bank = 0x0640000; break;// = Batman                                 07  (BAD ROM)
	case 0x08: m_bank = 0x06c0000; break;// = Cat                                    08
	case 0x09: m_bank = 0x0740000; break;// = DJ Boy                                 09
	case 0x0a: m_bank = 0x07c0000; break;// = Rambo 3                                0a  (BAD ROM)
	case 0x0b: m_bank = 0x0840000; break;// = Wardner                                0b
	case 0x0c: m_bank = 0x08c0000; break;// = Paperboy                               0c
	case 0x0d: m_bank = 0x0940000; break;// = Streets of Rage                        0d
	case 0x0e: m_bank = 0x09c0000; break;// = Tiny Toon Adventures                   0e
	case 0x0f: m_bank = 0x0a40000; break;// = Super Battleship                       0f
	case 0x10: m_bank = 0x0ac0000; break;// = Burning Force                          10
	case 0x11: m_bank = 0x0b40000; break;// = Cadash                                 11
	case 0x12: m_bank = 0x0bc0000; break;// = Caesars Palace                         12
	case 0x13: m_bank = 0x0c40000; break;// = chase HQ 2                             13
	case 0x14: m_bank = 0x0cc0000; break;// = Wonderboy 3                            14
	case 0x15: m_bank = 0x0d40000; break;// = Fighting Master                        15
	case 0x16: m_bank = 0x0dc0000; break;// = The Flintstones                        16
	case 0x17: m_bank = 0x0e40000; break;// = Ariel the Little Mermaid               17
	case 0x18: m_bank = 0x0ec0000; break;// = Hellfire                               18
	case 0x19: m_bank = 0x0f40000; break;// = Arrow Flash                            19
	case 0x1a: m_bank = 0x0fc0000; break;// = Shove It                               1a
	case 0x1b: m_bank = 0x1040000; break;// = Donkey Kong 99                         1b
	case 0x1c: m_bank = 0x1240000; break;// = Turtles Tournament                     1c
	case 0x1d: m_bank = 0x1440000; break;// = Thunder Force 2                        1d  (This is meant to be Revenge of Shinobi according to the menu, but incorrect game was in the ROM)
	case 0x1e: m_bank = 0x14c0000; break;// = Wings of Wor                           1e
	case 0x1f: m_bank = 0x1540000; break;// = Wrestle War                            1f
	case 0x20: m_bank = 0x15c0000; break;// = Afterburner 2                          20
	case 0x21: m_bank = 0x1640000; break;// = Altered Beast                          21
	case 0x22: m_bank = 0x16c0000; break;// = Captain Planet                         22
	case 0x23: m_bank = 0x1740000; break;// = Bimimi Run                             23
	case 0x24: m_bank = 0x17c0000; break;// = Osomatsu                               24
	case 0x25: m_bank = 0x1840000; break;// = Castle of Illusion                     25
	case 0x26: m_bank = 0x18c0000; break;// = Crackdown                              26
	case 0x27: m_bank = 0x1940000; break;// = Crossfire                              27
	case 0x28: m_bank = 0x19c0000; break;// = Curse                                  28
	case 0x29: m_bank = 0x1a40000; break;// = Dangerous Seed                         29
	case 0x2a: m_bank = 0x1ac0000; break;// = Dark Castle                            2a
	case 0x2b: m_bank = 0x1b40000; break;// = Darwin                                 2b
	case 0x2c: m_bank = 0x1bc0000; break;// = Thunder Force 2 (duplicate?)           2c
	case 0x2d: m_bank = 0x1c40000; break;// = Dynamite Duke                          2d
	case 0x2e: m_bank = 0x1cc0000; break;// = EA Hockey                              2e
	case 0x2f: m_bank = 0x1d40000; break;// = Elemenal Master                        2f
	case 0x30: m_bank = 0x1dc0000; break;// = Super Thunder Blade                    30
	case 0x31: m_bank = 0x1e40000; break;// = Target Earth                           31
	case 0x32: m_bank = 0x1ec0000; break;// = Rastan Saga 2                          32
	case 0x33: m_bank = 0x1f40000; break;// = Ghostbusters                           33
	case 0x34: m_bank = 0x1fc0000; break;// = Mahjong cop                            34
	case 0x35: m_bank = 0x2040000; break;// = High School Soccer                     35
	case 0x36: m_bank = 0x20c0000; break;// = Insector X                             36
	case 0x37: m_bank = 0x2140000; break;// = Pheilos                                37
	case 0x38: m_bank = 0x21c0000; break;// = Runark                                 38
	case 0x39: m_bank = 0x2240000; break;// = Saint Sword                            39
	case 0x3a: m_bank = 0x22c0000; break;// = Shadow Dancer                          3a
	case 0x3b: m_bank = 0x2340000; break;// = Shiten Myooh                           3b
	case 0x3c: m_bank = 0x23c0000; break;// = Wani Wani World                        3c
	case 0x3d: m_bank = 0x2440000; break;// = Street Mmart                           3d
	case 0x3e: m_bank = 0x24c0000; break;// = Toki                                   3e
	case 0x3f: m_bank = 0x2540000; break;// = Trouble Shooter                        3f
	case 0x40: m_bank = 0x25c0000; break;// = Truxton                                40
	case 0x41: m_bank = 0x2640000; break;// = James Pond 2                           41
	case 0x42: m_bank = 0x26c0000; break;// = Twin Hawk                              42
	case 0x43: m_bank = 0x2740000; break;// = Syd of Valis                           43
	case 0x44: m_bank = 0x27c0000; break;// = Zoom                                   44
	case 0x45: m_bank = 0x2840000; break;// = Streets of Rage 2                      45
	case 0x46: m_bank = 0x2a40000; break;// = Sonic & Knuckles                       46
	case 0x47: m_bank = 0x2c40000; break;// = Comix Zone                             47
	case 0x48: m_bank = 0x2e40000; break;// = Rolling Thunder 2                      48
	case 0x49: m_bank = 0x2f40000; break;// = Bubble & Squeek                        49
	case 0x4a: m_bank = 0x2fc0000; break;// = Alex Kidd                              4a
	case 0x4b: m_bank = 0x3040000; break;// = Super Mario 2                          4b
	case 0x4c: m_bank = 0x3240000; break;// = NBA All Star                           4c
	case 0x4d: m_bank = 0x3340000; break;// = Bio Hazard Battle                      4d
	case 0x4e: m_bank = 0x3440000; break;// = Prince of Persia                       4e
	case 0x4f: m_bank = 0x3540000; break;// = Champions World Soccer                 4f
	case 0x50: m_bank = 0x3640000; break;// = Lotus 2                                50
	case 0x51: m_bank = 0x3740000; break;// = Grandia                                51
	case 0x52: m_bank = 0x37c0000; break;// = World Cup Soccer                       52
	case 0x53: m_bank = 0x3840000; break;// = Ultimate Mortal Kombat 3               53
	case 0x54: m_bank = 0x3c40000; break;// = Street Fighter II (hacked MSI version) 54
	case 0x55: m_bank = 0x3f40000; break;// = Verytex                                55
	case 0x56: m_bank = 0x3fc0000; break;// = Space Invaders 90                      56

	//This 32MBytes of the ROM has its own unused '666666 in 1 menu' and only references games
	//within this block; it was probably released as a standalone using this 32Mbytes of ROM
	//                  0x4000000 = Block of unknown data/code
	//                  0x4040000 = 666666 in 1 menu
	//                  (includes unused Columns, Fatal Labyrinth, Blockout, Flicky, Shove It, Space Invaders 90 as part of menu data)
	//                  0x42c0000 - Golden Axe                             -- (not used by 145-in-1 menu, no assigned number either?!)
	case 0x57: m_bank = 0x4340000; break;// - Mega Bomberman                         57
	case 0x58: m_bank = 0x4440000; break;// - Dragon The Bruce Lee Story             58
	case 0x59: m_bank = 0x4640000; break;// - Twinkle Tale                           59
	case 0x5a: m_bank = 0x4740000; break;// - Jewel Master                           5a
	case 0x5b: m_bank = 0x47c0000; break;// - Pacmania                               5b
	case 0x5c: m_bank = 0x4840000; break;// - X-Men 2                                5c
	case 0x5d: m_bank = 0x4a40000; break;// - Top Gear 2                             5d
	case 0x5e: m_bank = 0x4b40000; break;// - Brian Lara Cricket                     5e
	case 0x5f: m_bank = 0x4c40000; break;// - Garfield                               5f
	case 0x60: m_bank = 0x4e40000; break;// - Raiden Trad                            60
	case 0x61: m_bank = 0x4f40000; break;// - James Pond                             61
	case 0x62: m_bank = 0x4fc0000; break;// - Mega Panel                             62
	case 0x63: m_bank = 0x5040000; break;// - James Pond 3                           63
	case 0x64: m_bank = 0x5240000; break;// - Contra Hardcorps                       64
	case 0x65: m_bank = 0x5440000; break;// - International Superstar Soccer Deluxe  65
	case 0x66: m_bank = 0x5640000; break;// - Double Dragon 3 (with Trainer)         66
	case 0x67: m_bank = 0x5740000; break;// - Micro Machines                         67
	case 0x68: m_bank = 0x57c0000; break;// - Hard Drivin'                           68
	case 0x69: m_bank = 0x5840000; break;// - Dragon Ball Z                          69
	case 0x6a: m_bank = 0x5a40000; break;// - F1 World Championship Edition          6a
	case 0x6b: m_bank = 0x5c40000; break;// - Megaman / Rockman                      -- (6b, but unused? probably due to save feature?)
	case 0x6c: m_bank = 0x5e40000; break;// - Operation Vapor Trail                  6c
	case 0x6d: m_bank = 0x5f40000; break;// - Roadblasters                           6d
	case 0x6e: m_bank = 0x5fc0000; break;// - Super Volleyball                       6e

	//The final 32MBytes of the ROM has its own unused '888888 in 1 menu' and only references games
	//within this block; it was probably released as a standalone using this 32Mbytes of ROM
	//                  0x6000000 = Block of unknown data/code
	//                  0x6040000 = 888888 in 1 menu
	//                  (includes unused Columns, Fatal Labyrinth, Blockout, Flicky, Shove It, Space Invaders 90 as part of menu data)
	case 0x6f: m_bank = 0x62c0000; break;// - Last Battle                            6f
	case 0x70: m_bank = 0x6340000; break;// - Shinobi III                            70
	case 0x71: m_bank = 0x6440000; break;// - Mickey Mania                           71
	case 0x72: m_bank = 0x6640000; break;// - Snow Bros                              72
	case 0x73: m_bank = 0x6740000; break;// - Space Harrier II                       73
	case 0x74: m_bank = 0x67c0000; break;// - Volfied                                74
	case 0x75: m_bank = 0x6840000; break;// - Goofy's Hysterical History Tour        75
	case 0x76: m_bank = 0x6940000; break;// - Mighty Max                             76
	case 0x77: m_bank = 0x6a40000; break;// - Mr Nutz                                77
	case 0x78: m_bank = 0x6b40000; break;// - Sonic 2                                78
	case 0x79: m_bank = 0x6c40000; break;// - Ecco Jr.                               79
	case 0x7a: m_bank = 0x6d40000; break;// - Desert Demolition                      7a
	case 0x7b: m_bank = 0x6e40000; break;// - Midnight Resistance                    7b
	case 0x7c: m_bank = 0x6f40000; break;// - Spiderman vs The Kingpin               7c
	case 0x7d: m_bank = 0x6fc0000; break;// - Trampoline Terror                      7d
	case 0x7e: m_bank = 0x7040000; break;// - MUSHA                                  7e
	case 0x7f: m_bank = 0x70c0000; break;// - Sonic                                  7f
	case 0x80: m_bank = 0x7140000; break;// - Thunder Force III                      80
	case 0x81: m_bank = 0x71c0000; break;// - Golden Axe (again)                     81
	//                  0x7240000; break;// - Controller Test Menu                   -- (unused)
	//                  0x7340000; break;// - Controller Test Menu (again)           -- (unused)
	case 0x82: m_bank = 0x7440000; break;// - Wacky Worlds                           82 (writes 93, incorrectly swapped in menu with Wacky Worlds)
	case 0x83: m_bank = 0x7540000; break;// - Bonkers                                83
	case 0x84: m_bank = 0x7640000; break;// - Alisia Dragoon                         84
	case 0x85: m_bank = 0x7740000; break;// - Super Hang On                          85
	case 0x86: m_bank = 0x77c0000; break;// - Dragon's Eye Shanghai 3                86
	case 0x87: m_bank = 0x7840000; break;// - Jimmy White's Whirlwind Snooker        87
	case 0x88: m_bank = 0x78c0000; break;// - Home Alone 2                           88
	case 0x89: m_bank = 0x7940000; break;// - Xenon 2 (with trainer)                 89
	case 0x8a: m_bank = 0x79c0000; break;// - Hit the Ice                            8a
	case 0x8b: m_bank = 0x7a40000; break;// - Tale Spin                              8b
	case 0x8c: m_bank = 0x7ac0000; break;// - Puyo Puyo                              8c
	case 0x8d: m_bank = 0x7b40000; break;// - Rampart (with trainer)                 8d
	case 0x8e: m_bank = 0x7bc0000; break;// - Crue Ball                              8e
	case 0x8f: m_bank = 0x7c40000; break;// - Marble Madness                         8f
	case 0x90: m_bank = 0x7cc0000; break;// - Patlabor                               90
	case 0x91: m_bank = 0x7d40000; break;// - The Aquatic Games                      91
	case 0x92: m_bank = 0x7dc0000; break;// - King Salmon                            92
	case 0x93: m_bank = 0x7e40000; break;// - Fun n Games                            93 (writes 82, incorrectly swapped in menu with Fun n Games)
	case 0x94: m_bank = 0x7f40000; break;// - Gynoug                                 94
	case 0x95: m_bank = 0x7fc0000; break;// - Ms. Pac-Man                            95
	default:   m_bank = 0x0040000; break;
	}

	m_bank = m_bank / 0x10000;
}


void megadriv_dgunl_state::megadriv_dgunl_map(address_map &map)
{
	radica_base_map(map);

	map(0xa16300, 0xa16301).r(FUNC(megadriv_dgunl_state::read_a16300));
	map(0xa16302, 0xa16303).r(FUNC(megadriv_dgunl_state::read_a16302));

	map(0xa1630a, 0xa1630b).w(FUNC(megadriv_dgunl_state::write_a1630a));
}

uint16_t megadriv_radica_state_base::read(offs_t offset)
{
	return m_rom[(((m_bank * 0x10000) + (offset << 1)) & (m_romsize - 1))/2];
}

uint16_t megadriv_radica_state_base::read_a13(offs_t offset)
{
	if (offset < 0x80)
		m_bank = offset & 0x3f;

	// low bit gets set when selecting cannon fodder or mega lo mania in the rad_ssoc set, pointing to the wrong area, but rad_gen1 needs it for the menu
	// as they're standalones it could just be different logic
	if (m_bank != 0x3f)
		m_bank &= 0x3e;

	return 0;
}

// controller is wired directly into unit, no controller slots
static INPUT_PORTS_START( radica_3button )
	PORT_INCLUDE( md_common )

	// TODO: how do the MENU buttons on the two controllers work?
INPUT_PORTS_END

// the 6-in-1 and Sonic Gold units really only have a single wired controller, and no way to connect a 2nd one, despite having some 2 player games!
static INPUT_PORTS_START( radica_3button_1player )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )

	// TODO: how does the MENU button on the controller work?
INPUT_PORTS_END

static INPUT_PORTS_START( radica_6button )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1") // Extra buttons for Joypad 1 (6 button + start + mode)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("%p Z")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("%p Y")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("%p X")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SELECT )  PORT_PLAYER(1) PORT_NAME("%p Mode")

	PORT_MODIFY("PAD2") // Extra buttons for Joypad 2 (6 button + start + mode)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("%p Z")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("%p Y")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("%p X")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SELECT )  PORT_PLAYER(2) PORT_NAME("%p Mode")
INPUT_PORTS_END

static INPUT_PORTS_START( msi_6button )
	PORT_INCLUDE( radica_6button )

	PORT_MODIFY("PAD2") // no 2nd pad
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET") // RESET button on controller to the left of START
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1) PORT_NAME("Reset")
INPUT_PORTS_END

static INPUT_PORTS_START( dgunl_1player )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED  )                                  PORT_CONDITION("DEBUG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("%p C") PORT_CONDITION("DEBUG", 0x01, EQUALS, 0x01)

	PORT_MODIFY("PAD2") // no 2nd pad
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET") // RESET button to the left of START
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1) PORT_NAME("Reset")

	// the unit only has 2 buttons, A and B, strings are changed to remove references to C, even if behavior in Pac-Mania still exists and differs between them
	// however, Pac-Man still has a test mode which requires holding A+C on startup
	PORT_START("DEBUG")
	PORT_CONFNAME( 0x01, 0x00, "Enable Button C" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Yes ) )
INPUT_PORTS_END


void megadriv_radica_state::machine_start()
{
	megadriv_radica_state_base::machine_start();

	m_vdp->stop_timers();

	save_item(NAME(m_bank));
}

void megadriv_dgunl_state::machine_start()
{
	megadriv_radica_state::machine_start();

	m_a1630a = 0;

	save_item(NAME(m_a1630a));
}


void megadriv_ra145_state::machine_reset()
{
	m_bank = 4;
	megadriv_radica_state_base::machine_reset();
}

void megadriv_radica_state::machine_reset()
{
	m_bank = 0;
	megadriv_radica_state_base::machine_reset();
}


void megadriv_radica_state::megadriv_radica_3button_ntsc(machine_config &config)
{
	md_ntsc(config);

	ctrl1_3button(config);
	ctrl2_3button(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
}

void megadriv_radica_state::megadriv_radica_3button_pal(machine_config &config)
{
	md_pal(config);

	ctrl1_3button(config);
	ctrl2_3button(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_radica_state::megadriv_radica_map);
}

void megadriv_radica_state::megadriv_radica_6button_pal(machine_config &config)
{
	megadriv_radica_3button_pal(config);

	ctrl1_6button(config);
	ctrl2_6button(config);
}

void megadriv_radica_state::megadriv_radica_6button_ntsc(machine_config &config)
{
	megadriv_radica_3button_ntsc(config);

	ctrl1_6button(config);
	ctrl2_6button(config);
}

void megadriv_dgunl_state::megadriv_dgunl_ntsc(machine_config &config)
{
	md_ntsc(config);

	ctrl1_3button(config);
	ctrl2_3button(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &megadriv_dgunl_state::megadriv_dgunl_map);
}

void megadriv_ra145_state::megadriv_ra145_ntsc(machine_config &config)
{
	megadriv_dgunl_ntsc(config);

	ctrl1_6button(config);
	ctrl2_6button(config);
}


ROM_START( rad_sf2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_streetfighter2_usa.bin", 0x000000, 0x400000, CRC(a4426df8) SHA1(091f2a95ebd091141de5bcb83562c6087708cb32) )
ROM_END

ROM_START( rad_sf2uk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_streetfighter2_uk.bin", 0x000000, 0x400000,  CRC(868afb44) SHA1(f4339e36272c18b1d49aa4095127ed18e0961df6) )
ROM_END

ROM_START( mdtvp3j )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "playtv_vol3.bin", 0x000000, 0x400000,  CRC(d2daf376) SHA1(147b88d7aff834146c649077b43312c71b973298) )
ROM_END

ROM_START( rad_gen1 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_vol1_blue_usa.bin", 0x000000, 0x400000,  CRC(3b4c8438) SHA1(5ed9c053f9ebc8d4bf571d57e562cf347585d158) )
ROM_END

ROM_START( rad_md1 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_vol1_blue_europe.bin", 0x000000, 0x400000, CRC(85867db1) SHA1(ddc596e2e68dc872bc0679a2de7a295b4c6d6b8e) )
ROM_END

ROM_START( rad_md1uk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radicauk.u2", 0x000000, 0x400000, CRC(03a6734b) SHA1(255048d46b593bc975b3a6c44e8b8e35917511c7) )
ROM_END

ROM_START( mdtvp1j )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "l08y6_i_32m.u2", 0x000000, 0x400000, CRC(740a8859) SHA1(cf1212ef28e75e2cea752cf10a06ea715a30ae07) ) // 04-07-23 date sticker (23 July 2004)
ROM_END

ROM_START( rad_gen2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_genesis_vol2_red_usa.bin", 0x000000, 0x400000, CRC(7c1a0f0e) SHA1(a6441f75a4cd48f1563aeafdfbdde00202d4067c) )
ROM_END

ROM_START( rad_md2uk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_megadrive_vol2_red_uk.bin", 0x000000, 0x400000, CRC(b68fd025) SHA1(b8f9c505653d6dd2b62840f078f828360faf8abc) )
ROM_END

ROM_START( mdtvp2j )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "playtv_vol2.bin", 0x000000, 0x400000, CRC(4d887d12) SHA1(b7f70abd12c3a3c68d1ad127a1475b704e898f51) )
ROM_END

ROM_START( rad_ssoc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "radica_sensiblesoccer_uk.bin", 0x000000, 0x400000,  CRC(b8745ab3) SHA1(0ab3f26e5ffd288e5a3a5db676951b9095299eb0) ) // should be byteswapped?
ROM_END

ROM_START( rad_sonic )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_supersonicgold_usa.bin", 0x000000, 0x400000, CRC(853c9140) SHA1(cf70a9cdd3be4d8d1b6195698db3a941f4908791) )
ROM_END

ROM_START( rad_sonicuk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "radica_supersonicgold_uk.bin", 0x000000, 0x400000, CRC(ed774018) SHA1(cc2f7183e128c947463e3a43a0184b835ea16db8) )
ROM_END

// once byteswapped this matches "outrun 2019 (usa) (beta).bin  megadriv:outr2019up Out Run 2019 (USA, Prototype)"
// this was dumped from a PAL/UK unit, so maybe that 'beta' is really an alt Euro release, or was simply dumped from one of these Radica units and mislabeled?
ROM_START( rad_orun )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "outrun.bin", 0x000000, 0x100000, CRC(4fd6d653) SHA1(57f0e4550ff883e4bb7857caef2c893c21f80b42) )
ROM_END

ROM_START( rad_mncr )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	// radica_menacer_byteswapped.bin = mpr-15075-f.u1        megadriv:menacer  Menacer 6-Game Cartridge (Europe, USA)
	ROM_LOAD16_WORD_SWAP( "radica_menacer.bin", 0x000000, 0x100000, CRC(5f9ef4a4) SHA1(f28350e7325cb7469d760d97ee452a9d846eb3d4) )
ROM_END

ROM_START( msi_sf2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	// The first part of the ROM seems to be a boot ROM for the enhanced MD clone menus, even if it does nothing here
	// and is probably leftover from one of the multigame systems, hacked to only launch one game. We should emulate it...
	// .. but the game ROM starts at 0xc8000 so we can cheat for now
	ROM_LOAD16_WORD_SWAP( "29lv320.bin", 0x000000, 0xc8000, CRC(465b12f0) SHA1(7a058f6feb4f08f56ae0f7369c2ca9a9fe2ed40e) )
	ROM_CONTINUE(0x00000,0x338000)
ROM_END

ROM_START( dgunl3227 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	// populated in init function

	ROM_REGION( 0x400000, "rom", 0 )
	ROM_LOAD16_WORD_SWAP( "pacmantc58fvm5t2a.bin", 0x000000, 0x400000, CRC(b09fa599) SHA1(3cc50bee7ef91608848fb34185a0723d2b82b46f) )
ROM_END

ROM_START( dgunl3227a )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	// populated in init function

	ROM_REGION( 0x400000, "rom", 0 )
	ROM_LOAD16_WORD_SWAP( "myarcadepacman_s99jl032hbt1_9991227e_as_s29jl032h55tai01.bin", 0x000000, 0x400000, CRC(ecead966) SHA1(971e8da6eb720f670f4148c7e07922e4f24eb609) )
ROM_END

ROM_START( matet )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "tetrismyarcade_s29gl032n90tfi04_0001227e.bin", 0x000000, 0x400000, CRC(09b5af89) SHA1(85e506923fd803f05cc8f579f37331b608fea744) )
	ROM_IGNORE(0x100)
ROM_END


ROM_START( atgame40 )
	ROM_REGION( 0x1000000, "rom", 0 )
	ROM_LOAD16_WORD_SWAP( "40bonusgamesin1.bin", 0x000000, 0x1000000, CRC(4eba6e83) SHA1(b8edf1b6ecb70a136b551f1454ba8afa45bd8bc1) )

	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_COPY( "rom", 0x800000, 0, 0x400000 )
ROM_END


ROM_START( ra145 )
	/*
	Data for the following games is corrupt (ranges approximate, based on areas of inconsistent readout)

	3c0000 - 43ffff - James Bond 007 - The Duel   (400171 - 41c600 is corrupt)
	4c0000 - 53ffff - Robocop 3                   (507bd4 - 50ad43 is corrupt)
	540000 - 5bffff - Hokuto no Ken               (540006 - 55fff2 is corrupt)
	640000 - 6bffff - Batman                      (640042 - 65ffaf is corrupt)
	7c0000 - 7fffff - Rambo 3                     (7e031a - 7fffb9 is corrupt)

	Unfortunately as many of the games in this unit have been hacked, or are using pirate versions of the games
	from the mid 90s (in a few cases, complete with trainers) that seem to have dropped out of circulation it
	is not possible to repair the data in dump from the damaged unit.

	The unit also includes a duplicate copy of Thunder Force II instead of Revenge of Shinobi, this however
	is not a dump issue, nor is Wacky Worlds being swapped with Fun and Games in the menu
	*/

	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ra145.bin", 0x000000, 0x8000000, BAD_DUMP CRC(30583950) SHA1(855eae232e3830a505f9bc1a26edb3a7d15ce4d1) )
ROM_END



void megadriv_dgunl_state::init_dgunl3227()
{
	uint8_t* rom = memregion("rom")->base();
	uint8_t* dst = memregion("maincpu")->base();
	size_t len = memregion("rom")->bytes();

	std::vector<u8> buffer(len);

	for (int i = 0; i < len; i++)
		buffer[i] = rom[i ^ 3];

	std::copy(buffer.begin(), buffer.end(), &rom[0]);

	int baseaddr, size, dstaddr;
	//baseaddr = 0x200000; size = 0x40000; // unknown data (unused menu data maybe?)
	//baseaddr = 0x240000; size = 0x20000; // 'sample' program with UWOL header later too (lower part of menu program)
	//baseaddr = 0x260000; size = 0x20000; // pirate version of Columns with Sega text removed
	//baseaddr = 0x280000; size = 0x20000; // Fatal Labyrinth
	//baseaddr = 0x2a0000; size = 0x20000; // pirate version of Block Out with EA logo and text removed
	//baseaddr = 0x2c0000; size = 0x20000; // Flicky
	//baseaddr = 0x2e0000; size = 0x20000; // Shove It
	//baseaddr = 0x300000; size = 0x40000; // pirate version of Space Invaders 90 with Taito logos and copyright removed (also upper part of menu program - has extra header + bits of code for '202 in 1' menu which has been hacked to run the 3-in-1 menu)

	// the following 3 games are available to select from the menu on this system
	//baseaddr = 0x340000; size = 0x40000; // Pac-Attack / Pac-Panic (used by this unit)
	//baseaddr = 0x380000; size = 0x40000; // Pac-Mania (used by this unit)
	//baseaddr = 0x3c0000; size = 0x40000; // Pac-Man (used by this unit) (2nd copy of header about halfway through?)

	// copy 1st part of menu code
	baseaddr = 0x240000;
	size     = 0x020000;
	dstaddr  = 0x000000;
	for (int i = 0; i < size; i++)
	{
		dst[i + dstaddr] = rom[baseaddr + i];
	}

	// copy 2nd part of menu code
	baseaddr = 0x300000;
	size =     0x040000;
	dstaddr =  0x0c0000;
	for (int i = 0; i < size; i++)
	{
		dst[i + dstaddr] = rom[baseaddr + i];
	}

	// copy pac-panic to first bank
	baseaddr = 0x340000;
	size =     0x040000;
	dstaddr =  0x100000;
	for (int i = 0; i < size; i++)
	{
		dst[i + dstaddr] = rom[baseaddr + i];
	}

	// copy pac-mania to 2nd bank
	baseaddr = 0x380000;
	size =     0x040000;
	dstaddr =  0x180000;
	for (int i = 0; i < size; i++)
	{
		dst[i + dstaddr] = rom[baseaddr + i];
	}

	// copy pac-man to 3nd bank
	baseaddr = 0x3c0000;
	size =     0x040000;
	dstaddr =  0x200000;
	for (int i = 0; i < size; i++)
	{
		dst[i + dstaddr] = rom[baseaddr + i];
	}

	// other data isn't copied because it's never referenced, therefore we don't know how it gets accessed

	init_megadriv();
}

void megadriv_ra145_state::init_ra145()
{
	m_romsize = 0x8000000;
	init_megadriv();
}

} // anonymous namespace


// US versions show 'Genesis' on the menu,    show a www.radicagames.com splash screen, and use NTSC versions of the ROMs, sometimes region locked
// EU versions show 'Mega Drive' on the menu, show a www.radicagames.com splash screen, and use PAL versions of the ROMs, sometimes region locked
// UK versions show "Mega Drive' on the menu, show a www.radicauk.com splash screen,    and use PAL versions of the ROMs, sometimes region locked

CONS( 2004, rad_gen1,  0,        0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Radica / Sega",                     "Genesis Collection Volume 1 (Radica, Arcade Legends) (USA)", 0)
CONS( 2004, rad_md1,   rad_gen1, 0, megadriv_radica_3button_pal,  radica_3button_1player, megadriv_radica_state, init_megadrie, "Radica / Sega",                     "Mega Drive Collection Volume 1 (Radica, Arcade Legends) (Europe)", 0)
CONS( 2004, rad_md1uk, rad_gen1, 0, megadriv_radica_3button_pal,  radica_3button_1player, megadriv_radica_state, init_megadrie, "Radica / Sega",                     "Mega Drive Collection Volume 1 (Radica, Arcade Legends) (UK)", 0)
CONS( 2004, mdtvp1j,   rad_gen1, 0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Sega Toys",                         "Mega Drive Play TV 1 (Japan)", 0) // expects US region despite being a Japanese unit (Bean Machine is region locked)

CONS( 2004, rad_gen2,  0,        0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Radica / Sega",                     "Genesis Collection Volume 2 (Radica, Arcade Legends) (USA)", 0)
CONS( 2004, rad_md2uk, rad_gen2, 0, megadriv_radica_3button_pal,  radica_3button_1player, megadriv_radica_state, init_megadrie, "Radica / Sega",                     "Mega Drive Collection Volume 2 (Radica, Arcade Legends) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?
CONS( 2004, mdtvp2j,   rad_gen2, 0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Sega Toys",                         "Mega Drive Play TV 2 (Japan)", 0)

// box calls this Volume 3
CONS( 2004, rad_sonic,  0,        0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Radica / Sega",                     "Super Sonic Gold (Radica Plug & Play) (USA)", 0)
CONS( 2004, rad_sonicuk,rad_sonic,0, megadriv_radica_3button_pal,  radica_3button_1player, megadriv_radica_state, init_megadrie, "Radica / Sega",                     "Super Sonic Gold (Radica Plug & Play) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?

CONS( 2004, rad_sf2,   0,        0, megadriv_radica_6button_ntsc, radica_6button,         megadriv_radica_state, init_megadriv, "Radica / Capcom / Sega",            "Street Fighter II: Special Champion Edition [Ghouls'n Ghosts] (Radica, Arcade Legends) (USA)", 0)
CONS( 2004, rad_sf2uk, rad_sf2,  0, megadriv_radica_6button_pal,  radica_6button,         megadriv_radica_state, init_megadrie, "Radica / Capcom / Sega",            "Street Fighter II: Special Champion Edition [Ghouls'n Ghosts] (Radica, Arcade Legends) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?
CONS( 2004, mdtvp3j,   rad_sf2,  0, megadriv_radica_6button_ntsc, radica_6button,         megadriv_radica_state, init_megadriv, "Sega Toys",                         "Mega Drive Play TV 3 (Japan)", 0) // This one does contain the Japanese ROM for SF2 (but the World release of GnG) so SF2 runs in Japanese, but GnG runs in English

// still branded as Arcade Legends even if none of these were ever arcade games, European exclusive
CONS( 2004, rad_ssoc,  0,        0, megadriv_radica_3button_pal,  radica_3button,         megadriv_radica_state, init_megadrie, "Radica / Sensible Software / Sega", "Sensible Soccer plus [Cannon Fodder, Mega lo Mania] (Radica, Arcade Legends) (UK)", 0)
// is there a Europe version with Radica Games boot screen and Mega Drive text?

// not region locked, no Radica logos, uncertain if other regions would differ
CONS( 2004, rad_orun,  0,        0, megadriv_radica_3button_pal,  radica_3button_1player, megadriv_radica_state, init_megadrie, "Radica / Sega",                     "Out Run 2019 (Radica Plug & Play, UK)", 0)

// this has been verified as identical to the 6-in-1 cartridge that came with the Menacer gun for the MD
CONS( 2004, rad_mncr,  0,        0, megadriv_radica_3button_ntsc, radica_3button_1player, megadriv_radica_state, init_megadriv, "Radica / Sega",                     "Menacer (Radica Plug & Play)", MACHINE_NOT_WORKING )


// From a European unit but NTSC? - code is hacked from original USA Genesis game with region check still intact? (does the clone hardware always identify as such? or does the bypassed boot code skip the check?)
// TODO: move out of here eventually once the enhanced MD part is emulated rather than bypassed (it's probably the same as the 145-in-1 multigame unit, but modified to only include this single game)
CONS( 2018, msi_sf2,   0,        0, megadriv_radica_6button_ntsc, msi_6button,         megadriv_radica_state, init_megadriv,    "MSI / Capcom / Sega",            "Street Fighter II: Special Champion Edition (MSI Plug & Play) (Europe)", 0)

// Are these (dgunl3227, ra145) actually emulation based? there is a block of 0x40000 bytes at the start of the ROM that doesn't
// appear to be used, very similar in both units.  Banking also seems entirely illogical unless something else is managing it.
// The menu code in both seems to have the same origin, containing a bunch of unused pirate versions of MD games.
// The version of SF2 in the 'ra145' unit is the same as the one in the MSI unit above, and expects region to report US even
// when some of the units run at PAL speed?
// It is also confirmed from real hardware videos that these units do not have the usual sprite limits (so masking effect on Sonic title screen fails)

// this is the only 'Pocket Player' unit to use Genesis on a Chip tech, the others are NES on a chip.
// the parent set has updated software explaining how to insert coins in Pac-Man as well as an updated copyright string
CONS( 2018, dgunl3227,  0,        0, megadriv_dgunl_ntsc, dgunl_1player,         megadriv_dgunl_state, init_dgunl3227,    "dreamGEAR",            "My Arcade Pac-Man Pocket Player (DGUNL-3227)", 0 )
CONS( 2018, dgunl3227a, dgunl3227,0, megadriv_dgunl_ntsc, dgunl_1player,         megadriv_dgunl_state, init_dgunl3227,    "dreamGEAR",            "My Arcade Pac-Man Pocket Player (DGUNL-3227, older)", 0 )

CONS( 2021, matet,      0,        0, megadriv_radica_3button_ntsc,  radica_3button, megadriv_radica_state, init_megadriv, "dreamGEAR",            "My Arcade Tetris (DGUNL-7028, Pocket Player Pro)", MACHINE_NOT_WORKING)

CONS( 2018, ra145,     0,        0, megadriv_ra145_ntsc, msi_6button,           megadriv_ra145_state, init_ra145,        "<unknown>",            "Retro Arcade 16 Bits Classic Edition Mini TV Game Console - 145 Classic Games - TV Arcade Plug and Play (Mega Drive bootlegs)", MACHINE_NOT_WORKING )

// Technically this is a MD type cartridge, but it doesn't seem to be designed for use with a standard MD as it contains
// nothing but the 16Mbyte ROM and a 5v to 3.3v converter yet the code clearly requires some extensive banking logic.
// Testing it on a real MD shows nothing, not even the menu.
//
// We don't seem to emulate the system it's designed for, so for now just treat it as its own thing (which may become
// the basis of a driver for that console)
CONS( 2012, atgame40,  0,        0, megadriv_radica_3button_pal,  radica_3button, megadriv_radica_state, init_megadrie, "AtGames",               "40 Bonus Games in 1 (AtGames)", MACHINE_NOT_WORKING)
