// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
// thanks-to:TS-Labs
/***************************************************************************

see: pentevo.cpp

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

Revisions:
    tsconf: Initial release for ZX Evolution baser on Altera's FPGA 50K
    tsconf2: Requires 100K FPGA and has extra feature set like 'Copper'

Refs:
TsConf: https://github.com/tslabs/zx-evo/blob/master/pentevo/docs/TSconf/tsconf_en.md
        https://github.com/tslabs/zx-evo/raw/master/pentevo/docs/TSconf/TSconf.xls
FAQ-RUS: https://forum.tslabs.info/viewtopic.php?f=35&t=157
    ROM: https://github.com/tslabs/zx-evo/blob/master/pentevo/rom/bin/ts-bios.rom (validated on: 2021-12-14)

 ****************************************************************************/

#include "emu.h"
#include "tsconf.h"

#include "bus/spectrum/ay/slot.h"
#include "bus/rs232/rs232.h"
#include "bus/spectrum/zxbus/bus.h"
#include "cpu/z80/z80.h"
#include "speaker.h"


ALLOW_SAVE_TYPE(tsconf_state::gluk_ext);


TILE_GET_INFO_MEMBER(tsconf_state::get_tile_info_txt)
{
	u8 *m_row_location = &m_ram->pointer()[get_vpage_offset() + (tile_index / tilemap.cols() * 256)];
	u8 col = tile_index % tilemap.cols();
	u8 symbol = m_row_location[col];
	tileinfo.set(TM_TS_CHAR, symbol, 0, 0);
}

template <u8 Layer>
TILE_GET_INFO_MEMBER(tsconf_state::get_tile_info_16c)
{
	const u8 col_offset = (tile_index & 0x03f) << 1;
	const u16 row_offset = (tile_index & 0xfc0) << 2;

	u8 *tile_info_addr = &m_ram->pointer()[(m_regs[T_MAP_PAGE] << 14) | row_offset | (Layer ? 0x80 : 0x00) | col_offset];
	u8 hi = tile_info_addr[1];

	u16 tile = ((u16(hi) & 0x0f) << 8) | tile_info_addr[0];
	u8 pal = (BIT(m_regs[PAL_SEL], 4 + Layer * 2, 2) << 2) | BIT(hi, 4, 2);
	tileinfo.set(TM_TILES0 + Layer, tile, pal, TILE_FLIPYX(BIT(hi, 6, 2)));
	tileinfo.category = tile == 0 ? 2 : 1;
}

void tsconf_state::tsconf_mem(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(tsconf_state::tsconf_ram_bank_r<0>), FUNC(tsconf_state::tsconf_bank_w<0>));
	map(0x0000, 0x3fff).view(m_bank0_rom);
	m_bank0_rom[0](0x0000, 0x3fff).bankr(m_bank_rom[0]);

	map(0x4000, 0x7fff).rw(FUNC(tsconf_state::tsconf_ram_bank_r<1>), FUNC(tsconf_state::tsconf_bank_w<1>));
	map(0x8000, 0xbfff).rw(FUNC(tsconf_state::tsconf_ram_bank_r<2>), FUNC(tsconf_state::tsconf_bank_w<2>));
	map(0xc000, 0xffff).rw(FUNC(tsconf_state::tsconf_ram_bank_r<3>), FUNC(tsconf_state::tsconf_bank_w<3>));
}

