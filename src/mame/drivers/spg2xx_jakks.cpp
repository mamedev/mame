// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

#include "emu.h"
#include "includes/spg2xx.h"

class jakks_state : public spg2xx_game_state
{
public:
	jakks_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void base_config(machine_config& config);
	void batman(machine_config &config);
	void walle(machine_config& config);

private:
	DECLARE_WRITE16_MEMBER(portc_w) override;
};

WRITE16_MEMBER(jakks_state::portc_w)
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

	PORT_START("P3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( walle )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
	PORT_BIT( 0xfff6, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
INPUT_PORTS_END

void jakks_state::base_config(machine_config& config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(jakks_state::base_porta_r));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_state::portc_w));

	I2CMEM(config, m_i2cmem, 0).set_data_size(0x200);
}

void jakks_state::batman(machine_config &config)
{
	base_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_4m);
}

void jakks_state::walle(machine_config &config)
{
	base_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_2m);
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

// JAKKS Pacific Inc TV games
CONS( 2004, jak_batm, 0, 0, batman, batman, jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd", "The Batman (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2008, jak_wall, 0, 0, walle,  walle,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd", "Wall-E (JAKKS Pacific TV Game)",     MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
