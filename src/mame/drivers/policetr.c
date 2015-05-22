// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    P&P Marketing Police Trainer hardware

    driver by Aaron Giles

    Games supported:
        * Police Trainer
        * Sharpshooter

    Known bugs:
        * Flip screen not supported

Note:   Police Trainer v1.3B runs on the same revision PCB as Sharpshooter - Rev 0.5B
        If you hold the test button down and boot the game, all program roms
        fail the checksum.  This has been verified on an original PCB.
        See below for specific information on each set.
Note:   Police Trainer v1.0 (Rev 0.2 PCB), the checksum results in MAME have been
        verified to be the same as an original PCB.

To ID the version of your SharpShooter, check the 2nd printed line on each type of ROM.

Program Roms:  C121012 - Code version 1.2, Graphics v1.0 & Sound v1.2
Graphic Roms:  G10     - Graphics rom v1.0 (in diagnostics mode it's called "Art")
  Sound Roms:  S12     - Sound rom v1.2

Noted differences in versions of SharpShooter:
 Added a "Welcome to Sharpshooter" start-up screen showing rom versions for v1.9
 Initial High Score names are changed between v1.1 and v1.2
  Circus of Mystery:
    The balloon challenge has been rewritten for v1.7
    Jugglers throw balls painted with targets for v1.1 & v1.2  Version 1.7 uses regular targets
  Alien Encounter:
    First saucer challenge has been modified for v1.7

The ATTILA Video System PCB (by EXIT Entertainment):

Sharpshooter PCB is Rev 0.5B
Police Trainer PCB is Rev 0.3 / Rev 0.2

|------------JAMMA Connector------------|
|                     CN7               |
| GUN1   XILINX-1              93C66    |
| GUN2                                  |
|                                       |
| LED1 LED2         IDT71024 x 2  Bt481 |
|             AT001                     |
|  DSW(8)                               |
|U127                       U113    U162|
|U125  IDT71256 x 4         U112        |
|U123                       U111    U160|
|U121                       U110        |
|U126                                   |
|U124  OSC    IDT79R3041  XILINX-2      |
|U122  48.000MHz             XILINX-3   |
|U120                          BSMT2000 |
|---------------------------------------|

Chips:
  CPU: IDT 79R3041-25J (MIPS R3000 core)
Sound: BSMT2000
Other: Bt481AKPJ110 (44 Pin PQFP, Brooktree RAMDAC)
       AT001 (160 Pin PQFP, P&P Marketing, custom programmed XILINX XC4310)
       ATMEL 93C66 (EEPROM)
       CN7 - 4 pin connector for stereo speaker output
PLDs:
       XILINX-1 XC9536 Labeled as U175A (Rev 2/3: Not Used)
       XILINX-2 XC9536 Labeled as U109A (Rev 2/3: Lattice ispLSI 2032-80LJ - U109.P)
       XILINX-3 XC9536 Labeled as U151A (Rev 2/3: Lattice ispLSI 2032-80LJ - U151.P)

Note #1: Bt481A 256-Word Color Palette 15, 16 & 24-bit Color Power-Down RAMDAC
Note #2: For Rev 2 & 3 PCBs there is an optional daughter card to help with horizontal
         light gun accuracy

The main video chip is stamped:

Rev 2 PCB              Rev 3 PCB              Rev 5B PCB
------------------------------------------------------------
XILINX                 P & P                  P & P
XC4310                 Marketing              Marketing
PQ160C 5380            AJ001                  AT001
PC5380-9651            5380-JY3306A           5380-N1045503A
 PROTO                                        AKI9749

***************************************************************************/

#include "emu.h"
#include "cpu/mips/r3000.h"
#include "machine/eepromser.h"
#include "includes/policetr.h"
#include "sound/bsmt2000.h"


/* constants */
#define MASTER_CLOCK    48000000


/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

void policetr_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_IRQ5_GEN:
		m_maincpu->set_input_line(R3000_IRQ5, ASSERT_LINE);
		break;
	default:
		assert_always(FALSE, "Unknown id in policetr_state::device_timer");
	}
}


