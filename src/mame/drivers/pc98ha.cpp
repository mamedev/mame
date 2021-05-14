// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    PC98LT/HA class machine "Handy98" aka 1st Gen LCD PC98

	TODO:
	- identify LCDC used here, reg 2 is clearly H display (0x4f+1)*8=640
	- merge from base pc98 class (WIP);
	- power handling;
	- PC98HA specifics:
        - optional docking station (for floppy device only or anything else?);
		- EMS;
		- RAM drive (?);
		- JEIDA memory card interface);

**************************************************************************************************/

#include "emu.h"
//#include "cpu/z80/z80.h"
#include "includes/pc9801.h"

void pc98lt_state::lt_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 160, 168, 160);
	palette.set_pen_color(1, 48, 56, 16);
}

uint32_t pc98lt_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
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

/*
 * Power Status Register
 *
 * x--- ---- docking station connected (HA only?)
 * -x-- ---- AC power supply connected
 * ---x ---- alarm enabled
 * ---- x--- unknown
 * ---- -x-- Lithium battery low (HA only?)
 * ---- --x- battery low
 * ---- ---x power off 
 */
u8 pc98lt_state::power_status_r()
{
	return 0x00;
}

void pc98lt_state::lt_map(address_map &map)
{
	map(0x00000, 0x5ffff).ram(); // 384 KB

	// no TVRAM
	map(0xa8000, 0xaffff).ram().share("gvram");
//  0xb0000-0xbffff unmapped GVRAM or mirror, check me
//	map(0xd0000, 0xd3fff) // learn (?)
//	map(0xd4000, 0xd7fff) // dictionary rom bank
	map(0xd8000, 0xdbfff).bankr("kanji_bank");
//	map(0xe0000, 0xeffff) // ROM drive
	map(0xf0000, 0xfffff).rom().region("ipl", 0);
}

void pc98ha_state::ha_map(address_map &map)
{
	lt_map(map);
	map(0x00000, 0x9ffff).ram(); // 640 KB

//	map(0xc0000, 0xcffff) // EMS
//	map(0xdc000, 0xdffff) // RAM drive / Memory card window area
}

void pc98lt_state::lt_io(address_map &map)
{
	map.unmap_value_high();
//	map(0x0000, 0x001f).rw(FUNC(pc9801_state::pic_r), FUNC(pc9801_state::pic_w)).umask16(0x00ff); // i8259 PIC (bit 3 ON slave / master) / <undefined>
//	map(0x0020, 0x002f).w(FUNC(pc9801_state::rtc_w)).umask16(0x00ff);
//	map(0x0030, 0x0037).rw(m_ppi_sys, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00); //i8251 RS232c / i8255 system port
//	map(0x0040, 0x0047).rw(m_ppi_prn, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
//	map(0x0040, 0x0047).rw(m_keyb, FUNC(pc9801_kbd_device::rx_r), FUNC(pc9801_kbd_device::tx_w)).umask16(0xff00); //i8255 printer port / i8251 keyboard
//	map(0x0070, 0x007f).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0xff00);

	// floppy actually requires a docking station on PC98HA, density should be 2dd given the mapping
//	map(0x00be, 0x00be) // floppy mode control (?)
//	map(0x00c8, 0x00cb).m(m_fdc_2dd, FUNC(upd765a_device::map)).umask16(0x00ff);
//	map(0x00cc, 0x00cc).rw(FUNC(pc9801_state::fdc_2dd_ctrl_r), FUNC(pc9801_state::fdc_2dd_ctrl_w)); //upd765a 2dd / <undefined>

//  map(0x00e0, 0x00ef) // uPD71071, internal to the V50 even?

//	map(0x0810, 0x0810) // <unknown device data>, LCDC?
//	map(0x0812, 0x0812) // <unknown device address> & 0xf

//	map(0x0c10, 0x0c10) // learn bank reg (odd)
//	map(0x0f8e, 0x0f8e) // card slot status 1
//	map(0x4810, 0x4810) // ?
//	map(0x4c10, 0x4c10) // dictionary bank reg
//	map(0x5e8e, 0x5e8e) // card slot status 2
//	map(0x6e8e, 0x6e8e) // modem control 1
//	map(0x7e8e, 0x7e8e) // modem control 2
	map(0x8810, 0x8810).r(FUNC(pc98lt_state::power_status_r));
	map(0x8c10, 0x8c10).lw8(NAME([this] (u8 data) { m_kanji_bank->set_entry(data & 0x0f); }));
//	map(0xc810, 0xc810) // ?
//	map(0xcc10, 0xcc10) // ROM drive bank reg
}

void pc98ha_state::ha_io(address_map &map)
{
	lt_io(map);
//	map(0x08e0, 0x08e7) // EMS bank regs (odd regs only)
//	map(0x0e8e, 0x0e8e) // RAM drive bank reg 
//	map(0x1e8e, 0x1e8e) // RAM drive view select
}

