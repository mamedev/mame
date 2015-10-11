// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Taito Crazy Balloon hardware

    To-Do:
        * Video timing from schematics
        * Watchdog length from schematics
        * Interrupt timing from schematics
        * DIP switches
        * Faithfully implement the custom chips
        * Coin counter and lock-out

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/crbaloon.h"


/*************************************
 *
 *  PC3092 custom chip
 *
 *  Inputs:  pins 1-4   - D0-D3 of data bus
 *           pin 5      - 1P ENTRY (start button 1)
 *           pin 6      - 2P ENTRY (start button 2)
 *           pin 7      - COIN
 *           pin 8      - /4HN (pixel timing)
 *           pin 9      - port 0x0a written to
 *           pin 10     - port 0x08 written to
 *           pin 11     - port 0x07 written to
 *           pin 13     - /RESET
 *           pin 14     - port 0x09 written to
 *           pin 15     - port 0x0b written to
 *
 *  Outputs: pins 16-19 - D28-D31 of I/O bus
 *           pin 20     - /V-INV (flip screen)
 *           pin 21     - coin counter
 *
 *************************************/

#define LOG_PC3092      0


void crbaloon_state::pc3092_reset(void)
{
	/* nothing yet */
}


void crbaloon_state::pc3092_update()
{
	flip_screen_set((m_pc3092_data[1] & 0x01) ? TRUE : FALSE);
}


WRITE8_MEMBER(crbaloon_state::pc3092_w)
{
	m_pc3092_data[offset] = data & 0x0f;

	if (LOG_PC3092) logerror("%04X:  write PC3092 #%d = 0x%02x\n", space.device().safe_pc(), offset, m_pc3092_data[offset]);

	pc3092_update();
}


CUSTOM_INPUT_MEMBER(crbaloon_state::pc3092_r)
{
	UINT32 ret;

	/* enable coin & start input? Wild guess!!! */
	if (m_pc3092_data[1] & 0x02)
		ret = ioport("PC3092")->read();
	else
		ret = 0x00;

	if (LOG_PC3092) logerror("%s:  read  PC3092 = 0x%02x\n", machine().describe_context(), ret);

	return ret;
}



/*************************************
 *
 *  PC3259 custom chip -
 *  collision detection
 *
 *  Inputs:  pins 1-10  - 128V/64V/32V/16V/8V
 *                        128H/64H/32H/16H/8H
 *                        video timing lines
 *           pin 11     - CLK
 *           pin 13     - /HTCLR
 *           pin 14     - MVID
 *           pin 15     - T1
 *           pin 16     - D7 of SOUND port buffer
 *           pin 21/22  - AD2/AD3 of address bus
 *           pin 23     - MSK (port 0x0c written to)
 *
 *  Outputs: pins 17-20 - D24-D27 of I/O bus
 *
 *************************************/

#define LOG_PC3259      0


void crbaloon_state::pc3259_update(void)
{
	/* nothing yet */
}


READ8_MEMBER(crbaloon_state::pc3259_r)
{
	UINT8 ret = 0;
	UINT8 reg = offset >> 2;

	UINT16 collision_address = crbaloon_get_collision_address();
	int collided = (collision_address != 0xffff);

	switch (reg)
	{
	case 0x00:
		ret = collided ? (collision_address & 0x0f) : 0;
		break;

	case 0x01:
		ret = collided ? ((collision_address >> 4) & 0x0f) : 0;
		break;

	case 0x02:;
		ret = collided ? (collision_address >> 8) : 0;
		break;

	default:
	case 0x03:
		ret = collided ? 0x08 : 0x07;
		break;
	}

	if (LOG_PC3259) logerror("%04X:  read PC3259 #%d = 0x%02x\n", space.device().safe_pc(), reg, ret);

	return ret | (ioport("DSW1")->read() & 0xf0);
}



/*************************************
 *
 *  I/O ports
 *
 *************************************/

WRITE8_MEMBER(crbaloon_state::port_sound_w)
{
	/* D0 - interrupt enable - also goes to PC3259 as /HTCTRL */
	m_irq_mask = data & 0x01;
	crbaloon_set_clear_collision_address((data & 0x01) ? TRUE : FALSE);

	/* D1 - SOUND STOP */
	machine().sound().system_enable((data & 0x02) ? TRUE : FALSE);

	/* D2 - unlabeled - music enable */
	crbaloon_audio_set_music_enable(space, 0, (data & 0x04) ? TRUE : FALSE);

	/* D3 - EXPLOSION */
	crbaloon_audio_set_explosion_enable((data & 0x08) ? TRUE : FALSE);

	/* D4 - BREATH */
	crbaloon_audio_set_breath_enable((data & 0x10) ? TRUE : FALSE);

	/* D5 - APPEAR */
	crbaloon_audio_set_appear_enable((data & 0x20) ? TRUE : FALSE);

	/* D6 - unlabeled - laugh enable */
	crbaloon_audio_set_laugh_enable(space, 0, (data & 0x40) ? TRUE : FALSE);

	/* D7 - unlabeled - goes to PC3259 pin 16 */

	pc3259_update();
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, crbaloon_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff) /* A15 is not decoded */
	AM_RANGE(0x0000, 0x3fff) AM_ROM     /* not fully populated */
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x0400) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0x0400) AM_RAM_WRITE(crbaloon_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x5000, 0x53ff) AM_MIRROR(0x0400) AM_RAM_WRITE(crbaloon_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x5800, 0x7fff) AM_NOP
ADDRESS_MAP_END