INTERRUPT_GEN_MEMBER(policetr_state::irq4_gen)
{
	device.execute().set_input_line(R3000_IRQ4, ASSERT_LINE);
	timer_set(m_screen->time_until_pos(0), TIMER_IRQ5_GEN);
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

WRITE32_MEMBER(policetr_state::control_w)
{
	UINT32 old = m_control_data;

	// bit $80000000 = BSMT access/ROM read
	// bit $20000000 = toggled every 64 IRQ4's
	// bit $10000000 = ????
	// bit $00800000 = EEPROM data
	// bit $00400000 = EEPROM clock
	// bit $00200000 = EEPROM enable (on 1)

	COMBINE_DATA(&m_control_data);

	/* handle EEPROM I/O */
	if (ACCESSING_BITS_16_23)
	{
		m_eeprom->di_write((data & 0x00800000) >> 23);
		m_eeprom->cs_write((data & 0x00200000) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->clk_write((data & 0x00400000) ? ASSERT_LINE : CLEAR_LINE);
	}

	/* toggling BSMT off then on causes a reset */
	if (!(old & 0x80000000) && (m_control_data & 0x80000000))
	{
		bsmt2000_device *bsmt = machine().device<bsmt2000_device>("bsmt");
		bsmt->reset();
	}

	/* log any unknown bits */
	if (data & 0x4f1fffff)
		logerror("%08X: control_w = %08X & %08X\n", space.device().safe_pcbase(), data, mem_mask);
}



/*************************************
 *
 *  BSMT2000 I/O
 *
 *************************************/

WRITE32_MEMBER(policetr_state::policetr_bsmt2000_reg_w)
{
	if (m_control_data & 0x80000000)
		machine().device<bsmt2000_device>("bsmt")->write_data(data);
	else
		COMBINE_DATA(&m_bsmt_data_offset);
}


WRITE32_MEMBER(policetr_state::policetr_bsmt2000_data_w)
{
	machine().device<bsmt2000_device>("bsmt")->write_reg(data);
	COMBINE_DATA(&m_bsmt_data_bank);
}


CUSTOM_INPUT_MEMBER(policetr_state::bsmt_status_r)
{
	return machine().device<bsmt2000_device>("bsmt")->read_status();
}


READ32_MEMBER(policetr_state::bsmt2000_data_r)
{
	return memregion("bsmt")->base()[m_bsmt_data_bank * 0x10000 + m_bsmt_data_offset] << 8;
}



/*************************************
 *
 *  Busy loop optimization
 *
 *************************************/

WRITE32_MEMBER(policetr_state::speedup_w)
{
	COMBINE_DATA(m_speedup_data);

	/* see if the PC matches */
	if ((space.device().safe_pcbase() & 0x1fffffff) == m_speedup_pc)
	{
		UINT64 curr_cycles = m_maincpu->total_cycles();

		/* if less than 50 cycles from the last time, count it */
		if (curr_cycles - m_last_cycles < 50)
		{
			m_loop_count++;

			/* more than 2 in a row and we spin */
			if (m_loop_count > 2)
				space.device().execute().spin_until_interrupt();
		}
		else
			m_loop_count = 0;

		m_last_cycles = curr_cycles;
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( policetr_map, AS_PROGRAM, 32, policetr_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x00200000, 0x0020000f) AM_WRITE(policetr_video_w)
	AM_RANGE(0x00400000, 0x00400003) AM_READ(policetr_video_r)
	AM_RANGE(0x00500000, 0x00500003) AM_WRITENOP        // copies ROM here at startup, plus checksum
	AM_RANGE(0x00600000, 0x00600003) AM_READ(bsmt2000_data_r)
	AM_RANGE(0x00700000, 0x00700003) AM_WRITE(policetr_bsmt2000_reg_w)
	AM_RANGE(0x00800000, 0x00800003) AM_WRITE(policetr_bsmt2000_data_w)
	AM_RANGE(0x00900000, 0x00900003) AM_WRITE(policetr_palette_offset_w)
	AM_RANGE(0x00920000, 0x00920003) AM_WRITE(policetr_palette_data_w)
	AM_RANGE(0x00a00000, 0x00a00003) AM_WRITE(control_w)
	AM_RANGE(0x00a00000, 0x00a00003) AM_READ_PORT("IN0")
	AM_RANGE(0x00a20000, 0x00a20003) AM_READ_PORT("IN1")
	AM_RANGE(0x00a40000, 0x00a40003) AM_READ_PORT("DSW")
	AM_RANGE(0x00e00000, 0x00e00003) AM_WRITENOP        // watchdog???
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sshooter_map, AS_PROGRAM, 32, policetr_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x00200000, 0x00200003) AM_WRITE(policetr_bsmt2000_data_w)
	AM_RANGE(0x00300000, 0x00300003) AM_WRITE(policetr_palette_offset_w)
	AM_RANGE(0x00320000, 0x00320003) AM_WRITE(policetr_palette_data_w)
	AM_RANGE(0x00400000, 0x00400003) AM_READ(policetr_video_r)
	AM_RANGE(0x00500000, 0x00500003) AM_WRITENOP        // copies ROM here at startup, plus checksum
	AM_RANGE(0x00600000, 0x00600003) AM_READ(bsmt2000_data_r)
	AM_RANGE(0x00700000, 0x00700003) AM_WRITE(policetr_bsmt2000_reg_w)
	AM_RANGE(0x00800000, 0x0080000f) AM_WRITE(policetr_video_w)
	AM_RANGE(0x00a00000, 0x00a00003) AM_WRITE(control_w)
	AM_RANGE(0x00a00000, 0x00a00003) AM_READ_PORT("IN0")
	AM_RANGE(0x00a20000, 0x00a20003) AM_READ_PORT("IN1")
	AM_RANGE(0x00a40000, 0x00a40003) AM_READ_PORT("DSW")
	AM_RANGE(0x00e00000, 0x00e00003) AM_WRITENOP        // watchdog???
	AM_RANGE(0x1fc00000, 0x1fcfffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( policetr )
	PORT_START("IN0")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x00200000, IP_ACTIVE_LOW )       /* Not actually a dipswitch */
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, policetr_state,bsmt_status_r, NULL)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) /* EEPROM read */
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x00010000, 0x00010000, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x00020000, 0x00020000, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x00040000, 0x00040000, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x00080000, 0x00080000, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x00100000, 0x00100000, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x00200000, 0x00200000, "SW1:6" )
	PORT_DIPNAME( 0x00400000, 0x00400000, "Monitor Sync") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(          0x00000000, "+")
	PORT_DIPSETTING(          0x00400000, "-")
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8") /* For use with mirrored CRTs - Not supported */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00800000, DEF_STR( On ) )   /* Will invert the Y axis of guns */
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("GUNX1")             /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.012, 0.008, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("GUNY1")             /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.05, 0.002, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("GUNX2")             /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.012, 0.008, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNY2")             /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.05, 0.002, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( polict10 )
	PORT_INCLUDE( policetr )

	PORT_MODIFY("GUNX1")                /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.018, -0.037, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_MODIFY("GUNY1")                /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, -0.033, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_MODIFY("GUNX2")                /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.018, -0.037, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_MODIFY("GUNY2")                /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, -0.033, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( sshoot11 )
	PORT_INCLUDE( policetr )

	PORT_MODIFY("GUNX1")                /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.012, 0.208, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_MODIFY("GUNY1")                /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.093, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_MODIFY("GUNX2")                /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.012, 0.208, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_MODIFY("GUNY2")                /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.093, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( policetr, policetr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R3041, MASTER_CLOCK/2)
	MCFG_R3000_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_CPU_PROGRAM_MAP(policetr_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", policetr_state,  irq4_gen)

	MCFG_EEPROM_SERIAL_93C66_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(400, 262)  /* needs to be verified */
	MCFG_SCREEN_VISIBLE_AREA(0, 393, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(policetr_state, screen_update_policetr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_BSMT2000_ADD("bsmt", MASTER_CLOCK/2)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sshooter, policetr )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sshooter_map)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( policetr ) /* Rev 0.3 PCB , with all chips dated 04/01/97 */
	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "pt-u121.bin", 0x000000, 0x100000, CRC(56b0b00a) SHA1(4034fe373a61f756f4813f0c20b1cf05e4338059) )
	ROM_LOAD16_BYTE( "pt-u120.bin", 0x000001, 0x100000, CRC(ca664142) SHA1(2727ecb9287b4ed30088e017bb6b8763dfb75b2f) )
	ROM_LOAD16_BYTE( "pt-u125.bin", 0x200000, 0x100000, CRC(e9ccf3a0) SHA1(b3fd8c094f76ace4cf403c3d0f6bd6c5d8db7d6a) )
	ROM_LOAD16_BYTE( "pt-u124.bin", 0x200001, 0x100000, CRC(f4acf921) SHA1(5b244e9a51304318fa0c03eb7365b3c12627d19b) )

	ROM_REGION32_BE( 0x80000, "user1", 0 )
	ROM_LOAD32_BYTE( "pt-u113.bin", 0x00000, 0x20000, CRC(7b34d366) SHA1(b86cfe155e0685992aebbcc7db705fdbadc42bf9) )
	ROM_LOAD32_BYTE( "pt-u112.bin", 0x00001, 0x20000, CRC(57d059c8) SHA1(ed0c624fc0afbeb6616bba8a67ce5b18d7c119fc) )
	ROM_LOAD32_BYTE( "pt-u111.bin", 0x00002, 0x20000, CRC(fb5ce933) SHA1(4a07ac3e2d86262061092f112cab89f8660dce3d) )
	ROM_LOAD32_BYTE( "pt-u110.bin", 0x00003, 0x20000, CRC(40bd6f60) SHA1(156000d3c439eab45962f0a2681bd806a17f47ee) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "pt-u160.bin", 0x000000, 0x100000, CRC(f267f813) SHA1(ae58507947fe2e9701b5df46565fd9908e2f9d77) )
	ROM_RELOAD(              0x3f8000, 0x100000 )
	ROM_LOAD( "pt-u162.bin", 0x100000, 0x100000, CRC(75fe850e) SHA1(ab8cf24ae6e5cf80f6a9a34e46f2b1596879643b) )
	ROM_RELOAD(              0x4f8000, 0x100000 )
