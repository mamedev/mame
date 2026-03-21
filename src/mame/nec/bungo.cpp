// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

NEC 文豪 / "Bungo" Word Processors (laptop family)

TODO:
- needs list of components, no documentation available from the net except for bare specs here:
  https://museum.ipsj.or.jp/en/computer/word/0058.html
- Needs an actual dump of the kanji ROM;
- Garbled message tells user to reset machine and hold SHIFT+MENU while having the
  auxiliary disk in
  https://www.leadedsolder.com/2022/10/15/pwp50sx-nec-mini5-psu-repair-pickup.html
  NB: MENU key doesn't exist with current PC98 keyboard device, also note that the usual
  I/O at $41-$43 is not polled (either expects an irq or perhaps they relocated)
- Verify what exactly the handwritten “Function 2 + F2 = Floppy.” printed on aux disk
  means.
- Verify how much of PC-98 this really uses if anything at all.

**************************************************************************************************/

#include "emu.h"
#include "bungo.h"

void bungo_mini5sx_state::bungo_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 160, 168, 160);
	palette.set_pen_color(1, 48, 56, 16);
}

uint32_t bungo_mini5sx_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 16)
		{
			u16 pen = bitswap<16>(m_gvram[(y*640+x)/16], 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7);

			for (int xi = 0; xi < 16; xi++)
			{
				u8 dot = (pen >> xi) & 1;
				bitmap.pix(y, x+xi) = m_palette->pen(dot);
			}
		}

	return 0;
}

// TODO: PoC offsets
// Reads this area for a pointer to the various alphabets that lives at $c0000 and onward
// The real ROM must have compressed GFX tables to bother like this.
u16 bungo_mini5sx_state::fake_dict_r(offs_t offset)
{
	switch(offset * 2)
	{
		case 0x08:
			return 0;
		case 0x0a:
			return 0x3c20; // hiragana
		case 0x0c:
			return 0x2000;
		case 0x0e:
			return 0x0048; // katakana

		//default:
		//  logerror("%04x\n", offset * 2);
	}

	return 0;
}

void bungo_mini5sx_state::mini5sx_map(address_map &map)
{
	map(0x00000, 0x9ffff).ram(); // 640 KB

	// same as PC98HA: it doesn't have 7220s but just a b&w framebuffer
	map(0xa8000, 0xaffff).ram().share("gvram");

	map(0xc0000, 0xcdfff).rom().region("kanji", 0);
	map(0xce000, 0xcffff).r(FUNC(bungo_mini5sx_state::fake_dict_r));

	map(0xe0000, 0xfffff).rom().region("ipl", 0);
}

void bungo_mini5sx_state::mini5sx_io(address_map &map)
{
	// ...
}

static INPUT_PORTS_START( mini5sx )
INPUT_PORTS_END

// debug
static const gfx_layout gfx_16x16x1 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( gfx_bungo )
	GFXDECODE_ENTRY( "kanji",   0x00000, gfx_8x8x1,   0x000, 0x01 )
	GFXDECODE_ENTRY( "kanji",   0x00000, gfx_16x16x1, 0x000, 0x01 )
GFXDECODE_END

void bungo_mini5sx_state::machine_start()
{
}

static void pc9801_floppies(device_slot_interface &device)
{
//  device.option_add("525dd", TEAC_FD_55F);
//  device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
}



