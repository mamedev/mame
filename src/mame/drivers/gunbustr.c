// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
/****************************************************************************

    Gunbuster (c) 1992 Taito

    Driver by Bryan McPhail & David Graves.

    Board Info:

        CPU   : 68EC020 68000
        SOUND : Ensoniq
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

        FLIPX support in taitoic.c is not quite correct - the Taito logo is wrong,
        and the floor in the Doom levels has horizontal scrolling where it shouldn't.

        No networked machine support

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/es5506.h"
#include "audio/taito_en.h"
#include "includes/gunbustr.h"


/*********************************************************************/

void gunbustr_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_GUNBUSTR_INTERRUPT5:
		m_maincpu->set_input_line(5, HOLD_LINE);
		break;
	default:
		assert_always(FALSE, "Unknown id in gunbustr_state::device_timer");
	}
}

INTERRUPT_GEN_MEMBER(gunbustr_state::gunbustr_interrupt)
{
	timer_set(downcast<cpu_device *>(&device)->cycles_to_attotime(200000-500), TIMER_GUNBUSTR_INTERRUPT5);
	device.execute().set_input_line(4, HOLD_LINE);
}

CUSTOM_INPUT_MEMBER(gunbustr_state::coin_word_r)
{
	return m_coin_word;
}

WRITE32_MEMBER(gunbustr_state::gunbustr_input_w)
{
	switch (offset)
	{
		case 0x00:
		{
			if (ACCESSING_BITS_24_31)
			{
				/* $400000 is watchdog */
				machine().watchdog_reset();
			}

			if (ACCESSING_BITS_0_7)
			{
				m_eeprom->clk_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
				m_eeprom->di_write((data & 0x40) >> 6);
				m_eeprom->cs_write((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
			}
			break;
		}

		case 0x01:
		{
			if (ACCESSING_BITS_24_31)
			{
				if (m_coin_lockout)
				{
					coin_lockout_w(machine(), 0, ~data & 0x01000000);
					coin_lockout_w(machine(), 1, ~data & 0x02000000);
				}

				// game does not write a separate counter for coin 2! maybe in linked mode?
				coin_counter_w(machine(), 0, data & 0x04000000);
				coin_counter_w(machine(), 1, data & 0x04000000);
				m_coin_word = (data >> 16) &0xffff;
			}
			//logerror("CPU #0 PC %06x: write input %06x\n",device->safe_pc(),offset);
			break;
		}
	}
}

WRITE32_MEMBER(gunbustr_state::motor_control_w)
{
	// Standard value poked into MSW is 0x3c00
	// (0x2000 and zero are written at startup)
	output_set_value("Player1_Gun_Recoil", (data & 0x1000000) ? 1 : 0);
	output_set_value("Player2_Gun_Recoil", (data & 0x10000) ? 1 : 0);
	output_set_value("Hit_lamp", (data & 0x40000) ? 1 : 0);
}



READ32_MEMBER(gunbustr_state::gunbustr_gun_r)
{
	return ( ioport("LIGHT0_X")->read() << 24) | (ioport("LIGHT0_Y")->read() << 16) |
			( ioport("LIGHT1_X")->read() << 8)  |  ioport("LIGHT1_Y")->read();
}

WRITE32_MEMBER(gunbustr_state::gunbustr_gun_w)
{
	/* 10000 cycle delay is arbitrary */
	timer_set(downcast<cpu_device *>(&space.device())->cycles_to_attotime(10000), TIMER_GUNBUSTR_INTERRUPT5);
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( gunbustr_map, AS_PROGRAM, 32, gunbustr_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM AM_SHARE("ram")                                     /* main CPUA ram */
	AM_RANGE(0x300000, 0x301fff) AM_RAM AM_SHARE("spriteram")               /* Sprite ram */
	AM_RANGE(0x380000, 0x380003) AM_WRITE(motor_control_w)                                          /* motor, lamps etc. */
	AM_RANGE(0x390000, 0x3907ff) AM_RAM AM_SHARE("snd_shared")                                      /* Sound shared ram */
	AM_RANGE(0x400000, 0x400003) AM_READ_PORT("P1_P2")
	AM_RANGE(0x400004, 0x400007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x400000, 0x400007) AM_WRITE(gunbustr_input_w)                                         /* eerom etc. */
	AM_RANGE(0x500000, 0x500003) AM_READWRITE(gunbustr_gun_r, gunbustr_gun_w)                       /* gun coord read */
	AM_RANGE(0x800000, 0x80ffff) AM_DEVREADWRITE("tc0480scp", tc0480scp_device, long_r, long_w)
	AM_RANGE(0x830000, 0x83002f) AM_DEVREADWRITE("tc0480scp", tc0480scp_device, ctrl_long_r, ctrl_long_w)
	AM_RANGE(0x900000, 0x901fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM                                                             /* network ram ?? */
ADDRESS_MAP_END

/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( gunbustr )
	PORT_START("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  /* Freeze input */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x00000001, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, gunbustr_state,coin_word_r, NULL)

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
	{ 0, 8, 16, 24 },
	{ 32, 33, 34, 35, 36, 37, 38, 39, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*64, 1*64,  2*64,  3*64,  4*64,  5*64,  6*64,  7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	64*16   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	16,16,    /* 16*16 characters */
	RGN_FRAC(1,1),
	4,        /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 5*4, 4*4, 3*4, 2*4, 7*4, 6*4, 9*4, 8*4, 13*4, 12*4, 11*4, 10*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8     /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gunbustr )
	GFXDECODE_ENTRY( "gfx2", 0x0, tile16x16_layout,  0, 256 )
	GFXDECODE_ENTRY( "gfx1", 0x0, charlayout,        0, 256 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

static MACHINE_CONFIG_START( gunbustr, gunbustr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(gunbustr_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gunbustr_state,  gunbustr_interrupt) /* VBL */

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(gunbustr_state, screen_update_gunbustr)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ORIENTATION_FLIP_X)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gunbustr)
	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_DEVICE_ADD("tc0480scp", TC0480SCP, 0)
	MCFG_TC0480SCP_GFX_REGION(1)
	MCFG_TC0480SCP_TX_REGION(2)
	MCFG_TC0480SCP_OFFSETS(0x20, 0x07)
	MCFG_TC0480SCP_OFFSETS_TX(-1, -1)
	MCFG_TC0480SCP_OFFSETS_FLIP(-1, 0)
	MCFG_TC0480SCP_GFXDECODE("gfxdecode")
	MCFG_TC0480SCP_PALETTE("palette")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(taito_en_sound)
MACHINE_CONFIG_END

/***************************************************************************/

ROM_START( gunbustr )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 1024K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d27-23.bin", 0x00000, 0x40000, CRC(cd1037cc) SHA1(8005a6a84081ce609e7a605ec8e00e740bfc6846) )
	ROM_LOAD32_BYTE( "d27-22.bin", 0x00001, 0x40000, CRC(475949fc) SHA1(3d5aa3411d2618004902f9d05dff61d9af01ff35) )
	ROM_LOAD32_BYTE( "d27-21.bin", 0x00002, 0x40000, CRC(60950a8a) SHA1(a0336bf6970baa6eaa998a112db840a7fd0452d7) )
	ROM_LOAD32_BYTE( "d27-27.bin", 0x00003, 0x40000, CRC(fd7d3d4c) SHA1(df42e135b1e9b7e371971ba7c8a2e161f3623aa3) )

	ROM_REGION( 0x140000, "audiocpu", 0 )   /* Sound cpu */
	ROM_LOAD16_BYTE( "d27-25.bin", 0x100000, 0x20000, CRC(c88203cf) SHA1(a918d395b471acdce56dacabd7a1e1e023948365) )
	ROM_LOAD16_BYTE( "d27-24.bin", 0x100001, 0x20000, CRC(084bd8bd) SHA1(93229bc7de4550ead1bb12f666ddbacbe357488d) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "d27-01.bin", 0x00000, 0x80000, CRC(f41759ce) SHA1(30789f43dd09b56399e1dfdb8c6a1e01a21562bd) ) /* SCR 16x16 tiles */
	ROM_LOAD16_BYTE( "d27-02.bin", 0x00001, 0x80000, CRC(92ab6430) SHA1(28ed80391c732b09d10c74ed6b78ac76cb62e083) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "d27-04.bin", 0x000003, 0x100000, CRC(ff8b9234) SHA1(6095b7daf9b7e9a22b0d44d9d6a642ddecb2bd29) )   /* OBJ 16x16 tiles: each rom has 1 bitplane */
	ROM_LOAD32_BYTE( "d27-05.bin", 0x000002, 0x100000, CRC(96d7c1a5) SHA1(93b6a7aea397280a5a778e736d433a85cb7da52c) )
	ROM_LOAD32_BYTE( "d27-06.bin", 0x000001, 0x100000, CRC(bbb934db) SHA1(9e9b5cf05b9275f1182f5b499b8ee897c4f25b96) )
	ROM_LOAD32_BYTE( "d27-07.bin", 0x000000, 0x100000, CRC(8ab4854e) SHA1(bd2750cdaa2918e56f8aef3732875952a1eeafea) )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "d27-03.bin", 0x00000, 0x80000, CRC(23bf2000) SHA1(49b29e771a47fcd7e6cd4e2704b217f9727f8299) ) /* STY, used to create big sprites on the fly */

	ROM_REGION16_BE( 0x800000, "ensoniq.0" , ROMREGION_ERASE00 )
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

	ROM_REGION( 0x140000, "audiocpu", 0 )   /* Sound cpu */
	ROM_LOAD16_BYTE( "d27-25.bin", 0x100000, 0x20000, CRC(c88203cf) SHA1(a918d395b471acdce56dacabd7a1e1e023948365) )
	ROM_LOAD16_BYTE( "d27-24.bin", 0x100001, 0x20000, CRC(084bd8bd) SHA1(93229bc7de4550ead1bb12f666ddbacbe357488d) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "d27-01.bin", 0x00000, 0x80000, CRC(f41759ce) SHA1(30789f43dd09b56399e1dfdb8c6a1e01a21562bd) ) /* SCR 16x16 tiles */
	ROM_LOAD16_BYTE( "d27-02.bin", 0x00001, 0x80000, CRC(92ab6430) SHA1(28ed80391c732b09d10c74ed6b78ac76cb62e083) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "d27-04.bin", 0x000003, 0x100000, CRC(ff8b9234) SHA1(6095b7daf9b7e9a22b0d44d9d6a642ddecb2bd29) )   /* OBJ 16x16 tiles: each rom has 1 bitplane */
	ROM_LOAD32_BYTE( "d27-05.bin", 0x000002, 0x100000, CRC(96d7c1a5) SHA1(93b6a7aea397280a5a778e736d433a85cb7da52c) )
	ROM_LOAD32_BYTE( "d27-06.bin", 0x000001, 0x100000, CRC(bbb934db) SHA1(9e9b5cf05b9275f1182f5b499b8ee897c4f25b96) )
	ROM_LOAD32_BYTE( "d27-07.bin", 0x000000, 0x100000, CRC(8ab4854e) SHA1(bd2750cdaa2918e56f8aef3732875952a1eeafea) )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "d27-03.bin", 0x00000, 0x80000, CRC(23bf2000) SHA1(49b29e771a47fcd7e6cd4e2704b217f9727f8299) ) /* STY, used to create big sprites on the fly */

	ROM_REGION16_BE( 0x800000, "ensoniq.0" , ROMREGION_ERASE00 )
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

	ROM_REGION( 0x140000, "audiocpu", 0 )   /* Sound cpu */
	ROM_LOAD16_BYTE( "d27-25.bin", 0x100000, 0x20000, CRC(c88203cf) SHA1(a918d395b471acdce56dacabd7a1e1e023948365) )
	ROM_LOAD16_BYTE( "d27-24.bin", 0x100001, 0x20000, CRC(084bd8bd) SHA1(93229bc7de4550ead1bb12f666ddbacbe357488d) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "d27-01.bin", 0x00000, 0x80000, CRC(f41759ce) SHA1(30789f43dd09b56399e1dfdb8c6a1e01a21562bd) ) /* SCR 16x16 tiles */
	ROM_LOAD16_BYTE( "d27-02.bin", 0x00001, 0x80000, CRC(92ab6430) SHA1(28ed80391c732b09d10c74ed6b78ac76cb62e083) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "d27-04.bin", 0x000003, 0x100000, CRC(ff8b9234) SHA1(6095b7daf9b7e9a22b0d44d9d6a642ddecb2bd29) )   /* OBJ 16x16 tiles: each rom has 1 bitplane */
	ROM_LOAD32_BYTE( "d27-05.bin", 0x000002, 0x100000, CRC(96d7c1a5) SHA1(93b6a7aea397280a5a778e736d433a85cb7da52c) )
	ROM_LOAD32_BYTE( "d27-06.bin", 0x000001, 0x100000, CRC(bbb934db) SHA1(9e9b5cf05b9275f1182f5b499b8ee897c4f25b96) )
	ROM_LOAD32_BYTE( "d27-07.bin", 0x000000, 0x100000, CRC(8ab4854e) SHA1(bd2750cdaa2918e56f8aef3732875952a1eeafea) )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "d27-03.bin", 0x00000, 0x80000, CRC(23bf2000) SHA1(49b29e771a47fcd7e6cd4e2704b217f9727f8299) ) /* STY, used to create big sprites on the fly */

	ROM_REGION16_BE( 0x800000, "ensoniq.0" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d27-08.bin", 0x000000, 0x100000, CRC(7c147e30) SHA1(b605045154967050ec06391798da4afe3686a6e1) ) // C8, C9
	ROM_RELOAD(0x400000,0x100000)
	ROM_LOAD16_BYTE( "d27-09.bin", 0x200000, 0x100000, CRC(3e060304) SHA1(c4da4a94c168c3a454409d758c3ed45babbab170) ) // CA, CB
	ROM_LOAD16_BYTE( "d27-10.bin", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-gunbustr.bin", 0x0000, 0x0080, CRC(ef3685a1) SHA1(899b4b6dd2fd78be3a2ce00a2ef1840de9f122c3) )
ROM_END

READ32_MEMBER(gunbustr_state::main_cycle_r)
{
	if (space.device().safe_pc()==0x55a && (m_ram[0x3acc/4]&0xff000000)==0)
		space.device().execute().spin_until_interrupt();

	return m_ram[0x3acc/4];
}

DRIVER_INIT_MEMBER(gunbustr_state,gunbustr)
{
	/* Speedup handler */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x203acc, 0x203acf, read32_delegate(FUNC(gunbustr_state::main_cycle_r),this));
}

DRIVER_INIT_MEMBER(gunbustr_state,gunbustrj)
{
	DRIVER_INIT_CALL(gunbustr);

	// no coin lockout, perhaps this was a prototype version without proper coin handling?
	m_coin_lockout = false;
}

GAME( 1992, gunbustr,  0,        gunbustr, gunbustr, gunbustr_state, gunbustr, ORIENTATION_FLIP_X, "Taito Corporation Japan", "Gunbuster (World)", 0 )
GAME( 1992, gunbustru, gunbustr, gunbustr, gunbustr, gunbustr_state, gunbustr, ORIENTATION_FLIP_X, "Taito America Corporation", "Gunbuster (US)", 0 )
GAME( 1992, gunbustrj, gunbustr, gunbustr, gunbustr, gunbustr_state, gunbustrj,ORIENTATION_FLIP_X, "Taito Corporation", "Gunbuster (Japan)", 0 )