void tsconf_state::tsconf_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).mirror(0x7ffd).w(FUNC(tsconf_state::tsconf_port_7ffd_w));
	map(0x001f, 0x001f).mirror(0xff00).r(FUNC(tsconf_state::tsconf_port_xx1f_r));
	map(0x0057, 0x0057).mirror(0xff00).rw(FUNC(tsconf_state::tsconf_port_57_zctr_r), FUNC(tsconf_state::tsconf_port_57_zctr_w)); // spi config
	map(0x0077, 0x0077).mirror(0xff00).rw(FUNC(tsconf_state::tsconf_port_77_zctr_r), FUNC(tsconf_state::tsconf_port_77_zctr_w)); // spi data

	// RS-232
	map(0x00ef, 0x00ef).mirror(0xff00).rw(m_uart, FUNC(tsconf_rs232_device::dr_r), FUNC(tsconf_rs232_device::dr_w)); // 0x00ef..0xbfef
	map(0xc0ef, 0xc0ef).mirror(0x3f00).unmaprw();
	map(0xc0ef, 0xc0ef).select(0x0f00)
		.lr8(NAME([this](offs_t offset) -> u8 { return m_uart->reg_r(offset >> 8); }))
		.lw8(NAME([this](offs_t offset, u8 data) { m_uart->reg_w(offset >> 8, data); }));

	map(0x00fe, 0x00fe).select(0xff00).rw(FUNC(tsconf_state::spectrum_ula_r), FUNC(tsconf_state::tsconf_ula_w));
	map(0x00af, 0x00af).select(0xff00).rw(FUNC(tsconf_state::tsconf_port_xxaf_r), FUNC(tsconf_state::tsconf_port_xxaf_w));
	map(0xfadf, 0xfadf).lr8(NAME([this]() -> u8 { return (m_io_mouse[3]->read() << 4) | m_io_mouse[2]->read(); }));
	map(0xfbdf, 0xfbdf).lr8(NAME([this]() -> u8 { return m_io_mouse[0]->read(); }));
	map(0xffdf, 0xffdf).lr8(NAME([this]() -> u8 { return m_io_mouse[1]->read(); }));
	map(0x8ff7, 0x8ff7).select(0x7000).w(FUNC(tsconf_state::tsconf_port_f7_w)); // 3:bff7 5:dff7 6:eff7
	map(0xbff7, 0xbff7).r(FUNC(tsconf_state::tsconf_port_f7_r));
	map(0x00fb, 0x00fb).mirror(0xff00).w(m_dac, FUNC(dac_byte_interface::data_w));
	map(0x80fd, 0x80fd).mirror(0x3f00).w("ay_slot", FUNC(ay_slot_device::data_w));
	map(0xc0fd, 0xc0fd).mirror(0x3f00).rw("ay_slot", FUNC(ay_slot_device::data_r), FUNC(ay_slot_device::address_w));

	map(0x0000, 0xffff).view(m_io_shadow_view);
	m_io_shadow_view[0]; // !Shadow

	// IO: Shadow
	m_io_shadow_view[1](0x0000, 0xffff).m(m_beta, FUNC(tsconf_beta_device::tsconf_beta_io));
	subdevice<zxbus_device>("zxbus")->set_io_space(m_io_shadow_view[0], m_io_shadow_view[1]);
}

void tsconf_state::tsconf_switch(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(tsconf_state::beta_neutral_r)); // Overlap with next because we want real addresses on the 3e00-3fff range
	map(0x3d00, 0x3dff).r(FUNC(tsconf_state::beta_enable_r));
	map(0x4000, 0xffff).r(FUNC(tsconf_state::beta_disable_r));
}