void bungo_mini5sx_state::mini5sx_config(machine_config &config)
{
	const XTAL xtal = XTAL(16'000'000);
	V33(config, m_maincpu, xtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &bungo_mini5sx_state::mini5sx_map);
	m_maincpu->set_addrmap(AS_IO, &bungo_mini5sx_state::mini5sx_io);
//  m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	I8251(config, m_sio_kbd, 0);
//  m_sio_kbd->txd_handler().set("keyb", FUNC(pc9801_kbd_device::input_txd));
//  m_sio_kbd->rxrdy_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ1);

//  clock_device &kbd_clock(CLOCK(config, "kbd_clock", 19'200));
//  kbd_clock.signal_handler().set(m_sio_kbd, FUNC(i8251_device::write_rxc));
//  kbd_clock.signal_handler().append(m_sio_kbd, FUNC(i8251_device::write_txc));

	// TODO: should be PC-98 based with no numpad and some extra keys.
	PC98_KBD(config, m_keyb, 0);
//  m_keyb->rxd_callback().set("sio_kbd", FUNC(i8251_device::write_rxd));

	I8255(config, m_ppi_sys, 0);
//  m_ppi_sys->in_pa_callback().set(m_ppi_sys, FUNC(i8255_device::pa_r));
//  m_ppi_sys->in_pb_callback().set_ioport("SYSB");
//  m_ppi_sys->in_pc_callback().set_constant(0xa0); // 0x80 cpu triple fault reset flag?
//  m_ppi_sys->out_pc_callback().set(FUNC(pc98lt_state::ppi_sys_beep_portc_w));

	// TODO: unverified, known to have a 8-pin "sheet feeder" port
	pc9801_serial(config);

	I8255(config, m_ppi_prn, 0);
//  m_ppi_prn->in_pb_callback().set_ioport("PRNB");

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	// TODO: copied verbatim from base PC98, verify clock et al.
	m_screen->set_raw(21.0526_MHz_XTAL, 848, 0, 640, 440, 0, 400);
	m_screen->set_screen_update(FUNC(bungo_mini5sx_state::screen_update));
//  m_screen->screen_vblank().set(FUNC(pc9801_state::vrtc_irq));

	PALETTE(config, m_palette, FUNC(bungo_mini5sx_state::bungo_palette), 2);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bungo);

	UPD765A(config, m_fdc, 8'000'000, false, true);
//  m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ6);
//  m_fdc->drq_wr_callback().set(m_maincpu, FUNC(v50_device::dreq_w<2>)).invert();
//  m_fdc->drq_wr_callback().set(m_maincpu, FUNC(v50_device::dreq_w<3>)).invert(); // 2dd

	FLOPPY_CONNECTOR(config, "upd765:0", pc9801_floppies, "35hd", pc9801_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", pc9801_floppies, "35hd", pc9801_state::floppy_formats);

	SOFTWARE_LIST(config, "disk_list").set_original("bungo_flop");

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2400).add_route(ALL_OUTPUTS, "mono", 0.05);
}

ROM_START( mini5sx )
	ROM_REGION16_LE( 0x100000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "upd23c4001eacz-048.ic16", 0x000000, 0x080000, CRC(c7ce7ad6) SHA1(1f8616c1c5f817030decda539b9561fab2eef327) )
	ROM_LOAD16_BYTE( "upd23c4001eacz-049.ic19", 0x000001, 0x080000, CRC(09740f3e) SHA1(18ece5fd79392fe86c85007192ca0702728b004d) )

	ROM_REGION16_LE( 0x20000, "ipl", ROMREGION_ERASEFF )
	ROM_COPY( "biosrom", 0x60000, 0x00000, 0x20000 )

	// TODO: none of these is actually provided in dump, backported from pc98ha for now
	ROM_REGION16_LE( 0x40000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom",    0x000000, 0x040000, BAD_DUMP CRC(4be5ff2f) SHA1(261d28419a2ddebe3177a282952806d7bb036b40) )

	ROM_REGION16_LE( 0x40000, "backup", ROMREGION_ERASEFF )
	ROM_LOAD( "backup.bin",   0x000000, 0x040000, BAD_DUMP CRC(3c5b2a99) SHA1(f8e2f5a4c7601d4e81d5e9c83621107ed3f5a29a) )

	ROM_REGION( 0x100000, "dict", ROMREGION_ERASEFF )
	ROM_LOAD( "dict.rom",     0x000000, 0x0c0000, BAD_DUMP CRC(6dc8493c) SHA1(3e04cdc3403a814969b6590cd78e239e72677fe5) )

	ROM_REGION( 0x100000, "romdrv", ROMREGION_ERASEFF )
	ROM_COPY( "biosrom", 0x00000, 0x00000, 0x100000 )

	// $00 filled with odd size
	ROM_REGION( 0x200000, "ramdrv", ROMREGION_ERASEFF )
	ROM_LOAD( "ramdrv.bin",   0x000000, 0x160000, BAD_DUMP CRC(f2cec994) SHA1(c986ad6d8f810ac0a9657c1af26b6fec712d56ed) )
ROM_END

COMP( 1991, mini5sx,     0,        0, mini5sx_config,    mini5sx,   bungo_mini5sx_state, empty_init,   "NEC",   "Bungo mini 5SX", MACHINE_NOT_WORKING )