ROM_END


ROM_START( policetr11 ) /* Rev 0.3 PCB with all chips dated 01/06/97 */
	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "pt-u121.bin", 0x000000, 0x100000, CRC(56b0b00a) SHA1(4034fe373a61f756f4813f0c20b1cf05e4338059) )
	ROM_LOAD16_BYTE( "pt-u120.bin", 0x000001, 0x100000, CRC(ca664142) SHA1(2727ecb9287b4ed30088e017bb6b8763dfb75b2f) )
	ROM_LOAD16_BYTE( "pt-u125.bin", 0x200000, 0x100000, CRC(e9ccf3a0) SHA1(b3fd8c094f76ace4cf403c3d0f6bd6c5d8db7d6a) )
	ROM_LOAD16_BYTE( "pt-u124.bin", 0x200001, 0x100000, CRC(f4acf921) SHA1(5b244e9a51304318fa0c03eb7365b3c12627d19b) )

	ROM_REGION32_BE( 0x80000, "user1", 0 )  /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "pt-u113.v11", 0x00000, 0x20000, CRC(3d62f6d6) SHA1(342ffa38a6972bbb03c89b4dd603c2cc60609d3d) )
	ROM_LOAD32_BYTE( "pt-u112.v11", 0x00001, 0x20000, CRC(942b280b) SHA1(c342ba3255203ce28ff59479da00f26f0bd026e0) )
	ROM_LOAD32_BYTE( "pt-u111.v11", 0x00002, 0x20000, CRC(da6c45a7) SHA1(471bd372d2ad5bcb29af19dae09f3cfab4b010fd) )
	ROM_LOAD32_BYTE( "pt-u110.v11", 0x00003, 0x20000, CRC(f1c8a8c0) SHA1(8a2d1ada002be6f2a3c2d21d193e7cde6531545a) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "pt-u160.bin", 0x000000, 0x100000, CRC(f267f813) SHA1(ae58507947fe2e9701b5df46565fd9908e2f9d77) )
	ROM_RELOAD(              0x3f8000, 0x100000 )
	ROM_LOAD( "pt-u162.bin", 0x100000, 0x100000, CRC(75fe850e) SHA1(ab8cf24ae6e5cf80f6a9a34e46f2b1596879643b) )
	ROM_RELOAD(              0x4f8000, 0x100000 )
