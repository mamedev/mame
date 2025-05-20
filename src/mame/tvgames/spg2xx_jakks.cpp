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
	void spg2xx_dpma(machine_config& config);
	void jakks_mpaco(machine_config& config);
	void jakks_mpac(machine_config& config);
	void jakks_rapm(machine_config& config);

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
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
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
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_spd3 )
	PORT_INCLUDE( spg2xx_jakks )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("C Button")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Menu / Pause")
INPUT_PORTS_END

static INPUT_PORTS_START( jak_supm )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x01ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_spdv )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_dpma )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_UNUSED )
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

static INPUT_PORTS_START( jak_mpaco )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DIALX") // for Pole Position, joystick can be twisted like a dial/wheel (limited?) (check range) (note, range is different to GKR unit)
	PORT_BIT(0x03ff, 0x0000, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x03ff)
INPUT_PORTS_END

static INPUT_PORTS_START( jak_mpac )
	PORT_INCLUDE( jak_mpaco )

	PORT_MODIFY("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
INPUT_PORTS_END

static INPUT_PORTS_START( jak_spac )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( jak_rapm )
	PORT_INCLUDE( jak_spac )

	PORT_START("DIALX") // for Pole Position, joystick can be twisted like a dial/wheel (limited?) (check range)
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_pacg )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void jakks_state::base_config(machine_config& config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(jakks_state::base_porta_r));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_state::portc_w));

	I2C_24C04(config, m_i2cmem, 0);
}

void jakks_state::spg2xx_jakks(machine_config &config)
{
	base_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_2m);
}

void jakks_state::spg2xx_dpma(machine_config& config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(jakks_state::base_porta_r));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_state::portc_w));
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

void jakks_state::jakks_mpaco(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(jakks_state::base_porta_r));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_state::portc_w));

	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_1m);
	m_maincpu->adc_in<0>().set_ioport("DIALX");
}


void jakks_state::jakks_mpac(machine_config &config)
{
	jakks_mpaco(config);
	I2C_24C04(config, m_i2cmem, 0); 
	m_maincpu->adc_in<0>().set_ioport("DIALX");
}

void jakks_state::jakks_rapm(machine_config &config)
{
	spg2xx_jakks(config);
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

ROM_START( jak_potc )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "pirates.u5", 0x000000, 0x400000, CRC(935fe66c) SHA1(8b5b11c61b7f32c313aa46e33a1c918ed82f7916) )
ROM_END

ROM_START( jak_cind )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkscinderella.u5", 0x000000, 0x200000, CRC(73008a51) SHA1(89168bc2c64836daa341a6fbacb1fa63c2fef14b) )
ROM_END

ROM_START( jak_slpb )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks_sleepingbeauty.u5", 0x000000, 0x200000, CRC(e5b20a73) SHA1(3c305c4b9265d9bbf090805daaf26ad43af54389) )
ROM_END

ROM_START( jak_supm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "superman.u3", 0x000000, 0x400000, CRC(626bdd85) SHA1(605b3193c17f606d2de5689f045b50ac0b7ff024) )
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

ROM_START( jak_mpacq )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks_qvc7in1.u2", 0x000000, 0x200000, CRC(a0a132a0) SHA1(acb5a98dfb597151015929bdd5b6eada1764e2fe) )
ROM_END

ROM_START( jak_mpaco )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks_mspacmanrev1.u1", 0x000000, 0x100000, CRC(de4c2262) SHA1(2a5bacdcd10ccfdb1ae792d0aaaf2d38ec936f94) )
ROM_END

