// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    G.I. Joe  (c) 1992 Konami


G.I. Joe
Konami 1992

PCB Layout
----------
GX069 PWB352065B
|--------------------------------------------------------|
|LA4705           069A04.1E      069A05.1H               |
|          84256                 069A06.2H               |
| 054986A          |------|      069A07.4H               |
|          Z80E    |054539|      069A08.6H               |
|CN6               |      |                              |
|          8416    |      |            |------| |------| |
| 051550           |------|            |053246| |053247A |
|          069A01.7C                   |      | |      | |
|                     32MHz            |      | |      | |
|J          ER5911.7D 18.432MHz        |      | |      | |
|A 052535       |------------|         |------| |------| |
|M              |   68000    |                           |
|M 052535       |            |         5168              |
|A              |------------|         5168              |
|  052535        84256                                   |
|                                                        |
|                069A12.13E            |------| |------| |
|          8416                        |054157| |054156| |
|                069UAB03.14E |------| |      | |      | |
|          8416               |053251| |      | |      | |
|  TEST_SW       84256        |      | |      | |      | |
|                             |------| |------| |------| |
|005273(X10)     069A11.16E                              |
|                                                   5168 |
|  CN9           069UAB02.18E           069A09.16J  5168 |
|  CN8   DSW(4)               24MHz     069A10.18J  5168 |
|--------------------------------------------------------|
Notes:
      68000   - Clock 16.000MHz [32/2]
      Z80E    - Clock 8.000MHz [32/4]
      8416    - Fujitsu MB8416 2kx8 SRAM (DIP24)
      84256   - Fujitsu MB84256 32kx8 SRAM (DIP28)
      5168    - Sharp LH5168 8kx8 SRAM (DIP28)
      ER5911  - EEPROM (128 bytes)
      CN6     - 4 pin connector for stereo sound output
      CN8/CN9 - 15 pin connectors for player 3 & player 4 controls
      069*    - EPROM/mask ROM
      LA4705  - 15W 2-channel BTL audio power AMP

      Custom Chips
      ------------
      053251  - Priority encoder
      054157  \
      054156  / Tilemap generators
      053246  \
      053247A / Sprite generators
      054539  - 8-Channel ADPCM sound generator. Clock input 18.432MHz. Clock outputs 18.432/4 & 18.432/8
      052535  - Video DAC (one for each R,G,B video signal)
      051550  - EMI filter for credit/coin counter
      005273  - Resistor array for player 3 & player 4 controls
      054986A - Audio DAC/filter + sound latch + Z80 memory mapper/banker (large ceramic SDIP64 module)
                This module contains several surface mounted capacitors and resistors, 4558 OP amp,
                Analog Devices AD1868 dual 18-bit audio DAC and a Konami 054321 QFP44 IC.

      Sync Measurements
      -----------------
      HSync - 15.2036kHz
      VSync - 59.6374Hz


****************************************************************************

Change Log
----------

AT070403:

tilemap.h,tilemap.c
- added tilemap_get_transparency_data() for transparency cache manipulation

video\konamiic.c
- added preliminary K056832 tilemap<->linemap switching and tileline code

drivers\gijoe.c
- updated video settings, memory map and irq handler
- added object blitter

video\gijoe.c
- completed K054157 to K056832 migration
- added ground scroll emulation
- fixed sprite and BG priority
- improved shadows and layer alignment


Known Issues
------------

