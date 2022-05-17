// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
/****************************************************************************

    Gunbuster (c) 1992 Taito

    Driver by Bryan McPhail & David Graves.

    Board Info:

        CPU   : 68EC020 68000
        SOUND : Ensoniq ES5505 + ES5510
        OSC.  : 40.000MHz 16.000MHz 30.47618MHz

        * This board (K11J0717A) uses following chips:
          - TC0470LIN
          - TC0480SCP
          - TC0570SPC
          - TC0260DAR
          - TC0510NIO

    Gunbuster uses a slightly enhanced sprite system from the one
    in Taito Z games.

    The key feature remains the use of a sprite map rom which allows
    the sprite hardware to create many large zoomed sprites on screen
    while minimizing the main cpu load.

    This feature makes the SZ system complementary to the F3 system
    which, owing to its F2 sprite hardware, is not very well suited to
    3d games. (Taito abandoned the SZ system once better 3d hardware
    platforms were available in the mid 1990s.)

    Gunbuster also uses the TC0480SCP tilemap chip (like the last Taito
    Z game, Double Axle).

    Todo:

        FLIPX support in the video chips is not quite correct - the Taito logo is wrong,
        and the floor in the Doom levels has horizontal scrolling where it shouldn't.

        No networked machine support

***************************************************************************/

#include "emu.h"
#include "includes/gunbustr.h"

#include "audio/taito_en.h"
#include "machine/taitoio.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/es5506.h"

#include "screen.h"
#include "speaker.h"


/*********************************************************************/

TIMER_CALLBACK_MEMBER(gunbustr_state::trigger_irq5)
{
	m_maincpu->set_input_line(5, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(gunbustr_state::gunbustr_interrupt)
{
	m_interrupt5_timer->adjust(m_maincpu->cycles_to_attotime(200000-500));
	device.execute().set_input_line(4, HOLD_LINE);
}

void gunbustr_state::coin_word_w(u8 data)
{
	if (m_coin_lockout)
	{
		machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
		machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	}

	// game does not write a separate counter for coin 2! maybe in linked mode?
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x04);
}

void gunbustr_state::motor_control_w(u32 data)
{
	// Standard value poked into MSW is 0x3c00
	// (0x2000 and zero are written at startup)
	output().set_value("Player1_Gun_Recoil", BIT(data, 24));
	output().set_value("Player2_Gun_Recoil", BIT(data, 16));
	output().set_value("Hit_lamp", BIT(data, 18));
}



u32 gunbustr_state::gun_r()
{
	return (m_io_light_x[0]->read() << 24) | (m_io_light_y[0]->read() << 16) |
			(m_io_light_x[1]->read() << 8)  |  m_io_light_y[1]->read();
}

void gunbustr_state::gun_w(u32 data)
{
	/* 10000 cycle delay is arbitrary */
	m_interrupt5_timer->adjust(m_maincpu->cycles_to_attotime(10000));
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

void gunbustr_state::gunbustr_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x21ffff).ram().share("ram");                                     /* main CPUA ram */
	map(0x300000, 0x301fff).ram().share("spriteram");               /* Sprite ram */
	map(0x380000, 0x380003).w(FUNC(gunbustr_state::motor_control_w));                                          /* motor, lamps etc. */
	map(0x390000, 0x3907ff).rw("taito_en:dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w)); /* Sound shared ram */
	map(0x400000, 0x400007).rw("tc0510nio", FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write));
	map(0x500000, 0x500003).rw(FUNC(gunbustr_state::gun_r), FUNC(gunbustr_state::gun_w));                       /* gun coord read */
	map(0x800000, 0x80ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));
	map(0x830000, 0x83002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x900000, 0x901fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0xc00000, 0xc03fff).ram();                                                             /* network ram ?? */
}

/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( gunbustr )
	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  /* Freeze input */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* Light gun inputs */

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


