// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Over Drive (GX789) (c) 1990 Konami

    driver by Nicola Salmoria

    Notes:
    - irq source for main CPU aren't understood, needs HW tests.
    - Missing road (two unemulated K053250)
    - Visible area and relative placement of sprites and tiles is most likely wrong.
    - Test mode doesn't work well with 3 IRQ5 per frame, the ROM check doesn't work
      and the coin A setting isn't shown. It's OK with 1 IRQ5 per frame.
    - The "Continue?" sprites are not visible until you press start
    - priorities


    The issues below are both IRQ timing, and relate to when the sprites get
    copied across by the DMA
        - Some flickering sprites, this might be an interrupt/timing issue
        - The screen is cluttered with sprites which aren't supposed to be visible,
          increasing the coordinate mask in k053247_sprites_draw() from 0x3ff to 0xfff
          fixes this but breaks other games (e.g. Vendetta).


***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/k053250.h"
#include "machine/eepromser.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "sound/k053260.h"
#include "includes/overdriv.h"

#include "overdriv.lh"

/***************************************************************************

  EEPROM

***************************************************************************/

static const UINT16 overdriv_default_eeprom[64] =
{
	0x7758,0xFFFF,0x0078,0x9000,0x0078,0x7000,0x0078,0x5000,
	0x5441,0x4B51,0x3136,0x4655,0x4AFF,0x0300,0x0270,0x0250,
	0x00B4,0x0300,0xB403,0x00B4,0x0300,0xB403,0x00B4,0x0300,
	0xB403,0x00B4,0x0300,0xB403,0x00B4,0x0300,0xB403,0x00B4,
	0x0300,0xB403,0x00B4,0x0300,0xB403,0x00B4,0x0300,0xB403,
	0x00B4,0x0300,0xB403,0x00B4,0x0300,0xB403,0x00B4,0x0300,
	0xB403,0x00B4,0x0300,0xB403,0x00B4,0x0300,0xB403,0x00B4,
	0x0300,0xB403,0x00B4,0x0300,0xB403,0x00B4,0x0300,0xB403
};


WRITE16_MEMBER(overdriv_state::eeprom_w)
{
//logerror("%06x: write %04x to eeprom_w\n",space.device().safe_pc(),data);
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 is data */
		/* bit 1 is clock (active high) */
		/* bit 2 is cs (active low) */
		ioport("EEPROMOUT")->write(data, 0xff);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(overdriv_state::overdriv_cpuA_scanline)
{
	int scanline = param;

	/* TODO: irqs routines are TOO slow right now, it ends up firing spurious irqs for whatever reason (shared ram fighting?) */
	/*       this is a temporary solution to get rid of deprecat lib and the crashes, but also makes the game timer to be too slow */
	if(scanline == 256 && m_screen->frame_number() & 1) // vblank-out irq
		m_maincpu->set_input_line(4, HOLD_LINE);
	else if((scanline % 128) == 0) // timer irq
		m_maincpu->set_input_line(5, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(overdriv_state::cpuB_interrupt)
{
	// this doesn't get turned on until the irq has happened? wrong irq?
//  if (m_k053246->k053246_is_irq_enabled())
	m_subcpu->set_input_line(4, HOLD_LINE); // likely wrong
}


WRITE16_MEMBER(overdriv_state::cpuA_ctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 probably enables the second 68000 */
		m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);

		/* bit 1 is clear during service mode - function unknown */

		output().set_led_value(0, data & 0x08);
		machine().bookkeeping().coin_counter_w(0, data & 0x10);
		machine().bookkeeping().coin_counter_w(1, data & 0x20);

//logerror("%06x: write %04x to cpuA_ctrl_w\n",space.device().safe_pc(),data);
	}
}

READ16_MEMBER(overdriv_state::cpuB_ctrl_r)
{
	return m_cpuB_ctrl;
}

WRITE16_MEMBER(overdriv_state::cpuB_ctrl_w)
{
	COMBINE_DATA(&m_cpuB_ctrl);

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 = enable sprite ROM reading */
		m_k053246->k053246_set_objcha_line( (data & 0x01) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 1 used but unknown (irq enable?) */

		/* other bits unused? */
	}
}

WRITE16_MEMBER(overdriv_state::overdriv_soundirq_w)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
}