- sprite gaps (K053247 zoom fraction rounding)
- shadow masking (eg. the shadow of Baroness' aircraft should not project on the sky)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/k054539.h"
#include "includes/konamipt.h"
#include "includes/gijoe.h"

#define JOE_DEBUG 0
#define JOE_DMADELAY (attotime::from_nsec(42700 + 341300))


READ16_MEMBER(gijoe_state::control2_r)
{
	return m_cur_control2;
}

WRITE16_MEMBER(gijoe_state::control2_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0  is data */
		/* bit 1  is cs (active low) */
		/* bit 2  is clock (active high) */
		/* bit 3  (unknown: coin) */
		/* bit 5  is enable irq 6 */
		/* bit 7  (unknown: enable irq 5?) */
		ioport("EEPROMOUT")->write(data, 0xff);

		m_cur_control2 = data;

		/* bit 6 = enable sprite ROM reading */
		m_k053246->k053246_set_objcha_line( (data & 0x0040) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void gijoe_state::gijoe_objdma(  )
{
	UINT16 *src_head, *src_tail, *dst_head, *dst_tail;

	src_head = m_spriteram;
	src_tail = m_spriteram + 255 * 8;
	m_k053246->k053247_get_ram( &dst_head);
	dst_tail = dst_head + 255 * 8;

	for (; src_head <= src_tail; src_head += 8)
	{
		if (*src_head & 0x8000)
		{
			memcpy(dst_head, src_head, 0x10);
			dst_head += 8;
		}
		else
		{
			*dst_tail = 0;
			dst_tail -= 8;
		}
	}
}

TIMER_CALLBACK_MEMBER(gijoe_state::dmaend_callback)
{
	if (m_cur_control2 & 0x0020)
		m_maincpu->set_input_line(6, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(gijoe_state::gijoe_interrupt)
{
	// global interrupt masking (*this game only)
	if (!m_k056832->is_irq_enabled(0))
		return;

	if (m_k053246->k053246_is_irq_enabled())
	{
		gijoe_objdma();

		// 42.7us(clr) + 341.3us(xfer) delay at 6Mhz dotclock
		m_dmadelay_timer->adjust(JOE_DMADELAY);
	}

	// trigger V-blank interrupt
	if (m_cur_control2 & 0x0080)
		device.execute().set_input_line(5, HOLD_LINE);
}

WRITE16_MEMBER(gijoe_state::sound_cmd_w)
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		soundlatch_byte_w(space, 0, data);
	}
}

WRITE16_MEMBER(gijoe_state::sound_irq_w)
{
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

READ16_MEMBER(gijoe_state::sound_status_r)
{
	return soundlatch2_byte_r(space, 0);
}

static ADDRESS_MAP_START( gijoe_map, AS_PROGRAM, 16, gijoe_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM AM_SHARE("spriteram")                               // Sprites
	AM_RANGE(0x110000, 0x110007) AM_DEVWRITE("k053246", k053247_device, k053246_word_w)
	AM_RANGE(0x120000, 0x121fff) AM_DEVREADWRITE("k056832", k056832_device, ram_word_r, ram_word_w)      // Graphic planes
	AM_RANGE(0x122000, 0x123fff) AM_DEVREADWRITE("k056832", k056832_device, ram_word_r, ram_word_w)      // Graphic planes mirror read
	AM_RANGE(0x130000, 0x131fff) AM_DEVREAD("k056832", k056832_device, rom_word_r)                               // Passthrough to tile roms
	AM_RANGE(0x160000, 0x160007) AM_DEVWRITE("k056832", k056832_device, b_word_w)                                    // VSCCS (board dependent)
	AM_RANGE(0x170000, 0x170001) AM_WRITENOP                                                // Watchdog
	AM_RANGE(0x180000, 0x18ffff) AM_RAM AM_SHARE("workram")                 // Main RAM.  Spec. 180000-1803ff, 180400-187fff
	AM_RANGE(0x190000, 0x190fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x1a0000, 0x1a001f) AM_DEVWRITE("k053251", k053251_device, lsb_w)
	AM_RANGE(0x1b0000, 0x1b003f) AM_DEVWRITE("k056832", k056832_device, word_w)
	AM_RANGE(0x1c000c, 0x1c000d) AM_WRITE(sound_cmd_w)
	AM_RANGE(0x1c0014, 0x1c0015) AM_READ(sound_status_r)
	AM_RANGE(0x1c0000, 0x1c001f) AM_RAM
	AM_RANGE(0x1d0000, 0x1d0001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x1e0000, 0x1e0001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x1e0002, 0x1e0003) AM_READ_PORT("P3_P4")
	AM_RANGE(0x1e4000, 0x1e4001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1e4002, 0x1e4003) AM_READ_PORT("START")
	AM_RANGE(0x1e8000, 0x1e8001) AM_READWRITE(control2_r, control2_w)
	AM_RANGE(0x1f0000, 0x1f0001) AM_DEVREAD("k053246", k053247_device, k053246_word_r)
#if JOE_DEBUG
	AM_RANGE(0x110000, 0x110007) AM_DEVREAD("k053246", k053247_device, k053246_reg_word_r)
	AM_RANGE(0x160000, 0x160007) AM_DEVREAD("k056832", k056832_device, b_word_r)
	AM_RANGE(0x1a0000, 0x1a001f) AM_DEVREAD("k053251", k053251_device, lsb_r)
	AM_RANGE(0x1b0000, 0x1b003f) AM_DEVREAD("k056832", k056832_device, word_r)
#endif
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, gijoe_state )
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xfa2f) AM_DEVREADWRITE("k054539", k054539_device, read, write)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(soundlatch2_byte_w)
	AM_RANGE(0xfc02, 0xfc02) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( gijoe )
	PORT_START("START")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_START3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_START4 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_COIN4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_SERVICE2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SERVICE3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_SERVICE4 )

	PORT_START("P1_P2")
	KONAMI16_LSB_40(1, IPT_BUTTON3 )
	PORT_DIPNAME( 0x0080, 0x0000, "Sound" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Mono ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Stereo ) )
	KONAMI16_MSB_40(2, IPT_BUTTON3 )
	PORT_DIPNAME( 0x8000, 0x8000, "Coin mechanism" )    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x8000, "Common" )
	PORT_DIPSETTING(      0x0000, "Independent" )

	PORT_START("P3_P4")
	KONAMI16_LSB_40(3, IPT_BUTTON3 )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Players ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	KONAMI16_MSB_40(4, IPT_BUTTON3 )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:4" )    /* Listed as "Unused" */