/*************************************
 *
 *  Port handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_io_map, AS_IO, 8, crbaloon_state )
	ADDRESS_MAP_GLOBAL_MASK(0xf)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x0c) AM_READ_PORT("DSW0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x0c) AM_READ_PORT("IN0")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x0c) AM_MASK(0x0c) AM_READ(pc3259_r)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x0c) AM_READ_PORT("IN1")

	AM_RANGE(0x00, 0x00) AM_WRITENOP    /* not connected */
	AM_RANGE(0x01, 0x01) AM_WRITENOP /* watchdog */
	AM_RANGE(0x02, 0x04) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x05, 0x05) AM_WRITE(crbaloon_audio_set_music_freq)
	AM_RANGE(0x06, 0x06) AM_WRITE(port_sound_w)
	AM_RANGE(0x07, 0x0b) AM_WRITE(pc3092_w) AM_SHARE("pc3092_data")
	AM_RANGE(0x0c, 0x0c) AM_WRITENOP /* MSK - to PC3259 */
	AM_RANGE(0x0d, 0x0d) AM_WRITENOP /* schematics has it in a box marked "NOT USE" */
	AM_RANGE(0x0e, 0x0f) AM_WRITENOP
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( crbaloon )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "Test?" ) PORT_DIPLOCATION("SW A:1")
	PORT_DIPSETTING(    0x01, "I/O Check?" )
	PORT_DIPSETTING(    0x00, "RAM Check?" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW A:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW A:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW A:5")
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "10000" )
	PORT_DIPNAME( 0xe0, 0x80, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW A:6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, "Disable" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* PC3259 */
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability") PORT_DIPLOCATION("SW B:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW B:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW B:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW B:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW B:5")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Name Reset")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, crbaloon_state,pc3092_r, NULL)

	PORT_START("PC3092")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("VR2")
	PORT_ADJUSTER(50, "VR2 - Beep")

	PORT_START("VR3")
	PORT_ADJUSTER(50, "VR3 - Music")

INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	1,  /* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


static GFXDECODE_START( crbaloon )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine reset
 *
 *************************************/

void crbaloon_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_IO);

	pc3092_reset();
	port_sound_w(space, 0, 0);
	crbaloon_audio_set_music_freq(space, 0, 0);
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

INTERRUPT_GEN_MEMBER(crbaloon_state::vblank_irq)
{
	if(m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}


static MACHINE_CONFIG_START( crbaloon, crbaloon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CRBALOON_MASTER_XTAL / 3)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", crbaloon_state,  vblank_irq)


	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", crbaloon)
	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(crbaloon_state, crbaloon)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(crbaloon_state, screen_update_crbaloon)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	/* audio hardware */
	MCFG_FRAGMENT_ADD(crbaloon_audio)

MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( crbaloon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cl01.bin",     0x0000, 0x0800, CRC(9d4eef0b) SHA1(a8dd814ac2612073982123c91fa62deaf5bee242) )
	ROM_LOAD( "cl02.bin",     0x0800, 0x0800, CRC(10f7a6f7) SHA1(e672a7dcdaae08b202cfc2e19033846ebb267e1b) )
	ROM_LOAD( "cl03.bin",     0x1000, 0x0800, CRC(44ed6030) SHA1(8bbf5d9e893710138be15e56682037f128c83527) )
	ROM_LOAD( "cl04.bin",     0x1800, 0x0800, CRC(62f66f6c) SHA1(d173b12d6b5e0719d7b25ff0cafebbe64ec6b134) )
	ROM_LOAD( "cl05.bin",     0x2000, 0x0800, CRC(c8f1e2be) SHA1(a4603ce0268fa987f7f780702b6ca04e28759674) )
	ROM_LOAD( "cl06.bin",     0x2800, 0x0800, CRC(7d465691) SHA1(f5dc7abe8db232f702419d126cee6607ea6a5168) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "cl07.bin",     0x0000, 0x0800, CRC(2c1fbea8) SHA1(41cf2aef74d56173057886512d989f6fa3682056) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "cl08.bin",     0x0000, 0x0800, CRC(ba898659) SHA1(4291059b113ff91896f1f61a4c14956716edfe1e) )
ROM_END


ROM_START( crbaloon2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cl01.bin",     0x0000, 0x0800, CRC(9d4eef0b) SHA1(a8dd814ac2612073982123c91fa62deaf5bee242) )
	ROM_LOAD( "crazybal.ep2", 0x0800, 0x0800, CRC(87572086) SHA1(dba842c7c4cb16154ae0da43d71f8f03a56441c3) )
	ROM_LOAD( "crazybal.ep3", 0x1000, 0x0800, CRC(575fe995) SHA1(829db1da27cc9b706db6d9563bd271ffcd42be4a) )
	ROM_LOAD( "cl04.bin",     0x1800, 0x0800, CRC(62f66f6c) SHA1(d173b12d6b5e0719d7b25ff0cafebbe64ec6b134) )
	ROM_LOAD( "cl05.bin",     0x2000, 0x0800, CRC(c8f1e2be) SHA1(a4603ce0268fa987f7f780702b6ca04e28759674) )
	ROM_LOAD( "crazybal.ep6", 0x2800, 0x0800, CRC(fed6ff5c) SHA1(e6ed276949fd1511c6abe97026793193fda36e92) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "cl07.bin",     0x0000, 0x0800, CRC(2c1fbea8) SHA1(41cf2aef74d56173057886512d989f6fa3682056) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "cl08.bin",     0x0000, 0x0800, CRC(ba898659) SHA1(4291059b113ff91896f1f61a4c14956716edfe1e) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980, crbaloon, 0,        crbaloon, crbaloon, driver_device, 0, ROT90, "Taito Corporation", "Crazy Balloon (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, crbaloon2,crbaloon, crbaloon, crbaloon, driver_device, 0, ROT90, "Taito Corporation", "Crazy Balloon (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