ROM_END


ROM_START( policetr10 ) /* Rev 0.2 PCB with all chips dated 10/07/96 */
	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	/* Same data as the other sets, but split in 4 meg roms */
	ROM_LOAD16_BYTE( "pt-u121.v10", 0x000000, 0x080000, CRC(9d31e805) SHA1(482f38e07ddb758e1fb444af7b56a0ef6ea945c8) )
	ROM_LOAD16_BYTE( "pt-u120.v10", 0x000001, 0x080000, CRC(b03b9d46) SHA1(2bb8fcb1df09aa762b98adf2e1edd186203746c0) )
	ROM_LOAD16_BYTE( "pt-u123.v10", 0x100000, 0x080000, CRC(80557cf1) SHA1(ba96fd5b6673b382013e1810a36edb827caaff4b) )
	ROM_LOAD16_BYTE( "pt-u122.v10", 0x100001, 0x080000, CRC(eca09f41) SHA1(bbb1466d39c09598899a3f50b3bb8f9d58b274ec) )
	ROM_LOAD16_BYTE( "pt-u125.v10", 0x200000, 0x080000, CRC(24bddc51) SHA1(6d7c85dba47c675c65e1cb751d581af0d2c678ad) )
	ROM_LOAD16_BYTE( "pt-u124.v10", 0x200001, 0x080000, CRC(f1a43dee) SHA1(2c0aa894e148315168239c7df391ef1f2b4d32a1) )
	ROM_LOAD16_BYTE( "pt-u127.v10", 0x300000, 0x080000, CRC(5031ea1e) SHA1(c1f9272f9874150d510f22c44c186fad0ed3a7e4) )
	ROM_LOAD16_BYTE( "pt-u126.v10", 0x300001, 0x080000, CRC(33bf2653) SHA1(357da2da7df417109adbf600f3455c224f6c076f) )

	ROM_REGION32_BE( 0x80000, "user1", 0 )  /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "pt-u113.v10", 0x00000, 0x20000, CRC(3e27a0ce) SHA1(0d010da68f950a10a74eddc57941e4c0e2144071) )
	ROM_LOAD32_BYTE( "pt-u112.v10", 0x00001, 0x20000, CRC(fcbcf4ca) SHA1(374291600043cfbbd87260b12961ac6d68caeda0) )
	ROM_LOAD32_BYTE( "pt-u111.v10", 0x00002, 0x20000, CRC(61f79667) SHA1(25298cd8706b5c59f7c9e0f8d44db0df73c23403) )
	ROM_LOAD32_BYTE( "pt-u110.v10", 0x00003, 0x20000, CRC(5c3c1548) SHA1(aab977274ecff7cb5fd540a3d0da7940e9707906) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	/* Same data as the other sets, but split in 4 meg roms */
	ROM_LOAD( "pt-u160.v10", 0x000000, 0x080000, CRC(cd374405) SHA1(e53689d4344c78c3faac22747ada28bc3add8c56) )
	ROM_RELOAD(              0x3f8000, 0x080000 )
	ROM_LOAD( "pt-u161.v10", 0x080000, 0x080000, CRC(c33e3497) SHA1(a7d488f04bba3f1b884b0df210c3793f41967d73) )
	ROM_RELOAD(              0x478000, 0x080000 )
	ROM_LOAD( "pt-u162.v10", 0x100000, 0x080000, CRC(e7e02312) SHA1(ac92b8615b18528820a40dad025173e9f24072bf) )
	ROM_RELOAD(              0x4f8000, 0x080000 )
	ROM_LOAD( "pt-u163.v10", 0x180000, 0x080000, CRC(a45b3f85) SHA1(21965dcf89e04d5ee21e27eefd6baa34d6d4479a) )
	ROM_RELOAD(              0x578000, 0x080000 )
ROM_END


ROM_START( policetr13a ) /* Rev 0.5B PCB , unknown program rom date. Actual version is V1.3B */
	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "pt-u121.bin", 0x000000, 0x100000, CRC(56b0b00a) SHA1(4034fe373a61f756f4813f0c20b1cf05e4338059) )
	ROM_LOAD16_BYTE( "pt-u120.bin", 0x000001, 0x100000, CRC(ca664142) SHA1(2727ecb9287b4ed30088e017bb6b8763dfb75b2f) )
	ROM_LOAD16_BYTE( "pt-u125.bin", 0x200000, 0x100000, CRC(e9ccf3a0) SHA1(b3fd8c094f76ace4cf403c3d0f6bd6c5d8db7d6a) )
	ROM_LOAD16_BYTE( "pt-u124.bin", 0x200001, 0x100000, CRC(f4acf921) SHA1(5b244e9a51304318fa0c03eb7365b3c12627d19b) )

	ROM_REGION32_BE( 0x100000, "user1", 0 ) /* Program roms are type 27C020 */