static INPUT_PORTS_START( pc98lt )
INPUT_PORTS_END

//static INPUT_PORTS_START( pc98ha )
//INPUT_PORTS_END

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

static GFXDECODE_START( gfx_pc98lt )
	GFXDECODE_ENTRY( "kanji",   0x00000, gfx_8x8x1,   0x000, 0x01 )
	GFXDECODE_ENTRY( "kanji",   0x00000, gfx_16x16x1, 0x000, 0x01 )
GFXDECODE_END

void pc98lt_state::machine_start()
{
	m_kanji_bank->configure_entries(0, 0x10, memregion("kanji")->base(), 0x4000);
}

void pc98lt_state::lt_config(machine_config &config)
{
	V50(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &pc98lt_state::lt_map);
	m_maincpu->set_addrmap(AS_IO, &pc98lt_state::lt_io);
//	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

//	UPD1990A

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	// TODO: copied verbatim from base PC98, verify clock et al.
	m_screen->set_raw(21.0526_MHz_XTAL, 848, 0, 640, 440, 0, 400);
	m_screen->set_screen_update(FUNC(pc98lt_state::screen_update));
//	m_screen->screen_vblank().set(FUNC(pc9801_state::vrtc_irq));

	PALETTE(config, m_palette, FUNC(pc98lt_state::lt_palette), 2);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pc98lt);

	SPEAKER(config, "mono").front_center();
//	BEEP(config, m_beeper, 2400).add_route(ALL_OUTPUTS, "mono", 0.15);
}

void pc98ha_state::ha_config(machine_config &config)
{
	lt_config(config);
	V50(config.replace(), m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &pc98ha_state::ha_map);
	m_maincpu->set_addrmap(AS_IO, &pc98ha_state::ha_io);
//	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

//	UPD4991A
}

ROM_START( pc98lt )
	ROM_REGION16_LE( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom",      0x000000, 0x010000, CRC(b6a6a382) SHA1(3f1767cccc1ae02b3e48f6ee327d3ef4fad05750) )

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom",    0x000000, 0x040000, CRC(26a81aa2) SHA1(bf12e40c608ef6ef1ac38f6b0b3ca79260a50cef) )

	ROM_REGION( 0x10000, "backup", ROMREGION_ERASEFF )
	ROM_LOAD( "backup.bin",   0x000000, 0x010000, CRC(56d7ca00) SHA1(d17942e166f98af1d484e497e97d31da515973f7) )
	
	ROM_REGION( 0x80000, "dict", ROMREGION_ERASEFF )
	ROM_LOAD( "dict.rom",     0x000000, 0x080000, CRC(421278ee) SHA1(f6066fc5085de521395ce1a8bb040536c1454c7e) )

	ROM_REGION( 0x80000, "romdrv", ROMREGION_ERASEFF )
	ROM_LOAD( "romdrv.rom",   0x000000, 0x080000, CRC(282ff6eb) SHA1(f4833e49dd9089ec40f5e86a713e08cd8c598578) )
ROM_END

ROM_START( pc98ha )
	ROM_REGION16_LE( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom",      0x000000, 0x010000, CRC(2f552bb9) SHA1(7f53bf95181d65b2f9942285da669d92c61247a3) )

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom",    0x000000, 0x040000, CRC(4be5ff2f) SHA1(261d28419a2ddebe3177a282952806d7bb036b40) )

	ROM_REGION( 0x40000, "backup", ROMREGION_ERASEFF )
	ROM_LOAD( "backup.bin",   0x000000, 0x040000, CRC(3c5b2a99) SHA1(f8e2f5a4c7601d4e81d5e9c83621107ed3f5a29a) )
	
	ROM_REGION( 0x100000, "dict", ROMREGION_ERASEFF )
	ROM_LOAD( "dict.rom",     0x000000, 0x0c0000, BAD_DUMP CRC(6dc8493c) SHA1(3e04cdc3403a814969b6590cd78e239e72677fe5) )

	ROM_REGION( 0x100000, "romdrv", ROMREGION_ERASEFF )
	ROM_LOAD( "romdrv.rom",   0x000000, 0x100000, CRC(2f59127f) SHA1(932cb970c2b22408f7895dbf9df6dbc47f8e055b) )

	ROM_REGION( 0x200000, "ramdrv", ROMREGION_ERASEFF )
	ROM_LOAD( "ramdrv.bin",   0x000000, 0x160000, BAD_DUMP CRC(f2cec994) SHA1(c986ad6d8f810ac0a9657c1af26b6fec712d56ed) )
ROM_END

COMP( 1989, pc98lt,      0,        0, lt_config,    pc98lt,   pc98lt_state, empty_init,   "NEC",   "PC-98LT",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1990, pc98ha,      0,        0, ha_config,    pc98lt,   pc98ha_state, empty_init,   "NEC",   "PC-98HA (Handy98)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