static const gfx_layout spectrum_charlayout =
{
	8, 8,          // 8 x 8 characters
	96,            // 96 characters
	1,             // 1 bits per pixel
	{0},           // no bitplanes
	{STEP8(0, 1)}, // x offsets
	{STEP8(0, 8)}, // y offsets
	8 * 8          // every char takes 8 bytes
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

static GFXDECODE_START(gfx_tsconf)
	GFXDECODE_ENTRY("maincpu", 0, tsconf_charlayout, 0xf7, 1)         // TM_TS_CHAR : TXT
	GFXDECODE_RAM("tiles0_raw", 0, gfx_8x8x8_raw, 0, 16)              // TM_TILES0  : T0 16cpp
	GFXDECODE_RAM("tiles1_raw", 0, gfx_8x8x8_raw, 0, 16)              // TM_TILES1  : T1 16cpp
	GFXDECODE_RAM("sprites_raw", 0, gfx_8x8x8_raw, 0, 16)             // TM_SPRITES : Sprites 16cpp
	GFXDECODE_ENTRY("maincpu", 0x1fd00, spectrum_charlayout, 0xf7, 1) // TM_ZX_CHAR
GFXDECODE_END

void tsconf_state::video_start()
{
	spectrum_128_state::video_start();

	m_ts_tilemap[TM_TS_CHAR] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tsconf_state::get_tile_info_txt)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);

	m_ts_tilemap[TM_TILES0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tsconf_state::get_tile_info_16c<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_ts_tilemap[TM_TILES0]->set_transparent_pen(0);
	m_gfxdecode->gfx(TM_TILES0)->set_granularity(16);

	m_ts_tilemap[TM_TILES1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tsconf_state::get_tile_info_16c<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_ts_tilemap[TM_TILES1]->set_transparent_pen(0);
	m_gfxdecode->gfx(TM_TILES1)->set_granularity(16);

	m_gfxdecode->gfx(TM_SPRITES)->set_granularity(16);

	m_frame_irq_timer = timer_alloc(FUNC(tsconf_state::irq_frame), this);
	m_scanline_irq_timer = timer_alloc(FUNC(tsconf_state::irq_scanline), this);
}

void tsconf_state::machine_start()
{
	spectrum_128_state::machine_start();

	// reconfigure ROMs
	memory_region *rom = memregion("maincpu");
	m_bank_rom[0]->configure_entries(0, rom->bytes() / 0x4000, rom->base(), 0x4000);
	m_bank_ram[0]->configure_entries(0, m_ram->size() / 0x4000, m_ram->pointer(), 0x4000);

	save_item(NAME(m_int_mask));
	save_item(NAME(m_update_on_m1));
	save_item(NAME(m_regs));
	save_item(NAME(m_cache_line_addr));
	save_item(NAME(m_zctl_di));
	save_item(NAME(m_zctl_cs));
	save_item(NAME(m_port_f7_ext));
	save_item(NAME(m_gfx_y_frame_offset));
}

void tsconf_state::machine_reset()
{
	m_update_on_m1 = false;

	m_frame_irq_timer->adjust(attotime::never);
	m_scanline_irq_timer->adjust(attotime::never);
	m_int_mask = 0;

	m_bank0_rom.select(0);
	update_io(0);
	m_cache_line_addr = -1;

	m_glukrs->disable();

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

	m_beta->fddvirt_w(m_regs[FDD_VIRT] & 0x0f);

	m_zctl_cs = 1;
	m_zctl_di = 0xff;

	m_sprites_cache.clear();
	tsconf_update_bank0();
	tsconf_update_video_mode();

	m_keyboard->write(0xff);
	while (m_keyboard->read() != 0) { /* invalidate buffer */ }

	u16 const *const cram_init = &memregion("cram_init")->as_u16();
	for (auto i = 0; i < 0x100; i++)
		cram_write16(i << 1, cram_init[i]); // init color RAM
}

void tsconf_state::device_post_load()
{
	spectrum_128_state::device_post_load();
	m_sprites_cache.clear();
	copy_tiles_to_raw(m_ram->pointer() + ((m_regs[SG_PAGE] & 0xf8) << 14), m_sprites_raw.target());
	copy_tiles_to_raw(m_ram->pointer() + ((m_regs[T0_G_PAGE] & 0xf8) << 14), m_sprites_raw.target());
	copy_tiles_to_raw(m_ram->pointer() + ((m_regs[T1_G_PAGE] & 0xf8) << 14), m_sprites_raw.target());
}

INPUT_PORTS_START( tsconf )
	PORT_INCLUDE( spec_plus )

	PORT_MODIFY("CONFIG")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)


	PORT_START("mouse_input1")
	PORT_BIT(0xff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(30)

	PORT_START("mouse_input2")
	PORT_BIT(0xff, 0, IPT_MOUSE_Y) PORT_REVERSE PORT_SENSITIVITY(30)

	PORT_START("mouse_input3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Mouse Button Left") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Mouse Button Right") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Mouse Button Middle") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("mouse_input4")
	PORT_BIT(0x0f, 0, IPT_DIAL_V) PORT_REVERSE PORT_NAME("Mouse Scroll V") PORT_SENSITIVITY(1) PORT_CODE(MOUSECODE_Z)

INPUT_PORTS_END

void tsconf_state::tsconf(machine_config &config)
{
	spectrum_128(config);

	config.device_remove("exp");
	config.device_remove("dma");
	config.device_remove("palette");

	Z80(config.replace(), m_maincpu, 14_MHz_XTAL / 4);
	m_maincpu->set_memory_map(&tsconf_state::tsconf_mem);
	m_maincpu->set_io_map(&tsconf_state::tsconf_io);
	m_maincpu->set_m1_map(&tsconf_state::tsconf_switch);
	m_maincpu->set_irq_acknowledge_callback(FUNC(tsconf_state::irq_vector));

	m_maincpu->set_vblank_int("screen", FUNC(tsconf_state::tsconf_vblank_interrupt));

	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sdhc();
	m_sdcard->spi_miso_callback().set(FUNC(tsconf_state::tsconf_spi_miso_w));

	TSCONF_RS232(config, m_uart, XTAL(11'059'200));
	m_uart->out_txd_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->out_rts_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(tsconf_rs232_device::rxd_w));
	rs232.cts_handler().set(m_uart, FUNC(tsconf_rs232_device::cts_w));

	zxbus_device &zxbus(ZXBUS(config, "zxbus", 0));
	ZXBUS_SLOT(config, "zxbus1", 0, zxbus, zxbus_cards, nullptr);
	//ZXBUS_SLOT(config, "zxbus2", 0, zxbus, zxbus_cards, nullptr);

	m_ram->set_default_size("4096K").set_default_value(0x00); // must be random but 0x00 behaves better than 0xff in tested software

	GLUKRS(config, m_glukrs, 32.768_kHz_XTAL);

	TSCONF_DMA(config, m_dma, 28_MHz_XTAL);
	m_dma->in_mreq_callback().set(FUNC(tsconf_state::ram_read16));
	m_dma->out_mreq_callback().set(FUNC(tsconf_state::ram_write16));
	m_dma->in_spireq_callback().set(FUNC(tsconf_state::spi_read16));
	m_dma->out_cram_callback().set(FUNC(tsconf_state::cram_write16));
	m_dma->out_sfile_callback().set(FUNC(tsconf_state::sfile_write16));
	m_dma->on_ready_callback().set(FUNC(tsconf_state::dma_ready));

	TSCONF_BETA(config, m_beta, 0);
	m_beta->out_dos_callback().set(FUNC(tsconf_state::update_io));
	m_beta->out_vdos_m1_callback().set([this](int state) { m_update_on_m1 = true; });

	SPEAKER(config.replace(), "speakers", 2).front();

	AY_SLOT(config.replace(), "ay_slot", 14_MHz_XTAL / 8, default_ay_slot_devices, "ay_ym2149")
		.add_route(0, "speakers", 0.50, 0)
		.add_route(1, "speakers", 0.25, 0)
		.add_route(1, "speakers", 0.25, 1)
		.add_route(2, "speakers", 0.50, 1);

	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speakers", 0.75);

	PALETTE(config, "palette", palette_device::BLACK, 256);
	m_screen->set_raw(14_MHz_XTAL / 2, 448, with_hblank(0), 448, 320, with_vblank(0), 320);
	m_screen->set_screen_update(FUNC(tsconf_state::screen_update));
	m_screen->set_no_palette();

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_tsconf);
	SPECTRUM_ULA_UNCONTENDED(config.replace(), m_ula);

	RAM(config, m_cram).set_default_size("512").set_default_value(0);
	RAM(config, m_sfile).set_default_size("512").set_default_value(0); // 85*6

	AT_KEYB(config, m_keyboard, pc_keyboard_device::KEYBOARD_TYPE::AT, 3);

	SOFTWARE_LIST(config, "betadisc_list_pent").set_original("spectrum_betadisc_flop");
	SOFTWARE_LIST(config, "betadisc_list_tsconf").set_original("tsconf_betadisc_flop");
}


