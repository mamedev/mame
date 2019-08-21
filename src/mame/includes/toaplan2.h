// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood
#ifndef MAME_INCLUDES_TOAPLAN2_H
#define MAME_INCLUDES_TOAPLAN2_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/bankdev.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/ticket.h"
#include "machine/upd4992.h"
#include "video/gp9001.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

/**************** Machine stuff ******************/

class toaplan2_state : public driver_device
{
public:
	toaplan2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_shared_ram(*this, "shared_ram")
		, m_tx_videoram(*this, "tx_videoram")
		, m_tx_lineselect(*this, "tx_lineselect")
		, m_tx_linescroll(*this, "tx_linescroll")
		, m_tx_gfxram(*this, "tx_gfxram")
		, m_mainram(*this, "mainram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001_%u", 0U)
		, m_oki(*this, "oki%u", 1U)
		, m_eeprom(*this, "eeprom")
		, m_rtc(*this, "rtc")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch2(*this, "soundlatch2")
		, m_hopper(*this, "hopper")
		, m_dma_space(*this, "dma_space")
		, m_z80_rom(*this, "audiocpu")
		, m_oki_rom(*this, "oki%u", 1U)
		, m_audiobank(*this, "audiobank")
		, m_okibank(*this, "okibank")
		, m_raizing_okibank{{*this, "raizing_okibank0_%u", 0U},
							{*this, "raizing_okibank1_%u", 0U}}
		, m_io_pad(*this, "PAD%u", 1U)
		, m_eepromout(*this, "EEPROMOUT")
	{ }

	void dogyuun(machine_config &config);
	void othldrby(machine_config &config);
	void snowbro2(machine_config &config);
	void bgareggabl(machine_config &config);
	void pwrkick(machine_config &config);
	void mahoudai(machine_config &config);
	void tekipaki(machine_config &config);
	void bbakraid(machine_config &config);
	void fixeightbl(machine_config &config);
	void fixeight(machine_config &config);
	void ghox(machine_config &config);
	void bgaregga(machine_config &config);
	void batrider(machine_config &config);
	void shippumd(machine_config &config);
	void kbash(machine_config &config);
	void pipibibs(machine_config &config);
	void pipibibsbl(machine_config &config);
	void batsugun(machine_config &config);
	void enmadaio(machine_config &config);
	void truxton2(machine_config &config);
	void vfive(machine_config &config);
	void kbash2(machine_config &config);

	void init_bbakraid();
	void init_pipibibsbl();
	void init_dogyuun();
	void init_fixeight();
	void init_bgaregga();
	void init_fixeightbl();
	void init_vfive();
	void init_batrider();
	void init_enmadaio();

	DECLARE_CUSTOM_INPUT_MEMBER(c2map_r);

protected:
	virtual void device_post_load() override;

private:
	// We encode priority with colour in the tilemaps, so need a larger palette
	static constexpr unsigned T2PALETTE_LENGTH = 0x10000;

	optional_shared_ptr<u8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	optional_shared_ptr<u16> m_tx_videoram;
	optional_shared_ptr<u16> m_tx_lineselect;
	optional_shared_ptr<u16> m_tx_linescroll;
	optional_shared_ptr<u16> m_tx_gfxram;
	optional_shared_ptr<u16> m_mainram;

	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device_array<gp9001vdp_device, 2> m_vdp;
	optional_device_array<okim6295_device, 2> m_oki;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<upd4992_device> m_rtc;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // tekipaki, batrider, bgaregga, batsugun
	optional_device<generic_latch_8_device> m_soundlatch2;
	optional_device<ticket_dispenser_device> m_hopper;

	optional_device<address_map_bank_device> m_dma_space;

	optional_region_ptr<u8> m_z80_rom;
	optional_region_ptr_array<u8, 2> m_oki_rom;
	optional_memory_bank m_audiobank;
	optional_memory_bank m_okibank;
	optional_memory_bank_array<8> m_raizing_okibank[2];

	optional_ioport_array<2> m_io_pad;
	optional_ioport m_eepromout;

	s8 m_old_p1_paddle_h; /* For Ghox */
	s8 m_old_p2_paddle_h;
	u8 m_sound_reset_bit; /* 0x20 for dogyuun/batsugun, 0x10 for vfive, 0x08 for fixeight */
	u8 m_sndirq_line;        /* IRQ4 for batrider, IRQ2 for bbakraid */
	u8 m_z80_busreq;
	u16 m_gfxrom_bank[8];       /* Batrider object bank */

	bitmap_ind8 m_custom_priority_bitmap;
	bitmap_ind16 m_secondary_render_bitmap;

