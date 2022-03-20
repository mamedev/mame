// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************

TS-Configuration (ZX Evolution) machine driver.
Implementation: Revision C / 5-bit VDAC

Hobby computer ZX Evolution is Spectrum-compatible with extensions.

Hardware (ZX Evolution):
- Z80 3.5 MHz (classic mode)/ 7 MHz (turbo mode without CPU wait circles)/ 14 MHz (mega turbo with CPU wait circles);
- 4 Mb RAM, 512Kb ROM;
- MiniITX board (172x170mm), 2 ZXBUS slots, power ATX or +5,+12V;
- Based on fpga (Altera EP1K50);
- Peripheral MCU ATMEGA128;
- PS/2 keyboard and mouse support;
- Floppy (WDC1793) Beta-disk compatible interface, IDE (one channel, up to 2 devices on master/slave mode), SD(HC) card, RS232;
- Sound: AY, Beeper, Covox (PWM);
- Real-time clock.

Features (TS-Configuration):
- Resolutions: 360x288, 320x240, 320x200, 256x192
- Hardware scrolled graphic planes
- 256 and 16 indexed colors per pixel
- Programmable color RAM with RGB555 color space and 256 cells
- Text mode with loadable font and hardware vertical scroll
- Up to 256 graphic screens

- Up to 85 sprites per line
- Sprites sized from 8x8 to 64x64 pixels
- Up to 3 sprite planes
- Up to 2 tile planes with 8x8 pixels tiles
- Up to 16 palettes for sprites per line
- Up to 4 palettes for tiles per line for each tile plane

- DRAM-to-Device, Device-to-DRAM and DRAM-to-DRAM DMA Controller

Refs:
ZxEvo: http://nedopc.com/zxevo/zxevo_eng.php
        Principal scheme (rev. C) :: http://nedopc.com/zxevo/zxevo_sch_revc.pdf
        Montage scheme (rev. C) :: http://nedopc.com/zxevo/zxevo_mon_revc.pdf
TsConf: https://github.com/tslabs/zx-evo/blob/master/pentevo/docs/TSconf/tsconf_en.md
        https://github.com/tslabs/zx-evo/raw/master/pentevo/docs/TSconf/TSconf.xls
FAQ-RUS: https://forum.tslabs.info/viewtopic.php?f=35&t=157
    ROM: https://github.com/tslabs/zx-evo/blob/master/pentevo/rom/bin/ts-bios.rom (validated on: 2021-12-14)

HowTo:
# Use ts-bios.rom above. You also need tr-dos roms which simpliest(?) to get from pentagon.
# Create SD image "wc.img"
# Copy WC files from archive https://github.com/tslabs/zx-evo/blob/master/pentevo/soft/WC/wc.zip
# Tech Demos (currently *.spg only): http://prods.tslabs.info/index.php?t=4
$ chdman createhd -i wc.img -o wc.chd -c none
$ mame tsconf -hard wc.chd
# BIOS Setup loads on fresh setup (return to BIOS: RShift+F3)
# Change "Reset To: BD boot.$c"
# Reset (F3)
# Enable keyboard: MAME Setup (Tab) > Keyboard Mode > AT Keyboard: Enabled

TODO:
- Ram cache
- VDos
- INTs not perfect. Currently all signals are invalidated after 32t(3.5MHz). Must only do so for frame, not scanline/DMA
- Palette change in the middle of the frame e.g. zapili-c0

 ****************************************************************************/

#include "emu.h"
#include "includes/tsconf.h"
#include "sound/ay8910.h"
#include "speaker.h"

TILE_GET_INFO_MEMBER(tsconf_state::get_tile_info_txt)
{
	u8 *m_row_location = &m_ram->pointer()[(m_regs[V_PAGE] << 14) + (tile_index / tilemap.cols() * 256)];
	u8 col = tile_index % tilemap.cols();
	u8 symbol = m_row_location[col];
	tileinfo.set(TM_TS_CHAR, symbol, 0, 0);
}