/*
Note: With this version, the program roms are twice the size of those found on all other Police Trainer sets. Like the set listed below,
      if you set the dipswitch to service mode and reset the game within Mame. All 4 program ROMs fail the checksum code and the listed
      checksums on the screen match the set below.  IE: U110=556D, U111=E5F1, U112=974C & U113=CB73

      However, if you check the Diagnostics screen, the program rom checksum is 6819480C which is different then the set below. So it
      looks like it's checking the extra code.  The roms do not contain identical halves, so it's unknown what the "new" data is or does.

      This set has also been found using mask roms for the program roms which would indicate it was the final version.
*/
	ROM_LOAD32_BYTE( "pt-av13.u113", 0x00000, 0x40000, CRC(909c052d) SHA1(23bd4849261ee5cc2414a4043ee929ccf1bd6806) ) /* Checksum printed on label  FB46 */
	ROM_LOAD32_BYTE( "pt-av13.u112", 0x00001, 0x40000, CRC(f9dc9ca8) SHA1(52de7bc8c9aa7834d953b9f9e2a65e06f8042f0a) ) /* Checksum printed on label  201D */
	ROM_LOAD32_BYTE( "pt-av13.u111", 0x00002, 0x40000, CRC(8c4f3a64) SHA1(4953e6fc26bae7d6e7c7230f4ca76e3f5032af14) ) /* Checksum printed on label  F343 */
	ROM_LOAD32_BYTE( "pt-av13.u110", 0x00003, 0x40000, CRC(738a8277) SHA1(423a9bcecb82959f38ae79a0728d72eb13ed93b3) ) /* Checksum printed on label  050C */

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "pt-u160.bin", 0x000000, 0x100000, CRC(f267f813) SHA1(ae58507947fe2e9701b5df46565fd9908e2f9d77) )
	ROM_RELOAD(              0x3f8000, 0x100000 )
	ROM_LOAD( "pt-u162.bin", 0x100000, 0x100000, CRC(75fe850e) SHA1(ab8cf24ae6e5cf80f6a9a34e46f2b1596879643b) )
	ROM_RELOAD(              0x4f8000, 0x100000 )
ROM_END


ROM_START( policetr13b ) /* Rev 0.5B PCB , unknown program rom date Actual version is V1.3B */
	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "pt-u121.bin", 0x000000, 0x100000, CRC(56b0b00a) SHA1(4034fe373a61f756f4813f0c20b1cf05e4338059) )
	ROM_LOAD16_BYTE( "pt-u120.bin", 0x000001, 0x100000, CRC(ca664142) SHA1(2727ecb9287b4ed30088e017bb6b8763dfb75b2f) )
	ROM_LOAD16_BYTE( "pt-u125.bin", 0x200000, 0x100000, CRC(e9ccf3a0) SHA1(b3fd8c094f76ace4cf403c3d0f6bd6c5d8db7d6a) )
	ROM_LOAD16_BYTE( "pt-u124.bin", 0x200001, 0x100000, CRC(f4acf921) SHA1(5b244e9a51304318fa0c03eb7365b3c12627d19b) )

	ROM_REGION32_BE( 0x100000, "user1", 0 ) /* Program roms are type 27C010 */
/*
Note: If you set the dipswitch to service mode and reset the game within Mame. All 4 program ROMs fail the checksum code, IE: they
      show in red instead of green.  But, the listed checksums on the screen match the checksums printed on the ROM labels. However,
      this has been verified to happen on a real PCB

      The program rom checksum in the diagnostic screen is 17551773
*/
	ROM_LOAD32_BYTE( "ptb-u113.v13", 0x00000, 0x20000, CRC(d636c00d) SHA1(ef989eb85b51a64ca640297c1286514c8d7f8f76) ) /* Checksum printed on label  CB73 */
	ROM_LOAD32_BYTE( "ptb-u112.v13", 0x00001, 0x20000, CRC(86f0497e) SHA1(d177023f7cb2e01de60ef072212836dc94759c1a) ) /* Checksum printed on label  974C */
	ROM_LOAD32_BYTE( "ptb-u111.v13", 0x00002, 0x20000, CRC(39e96d6a) SHA1(efe6ffe70432b94c98f3d7247408a6d2f6f9e33d) ) /* Checksum printed on label  E5F1 */
	ROM_LOAD32_BYTE( "ptb-u110.v13", 0x00003, 0x20000, CRC(d7e6f4cb) SHA1(9dffe4937bc5cf47d870f06ae0dced362cd2dd66) ) /* Checksum printed on label  556D */

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "pt-u160.bin", 0x000000, 0x100000, CRC(f267f813) SHA1(ae58507947fe2e9701b5df46565fd9908e2f9d77) )
	ROM_RELOAD(              0x3f8000, 0x100000 )
	ROM_LOAD( "pt-u162.bin", 0x100000, 0x100000, CRC(75fe850e) SHA1(ab8cf24ae6e5cf80f6a9a34e46f2b1596879643b) )
	ROM_RELOAD(              0x4f8000, 0x100000 )
ROM_END