/***********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ STEP4(0,16) },
	{ STEP16(0,1) },
	{ STEP16(0,16*4) },
	16*16*4   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_gunbustr )
	GFXDECODE_ENTRY( "sprites", 0x0, tile16x16_layout,  0, 256 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

void gunbustr_state::gunbustr(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &gunbustr_state::gunbustr_map);
	m_maincpu->set_vblank_int("screen", FUNC(gunbustr_state::gunbustr_interrupt)); /* VBL */

	EEPROM_93C46_16BIT(config, "eeprom");

	tc0510nio_device &tc0510nio(TC0510NIO(config, "tc0510nio", 0));
	tc0510nio.read_0_callback().set_ioport("EXTRA");
	tc0510nio.read_2_callback().set_ioport("INPUTS");
	tc0510nio.read_3_callback().set_ioport("SPECIAL");
	tc0510nio.write_3_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(5);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	tc0510nio.write_4_callback().set(FUNC(gunbustr_state::coin_word_w));
	tc0510nio.read_7_callback().set_ioport("SYSTEM");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0, 40*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(gunbustr_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gunbustr);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 4096);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x20, 0x07);
	m_tc0480scp->set_offsets_tx(-1, -1);
	m_tc0480scp->set_offsets_flip(-1, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	taito_en_device &taito_en(TAITO_EN(config, "taito_en", 0));
	taito_en.add_route(0, "lspeaker", 1.0);
	taito_en.add_route(1, "rspeaker", 1.0);
}

/***************************************************************************/