void tsconf_state::tsconf2(machine_config &config)
{
	tsconf(config);
	TSCONF_COPPER(config, m_copper, 28_MHz_XTAL);
	m_copper->out_wreg_cb().set(FUNC(tsconf_state::tsconf_port_xxaf_w));
	m_copper->set_in_until_pos_cb(FUNC(tsconf_state::copper_until_pos_r));

	m_dma->on_ready_callback().append(m_copper, FUNC(tsconf_copper_device::dma_ready_w));
}


ROM_START(tsconf)
	ROM_REGION(0x080000, "maincpu", ROMREGION_ERASEFF) // ROM: 32 * 16KB
	ROM_DEFAULT_BIOS("v2407")

	ROM_SYSTEM_BIOS(0, "v1", "v1")
	ROMX_LOAD("ts-bios.rom", 0, 0x10000, CRC(b060b0d9) SHA1(820d3539de115141daff220a3cb733fc880d1bab), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v2407", "Update 24.07.28")
	ROMX_LOAD("ts-bios.240728.rom", 0, 0x10000, CRC(19f8ad7b) SHA1(9cee82d4a6212686358a50b0fd5a2981b3323ab6), ROM_BIOS(1))

	ROM_REGION(0x200, "cram_init", ROMREGION_ERASEFF)
	ROM_LOAD( "cram-init.bin", 0, 0x200, CRC(8b96ffb7) SHA1(4dbd22f4312251e922911a01526cbfba77a122fc))
ROM_END

ROM_START(tsconf2)
	ROM_REGION(0x080000, "maincpu", ROMREGION_ERASEFF) // ROM: 32 * 16KB
	ROM_DEFAULT_BIOS("v2407")

	ROM_SYSTEM_BIOS(0, "v2407", "Update 24.07.28")
	ROMX_LOAD("ts-bios.240728.rom", 0, 0x10000, CRC(19f8ad7b) SHA1(9cee82d4a6212686358a50b0fd5a2981b3323ab6), ROM_BIOS(0))

	ROM_REGION(0x200, "cram_init", ROMREGION_ERASEFF)
	ROM_LOAD( "cram-init.bin", 0, 0x200, CRC(8b96ffb7) SHA1(4dbd22f4312251e922911a01526cbfba77a122fc))
ROM_END


//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       CLASS           INIT        COMPANY             FULLNAME                            FLAGS
COMP( 2011, tsconf,     0,          0,      tsconf,     tsconf,     tsconf_state,   empty_init, "NedoPC, TS-Labs",  "ZX Evolution: TS-Configuration",   MACHINE_SUPPORTS_SAVE)
COMP( 2024, tsconf2,    tsconf,     0,      tsconf2,    tsconf,     tsconf_state,   empty_init, "TS-Labs",          "EvoMAX3: TS-Configuration 2",      MACHINE_SUPPORTS_SAVE)
