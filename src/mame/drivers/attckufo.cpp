// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/***************************************
Attack ufo / Ryoto Electric Co. 1980(?)
 driver  by Tomasz Slanina

Video and sound emulation based on
 VIC 656x driver by PeT

TODO:
 - correct I/O chip emulation
 - is game speed ok? a bit slow sometimes

--

CPU - 6502
  2,38 +5
  21 gnd
  26-33 data bus

I/O - CIA 6526 ?
  37,38 +5
  1 gnd
  2-18 I/O
  26-33 data bus

Vid  - VIC 6560 derivative ???
  40 +5
  20 gnd
  16-32 data bus

--

Attack UFO
???? 1980
LOIPOIO-B

2  1   2114 2114 2114 2114 2114
8  7  6  5   4  3               40pin IC "C"

53354    40pin IC "B" 14.318MHz 40pin IC "A"
                                SW

****************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/mos6560.h"


class attckufo_state : public driver_device
{
public:
	attckufo_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_mos6560(*this, "mos6560") ,
		m_mainram(*this, "mainram"),
		m_tileram(*this, "tileram"){ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<mos6560_device> m_mos6560;

	/* memory pointers */
	required_shared_ptr<UINT8> m_mainram;
	required_shared_ptr<UINT8> m_tileram;

	DECLARE_READ8_MEMBER(attckufo_io_r);
	DECLARE_WRITE8_MEMBER(attckufo_io_w);

	DECLARE_READ8_MEMBER( vic_videoram_r );
	DECLARE_READ8_MEMBER( vic_colorram_r );
};

READ8_MEMBER(attckufo_state::attckufo_io_r)
{
	switch(offset)
	{
		case 0: return ioport("DSW")->read();
		case 2: return ioport("INPUT")->read();
	}
	return 0xff;
}


WRITE8_MEMBER(attckufo_state::attckufo_io_w)
{
	/*
	offset, data:
	 0   $00,$30
	 1   $00,$04
	 2   $00
	 3   $00,$04
	*/
}

READ8_MEMBER(attckufo_state::vic_videoram_r)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

READ8_MEMBER(attckufo_state::vic_colorram_r)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset + 0x400);
}

static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 8, attckufo_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x1000, 0x100f) AM_DEVREADWRITE("mos6560", mos6560_device, read, write)
	AM_RANGE(0x1400, 0x1403) AM_READWRITE(attckufo_io_r, attckufo_io_w)
	AM_RANGE(0x1c00, 0x1fff) AM_RAM AM_SHARE("tileram")
	AM_RANGE(0x2000, 0x3fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( vic_videoram_map, AS_0, 8, attckufo_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(vic_videoram_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vic_colorram_map, AS_1, 8, attckufo_state )
	AM_RANGE(0x000, 0x3ff) AM_READ(vic_colorram_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( attckufo )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "1000" )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INPUT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
INPUT_PORTS_END

static MACHINE_CONFIG_START( attckufo, attckufo_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_14_31818MHz/14)
	MCFG_CPU_PROGRAM_MAP(cpu_map)

	/* video/sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_MOS656X_ATTACK_UFO_ADD("mos6560", "screen", XTAL_14_31818MHz/14, vic_videoram_map, vic_colorram_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

ROM_START( attckufo )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "1", 0x2000, 0x0400, CRC(b32a36ab) SHA1(4e64686d498f7a79f5213c42b7afbf35aac2b622) )
	ROM_LOAD( "2", 0x2400, 0x0400, CRC(35fc8424) SHA1(d4926768f2e5b21476c7ec33743fe3e1c76662db) )
	ROM_LOAD( "3", 0x2800, 0x0400, CRC(6341c8c4) SHA1(8647a4fabad0399769dd068d784be72e27afca35) )
	ROM_LOAD( "4", 0x2c00, 0x0400, CRC(1ed1d93f) SHA1(2e04c63ea2fc958415f7c7de9d18dadf9e085755) )
	ROM_LOAD( "5", 0x3000, 0x0400, CRC(3380e0f4) SHA1(e1d681f7370ba7fc2bf3561533f4aaf12eefbcb8) )
	ROM_LOAD( "6", 0x3400, 0x0400, CRC(8103e031) SHA1(86bc8dd6c74b84804ede31a8454b5b3d3e4d88b1) )
	ROM_LOAD( "7", 0x3800, 0x0400, CRC(43a41012) SHA1(edd14f49dc9ae7a5a14583b9a92ebbbdd021d7b1) )
	ROM_LOAD( "8", 0x3c00, 0x0400, CRC(9ce93eb0) SHA1(68753e88db4e920446b9582b5cb713b1beec3b27) )
ROM_END

GAME( 1980, attckufo, 0,      attckufo, attckufo, driver_device, 0, ROT270, "Ryoto Electric Co.", "Attack Ufo", MACHINE_SUPPORTS_SAVE )
