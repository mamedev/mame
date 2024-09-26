// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

// Mortal Kombat - attract demo almost always picked Johnny Cage vs. Johhny Cage until some unknown factor starts to randomize it
//               - Scorpion's 'get over here' sounds don't decode well

#include "emu.h"
#include "spg2xx.h"
#include "machine/nvram.h"


namespace {

class jakks_state : public spg2xx_game_state
{
public:
	jakks_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void base_config(machine_config& config);
	void spg2xx_jakks(machine_config& config);
	void mk(machine_config& config);

	void jakks_mpac(machine_config& config);

private:
	void mem_map_2m_mkram(address_map &map) ATTR_COLD;
	void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};


void jakks_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(mem_mask, 1))
		m_i2cmem->write_scl(BIT(data, 1));
	if (BIT(mem_mask, 0))
		m_i2cmem->write_sda(BIT(data, 0));
}

static INPUT_PORTS_START( batman )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Menu")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("X Button")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_jakks )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED ) // Button 3 if used
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED ) // Button 4 if used
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mk )
	PORT_START("P1")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON7 )        PORT_PLAYER(1) PORT_NAME("Pause / Menu")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON6 )        PORT_PLAYER(1) PORT_NAME("Block (alt)") // which one of these is actually connected to the button?
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON5 )        PORT_PLAYER(1) PORT_NAME("Block")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("High Kick")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("High Punch")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Low Kick")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("Low Punch")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")

	PORT_START("P3") // In addition to the "M/T" pad documented below, PCB also has "P/N" (PAL / NTSC) pad (not read?) and a "F/S" pad (also not read?)
	PORT_BIT( 0x0fff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_CONFNAME( 0x1000, 0x1000, "Blood" ) // see code at 05EC30 - "M/T" (Mature / Teen?) pad on PCB, set at factory
	PORT_CONFSETTING(      0x0000, "Disabled" )
	PORT_CONFSETTING(      0x1000, "Enabled" )
	PORT_BIT( 0x6000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_CONFNAME( 0x8000, 0x8000, "Link State" ) // see code at 05EA54
	PORT_CONFSETTING(      0x8000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_mpac )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DIALX") // for Pole Position, joystick can be twisted like a dial/wheel (limited?) (check range) (note, range is different to GKR unit)
	PORT_BIT(0x03ff, 0x0000, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x03ff)
INPUT_PORTS_END

void jakks_state::base_config(machine_config& config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(jakks_state::base_porta_r));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_state::portc_w));

	I2C_24C04(config, m_i2cmem, 0); // ?
}

void jakks_state::spg2xx_jakks(machine_config &config)
{
	base_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_2m);
}

void jakks_state::mem_map_2m_mkram(address_map &map)
{
	map(0x000000, 0x1fffff).bankr("cartbank");
	map(0x3e0000, 0x3fffff).ram().share("nvram"); // backed up by the CR2032
}

void jakks_state::mk(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);

	spg2xx_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_2m_mkram);

	m_maincpu->porta_in().set_ioport("P1");
	//m_maincpu->portb_in().set(FUNC(jakks_state::base_portb_r));
	m_maincpu->portc_in().set_ioport("P3");

	NVRAM(config, "nvram", nvram_device::DEFAULT_RANDOM);
}

void jakks_state::jakks_mpac(machine_config &config)
{
	base_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_1m);

	m_maincpu->adc_in<0>().set_ioport("DIALX");
}


ROM_START( jak_batm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "batman.bin", 0x000000, 0x400000, CRC(46f848e5) SHA1(5875d57bb3fe0cac5d20e626e4f82a0e5f9bb94c) )
ROM_END

ROM_START( jak_wall )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "walle.bin", 0x000000, 0x400000, BAD_DUMP CRC(bd554cba) SHA1(6cd06a036ab12e7b0e1fd8003db873b0bb783868) )
	// both of these dumps are bad, but in slightly different ways, note the random green pixels around the text (bad data is reported in secret test mode)
	//ROM_LOAD16_WORD_SWAP( "walle.bin", 0x000000, 0x400000, BAD_DUMP CRC(6bc90b16) SHA1(184d72de059057aae7800da510fcf05ed1da9ec9))
ROM_END

ROM_START( jak_sbjd )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "spongebobjelly.bin", 0x000000, 0x400000, CRC(804fbd87) SHA1(519aa7fada993837cb57fce26a1d721547af1861) )
ROM_END

ROM_START( jak_mk )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	// Sources indicate this should use a 6MB ROM.  The ROM here dosen't end on a blank fill and even the ROM checksum listed in the header seems to be about 50% off.
	// However no content actually seems to be missing, so are sources claiming a 6MB ROM just incorrect, with the checksum also being misleading?
	ROM_LOAD16_WORD_SWAP( "jakmk.bin", 0x000000, 0x400000, CRC(b7d7683e) SHA1(e54a020ee746d240267ef78bed7aea744351b421) )
ROM_END

ROM_START( jak_mpacw )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wirelessnamco.bin", 0x000000, 0x200000, CRC(78a318ca) SHA1(3c2601cbb023edb6a1f3d4bce686e0be1ef63eee) )
ROM_END

} // anonymous namespace


// Pre-GameKey units

CONS( 2004, jak_batm, 0, 0, spg2xx_jakks,  batman,        jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "The Batman (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// you could link 2 pads of this together for 2 player mode as you could with WWE (feature not emulated)
CONS( 2004, jak_mk,   0, 0, mk,     mk,     jakks_state, empty_init, "JAKKS Pacific Inc / Digital Eclipse", "Mortal Kombat (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// this is an older unit than the jak_mpac Game Key Ready set and features no GameKey branding
CONS( 2004, jak_mpacw,0, 0, jakks_mpac, jak_mpac,   jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd",      "Ms. Pac-Man 7-in-1 (Wireless) (Ms. Pac-Man, Pole Position, Galaga, Xevious, Mappy, New Rally X, Bosconian) (18 AUG 2004 A)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NM (3 keys available [Dig Dug, New Rally-X], [Rally-X, Pac-Man, Bosconian], [Pac-Man, Bosconian])


// Post-GameKey units (all of these still have GameKey references in the code even if the physical connector was no longer present on the PCB)

// This was available in 2 different case styles, initially an underwater / jellyfish themed one, then later
// reissued in a 'SpongeBob head' style case reminiscent of the undumpable 2003 SpongeBob plug and play but
// with 2 buttons in the top left corner instead of 1
//
// The software on both versions of Jellyfish Dodge is believed to be the same, the build date can be seen in
// the 'hidden' test mode.
//
// A further updated version of this, adapted for touch controls, was released as a 'TV Touch' unit, see
// spg2xx_jakks_tvtouch.cpp
CONS( 2007, jak_sbjd, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "SpongeBob SquarePants Jellyfish Dodge (JAKKS Pacific TV Game) (Apr 5 2007)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2008, jak_wall, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "Wall-E (JAKKS Pacific TV Game) (Dec 18 2007 11:34:25)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
