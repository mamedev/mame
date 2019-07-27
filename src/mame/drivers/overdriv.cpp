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
#include "includes/overdriv.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m6809/m6809.h"
#include "machine/eepromser.h"
#include "sound/k053260.h"
#include "sound/ym2151.h"
#include "video/k053250.h"
#include "emupal.h"
#include "speaker.h"

#include "overdriv.lh"

/***************************************************************************

  EEPROM

***************************************************************************/

static const uint16_t overdriv_default_eeprom[64] =
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
//logerror("%s: write %04x to eeprom_w\n",machine().describe_context(),data);
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
	const int timer_threshold = 168; // fwiw matches 0 on mask ROM check, so IF it's a timer irq then should be close ...
	int scanline = param;

	m_fake_timer ++;

	// TODO: irqs routines are TOO slow right now, it ends up firing spurious irqs for whatever reason (shared ram fighting?)
	//       this is a temporary solution to get rid of deprecat lib and the crashes, but also makes the game timer to be too slow.
	//       Update: gameplay is actually too fast compared to timer, first attract mode shouldn't even surpass first blue car on right.
	if(scanline == 256) // vblank-out irq
	{
		// m_screen->frame_number() & 1
		m_maincpu->set_input_line(4, HOLD_LINE);
	}
	else if(m_fake_timer >= timer_threshold) // timer irq
	{
		m_fake_timer -= timer_threshold;
		m_maincpu->set_input_line(5, HOLD_LINE);
	}
}

#ifdef UNUSED_FUNCTION
INTERRUPT_GEN_MEMBER(overdriv_state::cpuB_interrupt)
{
	// this doesn't get turned on until the irq has happened? wrong irq?
}
#endif

WRITE16_MEMBER(overdriv_state::cpuA_ctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 probably enables the second 68000 */
		m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);

		/* bit 1 is clear during service mode - function unknown */

		m_led = BIT(data, 3);
		machine().bookkeeping().coin_counter_w(0, data & 0x10);
		machine().bookkeeping().coin_counter_w(1, data & 0x20);

//logerror("%s: write %04x to cpuA_ctrl_w\n",machine().describe_context(),data);
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
	m_audiocpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}




WRITE16_MEMBER(overdriv_state::slave_irq4_assert_w)
{
	// used in-game
	m_subcpu->set_input_line(4, HOLD_LINE);
}

WRITE16_MEMBER(overdriv_state::slave_irq5_assert_w)
{
	// tests GFX ROMs with this irq (indeed enabled only in test mode)
	m_subcpu->set_input_line(5, HOLD_LINE);
}

void overdriv_state::overdriv_master_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x043fff).ram();                 /* work RAM */
	map(0x080000, 0x080fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x0c0000, 0x0c0001).portr("INPUTS");
	map(0x0c0002, 0x0c0003).portr("SYSTEM");
	map(0x0e0000, 0x0e0001).nopw();            /* unknown (always 0x30) */
	map(0x100000, 0x10001f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write)).umask16(0x00ff); /* 053252? (LSB) */
	map(0x140000, 0x140001).nopw(); //watchdog reset?
	map(0x180000, 0x180001).portr("PADDLE").nopw();  // writes 0 at POST and expect that motor busy flag is off, then checks if paddle is at center otherwise throws a "VOLUME ERROR".
	map(0x1c0000, 0x1c001f).w(m_k051316_1, FUNC(k051316_device::ctrl_w)).umask16(0xff00);
	map(0x1c8000, 0x1c801f).w(m_k051316_2, FUNC(k051316_device::ctrl_w)).umask16(0xff00);
	map(0x1d0000, 0x1d001f).w(m_k053251, FUNC(k053251_device::write)).umask16(0xff00);
	map(0x1d8000, 0x1d8003).rw("k053260_1", FUNC(k053260_device::main_read), FUNC(k053260_device::main_write)).umask16(0x00ff);
	map(0x1e0000, 0x1e0003).rw("k053260_2", FUNC(k053260_device::main_read), FUNC(k053260_device::main_write)).umask16(0x00ff);
	map(0x1e8000, 0x1e8001).w(FUNC(overdriv_state::overdriv_soundirq_w));
	map(0x1f0000, 0x1f0001).w(FUNC(overdriv_state::cpuA_ctrl_w));  /* halt cpu B, coin counter, start lamp, other? */
	map(0x1f8000, 0x1f8001).w(FUNC(overdriv_state::eeprom_w));
	map(0x200000, 0x203fff).ram().share("share1");
	map(0x210000, 0x210fff).rw(m_k051316_1, FUNC(k051316_device::read), FUNC(k051316_device::write)).umask16(0xff00);
	map(0x218000, 0x218fff).rw(m_k051316_2, FUNC(k051316_device::read), FUNC(k051316_device::write)).umask16(0xff00);
	map(0x220000, 0x220fff).r(m_k051316_1, FUNC(k051316_device::rom_r)).umask16(0xff00);
	map(0x228000, 0x228fff).r(m_k051316_2, FUNC(k051316_device::rom_r)).umask16(0xff00);
	map(0x230000, 0x230001).w(FUNC(overdriv_state::slave_irq4_assert_w));
	map(0x238000, 0x238001).w(FUNC(overdriv_state::slave_irq5_assert_w));
}