ROM_START( sshooter ) /* Rev 0.5B PCB , Added a "Welcome" start-up screen which shows "This is Version C191012" */
	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 ) /* Graphics v1.0 */
	ROM_LOAD16_BYTE( "ss-u121.bin", 0x000000, 0x100000, CRC(22e27dd6) SHA1(cb9e8c450352bb116a9c0407cc8ce6d8ae9d9881) ) // 1:1
	ROM_LOAD16_BYTE( "ss-u120.bin", 0x000001, 0x100000, CRC(30173b1b) SHA1(366464444ce208391ca350f1639403f0c2217330) ) // 1:2
	ROM_LOAD16_BYTE( "ss-u125.bin", 0x200000, 0x100000, CRC(79e8520a) SHA1(682e5c7954f96db65a137f05cde67c310b85b526) ) // 2:1
	ROM_LOAD16_BYTE( "ss-u124.bin", 0x200001, 0x100000, CRC(8e805970) SHA1(bfc9940ed6425f136d768170275279c590da7003) ) // 2:2
	ROM_LOAD16_BYTE( "ss-u123.bin", 0x400000, 0x100000, CRC(d045bb62) SHA1(839209ff6a8e5db63a51a3494a6c973e0068a3c6) ) // 3:1
	ROM_LOAD16_BYTE( "ss-u122.bin", 0x400001, 0x100000, CRC(163cc133) SHA1(a5e84b5060fd32362aa097d0194ce72e8a90357c) ) // 3:2
	ROM_LOAD16_BYTE( "ss-u127.bin", 0x600000, 0x100000, CRC(76a7a591) SHA1(9fd7cce21b01f388966a3e8388ba95820ac10bfd) ) // 4:1
	ROM_LOAD16_BYTE( "ss-u126.bin", 0x600001, 0x100000, CRC(ab1b9d60) SHA1(ff51a71443f7774d3abf96c2eb8ef6a54d73dd8e) ) // 4:2

	ROM_REGION32_BE( 0x100000, "user1", 0 )
	ROM_LOAD32_BYTE( "ss-u113.v19", 0x00000, 0x40000, CRC(de536a90) SHA1(76f0e0e2457d91b3c1bd2b3501591646a18db348) ) // 1:1
	ROM_LOAD32_BYTE( "ss-u112.v19", 0x00001, 0x40000, CRC(2e4e1837) SHA1(b4088269e1e7a3913d2841eb24f53b1c413cd0cc) ) // 1:2
	ROM_LOAD32_BYTE( "ss-u111.v19", 0x00002, 0x40000, CRC(485d03e8) SHA1(ebdf166b2354b318e6bfb68e0fb5647381b9c405) ) // 1:3
	ROM_LOAD32_BYTE( "ss-u110.v19", 0x00003, 0x40000, CRC(df6a0a45) SHA1(a73a9dcdc669c6e61a5983f3b2a2721fe1b35f34) ) // 1:4

	ROM_REGION( 0x1000000, "bsmt", 0 ) /* Sound v1.2 */
	ROM_LOAD( "ss-u160.bin", 0x000000, 0x100000, CRC(1c603d42) SHA1(880992871be52129684052d542946de0cc32ba9a) ) // 1:1
	ROM_RELOAD(              0x3f8000, 0x100000 )
	ROM_LOAD( "ss-u162.bin", 0x100000, 0x100000, CRC(40ef448a) SHA1(c96f7b169be2576e9f3783af84c07259efefb812) ) // 2:1
	ROM_RELOAD(              0x4f8000, 0x100000 )
ROM_END


ROM_START( sshooter17 ) /* Rev 0.5B PCB , unknown program rom date */
	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 ) /* Graphics v1.0 */
	ROM_LOAD16_BYTE( "ss-u121.bin", 0x000000, 0x100000, CRC(22e27dd6) SHA1(cb9e8c450352bb116a9c0407cc8ce6d8ae9d9881) ) // 1:1
	ROM_LOAD16_BYTE( "ss-u120.bin", 0x000001, 0x100000, CRC(30173b1b) SHA1(366464444ce208391ca350f1639403f0c2217330) ) // 1:2
	ROM_LOAD16_BYTE( "ss-u125.bin", 0x200000, 0x100000, CRC(79e8520a) SHA1(682e5c7954f96db65a137f05cde67c310b85b526) ) // 2:1
	ROM_LOAD16_BYTE( "ss-u124.bin", 0x200001, 0x100000, CRC(8e805970) SHA1(bfc9940ed6425f136d768170275279c590da7003) ) // 2:2
	ROM_LOAD16_BYTE( "ss-u123.bin", 0x400000, 0x100000, CRC(d045bb62) SHA1(839209ff6a8e5db63a51a3494a6c973e0068a3c6) ) // 3:1
	ROM_LOAD16_BYTE( "ss-u122.bin", 0x400001, 0x100000, CRC(163cc133) SHA1(a5e84b5060fd32362aa097d0194ce72e8a90357c) ) // 3:2
	ROM_LOAD16_BYTE( "ss-u127.bin", 0x600000, 0x100000, CRC(76a7a591) SHA1(9fd7cce21b01f388966a3e8388ba95820ac10bfd) ) // 4:1
	ROM_LOAD16_BYTE( "ss-u126.bin", 0x600001, 0x100000, CRC(ab1b9d60) SHA1(ff51a71443f7774d3abf96c2eb8ef6a54d73dd8e) ) // 4:2

	ROM_REGION32_BE( 0x100000, "user1", 0 )
	ROM_LOAD32_BYTE( "ss-u113.v17", 0x00000, 0x40000, CRC(a8c96af5) SHA1(a62458156603b74e0d84ce6928f7bb868bf5a219) ) // 1:1
	ROM_LOAD32_BYTE( "ss-u112.v17", 0x00001, 0x40000, CRC(c732d5fa) SHA1(2bcc26c8bbf55394173ca65b4b0df01bc6b719bb) ) // 1:2
	ROM_LOAD32_BYTE( "ss-u111.v17", 0x00002, 0x40000, CRC(4240fa2f) SHA1(54223207c1e228d6b836918601c0f65c2692e5bc) ) // 1:3
	ROM_LOAD32_BYTE( "ss-u110.v17", 0x00003, 0x40000, CRC(8ae744ce) SHA1(659cd27865cf5507aae6b064c5bc24b927cf5f5a) ) // 1:4

	ROM_REGION( 0x1000000, "bsmt", 0 ) /* Sound v1.2 */
	ROM_LOAD( "ss-u160.bin", 0x000000, 0x100000, CRC(1c603d42) SHA1(880992871be52129684052d542946de0cc32ba9a) ) // 1:1
	ROM_RELOAD(              0x3f8000, 0x100000 )
	ROM_LOAD( "ss-u162.bin", 0x100000, 0x100000, CRC(40ef448a) SHA1(c96f7b169be2576e9f3783af84c07259efefb812) ) // 2:1
	ROM_RELOAD(              0x4f8000, 0x100000 )
ROM_END


ROM_START( sshooter12 ) /* Rev 0.5B PCB , program roms dated 04/17/98 */
	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 ) /* Graphics v1.0 */
	ROM_LOAD16_BYTE( "ss-u121.bin", 0x000000, 0x100000, CRC(22e27dd6) SHA1(cb9e8c450352bb116a9c0407cc8ce6d8ae9d9881) ) // 1:1
	ROM_LOAD16_BYTE( "ss-u120.bin", 0x000001, 0x100000, CRC(30173b1b) SHA1(366464444ce208391ca350f1639403f0c2217330) ) // 1:2
	ROM_LOAD16_BYTE( "ss-u125.bin", 0x200000, 0x100000, CRC(79e8520a) SHA1(682e5c7954f96db65a137f05cde67c310b85b526) ) // 2:1
	ROM_LOAD16_BYTE( "ss-u124.bin", 0x200001, 0x100000, CRC(8e805970) SHA1(bfc9940ed6425f136d768170275279c590da7003) ) // 2:2
	ROM_LOAD16_BYTE( "ss-u123.bin", 0x400000, 0x100000, CRC(d045bb62) SHA1(839209ff6a8e5db63a51a3494a6c973e0068a3c6) ) // 3:1
	ROM_LOAD16_BYTE( "ss-u122.bin", 0x400001, 0x100000, CRC(163cc133) SHA1(a5e84b5060fd32362aa097d0194ce72e8a90357c) ) // 3:2
	ROM_LOAD16_BYTE( "ss-u127.bin", 0x600000, 0x100000, CRC(76a7a591) SHA1(9fd7cce21b01f388966a3e8388ba95820ac10bfd) ) // 4:1
	ROM_LOAD16_BYTE( "ss-u126.bin", 0x600001, 0x100000, CRC(ab1b9d60) SHA1(ff51a71443f7774d3abf96c2eb8ef6a54d73dd8e) ) // 4:2

	ROM_REGION32_BE( 0x100000, "user1", 0 )
	ROM_LOAD32_BYTE( "ss-u113.v12", 0x00000, 0x40000, CRC(73dbaf4b) SHA1(a85fad95d63333f4fe5647f31258b3a22c5c2c0d) ) // 1:1
	ROM_LOAD32_BYTE( "ss-u112.v12", 0x00001, 0x40000, CRC(06fbc2de) SHA1(8bdfcbc33b5fc010464dcd7691f9ecd6ba2168ba) ) // 1:2
	ROM_LOAD32_BYTE( "ss-u111.v12", 0x00002, 0x40000, CRC(0b291731) SHA1(bd04f0b1b52198344df625fcddfc6c6ccb0bd923) ) // 1:3
	ROM_LOAD32_BYTE( "ss-u110.v12", 0x00003, 0x40000, CRC(76841008) SHA1(ccbb88c8d63bf929814144a9d8757c9c7048fdef) ) // 1:4

	ROM_REGION( 0x1000000, "bsmt", 0 ) /* Sound v1.2 */
	ROM_LOAD( "ss-u160.bin", 0x000000, 0x100000, CRC(1c603d42) SHA1(880992871be52129684052d542946de0cc32ba9a) ) // 1:1
	ROM_RELOAD(              0x3f8000, 0x100000 )
	ROM_LOAD( "ss-u162.bin", 0x100000, 0x100000, CRC(40ef448a) SHA1(c96f7b169be2576e9f3783af84c07259efefb812) ) // 2:1
	ROM_RELOAD(              0x4f8000, 0x100000 )
ROM_END


ROM_START( sshooter11 ) /* Rev 0.5B PCB , program roms dated 04/03/98 */
	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 ) /* Graphics v1.0 */
	ROM_LOAD16_BYTE( "ss-u121.bin", 0x000000, 0x100000, CRC(22e27dd6) SHA1(cb9e8c450352bb116a9c0407cc8ce6d8ae9d9881) ) // 1:1
	ROM_LOAD16_BYTE( "ss-u120.bin", 0x000001, 0x100000, CRC(30173b1b) SHA1(366464444ce208391ca350f1639403f0c2217330) ) // 1:2
	ROM_LOAD16_BYTE( "ss-u125.bin", 0x200000, 0x100000, CRC(79e8520a) SHA1(682e5c7954f96db65a137f05cde67c310b85b526) ) // 2:1
	ROM_LOAD16_BYTE( "ss-u124.bin", 0x200001, 0x100000, CRC(8e805970) SHA1(bfc9940ed6425f136d768170275279c590da7003) ) // 2:2
	ROM_LOAD16_BYTE( "ss-u123.bin", 0x400000, 0x100000, CRC(d045bb62) SHA1(839209ff6a8e5db63a51a3494a6c973e0068a3c6) ) // 3:1
	ROM_LOAD16_BYTE( "ss-u122.bin", 0x400001, 0x100000, CRC(163cc133) SHA1(a5e84b5060fd32362aa097d0194ce72e8a90357c) ) // 3:2
	ROM_LOAD16_BYTE( "ss-u127.bin", 0x600000, 0x100000, CRC(76a7a591) SHA1(9fd7cce21b01f388966a3e8388ba95820ac10bfd) ) // 4:1
	ROM_LOAD16_BYTE( "ss-u126.bin", 0x600001, 0x100000, CRC(ab1b9d60) SHA1(ff51a71443f7774d3abf96c2eb8ef6a54d73dd8e) ) // 4:2

	ROM_REGION32_BE( 0x100000, "user1", 0 )
	ROM_LOAD32_BYTE( "ss-u113.v11", 0x00000, 0x40000, CRC(c19693f3) SHA1(2f1576261f741d5e69d30f645aea0ed359b8dc03) ) // 1:1
	ROM_LOAD32_BYTE( "ss-u112.v11", 0x00001, 0x40000, CRC(a5ab6d82) SHA1(b2cc3fd875f0c6702cee973b77fd608f4cfe0555) ) // 1:2
	ROM_LOAD32_BYTE( "ss-u111.v11", 0x00002, 0x40000, CRC(ec209b5f) SHA1(1408b509853b325e865d0b23d237bca321e73f60) ) // 1:3
	ROM_LOAD32_BYTE( "ss-u110.v11", 0x00003, 0x40000, CRC(0f1de201) SHA1(5001de3349357545a6a45102340caf0008b50d7b) ) // 1:4

	ROM_REGION( 0x1000000, "bsmt", 0 ) /* Sound v1.2 */
	ROM_LOAD( "ss-u160.bin", 0x000000, 0x100000, CRC(1c603d42) SHA1(880992871be52129684052d542946de0cc32ba9a) ) // 1:1
	ROM_RELOAD(              0x3f8000, 0x100000 )
	ROM_LOAD( "ss-u162.bin", 0x100000, 0x100000, CRC(40ef448a) SHA1(c96f7b169be2576e9f3783af84c07259efefb812) ) // 2:1
	ROM_RELOAD(              0x4f8000, 0x100000 )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(policetr_state,policetr)
{
	m_speedup_data = m_maincpu->space(AS_PROGRAM).install_write_handler(0x00000fc8, 0x00000fcb, write32_delegate(FUNC(policetr_state::speedup_w),this));
	m_speedup_pc = 0x1fc028ac;
}

DRIVER_INIT_MEMBER(policetr_state,plctr13b)
{
	m_speedup_data = m_maincpu->space(AS_PROGRAM).install_write_handler(0x00000fc8, 0x00000fcb, write32_delegate(FUNC(policetr_state::speedup_w),this));
	m_speedup_pc = 0x1fc028bc;
}


DRIVER_INIT_MEMBER(policetr_state,sshooter)
{
	m_speedup_data = m_maincpu->space(AS_PROGRAM).install_write_handler(0x00018fd8, 0x00018fdb, write32_delegate(FUNC(policetr_state::speedup_w),this));
	m_speedup_pc = 0x1fc03470;
}

DRIVER_INIT_MEMBER(policetr_state,sshoot12)
{
	m_speedup_data = m_maincpu->space(AS_PROGRAM).install_write_handler(0x00018fd8, 0x00018fdb, write32_delegate(FUNC(policetr_state::speedup_w),this));
	m_speedup_pc = 0x1fc033e0;
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1996, policetr,    0,        policetr, policetr, policetr_state, policetr, ROT0, "P&P Marketing", "Police Trainer (Rev 1.3)", 0 )
GAME( 1996, policetr11,  policetr, policetr, polict10, policetr_state, policetr, ROT0, "P&P Marketing", "Police Trainer (Rev 1.1)", 0 )
GAME( 1996, policetr10,  policetr, policetr, polict10, policetr_state, policetr, ROT0, "P&P Marketing", "Police Trainer (Rev 1.0)", 0 )

GAME( 1996, policetr13a, policetr, sshooter, policetr, policetr_state, plctr13b, ROT0, "P&P Marketing", "Police Trainer (Rev 1.3B Newer)", 0 )
GAME( 1996, policetr13b, policetr, sshooter, policetr, policetr_state, plctr13b, ROT0, "P&P Marketing", "Police Trainer (Rev 1.3B)", 0 )

GAME( 1998, sshooter,    0,        sshooter, policetr, policetr_state, sshooter, ROT0, "P&P Marketing", "Sharpshooter (Rev 1.9)", 0 )
GAME( 1998, sshooter17,  sshooter, sshooter, policetr, policetr_state, sshooter, ROT0, "P&P Marketing", "Sharpshooter (Rev 1.7)", 0 )
GAME( 1998, sshooter12,  sshooter, sshooter, sshoot11, policetr_state, sshoot12, ROT0, "P&P Marketing", "Sharpshooter (Rev 1.2)", 0 )
GAME( 1998, sshooter11,  sshooter, sshooter, sshoot11, policetr_state, sshoot12, ROT0, "P&P Marketing", "Sharpshooter (Rev 1.1)", 0 )