template <u8 Layer>
TILE_GET_INFO_MEMBER(tsconf_state::get_tile_info_16c)
{
	u8 col_offset = (tile_index % tilemap.cols() + Layer * 64) << 1;
	u16 row_offset = (tile_index / tilemap.cols() * 64 * 2) << 1;

	u8 *tile_info_addr = &m_ram->pointer()[(m_regs[T_MAP_PAGE] << 14) + row_offset + col_offset];
	u8 hi = tile_info_addr[1];

	u32 /*u16*/ tile = ((u16(hi) & 0x0f) << 8) | tile_info_addr[0];
	tile = tile / tilemap.cols() * 64 * 8 + (tile % tilemap.cols()); // same as: tmp_tile_oversized_to_code()
	u8 pal = (BIT(m_regs[PAL_SEL], 4 + Layer * 2, 2) << 2) | BIT(hi, 4, 2);
	tileinfo.set(TM_TILES0 + Layer, tile, pal, TILE_FLIPYX(BIT(hi, 6, 2)));
	tileinfo.category = tile == 0 ? 2 : 1;
}

void tsconf_state::tsconf_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankr(m_banks[0]).w(FUNC(tsconf_state::tsconf_bank_w<0>));
	map(0x0000, 0x3fff).view(m_bank0_rom);
	m_bank0_rom[0](0x0000, 0x3fff).bankr(m_banks[4]);

	map(0x4000, 0x7fff).bankr(m_banks[1]).w(FUNC(tsconf_state::tsconf_bank_w<1>));
	map(0x8000, 0xbfff).bankr(m_banks[2]).w(FUNC(tsconf_state::tsconf_bank_w<2>));
	map(0xc000, 0xffff).bankr(m_banks[3]).w(FUNC(tsconf_state::tsconf_bank_w<3>));
}

void tsconf_state::tsconf_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).mirror(0x7ffd).w(FUNC(tsconf_state::tsconf_port_7ffd_w));
	map(0x001f, 0x001f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::status_r), FUNC(beta_disk_device::command_w));
	map(0x003f, 0x003f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::track_r), FUNC(beta_disk_device::track_w));
	map(0x0057, 0x0057).mirror(0xff00).rw(FUNC(tsconf_state::tsconf_port_57_zctr_r), FUNC(tsconf_state::tsconf_port_57_zctr_w)); // spi config
	map(0x0077, 0x0077).mirror(0xff00).rw(FUNC(tsconf_state::tsconf_port_77_zctr_r), FUNC(tsconf_state::tsconf_port_77_zctr_w)); // spi data
	map(0x005f, 0x005f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::sector_r), FUNC(beta_disk_device::sector_w));
	map(0x007f, 0x007f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::data_r), FUNC(beta_disk_device::data_w));
	map(0x00fe, 0x00fe).select(0xff00).rw(FUNC(tsconf_state::spectrum_ula_r), FUNC(tsconf_state::tsconf_ula_w));
	map(0x00ff, 0x00ff).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::state_r), FUNC(beta_disk_device::param_w));
	map(0x00af, 0x00af).select(0xff00).rw(FUNC(tsconf_state::tsconf_port_xxaf_r), FUNC(tsconf_state::tsconf_port_xxaf_w));
	map(0x8ff7, 0x8ff7).select(0x7000).w(FUNC(tsconf_state::tsconf_port_f7_w)); // 3:bff7 5:dff7 6:eff7
	map(0xbff7, 0xbff7).r(FUNC(tsconf_state::tsconf_port_f7_r));
	map(0x8000, 0x8000).mirror(0x3ffd).w("ay8912", FUNC(ay8910_device::data_w));
	map(0xc000, 0xc000).mirror(0x3ffd).rw("ay8912", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
}

void tsconf_state::tsconf_switch(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(tsconf_state::beta_neutral_r)); // Overlap with next because we want real addresses on the 3e00-3fff range
	map(0x3d00, 0x3dff).r(FUNC(tsconf_state::beta_enable_r));
	map(0x4000, 0xffff).r(FUNC(tsconf_state::beta_disable_r));
}

template <u8 Bank>
void tsconf_state::tsconf_bank_w(offs_t offset, u8 data)
{
	tsconf_state::ram_bank_write(Bank, offset, data);
}