#ifdef UNUSED_FUNCTION
WRITE8_MEMBER( overdriv_state::overdriv_k053246_w )
{
	m_k053246->k053246_w(offset,data);

	uint16_t *src, *dst;

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
#endif

TIMER_CALLBACK_MEMBER(overdriv_state::objdma_end_cb )
{
	m_subcpu->set_input_line(6, HOLD_LINE);
}

WRITE8_MEMBER(overdriv_state::objdma_w)
{
	if(data & 0x10)
		m_objdma_end_timer->adjust(attotime::from_usec(100));

	m_k053246->k053246_w(5,data);
}

void overdriv_state::overdriv_slave_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x083fff).ram(); /* work RAM */
	map(0x0c0000, 0x0c1fff).ram(); //.rw("k053250_1", FUNC(k053250_device::ram_r), FUNC(k053250_device::ram_w));
	map(0x100000, 0x10000f).rw("k053250_1", FUNC(k053250_device::reg_r), FUNC(k053250_device::reg_w));
	map(0x108000, 0x10800f).rw("k053250_2", FUNC(k053250_device::reg_r), FUNC(k053250_device::reg_w));
	map(0x118000, 0x118fff).rw(m_k053246, FUNC(k053247_device::k053247_word_r), FUNC(k053247_device::k053247_word_w)); // data gets copied to sprite chip with DMA..
	map(0x120000, 0x120001).r(m_k053246, FUNC(k053247_device::k053246_r));
	map(0x128000, 0x128001).rw(FUNC(overdriv_state::cpuB_ctrl_r), FUNC(overdriv_state::cpuB_ctrl_w)); /* enable K053247 ROM reading, plus something else */
	map(0x130000, 0x130007).rw(m_k053246, FUNC(k053247_device::k053246_r), FUNC(k053247_device::k053246_w));
	map(0x130005, 0x130005).w(FUNC(overdriv_state::objdma_w));
	//map(0x140000, 0x140001) used in later stages, set after writes at 0x208000-0x20bfff range
	map(0x200000, 0x203fff).ram().share("share1");
	map(0x208000, 0x20bfff).ram(); // sprite indirect table?
	map(0x218000, 0x219fff).r("k053250_1", FUNC(k053250_device::rom_r));
	map(0x220000, 0x221fff).r("k053250_2", FUNC(k053250_device::rom_r));
}

WRITE8_MEMBER(overdriv_state::sound_ack_w)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void overdriv_state::overdriv_sound_map(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(overdriv_state::sound_ack_w));
	// 0x012 read during explosions
	// 0x180
	map(0x0200, 0x0201).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x0400, 0x042f).rw("k053260_1", FUNC(k053260_device::read), FUNC(k053260_device::write));
	map(0x0600, 0x062f).rw("k053260_2", FUNC(k053260_device::read), FUNC(k053260_device::write));
	map(0x0800, 0x0fff).ram();
	map(0x1000, 0xffff).rom();
}

/* Both IPT_START1 assignments are needed. The game will reset during */
/* the "continue" sequence if the assignment on the first port        */
/* is missing.                                                        */

static INPUT_PORTS_START( overdriv )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   // motor busy flag

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(50)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
INPUT_PORTS_END


void overdriv_state::machine_start()
{
	m_led.resolve();
	m_objdma_end_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(overdriv_state::objdma_end_cb), this));

	save_item(NAME(m_cpuB_ctrl));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_zoom_colorbase));
	save_item(NAME(m_road_colorbase));
	save_item(NAME(m_fake_timer));
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