ROM_START( gunbustr )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 1024K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d27-23.bin", 0x00000, 0x40000, CRC(cd1037cc) SHA1(8005a6a84081ce609e7a605ec8e00e740bfc6846) )
	ROM_LOAD32_BYTE( "d27-22.bin", 0x00001, 0x40000, CRC(475949fc) SHA1(3d5aa3411d2618004902f9d05dff61d9af01ff35) )
	ROM_LOAD32_BYTE( "d27-21.bin", 0x00002, 0x40000, CRC(60950a8a) SHA1(a0336bf6970baa6eaa998a112db840a7fd0452d7) )
	ROM_LOAD32_BYTE( "d27-27.bin", 0x00003, 0x40000, CRC(fd7d3d4c) SHA1(df42e135b1e9b7e371971ba7c8a2e161f3623aa3) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )   /* Sound cpu */
	ROM_LOAD16_BYTE( "d27-25.bin", 0x100000, 0x20000, CRC(c88203cf) SHA1(a918d395b471acdce56dacabd7a1e1e023948365) )
	ROM_LOAD16_BYTE( "d27-24.bin", 0x100001, 0x20000, CRC(084bd8bd) SHA1(93229bc7de4550ead1bb12f666ddbacbe357488d) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d27-01.bin", 0x00000, 0x80000, CRC(f41759ce) SHA1(30789f43dd09b56399e1dfdb8c6a1e01a21562bd) ) /* SCR 16x16 tiles */
	ROM_LOAD32_WORD( "d27-02.bin", 0x00002, 0x80000, CRC(92ab6430) SHA1(28ed80391c732b09d10c74ed6b78ac76cb62e083) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "d27-04.bin", 0x000006, 0x100000, CRC(ff8b9234) SHA1(6095b7daf9b7e9a22b0d44d9d6a642ddecb2bd29) )   /* OBJ 16x16 tiles: each rom has 1 bitplane */
	ROM_LOAD64_WORD_SWAP( "d27-05.bin", 0x000004, 0x100000, CRC(96d7c1a5) SHA1(93b6a7aea397280a5a778e736d433a85cb7da52c) )
	ROM_LOAD64_WORD_SWAP( "d27-06.bin", 0x000002, 0x100000, CRC(bbb934db) SHA1(9e9b5cf05b9275f1182f5b499b8ee897c4f25b96) )
	ROM_LOAD64_WORD_SWAP( "d27-07.bin", 0x000000, 0x100000, CRC(8ab4854e) SHA1(bd2750cdaa2918e56f8aef3732875952a1eeafea) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "d27-03.bin", 0x00000, 0x80000, CRC(23bf2000) SHA1(49b29e771a47fcd7e6cd4e2704b217f9727f8299) ) /* STY, used to create big sprites on the fly */

	ROM_REGION16_BE( 0x800000, "taito_en:ensoniq" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d27-08.bin", 0x000000, 0x100000, CRC(7c147e30) SHA1(b605045154967050ec06391798da4afe3686a6e1) ) // C8, C9
	ROM_RELOAD(0x400000,0x100000)
	ROM_LOAD16_BYTE( "d27-09.bin", 0x200000, 0x100000, CRC(3e060304) SHA1(c4da4a94c168c3a454409d758c3ed45babbab170) ) // CA, CB
	ROM_LOAD16_BYTE( "d27-10.bin", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-gunbustr.bin", 0x0000, 0x0080, CRC(ef3685a1) SHA1(899b4b6dd2fd78be3a2ce00a2ef1840de9f122c3) )
ROM_END

ROM_START( gunbustru )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 1024K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d27-23.bin", 0x00000, 0x40000, CRC(cd1037cc) SHA1(8005a6a84081ce609e7a605ec8e00e740bfc6846) )
	ROM_LOAD32_BYTE( "d27-22.bin", 0x00001, 0x40000, CRC(475949fc) SHA1(3d5aa3411d2618004902f9d05dff61d9af01ff35) )
	ROM_LOAD32_BYTE( "d27-21.bin", 0x00002, 0x40000, CRC(60950a8a) SHA1(a0336bf6970baa6eaa998a112db840a7fd0452d7) )
	ROM_LOAD32_BYTE( "d27-26.bin", 0x00003, 0x40000, CRC(8a7a0dda) SHA1(59ee7c391c170ab05a3d3d940d833c65e265d9b3) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )   /* Sound cpu */
	ROM_LOAD16_BYTE( "d27-25.bin", 0x100000, 0x20000, CRC(c88203cf) SHA1(a918d395b471acdce56dacabd7a1e1e023948365) )
	ROM_LOAD16_BYTE( "d27-24.bin", 0x100001, 0x20000, CRC(084bd8bd) SHA1(93229bc7de4550ead1bb12f666ddbacbe357488d) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d27-01.bin", 0x00000, 0x80000, CRC(f41759ce) SHA1(30789f43dd09b56399e1dfdb8c6a1e01a21562bd) ) /* SCR 16x16 tiles */
	ROM_LOAD32_WORD( "d27-02.bin", 0x00002, 0x80000, CRC(92ab6430) SHA1(28ed80391c732b09d10c74ed6b78ac76cb62e083) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "d27-04.bin", 0x000006, 0x100000, CRC(ff8b9234) SHA1(6095b7daf9b7e9a22b0d44d9d6a642ddecb2bd29) )   /* OBJ 16x16 tiles: each rom has 1 bitplane */
	ROM_LOAD64_WORD_SWAP( "d27-05.bin", 0x000004, 0x100000, CRC(96d7c1a5) SHA1(93b6a7aea397280a5a778e736d433a85cb7da52c) )
	ROM_LOAD64_WORD_SWAP( "d27-06.bin", 0x000002, 0x100000, CRC(bbb934db) SHA1(9e9b5cf05b9275f1182f5b499b8ee897c4f25b96) )
	ROM_LOAD64_WORD_SWAP( "d27-07.bin", 0x000000, 0x100000, CRC(8ab4854e) SHA1(bd2750cdaa2918e56f8aef3732875952a1eeafea) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "d27-03.bin", 0x00000, 0x80000, CRC(23bf2000) SHA1(49b29e771a47fcd7e6cd4e2704b217f9727f8299) ) /* STY, used to create big sprites on the fly */

	ROM_REGION16_BE( 0x800000, "taito_en:ensoniq" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d27-08.bin", 0x000000, 0x100000, CRC(7c147e30) SHA1(b605045154967050ec06391798da4afe3686a6e1) ) // C8, C9
	ROM_RELOAD(0x400000,0x100000)
	ROM_LOAD16_BYTE( "d27-09.bin", 0x200000, 0x100000, CRC(3e060304) SHA1(c4da4a94c168c3a454409d758c3ed45babbab170) ) // CA, CB
	ROM_LOAD16_BYTE( "d27-10.bin", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-gunbustr.bin", 0x0000, 0x0080, CRC(ef3685a1) SHA1(899b4b6dd2fd78be3a2ce00a2ef1840de9f122c3) )
ROM_END

ROM_START( gunbustrj )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 1024K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d27-23.bin", 0x00000, 0x40000, CRC(cd1037cc) SHA1(8005a6a84081ce609e7a605ec8e00e740bfc6846) )
	ROM_LOAD32_BYTE( "d27-22.bin", 0x00001, 0x40000, CRC(475949fc) SHA1(3d5aa3411d2618004902f9d05dff61d9af01ff35) )
	ROM_LOAD32_BYTE( "d27-21.bin", 0x00002, 0x40000, CRC(60950a8a) SHA1(a0336bf6970baa6eaa998a112db840a7fd0452d7) )
	ROM_LOAD32_BYTE( "d27-20.bin", 0x00003, 0x40000, CRC(13735c60) SHA1(65b762b28d51b295f6fe190420af566b1b3d4a82) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )   /* Sound cpu */
	ROM_LOAD16_BYTE( "d27-25.bin", 0x100000, 0x20000, CRC(c88203cf) SHA1(a918d395b471acdce56dacabd7a1e1e023948365) )
	ROM_LOAD16_BYTE( "d27-24.bin", 0x100001, 0x20000, CRC(084bd8bd) SHA1(93229bc7de4550ead1bb12f666ddbacbe357488d) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d27-01.bin", 0x00000, 0x80000, CRC(f41759ce) SHA1(30789f43dd09b56399e1dfdb8c6a1e01a21562bd) ) /* SCR 16x16 tiles */
	ROM_LOAD32_WORD( "d27-02.bin", 0x00002, 0x80000, CRC(92ab6430) SHA1(28ed80391c732b09d10c74ed6b78ac76cb62e083) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "d27-04.bin", 0x000006, 0x100000, CRC(ff8b9234) SHA1(6095b7daf9b7e9a22b0d44d9d6a642ddecb2bd29) )   /* OBJ 16x16 tiles: each rom has 1 bitplane */
	ROM_LOAD64_WORD_SWAP( "d27-05.bin", 0x000004, 0x100000, CRC(96d7c1a5) SHA1(93b6a7aea397280a5a778e736d433a85cb7da52c) )
	ROM_LOAD64_WORD_SWAP( "d27-06.bin", 0x000002, 0x100000, CRC(bbb934db) SHA1(9e9b5cf05b9275f1182f5b499b8ee897c4f25b96) )
	ROM_LOAD64_WORD_SWAP( "d27-07.bin", 0x000000, 0x100000, CRC(8ab4854e) SHA1(bd2750cdaa2918e56f8aef3732875952a1eeafea) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "d27-03.bin", 0x00000, 0x80000, CRC(23bf2000) SHA1(49b29e771a47fcd7e6cd4e2704b217f9727f8299) ) /* STY, used to create big sprites on the fly */

	ROM_REGION16_BE( 0x800000, "taito_en:ensoniq" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d27-08.bin", 0x000000, 0x100000, CRC(7c147e30) SHA1(b605045154967050ec06391798da4afe3686a6e1) ) // C8, C9
	ROM_RELOAD(0x400000,0x100000)
	ROM_LOAD16_BYTE( "d27-09.bin", 0x200000, 0x100000, CRC(3e060304) SHA1(c4da4a94c168c3a454409d758c3ed45babbab170) ) // CA, CB
	ROM_LOAD16_BYTE( "d27-10.bin", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-gunbustr.bin", 0x0000, 0x0080, CRC(ef3685a1) SHA1(899b4b6dd2fd78be3a2ce00a2ef1840de9f122c3) )
ROM_END

u32 gunbustr_state::main_cycle_r()
{
	if (m_maincpu->pc() == 0x55a && (m_ram[0x3acc/4] & 0xff000000) == 0)
		m_maincpu->spin_until_interrupt();

	return m_ram[0x3acc/4];
}

void gunbustr_state::init_gunbustr()
{
	/* Speedup handler */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x203acc, 0x203acf, read32smo_delegate(*this, FUNC(gunbustr_state::main_cycle_r)));

	m_interrupt5_timer = timer_alloc(FUNC(gunbustr_state::trigger_irq5), this);
}

void gunbustr_state::init_gunbustrj()
{
	init_gunbustr();

	// no coin lockout, perhaps this was a prototype version without proper coin handling?
	m_coin_lockout = false;
}

GAME( 1992, gunbustr,  0,        gunbustr, gunbustr, gunbustr_state, init_gunbustr, ORIENTATION_FLIP_X, "Taito Corporation Japan",   "Gunbuster (World)", MACHINE_NODEVICE_LAN )
GAME( 1992, gunbustru, gunbustr, gunbustr, gunbustr, gunbustr_state, init_gunbustr, ORIENTATION_FLIP_X, "Taito America Corporation", "Gunbuster (US)",    MACHINE_NODEVICE_LAN )
GAME( 1992, gunbustrj, gunbustr, gunbustr, gunbustr, gunbustr_state, init_gunbustrj,ORIENTATION_FLIP_X, "Taito Corporation",         "Gunbuster (Japan)", MACHINE_NODEVICE_LAN )