WRITE16_MEMBER(overdriv_state::overdriv_cpuB_irq_x_w)
{
	m_subcpu->set_input_line(5, HOLD_LINE); // likely wrong
}

WRITE16_MEMBER(overdriv_state::overdriv_cpuB_irq_y_w)
{
	m_subcpu->set_input_line(6, HOLD_LINE); // likely wrong
}

static ADDRESS_MAP_START( overdriv_master_map, AS_PROGRAM, 16, overdriv_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x043fff) AM_RAM                 /* work RAM */
	AM_RANGE(0x080000, 0x080fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ_PORT("INPUTS")
	AM_RANGE(0x0c0002, 0x0c0003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0e0000, 0x0e0001) AM_WRITENOP            /* unknown (always 0x30) */
	AM_RANGE(0x100000, 0x10001f) AM_DEVREADWRITE8("k053252", k053252_device, read, write, 0x00ff) /* 053252? (LSB) */
	AM_RANGE(0x140000, 0x140001) AM_WRITENOP //watchdog reset?
	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("PADDLE") AM_WRITENOP  // writes 0 at POST and expect that motor busy flag is off, then checks if paddle is at center otherwise throws a "VOLUME ERROR".
	AM_RANGE(0x1c0000, 0x1c001f) AM_DEVWRITE8("k051316_1", k051316_device, ctrl_w, 0xff00)
	AM_RANGE(0x1c8000, 0x1c801f) AM_DEVWRITE8("k051316_2", k051316_device, ctrl_w, 0xff00)
	AM_RANGE(0x1d0000, 0x1d001f) AM_DEVWRITE("k053251", k053251_device, msb_w)
	AM_RANGE(0x1d8000, 0x1d8003) AM_DEVREADWRITE8("k053260_1", k053260_device, main_read, main_write, 0x00ff)
	AM_RANGE(0x1e0000, 0x1e0003) AM_DEVREADWRITE8("k053260_2", k053260_device, main_read, main_write, 0x00ff)
	AM_RANGE(0x1e8000, 0x1e8001) AM_WRITE(overdriv_soundirq_w)
	AM_RANGE(0x1f0000, 0x1f0001) AM_WRITE(cpuA_ctrl_w)  /* halt cpu B, coin counter, start lamp, other? */
	AM_RANGE(0x1f8000, 0x1f8001) AM_WRITE(eeprom_w)
	AM_RANGE(0x200000, 0x203fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x210000, 0x210fff) AM_DEVREADWRITE8("k051316_1", k051316_device, read, write, 0xff00)
	AM_RANGE(0x218000, 0x218fff) AM_DEVREADWRITE8("k051316_2", k051316_device, read, write, 0xff00)
	AM_RANGE(0x220000, 0x220fff) AM_DEVREAD8("k051316_1", k051316_device, rom_r, 0xff00)
	AM_RANGE(0x228000, 0x228fff) AM_DEVREAD8("k051316_2", k051316_device, rom_r, 0xff00)
	AM_RANGE(0x230000, 0x230001) AM_WRITE(overdriv_cpuB_irq_y_w)
	AM_RANGE(0x238000, 0x238001) AM_WRITE(overdriv_cpuB_irq_x_w)
ADDRESS_MAP_END

// HACK ALERT
WRITE16_MEMBER( overdriv_state::overdriv_k053246_word_w )
{
	m_k053246->k053246_word_w(space,offset,data,mem_mask);

	UINT16 *src, *dst;

	m_k053246->k053247_get_ram(&dst);

	src = m_sprram;

	// this should be the sprite dma/irq bit...
	// but it is already turned off by the time overdriv_state::cpuB_interrupt is executed?
	// even now it rarely gets set, I imagine because the communication / irq is actually
	// worse than we thought. (drive very slowly and things update..)
	if (m_k053246->k053246_is_irq_enabled())
	{
		memcpy(dst,src,0x1000);
	}

	//printf("%02x %04x %04x\n", offset, data, mem_mask);

}

static ADDRESS_MAP_START( overdriv_slave_map, AS_PROGRAM, 16, overdriv_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_RAM /* work RAM */
	AM_RANGE(0x0c0000, 0x0c1fff) AM_RAM //AM_DEVREADWRITE("k053250_1", k053250_device, ram_r, ram_w)
	AM_RANGE(0x100000, 0x10000f) AM_DEVREADWRITE("k053250_1", k053250_device, reg_r, reg_w)
	AM_RANGE(0x108000, 0x10800f) AM_DEVREADWRITE("k053250_2", k053250_device, reg_r, reg_w)
	AM_RANGE(0x118000, 0x118fff) AM_RAM AM_SHARE("sprram") //AM_DEVREADWRITE("k053246", k053247_device, k053247_word_r, k053247_word_w) // data gets copied to sprite chip with DMA..
	AM_RANGE(0x120000, 0x120001) AM_DEVREAD("k053246", k053247_device, k053246_word_r)
	AM_RANGE(0x128000, 0x128001) AM_READWRITE(cpuB_ctrl_r, cpuB_ctrl_w) /* enable K053247 ROM reading, plus something else */
	AM_RANGE(0x130000, 0x130007) AM_WRITE(overdriv_k053246_word_w) // AM_DEVWRITE("k053246", k053247_device, k053246_word_w)
	AM_RANGE(0x200000, 0x203fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x208000, 0x20bfff) AM_RAM
	AM_RANGE(0x218000, 0x219fff) AM_DEVREAD("k053250_1", k053250_device, rom_r)
	AM_RANGE(0x220000, 0x221fff) AM_DEVREAD("k053250_2", k053250_device, rom_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( overdriv_sound_map, AS_PROGRAM, 8, overdriv_state )
	AM_RANGE(0x0200, 0x0201) AM_DEVREADWRITE("ymsnd", ym2151_device,read,write)
	AM_RANGE(0x0400, 0x042f) AM_DEVREADWRITE("k053260_1", k053260_device, read, write)
	AM_RANGE(0x0600, 0x062f) AM_DEVREADWRITE("k053260_2", k053260_device, read, write)
	AM_RANGE(0x0800, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* Both IPT_START1 assignments are needed. The game will reset during */
/* the "continue" sequence if the assignment on the first port        */
/* is missing.                                                        */

static INPUT_PORTS_START( overdriv )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )   // motor busy flag

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(50)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
INPUT_PORTS_END


void overdriv_state::machine_start()
{
	save_item(NAME(m_cpuB_ctrl));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_zoom_colorbase));
	save_item(NAME(m_road_colorbase));
}

void overdriv_state::machine_reset()
{
	m_cpuB_ctrl = 0;
	m_sprite_colorbase = 0;
	m_zoom_colorbase[0] = 0;
	m_zoom_colorbase[1] = 0;
	m_road_colorbase[0] = 0;
	m_road_colorbase[1] = 0;

	/* start with cpu B halted */
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


static MACHINE_CONFIG_START( overdriv, overdriv_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)  /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(overdriv_master_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", overdriv_state, overdriv_cpuA_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M68000, XTAL_24MHz/2)  /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(overdriv_slave_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", overdriv_state,  cpuB_interrupt)   /* IRQ 5 and 6 are generated by the main CPU. */
													/* IRQ 5 is used only in test mode, to request the checksums of the gfx ROMs. */

	MCFG_CPU_ADD("audiocpu", M6809, XTAL_3_579545MHz)     /* 1.789 MHz?? This might be the right speed, but ROM testing */
	MCFG_CPU_PROGRAM_MAP(overdriv_sound_map)    /* takes a little too much (the counter wraps from 0000 to 9999). */
												/* This might just mean that the video refresh rate is less than */
												/* 60 fps, that's how I fixed it for now. */

	MCFG_QUANTUM_TIME(attotime::from_hz(12000))

	MCFG_EEPROM_SERIAL_ER5911_16BIT_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DATA(overdriv_default_eeprom, 128)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(64*8, 40*8)
	MCFG_SCREEN_VISIBLE_AREA(13*8, (64-13)*8-1, 0*8, 32*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(overdriv_state, screen_update_overdriv)
	MCFG_SCREEN_PALETTE("palette")

//  MCFG_GFXDECODE_ADD("gfxdecode", "palette", overdriv)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	MCFG_DEVICE_ADD("k053246", K053246, 0)
	MCFG_K053246_CB(overdriv_state, sprite_callback)
	MCFG_K053246_CONFIG("gfx1", 0, NORMAL_PLANE_ORDER, 77, 22)
	MCFG_K053246_GFXDECODE("gfxdecode")
	MCFG_K053246_PALETTE("palette")

	MCFG_DEVICE_ADD("k051316_1", K051316, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051316_OFFSETS(14, -1)
	MCFG_K051316_WRAP(1)
	MCFG_K051316_CB(overdriv_state, zoom_callback_1)

	MCFG_DEVICE_ADD("k051316_2", K051316, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051316_OFFSETS(15, 1)
	MCFG_K051316_CB(overdriv_state, zoom_callback_2)

	MCFG_K053251_ADD("k053251")
	MCFG_K053250_ADD("k053250_1", "palette", "screen", 0, 0)
	MCFG_K053250_ADD("k053250_2", "palette", "screen", 0, 0)

	MCFG_DEVICE_ADD("k053252", K053252, XTAL_24MHz/4)
	MCFG_K053252_OFFSETS(13*8, 2*8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.5)

	MCFG_K053260_ADD("k053260_1", XTAL_3_579545MHz)
	MCFG_K053260_REGION("shared")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.35)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.35)

	MCFG_K053260_ADD("k053260_2", XTAL_3_579545MHz)
	MCFG_K053260_REGION("shared")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.35)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.35)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( overdriv )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "789_n05.d17", 0x00000, 0x20000, CRC(f7885713) SHA1(8e84929dcc6ab889c3e11c450d22c56b183b0198) )
	ROM_LOAD16_BYTE( "789_n04.b17", 0x00001, 0x20000, CRC(aefe87a6) SHA1(1bdf5a1f4c5e2b84d02b2981b3be91ed2406a1f8) )

	ROM_REGION( 0x40000, "sub", 0 )
	ROM_LOAD16_BYTE( "789_e09.l10", 0x00000, 0x20000, CRC(46fb7e88) SHA1(f706a76aff9bec64abe6da325cba0715d6e6ed0a) ) /* also found labeled as "4" as well as "7" */
	ROM_LOAD16_BYTE( "789_e08.k10", 0x00001, 0x20000, CRC(24427195) SHA1(48f4f81729acc0e497b40fddbde11242c5c4c573) ) /* also found labeled as "3" as well as "6" */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "789_e01.e4", 0x00000, 0x10000, CRC(1085f069) SHA1(27228cedb357ff2e130a4bd6d8aa01cf537e034f) ) /* also found labeled as "5" */

	ROM_REGION( 0x400000, "gfx1", 0 )   /* graphics (addressable by the CPU) */
	ROM_LOAD64_WORD( "789e12.r1",  0x000000, 0x100000, CRC(14a10fb2) SHA1(03fb9c15514c5ecc2d9ae4a53961c4bbb49cec73) )    /* sprites */
	ROM_LOAD64_WORD( "789e13.r4",  0x000002, 0x100000, CRC(6314a628) SHA1(f8a8918998c266109348c77427a7696b503daeb3) )
	ROM_LOAD64_WORD( "789e14.r10", 0x000004, 0x100000, CRC(b5eca14b) SHA1(a1c5f5e9cd8bbcfc875e2acb33be024724da63aa) )
	ROM_LOAD64_WORD( "789e15.r15", 0x000006, 0x100000, CRC(5d93e0c3) SHA1(d5cb7666c0c28fd465c860c7f9dbb18a7f739a93) )

	ROM_REGION( 0x020000, "k051316_1", 0 )
	ROM_LOAD( "789e06.a21", 0x000000, 0x020000, CRC(14a085e6) SHA1(86dad6f223e13ff8af7075c3d99bb0a83784c384) )    /* zoom/rotate */

	ROM_REGION( 0x020000, "k051316_2", 0 )
	ROM_LOAD( "789e07.c23", 0x000000, 0x020000, CRC(8a6ceab9) SHA1(1a52b7361f71a6126cd648a76af00223d5b25c7a) )    /* zoom/rotate */

	ROM_REGION( 0x0c0000, "k053250_1", 0 )
	ROM_LOAD( "789e18.p22", 0x000000, 0x040000, CRC(985a4a75) SHA1(b726166c295be6fbec38a9d11098cc4a4a5de456) )
	ROM_LOAD( "789e19.r22", 0x040000, 0x040000, CRC(15c54ea2) SHA1(5b10bd28e48e51613359820ba8c75d4a91c2d322) )
	ROM_LOAD( "789e20.s22", 0x080000, 0x040000, CRC(ea204acd) SHA1(52b8c30234eaefcba1074496028a4ac2bca48e95) )

	ROM_REGION( 0x080000, "k053250_2", 0 )
	ROM_LOAD( "789e17.p17", 0x000000, 0x040000, CRC(04c07248) SHA1(873445002cbf90c9fc5a35bf4a8f6c43193ee342) )
	ROM_LOAD( "789e16.p12", 0x040000, 0x040000, CRC(9348dee1) SHA1(367193373e28962b5b0e54cc15d68ed88ab83f12) )

	ROM_REGION( 0x200000, "shared", 0 ) /* 053260 samples */
	ROM_LOAD( "789e03.j1", 0x000000, 0x100000, CRC(51ebfebe) SHA1(17f0c23189258e801f48d5833fe934e7a48d071b) )
	ROM_LOAD( "789e02.f1", 0x100000, 0x100000, CRC(bdd3b5c6) SHA1(412332d64052c0a3714f4002c944b0e7d32980a4) )
