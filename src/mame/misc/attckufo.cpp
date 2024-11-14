// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/***************************************************************************

    "Attack UFO" (c) 1980 Ryoto Electric Co.

    TODO:
    - Verify I/O chip
    - Is game speed ok? A bit slow sometimes

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "sound/mos6560.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class attckufo_state : public driver_device
{
public:
	attckufo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{}

	void attckufo(machine_config &config);

private:
	uint8_t vic_videoram_r(offs_t offset);
	uint8_t vic_colorram_r(offs_t offset);

	void cpu_map(address_map &map) ATTR_COLD;
	void vic_colorram_map(address_map &map) ATTR_COLD;
	void vic_videoram_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void attckufo_state::cpu_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x100f).rw("mos6560", FUNC(mos6560_device::read), FUNC(mos6560_device::write));
	map(0x1400, 0x1403).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1c00, 0x1fff).ram();
	map(0x2000, 0x3fff).rom().region("maincpu", 0);
}

void attckufo_state::vic_videoram_map(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(attckufo_state::vic_videoram_r));
}

void attckufo_state::vic_colorram_map(address_map &map)
{
	map(0x000, 0x3ff).r(FUNC(attckufo_state::vic_colorram_r));
}


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( attckufo )
	PORT_START("DSW")
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(   0x00, "3")
	PORT_DIPSETTING(   0x01, "4")
	PORT_DIPSETTING(   0x02, "5")
	PORT_DIPSETTING(   0x03, "6")
	PORT_DIPNAME(0x04, 0x04, DEF_STR( Bonus_Life ))
	PORT_DIPSETTING(   0x04, "1000")
	PORT_DIPSETTING(   0x00, "1500")
	PORT_DIPUNUSED(0x08, IP_ACTIVE_LOW)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_COIN1)

	PORT_START("INPUT")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1) PORT_2WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_2WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2) PORT_2WAY
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_2WAY
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2)
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint8_t attckufo_state::vic_videoram_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

uint8_t attckufo_state::vic_colorram_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset + 0x400);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void attckufo_state::attckufo(machine_config &config)
{
	M6502(config, m_maincpu, XTAL(14'318'181) / 14);
	m_maincpu->set_addrmap(AS_PROGRAM, &attckufo_state::cpu_map);

	pia6821_device &pia(PIA6821(config, "pia"));
	pia.readpa_handler().set_ioport("DSW");
	pia.readpb_handler().set_ioport("INPUT");

	SPEAKER(config, "mono").front_center();

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(MOS6560_VRETRACERATE);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size((MOS6560_XSIZE + 7) & ~7, MOS6560_YSIZE);
	screen.set_visarea(0, 23*8 - 1, 0, 22*8 - 1);
	screen.set_screen_update("mos6560", FUNC(mos6560_device::screen_update));

	mos6560_device &mos6560(MOS656X_ATTACK_UFO(config, "mos6560", XTAL(14'318'181) / 14));
	mos6560.set_screen("screen");
	mos6560.set_addrmap(0, &attckufo_state::vic_videoram_map);
	mos6560.set_addrmap(1, &attckufo_state::vic_colorram_map);
	mos6560.add_route(ALL_OUTPUTS, "mono", 0.25);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( attckufo )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("1", 0x0000, 0x0400, CRC(b32a36ab) SHA1(4e64686d498f7a79f5213c42b7afbf35aac2b622))
	ROM_LOAD("2", 0x0400, 0x0400, CRC(35fc8424) SHA1(d4926768f2e5b21476c7ec33743fe3e1c76662db))
	ROM_LOAD("3", 0x0800, 0x0400, CRC(6341c8c4) SHA1(8647a4fabad0399769dd068d784be72e27afca35))
	ROM_LOAD("4", 0x0c00, 0x0400, CRC(1ed1d93f) SHA1(2e04c63ea2fc958415f7c7de9d18dadf9e085755))
	ROM_LOAD("5", 0x1000, 0x0400, CRC(3380e0f4) SHA1(e1d681f7370ba7fc2bf3561533f4aaf12eefbcb8))
	ROM_LOAD("6", 0x1400, 0x0400, CRC(8103e031) SHA1(86bc8dd6c74b84804ede31a8454b5b3d3e4d88b1))
	ROM_LOAD("7", 0x1800, 0x0400, CRC(43a41012) SHA1(edd14f49dc9ae7a5a14583b9a92ebbbdd021d7b1))
	ROM_LOAD("8", 0x1c00, 0x0400, CRC(9ce93eb0) SHA1(68753e88db4e920446b9582b5cb713b1beec3b27))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROTATION  COMPANY               FULLNAME      FLAGS
GAME( 1980, attckufo, 0,      attckufo, attckufo, attckufo_state, empty_init, ROT270,   "Ryoto Electric Co.", "Attack UFO", MACHINE_SUPPORTS_SAVE )