static const gfx_layout spectrum_charlayout =
{
	8, 8, /* 8 x 8 characters */
	96,   /* 96 characters */
	1,    /* 1 bits per pixel */
	{0},  /* no bitplanes */
	/* x offsets */
	{STEP8(0, 1)},
	/* y offsets */
	{STEP8(0, 8)},
	8 * 8 /* every char takes 8 bytes */
};

static const gfx_layout tsconf_charlayout =
{
	8, 8,
	256,
	1,
	{0},
	{STEP8(0, 1)},
	{STEP8(0, 8)},
	8 * 8
};

static const gfx_layout tsconf_tile_16cpp_layout =
{
	8,
	8,
	64 * 64 * 8,
	4,
	{STEP4(0, 1)},
	{STEP8(0, 4)},
	{STEP8(0, 256 * 8)},
	// Much more tiles when needed. Because tiles are in RAW formut but we don't know region properties.
	8 * 4
};

static GFXDECODE_START(gfx_tsconf)
	GFXDECODE_ENTRY("maincpu", 0, tsconf_charlayout, 0xf7, 1)         // TM_TS_CHAR : TXT
	GFXDECODE_ENTRY("maincpu", 0, tsconf_tile_16cpp_layout, 0, 16)    // TM_TILES0  : T0 16cpp
	GFXDECODE_ENTRY("maincpu", 0, tsconf_tile_16cpp_layout, 0, 16)    // TM_TILES1  : T1 16cpp
	GFXDECODE_ENTRY("maincpu", 0, tsconf_tile_16cpp_layout, 0, 16)    // TM_SPRITES : Sprites 16cpp
	GFXDECODE_ENTRY("maincpu", 0x1fd00, spectrum_charlayout, 0xf7, 1) // TM_ZX_CHAR
GFXDECODE_END

void tsconf_state::video_start()
{
	spectrum_128_state::video_start();
	m_scanline_timer->reset();

	m_ts_tilemap[TM_TS_CHAR] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tsconf_state::get_tile_info_txt)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);

	m_ts_tilemap[TM_TILES0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tsconf_state::get_tile_info_16c<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_ts_tilemap[TM_TILES0]->set_transparent_pen(0);

	m_ts_tilemap[TM_TILES1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tsconf_state::get_tile_info_16c<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_ts_tilemap[TM_TILES1]->set_transparent_pen(0);

	m_frame_irq_timer = timer_alloc(TIMER_IRQ_FRAME);
	m_line_irq_timer = timer_alloc(TIMER_IRQ_SCANLINE);
}

void tsconf_state::machine_start()
{
	for (auto i = 0; i < 4; i++)
		m_banks[i]->configure_entries(0, m_ram->size() / 0x4000, m_ram->pointer(), 0x4000);

	memory_region *rom = memregion("maincpu");
	m_banks[4]->configure_entries(0, rom->bytes() / 0x4000, rom->base(), 0x4000);

	save_item(NAME(m_regs));
	// TODO save'm'all!
}

void tsconf_state::machine_reset()
{
	m_bank0_rom.select(0);
	m_program = &m_maincpu->space(AS_PROGRAM);

	m_port_f7_ext = DISABLED;

	m_scanline_delayed_regs_update = {};
	m_regs[V_CONFIG] = 0x00;        // 00000000
	m_regs[V_PAGE] = 0x05;          // 00000101
	m_regs[G_X_OFFS_L] = 0x00;      // 00000000
	m_regs[G_X_OFFS_H] &= 0xfe;     // xxxxxxx0
	m_regs[G_Y_OFFS_L] = 0x00;      // 00000000
	m_regs[G_Y_OFFS_H] &= 0xfe;     // xxxxxxx0
	m_regs[TS_CONFIG] &= 0x03;      // 000000xx
	m_regs[PAL_SEL] = 0x0f;         // 00001111
	m_regs[PAGE0] = 0x00;           // 00000000
	m_regs[PAGE1] = 0x05;           // 00000101
	m_regs[PAGE2] = 0x02;           // 00000010
	m_regs[PAGE3] = 0x00;           // 00000000
	m_regs[FMAPS] &= 0xef;          // xxx0xxxx
	m_regs[SYS_CONFIG] = 0x00;      // 00000000
	m_regs[MEM_CONFIG] = 0x04;      // 00000100
	m_regs[HS_INT] = 0x01;          // 00000001
	m_regs[VS_INT_L] = 0x00;        // 00000000
	m_regs[VS_INT_H] &= 0x0e;       // 0000xxx0
	m_regs[FDD_VIRT] &= 0xf0;       // xxxx0000
	m_regs[INT_MASK] = 0x01;        // xxxxx001
	m_regs[CACHE_CONFIG] &= 0xf0;   // xxxx0000

	if (m_beta->started())
		m_beta->enable();

	m_zctl_cs = 1;
	m_zctl_di = 0xff;

	tsconf_update_bank0();
	tsconf_update_video_mode();

	m_keyboard->write(0xff);
	while (m_keyboard->read() != 0) { /* invalidate buffer */ }
}

void tsconf_state::tsconf(machine_config &config)
{
	spectrum_128(config);
	m_maincpu->set_clock(3.5_MHz_XTAL);

	config.device_remove("exp");
	config.device_remove("palette");

	m_maincpu->set_addrmap(AS_PROGRAM, &tsconf_state::tsconf_mem);
	m_maincpu->set_addrmap(AS_IO, &tsconf_state::tsconf_io);
	m_maincpu->set_addrmap(AS_OPCODES, &tsconf_state::tsconf_switch);

	m_maincpu->set_vblank_int("screen", FUNC(tsconf_state::tsconf_vblank_interrupt));

	m_ram->set_default_size("4096K");

	GLUKRS(config, m_glukrs);

	TSCONF_DMA(config, m_dma, 7_MHz_XTAL);
	m_dma->in_mreq_callback().set(FUNC(tsconf_state::ram_read16));
	m_dma->out_mreq_callback().set(FUNC(tsconf_state::ram_write16));
	m_dma->in_spireq_callback().set(FUNC(tsconf_state::spi_read16));
	m_dma->out_cram_callback().set(FUNC(tsconf_state::cram_write16));
	m_dma->out_sfile_callback().set(FUNC(tsconf_state::sfile_write16));
	m_dma->on_ready_callback().set(FUNC(tsconf_state::dma_ready));

	BETA_DISK(config, m_beta, 0);
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->spi_miso_callback().set(FUNC(tsconf_state::tsconf_spi_miso_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2149(config.replace(), "ay8912", 14_MHz_XTAL / 8)
		.add_route(0, "lspeaker", 0.50)
		.add_route(1, "lspeaker", 0.25)
		.add_route(1, "rspeaker", 0.25)
		.add_route(2, "rspeaker", 0.50);

	PALETTE(config, "palette", FUNC(tsconf_state::tsconf_palette), 256);
	m_screen->set_raw(7_MHz_XTAL, 448, with_hblank(), 448, 320, with_vblank(), 320);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_tsconf);
	RAM(config, m_cram).set_default_size("512").set_default_value(0);
	RAM(config, m_sfile).set_default_size("512").set_default_value(0); // 85*6

	AT_KEYB(config, m_keyboard, pc_keyboard_device::KEYBOARD_TYPE::AT, 3);
}

ROM_START(tsconf)
	ROM_REGION(0x080000, "maincpu", ROMREGION_ERASEFF) // ROM: 32 * 16KB
	ROM_LOAD("ts-bios.rom", 0, 0x10000, CRC(b060b0d9) SHA1(820d3539de115141daff220a3cb733fc880d1bab))
ROM_END

//    YEAR  NAME    PARENT      COMPAT  MACHINE     INPUT       CLASS           INIT        COMPANY             FULLNAME                            FLAGS
COMP( 2011, tsconf, spec128,    0,      tsconf,     spec_plus,  tsconf_state,   empty_init, "NedoPC, TS-Labs",  "ZX Evolution TS-Configuration",    MACHINE_IS_INCOMPLETE)