ROM_END

ROM_START( overdriva )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.d17", 0x00000, 0x20000, CRC(77f18f3f) SHA1(a8c91435573c7851a7864d07eeacfb2f142abbe2) )
	ROM_LOAD16_BYTE( "1.b17", 0x00001, 0x20000, CRC(4f44e6ad) SHA1(9fa871f55e6b2ec353dd979ded568cd9da83f5d6) ) /* also found labeled as "3" */

	ROM_REGION( 0x40000, "sub", 0 )
	ROM_LOAD16_BYTE( "789_e09.l10", 0x00000, 0x20000, CRC(46fb7e88) SHA1(f706a76aff9bec64abe6da325cba0715d6e6ed0a) ) /* also found labeled as "4" as well as "7" */
	ROM_LOAD16_BYTE( "789_e08.k10", 0x00001, 0x20000, CRC(24427195) SHA1(48f4f81729acc0e497b40fddbde11242c5c4c573) ) /* also found labeled as "3" as well as "6" */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "789_e01.e4", 0x00000, 0x10000, CRC(1085f069) SHA1(27228cedb357ff2e130a4bd6d8aa01cf537e034f) ) /* also found labeled as "5" */

	ROM_REGION( 0x400000, "gfx1", 0 )   /* graphics (addressable by the CPU) */
	ROM_LOAD64_WORD( "789e12.r1",  0x000000, 0x100000, CRC(14a10fb2) SHA1(03fb9c15514c5ecc2d9ae4a53961c4bbb49cec73) )    /* sprites */
	ROM_LOAD64_WORD( "789e13.r4",  0x000002, 0x100000, CRC(6314a628) SHA1(f8a8918998c266109348c77427a7696b503daeb3) )
	ROM_LOAD64_WORD( "789e14.r10", 0x000004, 0x100000, CRC(b5eca14b) SHA1(a1c5f5e9cd8bbcfc875e2acb33be024724da63aa) )
	ROM_LOAD64_WORD( "789e15.r15", 0x000006, 0x100000, CRC(5d93e0c3) SHA1(d5cb7666c0c28fd465c860c7f9dbb18a7f739a93) )

	ROM_REGION( 0x020000, "k051316_1", 0 )
	ROM_LOAD( "789e06.a21", 0x000000, 0x020000, CRC(14a085e6) SHA1(86dad6f223e13ff8af7075c3d99bb0a83784c384) )    /* zoom/rotate */

	ROM_REGION( 0x020000, "k051316_2", 0 )
	ROM_LOAD( "789e07.c23", 0x000000, 0x020000, CRC(8a6ceab9) SHA1(1a52b7361f71a6126cd648a76af00223d5b25c7a) )    /* zoom/rotate */

	ROM_REGION( 0x0c0000, "k053250_1", 0 )
	ROM_LOAD( "789e18.p22", 0x000000, 0x040000, CRC(985a4a75) SHA1(b726166c295be6fbec38a9d11098cc4a4a5de456) )
	ROM_LOAD( "789e19.r22", 0x040000, 0x040000, CRC(15c54ea2) SHA1(5b10bd28e48e51613359820ba8c75d4a91c2d322) )
	ROM_LOAD( "789e20.s22", 0x080000, 0x040000, CRC(ea204acd) SHA1(52b8c30234eaefcba1074496028a4ac2bca48e95) )

	ROM_REGION( 0x080000, "k053250_2", 0 )
	ROM_LOAD( "789e17.p17", 0x000000, 0x040000, CRC(04c07248) SHA1(873445002cbf90c9fc5a35bf4a8f6c43193ee342) )
	ROM_LOAD( "789e16.p12", 0x040000, 0x040000, CRC(9348dee1) SHA1(367193373e28962b5b0e54cc15d68ed88ab83f12) )

	ROM_REGION( 0x200000, "shared", 0 ) /* 053260 samples */
	ROM_LOAD( "789e03.j1", 0x000000, 0x100000, CRC(51ebfebe) SHA1(17f0c23189258e801f48d5833fe934e7a48d071b) )
	ROM_LOAD( "789e02.f1", 0x100000, 0x100000, CRC(bdd3b5c6) SHA1(412332d64052c0a3714f4002c944b0e7d32980a4) )