ROM_START( jak_rapm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks_retroarcade.u3", 0x000000, 0x400000, CRC(f2dcb1c8) SHA1(2361b8598279bc6642ca997bbf2072a9e6ac045e) )
ROM_END

ROM_START( jak_pacg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkspacmangold.u2", 0x000000, 0x200000, CRC(59904862) SHA1(f86a8847742fa918bdfa0fa927ec6e1f573ce31c) )
ROM_END

ROM_START( jak_spac ) // this is actually built as a gamekey, with the header text in it, sources suggest it was also going to be released as one
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkssuperpacman.u2", 0x000000, 0x100000, CRC(490db7cc) SHA1(9b599f28b9fa8e4a502ae7a167c410d4738b8e7a) )
ROM_END

ROM_START( jak_spd3 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "spiderman3.u4", 0x000000, 0x200000, CRC(87019271) SHA1(80d126af970236a1cecf7ade49f916caf8f67ceb) )
ROM_END

ROM_START( jak_powr ) // shows Game-Key screen but was never produced with a GK slot
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "powerrangers.u2", 0x000000, 0x200000, CRC(859c6cff) SHA1(12bb08657e333c2644d707deead8cd3e34a140b2) )
ROM_END

ROM_START( jak_pix )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkspixar.bin", 0x000000, 0x200000, CRC(ec110f2b) SHA1(a57e1d45cfb537173f94d1a95323183a62976bb4) )
ROM_END

ROM_START( jak_shrk )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkshrek.u2", 0x000000, 0x400000, CRC(4abb910d) SHA1(e663abd2ba6eacca2f4a046d8b8ebac2cf3fd36a) )
ROM_END

ROM_START( jak_marv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakmarvelheroes.u4", 0x000000, 0x200000, CRC(e63259a3) SHA1(8745f3071ba460c8ba4a5bd376a72154b2edd8bd) )
ROM_END

ROM_START( jak_spdv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkspiderweb.u2", 0x000000, 0x200000, CRC(408c94bc) SHA1(350f7b84abf3d0d56f081647b3a228505751ff70) )
ROM_END

ROM_START( jak_dpma )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks_magicaladventures.u2", 0x000000, 0x200000, CRC(3c3fdf54) SHA1(9847412a0ee21819cc714ca1a2dd519edb892c95) )
ROM_END

ROM_START( jak_hsm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkshighschoolmusical.u3", 0x000000, 0x400000, CRC(ad83549a) SHA1(1d198209055e418402c9f3f389bccde156ca5a43) )
ROM_END


} // anonymous namespace


// Pre-GameKey units

CONS( 2004, jak_batm, 0, 0, spg2xx_jakks,  batman,        jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "The Batman (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// you could link 2 pads of this together for 2 player mode as you could with WWE (feature not emulated)
CONS( 2004, jak_mk,   0, 0, mk,     mk,     jakks_state, empty_init, "JAKKS Pacific Inc / Digital Eclipse", "Mortal Kombat (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// this is an older unit than the jak_mpac Game Key Ready set and features no GameKey branding
CONS( 2004, jak_mpacw, 0,         0, jakks_mpac, jak_mpac,   jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd",      "Ms. Pac-Man Collection 7-in-1 (JAKKS Pacific TV Game) (wireless, 18 AUG 2004 A)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2004, jak_mpacq, jak_mpacw, 0, jakks_mpac, jak_mpac,   jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd",      "Ms. Pac-Man Collection 7-in-1 (JAKKS Pacific TV Game) (QVC version, 12 JUL 2004 A)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2004, jak_mpaco, jak_mpac,  0, jakks_mpaco, jak_mpaco,   jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd",      "Ms. Pac-Man Collection 5-in-1 (JAKKS Pacific TV Game) (01 APR 2004 A)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

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
CONS( 2007, jak_sbjd, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "SpongeBob SquarePants Jellyfish Dodge (JAKKS Pacific TV Game) (Apr 5 2007)", MACHINE_IMPERFECT_SOUND )

CONS( 2008, jak_wall, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "Wall-E (JAKKS Pacific TV Game) (Dec 18 2007 11:34:25)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2007, jak_potc, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "Pirates of the Caribbean - Islands of Fortune (JAKKS Pacific TV Game) (Jun 1 2007 12:34:28)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_supm, 0, 0, spg2xx_jakks,  jak_supm,      jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "Superman in Super Villain Showdown (JAKKS Pacific TV Game) (26 Jan 2006 A)", MACHINE_IMPERFECT_SOUND ) // has AT24C04

CONS( 2006, jak_spdv, 0, 0, spg2xx_jakks,  spg2xx_spdv,   jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "The Amazing Spider-Man in Villain Round-Up (JAKKS Pacific TV Game) (24 Apr 2006 A)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_pacg, 0, 0, spg2xx_jakks,  spg2xx_pacg,   jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd", "Arcade Gold featuring Pac-Man (20 APR 2007 A SKU O)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_spac, 0, 0, spg2xx_jakks,  jak_mpac,      jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd", "Super Pac-Man Collection (26 JAN 2006 A SKU L)", MACHINE_IMPERFECT_SOUND )

CONS( 2008, jak_rapm, 0, 0, jakks_rapm,    jak_rapm,      jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd", "Retro Arcade featuring Pac-Man (20 AUG 2008 A SKU N)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_spd3, 0, 0, spg2xx_jakks,  jak_spd3,      jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Spider-Man 3 (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_cind, 0, 0, spg2xx_jakks,  spg2xx_pacg,   jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Cinderella - Once Upon a Midnight (JAKKS Pacific TV Game) (Aug 29 2007 11:15:55)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_slpb, 0, 0, spg2xx_jakks,  spg2xx_pacg,   jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Sleeping Beauty - Tales of Enchantment (JAKKS Pacific TV Game) (Sep 17 2007 14:45:02)", MACHINE_IMPERFECT_SOUND )

CONS( 2005, jak_powr, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Power Rangers S.P.D. (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_pix,  0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Disney Pixar Classics (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_hsm,  0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "High School Musical (JAKKS Pacific TV Game) (Dec 19 2007 17:08:20)", MACHINE_IMPERFECT_SOUND )

// menu sounds don't work until you go into a game (work after a reset, bad default initializations?)
CONS( 2006, jak_shrk , 0, 0, spg2xx_jakks, spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Dreamworks Shrek / Over The Hedge (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_marv , 0, 0, spg2xx_jakks, spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Marvel Heroes: Ultimate Action (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_dpma,  0, 0, spg2xx_dpma,  spg2xx_dpma,   jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Disney Princess Magical Adventures (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )
