// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Taito Crazy Balloon hardware

    To-Do:
        * Faithfully implement the custom chips
        * Coin counter and lock-out

***************************************************************************/

#include "emu.h"
#include "crbaloon.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "screen.h"

#define LOG_PC3092 (1U << 1)
#define LOG_PC3259 (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


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

void crbaloon_state::pc3092_reset(void)
{
	/* nothing yet */
}


void crbaloon_state::pc3092_update()
{
	flip_screen_set((m_pc3092_data[1] & 0x01) ? true : false);
}


void crbaloon_state::pc3092_w(offs_t offset, uint8_t data)
{
	m_pc3092_data[offset] = data & 0x0f;

	LOGMASKED(LOG_PC3092, "%04X:  write PC3092 #%d = 0x%02x\n", m_maincpu->pc(), offset, m_pc3092_data[offset]);

	pc3092_update();
}


ioport_value crbaloon_state::pc3092_r()
{
	uint32_t ret;

	/* enable coin & start input? Wild guess!!! */
	if (m_pc3092_data[1] & 0x02)
		ret = ioport("PC3092")->read();
	else
		ret = 0x00;

	LOGMASKED(LOG_PC3092, "%s:  read  PC3092 = 0x%02x\n", machine().describe_context(), ret);

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


void crbaloon_state::pc3259_update(void)
{
	/* nothing yet */
}


uint8_t crbaloon_state::pc3259_r(offs_t offset)
{
	uint8_t ret = 0;
	uint8_t reg = offset >> 2;

	uint16_t collision_address = crbaloon_get_collision_address();
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

	LOGMASKED(LOG_PC3259, "%04X:  read PC3259 #%d = 0x%02x\n", m_maincpu->pc(), reg, ret);

	return ret | (ioport("DSW1")->read() & 0xf0);
}



/*************************************
 *
 *  I/O ports
 *
 *************************************/

void crbaloon_state::port_sound_w(uint8_t data)
{
	// D0 - interrupt enable - also goes to PC3259 as /HTCTRL
	m_irq_mask = data & 0x01;
	crbaloon_set_clear_collision_address(BIT(data, 0));
	if (!m_irq_mask)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

	// D1 - SOUND STOP
	machine().sound().system_mute(!BIT(data, 1));

	// D2 - unlabeled - music enable
	crbaloon_audio_set_music_enable(BIT(data, 2));

	// D3 - EXPLOSION
	crbaloon_audio_set_explosion_enable(BIT(data, 3));

	// D4 - BREATH
	crbaloon_audio_set_breath_enable(BIT(data, 4));

	// D5 - APPEAR
	crbaloon_audio_set_appear_enable(BIT(data, 5));

	// D6 - unlabeled - laugh enable
	crbaloon_audio_set_laugh_enable(BIT(data, 6));

	// D7 - unlabeled - goes to PC3259 pin 16

	pc3259_update();
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void crbaloon_state::main_map(address_map &map)
{
	map.global_mask(0x7fff); /* A15 is not decoded */
	map(0x0000, 0x3fff).rom();     /* not fully populated */
	map(0x4000, 0x43ff).mirror(0x0400).ram();
	map(0x4800, 0x4bff).mirror(0x0400).ram().w(FUNC(crbaloon_state::crbaloon_videoram_w)).share("videoram");
	map(0x5000, 0x53ff).mirror(0x0400).ram().w(FUNC(crbaloon_state::crbaloon_colorram_w)).share("colorram");
	map(0x5800, 0x7fff).noprw();
}



/*************************************
 *
 *  Port handlers
 *
 *************************************/

void crbaloon_state::main_io_map(address_map &map)
{
	map.global_mask(0xf);
	map(0x00, 0x00).mirror(0x0c).portr("DSW0");
	map(0x01, 0x01).mirror(0x0c).portr("IN0");
	map(0x02, 0x02).select(0x0c).r(FUNC(crbaloon_state::pc3259_r));
	map(0x03, 0x03).mirror(0x0c).portr("IN1");

	map(0x00, 0x00).nopw();    /* not connected */
	map(0x01, 0x01).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x02, 0x04).writeonly().share("spriteram");
	map(0x05, 0x05).w(FUNC(crbaloon_state::crbaloon_audio_set_music_freq));
	map(0x06, 0x06).w(FUNC(crbaloon_state::port_sound_w));
	map(0x07, 0x0b).w(FUNC(crbaloon_state::pc3092_w)).share("pc3092_data");
	map(0x0c, 0x0c).nopw(); /* MSK - to PC3259 */
	map(0x0d, 0x0d).nopw(); /* schematics has it in a box marked "NOT USE" */
	map(0x0e, 0x0f).nopw();
}



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
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* PC3259 */
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
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(crbaloon_state, pc3092_r)

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


static GFXDECODE_START( gfx_crbaloon )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine reset
 *
 *************************************/

void crbaloon_state::machine_reset()
{
	pc3092_reset();
	port_sound_w(0);
	crbaloon_audio_set_music_freq(0);
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void crbaloon_state::vbl_int_w(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}


void crbaloon_state::crbaloon(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, CRBALOON_MASTER_XTAL / 4); // CPU clock based on 1H?
	m_maincpu->set_addrmap(AS_PROGRAM, &crbaloon_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &crbaloon_state::main_io_map);

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 256);
	// Reset pulse comes from last stage of 74LS393 (8J) driven by 2VBL

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, "palette", gfx_crbaloon);
	PALETTE(config, "palette", FUNC(crbaloon_state::crbaloon_palette), 32);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_ALWAYS_UPDATE);
	screen.set_raw(CRBALOON_MASTER_XTAL / 2, 320, 0, 256, 262, 0, 224);
	screen.set_screen_update(FUNC(crbaloon_state::screen_update_crbaloon));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(crbaloon_state::vbl_int_w));

	/* audio hardware */
	crbaloon_audio(config);
}



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

GAME( 1980, crbaloon,  0,        crbaloon, crbaloon, crbaloon_state, empty_init, ROT90, "Taito Corporation", "Crazy Balloon (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, crbaloon2, crbaloon, crbaloon, crbaloon, crbaloon_state, empty_init, ROT90, "Taito Corporation", "Crazy Balloon (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