ROM_END

ROM_START( overdrivb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "4.d17", 0x00000, 0x20000, CRC(93c8e892) SHA1(fb41bb13787b93f533b962c3119e6b9f61e2f3f3) )
	ROM_LOAD16_BYTE( "3.b17", 0x00001, 0x20000, CRC(4f44e6ad) SHA1(9fa871f55e6b2ec353dd979ded568cd9da83f5d6) ) /* also found labeled as "1" */

	ROM_REGION( 0x40000, "sub", 0 )
	ROM_LOAD16_BYTE( "789_e09.l10", 0x00000, 0x20000, CRC(46fb7e88) SHA1(f706a76aff9bec64abe6da325cba0715d6e6ed0a) ) /* also found labeled as "4" as well as "7" */
	ROM_LOAD16_BYTE( "789_e08.k10", 0x00001, 0x20000, CRC(24427195) SHA1(48f4f81729acc0e497b40fddbde11242c5c4c573) ) /* also found labeled as "3" as well as "6" */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "789_e01.e4", 0x00000, 0x10000, CRC(1085f069) SHA1(27228cedb357ff2e130a4bd6d8aa01cf537e034f) ) /* also found labeled as "5" */

	ROM_REGION( 0x400000, "gfx1", 0 )   /* graphics (addressable by the CPU) */
	ROM_LOAD64_WORD( "789e12.r1",  0x000000, 0x100000, CRC(14a10fb2) SHA1(03fb9c15514c5ecc2d9ae4a53961c4bbb49cec73) )    /* sprites */
	ROM_LOAD64_WORD( "789e13.r4",  0x000002, 0x100000, CRC(6314a628) SHA1(f8a8918998c266109348c77427a7696b503daeb3) )
	ROM_LOAD64_WORD( "789e14.r10", 0x000004, 0x100000, CRC(b5eca14b) SHA1(a1c5f5e9cd8bbcfc875e2acb33be024724da63aa) )
	ROM_LOAD64_WORD( "789e15.r15", 0x000006, 0x100000, CRC(5d93e0c3) SHA1(d5cb7666c0c28fd465c860c7f9dbb18a7f739a93) )

	ROM_REGION( 0x020000, "k051316_1", 0 )
	ROM_LOAD( "789e06.a21", 0x000000, 0x020000, CRC(14a085e6) SHA1(86dad6f223e13ff8af7075c3d99bb0a83784c384) )    /* zoom/rotate */

	ROM_REGION( 0x020000, "k051316_2", 0 )
	ROM_LOAD( "789e07.c23", 0x000000, 0x020000, CRC(8a6ceab9) SHA1(1a52b7361f71a6126cd648a76af00223d5b25c7a) )    /* zoom/rotate */

	ROM_REGION( 0x0c0000, "k053250_1", 0 )
	ROM_LOAD( "789e18.p22", 0x000000, 0x040000, CRC(985a4a75) SHA1(b726166c295be6fbec38a9d11098cc4a4a5de456) )
	ROM_LOAD( "789e19.r22", 0x040000, 0x040000, CRC(15c54ea2) SHA1(5b10bd28e48e51613359820ba8c75d4a91c2d322) )
	ROM_LOAD( "789e20.s22", 0x080000, 0x040000, CRC(ea204acd) SHA1(52b8c30234eaefcba1074496028a4ac2bca48e95) )

	ROM_REGION( 0x080000, "k053250_2", 0 )
	ROM_LOAD( "789e17.p17", 0x000000, 0x040000, CRC(04c07248) SHA1(873445002cbf90c9fc5a35bf4a8f6c43193ee342) )
	ROM_LOAD( "789e16.p12", 0x040000, 0x040000, CRC(9348dee1) SHA1(367193373e28962b5b0e54cc15d68ed88ab83f12) )

	ROM_REGION( 0x200000, "shared", 0 ) /* 053260 samples */
	ROM_LOAD( "789e03.j1", 0x000000, 0x100000, CRC(51ebfebe) SHA1(17f0c23189258e801f48d5833fe934e7a48d071b) )
	ROM_LOAD( "789e02.f1", 0x100000, 0x100000, CRC(bdd3b5c6) SHA1(412332d64052c0a3714f4002c944b0e7d32980a4) )
ROM_END

GAMEL( 1990, overdriv,         0, overdriv, overdriv, driver_device, 0, ROT90, "Konami", "Over Drive (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_overdriv )
GAMEL( 1990, overdriva, overdriv, overdriv, overdriv, driver_device, 0, ROT90, "Konami", "Over Drive (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_overdriv )
GAMEL( 1990, overdrivb, overdriv, overdriv, overdriv, driver_device, 0, ROT90, "Konami", "Over Drive (set 3)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_overdriv )