INPUT_PORTS_END

void gijoe_state::machine_start()
{
	m_dmadelay_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gijoe_state::dmaend_callback),this));

	save_item(NAME(m_cur_control2));
}

void gijoe_state::machine_reset()
{
	m_cur_control2 = 0;
}

static MACHINE_CONFIG_START( gijoe, gijoe_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2)   /* 16MHz Confirmed */
	MCFG_CPU_PROGRAM_MAP(gijoe_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gijoe_state,  gijoe_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_32MHz/4)  /* Amuse & confirmed. Z80E at 8MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_EEPROM_SERIAL_ER5911_8BIT_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(24, 24+288-1, 16, 16+224-1)
	MCFG_SCREEN_UPDATE_DRIVER(gijoe_state, screen_update_gijoe)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	MCFG_DEVICE_ADD("k056832", K056832, 0)
	MCFG_K056832_CB(gijoe_state, tile_callback)
	MCFG_K056832_CONFIG("gfx1", 0, K056832_BPP_4, 1, 0, "none")
	MCFG_K056832_GFXDECODE("gfxdecode")
	MCFG_K056832_PALETTE("palette")

	MCFG_DEVICE_ADD("k053246", K053246, 0)
	MCFG_K053246_CB(gijoe_state, sprite_callback)
	MCFG_K053246_CONFIG("gfx2", 1, NORMAL_PLANE_ORDER, -37, 20)
	MCFG_K053246_GFXDECODE("gfxdecode")
	MCFG_K053246_PALETTE("palette")

	MCFG_K053251_ADD("k053251")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("k054539", K054539, XTAL_18_432MHz)
	MCFG_K054539_TIMER_HANDLER(INPUTLINE("audiocpu", INPUT_LINE_NMI))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


ROM_START( gijoe )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "069eab03.14e", 0x000000,  0x40000, CRC(dd2d533f) SHA1(6fc9f7a8fc89155ef2b9ee43fe5e456d9b574f8c) )
	ROM_LOAD16_BYTE( "069eab02.18e", 0x000001,  0x40000, CRC(6bb11c87) SHA1(86581d24f73f2e837f1d4fc5f1f2188f610c50b6) )
	ROM_LOAD16_BYTE( "069a12.13e",   0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11.16e",   0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01.7c", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD32_WORD( "069a10.18j", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD32_WORD( "069a09.16j", 0x000002, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD64_WORD( "069a08.6h", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD64_WORD( "069a05.1h", 0x000002, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD64_WORD( "069a07.4h", 0x000004, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD64_WORD( "069a06.2h", 0x000006, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "k054539", 0 )
	ROM_LOAD( "069a04.1e", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.7d", 0x0000, 0x080, CRC(a0d50a79) SHA1(972533ea45a0e84d9dd14c55f58cd7247926792e) )
ROM_END

// this set is strange, instead of showing program OK it shows the location and checksums of the ROMs
// this doesn't indicate failure, as if you hack the parent set it will show the checksum and the word 'BAD' and refuse to boot
// It will boot as whatever version string is in the EEPROM.  If no version string is in the EEPROM it just shows a blank string
// If you factory default it you get the string 'EB8'
// the roms had no proper labels
// maybe it's some interim / test revision
ROM_START( gijoea )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom3.14e",   0x000000,  0x40000, CRC(0a11f63a) SHA1(06174682907e718017146665b8636be20843b119) )
	ROM_LOAD16_BYTE( "rom2.18e",   0x000001,  0x40000, CRC(8313c559) SHA1(00ae945c65439d4092eaa1780a182dbe3753bb02) )
	ROM_LOAD16_BYTE( "069a12.13e", 0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11.16e", 0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01.7c", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD32_WORD( "069a10.18j", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD32_WORD( "069a09.16j", 0x000002, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD64_WORD( "069a08.6h", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD64_WORD( "069a05.1h", 0x000002, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD64_WORD( "069a07.4h", 0x000004, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD64_WORD( "069a06.2h", 0x000006, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "k054539", 0 )
	ROM_LOAD( "069a04.1e", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom
	ROM_LOAD( "er5911.7d", 0x0000, 0x080, CRC(64f5c87b) SHA1(af81abc54eb59ef7d2250b5ab6cc9642fbd9bfb2) ) // sldh - No actual label so create a unique one for this set
ROM_END

ROM_START( gijoeu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "069uab03.14e", 0x000000,  0x40000, CRC(25ff77d2) SHA1(bea2ae975718806698fd35ef1217bd842b2b69ec) )
	ROM_LOAD16_BYTE( "069uab02.18e", 0x000001,  0x40000, CRC(31cced1c) SHA1(3df1def671966b3c3d8117ac1b68adeeef9d98c0) )
	ROM_LOAD16_BYTE( "069a12.13e",   0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11.16e",   0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01.7c", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD32_WORD( "069a10.18j", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD32_WORD( "069a09.16j", 0x000002, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD64_WORD( "069a08.6h", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD64_WORD( "069a05.1h", 0x000002, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD64_WORD( "069a07.4h", 0x000004, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD64_WORD( "069a06.2h", 0x000006, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "k054539", 0 )
	ROM_LOAD( "069a04.1e", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.7d", 0x0000, 0x080, CRC(ca966023) SHA1(6f07ece0f95213bc12387192986f468d23dfdfc8) ) // sldh - No actual label so create a unique one for this set
ROM_END

ROM_START( gijoej )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "069jaa03.14e", 0x000000,  0x40000, CRC(4b398901) SHA1(98fcc6ae9cc69c67d82eb1a7ab0bb71e61aee623) )
	ROM_LOAD16_BYTE( "069jaa02.18e", 0x000001,  0x40000, CRC(8bb22392) SHA1(9f066ce2b529f7dad6f80a91fff266c478d56414) )
	ROM_LOAD16_BYTE( "069a12.13e",   0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11.16e",   0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01.7c", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD32_WORD( "069a10.18j", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD32_WORD( "069a09.16j", 0x000002, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD64_WORD( "069a08.6h", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD64_WORD( "069a05.1h", 0x000002, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD64_WORD( "069a07.4h", 0x000004, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD64_WORD( "069a06.2h", 0x000006, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "k054539", 0 )
	ROM_LOAD( "069a04.1e", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "er5911.7d", 0x0000, 0x080, CRC(c914fcf2) SHA1(b4f0a0b5d6d4075b004b061336d162336ce1a754) ) // sldh - No actual label so create a unique one for this set
ROM_END


GAME( 1992, gijoe,  0,     gijoe, gijoe, driver_device, 0, ROT0, "Konami", "G.I. Joe (World, EAB, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, gijoea, gijoe, gijoe, gijoe, driver_device, 0, ROT0, "Konami", "G.I. Joe (World, EB8, prototype?)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, gijoeu, gijoe, gijoe, gijoe, driver_device, 0, ROT0, "Konami", "G.I. Joe (US, UAB)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, gijoej, gijoe, gijoe, gijoe, driver_device, 0, ROT0, "Konami", "G.I. Joe (Japan, JAA)", MACHINE_SUPPORTS_SAVE )