	tilemap_t *m_tx_tilemap;    /* Tilemap for extra-text-layer */
	u16 video_count_r();
	void coin_w(u8 data);
	void coin_sound_reset_w(u8 data);
	void shippumd_coin_w(u8 data);
	u8 shared_ram_r(offs_t offset);
	void shared_ram_w(offs_t offset, u8 data);
	u16 ghox_p1_h_analog_r();
	u16 ghox_p2_h_analog_r();
	void sound_reset_w(u8 data);
	void fixeightbl_oki_bankswitch_w(u8 data);
	void raizing_z80_bankswitch_w(u8 data);
	void raizing_oki_bankswitch_w(offs_t offset, u8 data);
	u8 bgaregga_E01D_r();
	u16 batrider_z80_busack_r();
	void batrider_z80_busreq_w(u8 data);
	u16 batrider_z80rom_r(offs_t offset);
	void batrider_soundlatch_w(u8 data);
	void batrider_soundlatch2_w(u8 data);
	void batrider_unknown_sound_w(u16 data);
	void batrider_clear_sndirq_w(u16 data);
	void batrider_sndirq_w(u8 data);
	void batrider_clear_nmi_w(u8 data);
	u16 bbakraid_eeprom_r();
	void bbakraid_eeprom_w(u8 data);
	void tx_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tx_linescroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tx_gfxram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void batrider_tx_gfxram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void batrider_textdata_dma_w(u16 data);
	void batrider_pal_text_dma_w(u16 data);
	void batrider_objectbank_w(offs_t offset, u8 data);
	void batrider_bank_cb(u8 layer, u32 &code);

	template<int Chip> void oki_bankswitch_w(u8 data);
	void enmadaio_oki_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void install_raizing_okibank(int chip);

	TILE_GET_INFO_MEMBER(get_text_tile_info);
	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(toaplan2);
	DECLARE_MACHINE_RESET(bgaregga);
	DECLARE_VIDEO_START(toaplan2);
	DECLARE_MACHINE_RESET(ghox);
	DECLARE_VIDEO_START(truxton2);
	DECLARE_VIDEO_START(fixeightbl);
	DECLARE_VIDEO_START(bgaregga);
	DECLARE_VIDEO_START(bgareggabl);
	DECLARE_VIDEO_START(batrider);

	// Teki Paki sound
	u8 tekipaki_cmdavailable_r();

	u32 screen_update_toaplan2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_dogyuun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_batsugun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_truxton2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void cpu_space_fixeightbl_map(address_map &map);
	void cpu_space_pipibibsbl_map(address_map &map);
	INTERRUPT_GEN_MEMBER(bbakraid_snd_interrupt);
	void create_tx_tilemap(int dx = 0, int dx_flipped = 0);

	void pwrkick_coin_w(u8 data);
	void pwrkick_coin_lockout_w(u8 data);

	DECLARE_WRITE_LINE_MEMBER(toaplan2_reset);

	void batrider_68k_mem(address_map &map);
	void batrider_dma_mem(address_map &map);
	void batrider_sound_z80_mem(address_map &map);
	void batrider_sound_z80_port(address_map &map);
	void batsugun_68k_mem(address_map &map);
	void bbakraid_68k_mem(address_map &map);
	void bbakraid_sound_z80_mem(address_map &map);
	void bbakraid_sound_z80_port(address_map &map);
	void bgaregga_68k_mem(address_map &map);
	void bgaregga_sound_z80_mem(address_map &map);
	void dogyuun_68k_mem(address_map &map);
	void enmadaio_68k_mem(address_map &map);
	void enmadaio_oki(address_map &map);
	void fixeight_68k_mem(address_map &map);
	void fixeight_v25_mem(address_map &map);
	void fixeightbl_68k_mem(address_map &map);
	void fixeightbl_oki(address_map &map);
	void ghox_68k_mem(address_map &map);
	void ghox_hd647180_mem_map(address_map &map);
	void hd647180_io_map(address_map &map);
	void hd647180_mem_map(address_map &map);
	void kbash2_68k_mem(address_map &map);
	void kbash_68k_mem(address_map &map);
	void kbash_v25_mem(address_map &map);
	void mahoudai_68k_mem(address_map &map);
	void othldrby_68k_mem(address_map &map);
	void pipibibi_bootleg_68k_mem(address_map &map);
	void pipibibs_68k_mem(address_map &map);
	void pipibibs_sound_z80_mem(address_map &map);
	void pwrkick_68k_mem(address_map &map);
	template<unsigned Chip> void raizing_oki(address_map &map);
	void raizing_sound_z80_mem(address_map &map);
	void shippumd_68k_mem(address_map &map);
	void snowbro2_68k_mem(address_map &map);
	void tekipaki_68k_mem(address_map &map);
	void truxton2_68k_mem(address_map &map);
	void v25_mem(address_map &map);
	void vfive_68k_mem(address_map &map);
	void vfive_v25_mem(address_map &map);
};

#endif // MAME_INCLUDES_TOAPLAN2_H