void overdriv_state::overdriv(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2); /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &overdriv_state::overdriv_master_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(overdriv_state::overdriv_cpuA_scanline), "screen", 0, 1);

	M68000(config, m_subcpu, XTAL(24'000'000)/2);  /* 12 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &overdriv_state::overdriv_slave_map);
	//m_subcpu->set_vblank_int("screen", FUNC(overdriv_state::cpuB_interrupt));
	/* IRQ 5 and 6 are generated by the main CPU. */
	/* IRQ 5 is used only in test mode, to request the checksums of the gfx ROMs. */

	MC6809E(config, m_audiocpu, XTAL(3'579'545));                             /* 1.789 MHz?? This might be the right speed, but ROM testing */
	m_audiocpu->set_addrmap(AS_PROGRAM, &overdriv_state::overdriv_sound_map); /* takes a little too much (the counter wraps from 0000 to 9999). */
																			  /* This might just mean that the video refresh rate is less than */
																			  /* 60 fps, that's how I fixed it for now. */

	config.m_minimum_quantum = attotime::from_hz(12000);

	EEPROM_ER5911_16BIT(config, "eeprom").default_data(overdriv_default_eeprom, 128);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(24'000'000)/4,384,0,305,264,0,224);
	screen.set_screen_update(FUNC(overdriv_state::screen_update_overdriv));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 2048).enable_shadows();

	K053246(config, m_k053246, 0);
	m_k053246->set_sprite_callback(FUNC(overdriv_state::sprite_callback), this);
	m_k053246->set_config(NORMAL_PLANE_ORDER, 77, 22);
	m_k053246->set_palette("palette");

	K051316(config, m_k051316_1, 0);
	m_k051316_1->set_palette("palette");
	m_k051316_1->set_offsets(14, -1);
	m_k051316_1->set_wrap(1);
	m_k051316_1->set_zoom_callback(FUNC(overdriv_state::zoom_callback_1), this);

	K051316(config, m_k051316_2, 0);
	m_k051316_2->set_palette("palette");
	m_k051316_2->set_offsets(15, 1);
	m_k051316_2->set_zoom_callback(FUNC(overdriv_state::zoom_callback_2), this);

	K053251(config, m_k053251, 0);

	K053250(config, "k053250_1", 0, "palette", m_screen, 0, 0);
	K053250(config, "k053250_2", 0, "palette", m_screen, 0, 0);

	K053252(config, m_k053252, XTAL(24'000'000)/4);
	m_k053252->set_offsets(13*8, 2*8);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "lspeaker", 0.5).add_route(1, "rspeaker", 0.5);

	k053260_device &k053260_1(K053260(config, "k053260_1", XTAL(3'579'545)));
	k053260_1.set_device_rom_tag("k053260");
	k053260_1.add_route(0, "lspeaker", 0.35);
	k053260_1.add_route(1, "rspeaker", 0.35);

	k053260_device &k053260_2(K053260(config, "k053260_2", XTAL(3'579'545)));
	k053260_2.set_device_rom_tag("k053260");
	k053260_2.add_route(0, "lspeaker", 0.35);
	k053260_2.add_route(1, "rspeaker", 0.35);
}



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

	ROM_REGION( 0x400000, "k053246", 0 )   /* graphics (addressable by the CPU) */
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

	ROM_REGION( 0x200000, "k053260", 0 ) /* 053260 samples */
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

	ROM_REGION( 0x400000, "k053246", 0 )   /* graphics (addressable by the CPU) */
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

	ROM_REGION( 0x200000, "k053260", 0 ) /* 053260 samples */
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

	ROM_REGION( 0x400000, "k053246", 0 )   /* graphics (addressable by the CPU) */
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

	ROM_REGION( 0x200000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "789e03.j1", 0x000000, 0x100000, CRC(51ebfebe) SHA1(17f0c23189258e801f48d5833fe934e7a48d071b) )
	ROM_LOAD( "789e02.f1", 0x100000, 0x100000, CRC(bdd3b5c6) SHA1(412332d64052c0a3714f4002c944b0e7d32980a4) )
ROM_END

GAMEL( 1990, overdriv,         0, overdriv, overdriv, overdriv_state, empty_init, ROT90, "Konami", "Over Drive (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_overdriv ) // US version
GAMEL( 1990, overdriva, overdriv, overdriv, overdriv, overdriv_state, empty_init, ROT90, "Konami", "Over Drive (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_overdriv ) // Overseas?
GAMEL( 1990, overdrivb, overdriv, overdriv, overdriv, overdriv_state, empty_init, ROT90, "Konami", "Over Drive (set 3)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_overdriv ) // Overseas?
