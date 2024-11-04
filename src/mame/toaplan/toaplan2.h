// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood
#ifndef MAME_TOAPLAN_TOAPLAN2_H
#define MAME_TOAPLAN_TOAPLAN2_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/bankdev.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/ticket.h"
#include "machine/upd4992.h"
#include "gp9001.h"
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
		, m_mainram(*this, "mainram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001_%u", 0U)
		, m_oki(*this, "oki%u", 1U)
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch%u", 1U)
		, m_z80_rom(*this, "audiocpu")
		, m_oki_rom(*this, "oki%u", 1U)
		, m_okibank(*this, "okibank")
	{ }

	void batsugun(machine_config &config);
	void batsugunbl(machine_config &config);
	void dogyuun(machine_config &config);
	void dogyuunto(machine_config &config);
	void enmadaio(machine_config &config);
	void kbash(machine_config &config);
	void kbash2(machine_config &config);
	void pipibibs(machine_config &config);
	void pipibibsbl(machine_config &config);
	void snowbro2(machine_config &config);
	void snowbro2b3(machine_config &config);
	void tekipaki(machine_config &config);
	void vfive(machine_config &config);

	void init_dogyuun();
	void init_enmadaio();
	void init_fixeightbl();
	void init_pipibibsbl();
	void init_vfive();

	int c2map_r();

protected:
	// We encode priority with colour in the tilemaps, so need a larger palette
	static constexpr unsigned T2PALETTE_LENGTH = 0x10000;

	optional_shared_ptr<u8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	optional_shared_ptr<u16> m_mainram;

	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device_array<gp9001vdp_device, 2> m_vdp;
	optional_device_array<okim6295_device, 2> m_oki;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device_array<generic_latch_8_device, 4> m_soundlatch; // tekipaki, batrider, bgaregga, batsugun

	optional_region_ptr<u8> m_z80_rom;
	optional_region_ptr_array<u8, 2> m_oki_rom;
	optional_memory_bank m_okibank;

	u8 m_sound_reset_bit = 0; /* 0x20 for dogyuun/batsugun, 0x10 for vfive, 0x08 for fixeight */

	bitmap_ind8 m_custom_priority_bitmap;
	bitmap_ind16 m_secondary_render_bitmap;

	u16 video_count_r();
	void coin_w(u8 data);
	void coin_sound_reset_w(u8 data);
	u8 shared_ram_r(offs_t offset);
	void shared_ram_w(offs_t offset, u8 data);
	void sound_reset_w(u8 data);
	void fixeightbl_oki_bankswitch_w(u8 data);

	template<int Chip> void oki_bankswitch_w(u8 data);
	void enmadaio_oki_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	DECLARE_VIDEO_START(toaplan2);
	DECLARE_VIDEO_START(batsugunbl);

	// Teki Paki sound
	u8 tekipaki_cmdavailable_r();

	u32 screen_update_toaplan2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_dogyuun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_batsugun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void cpu_space_fixeightbl_map(address_map &map) ATTR_COLD;
	void cpu_space_pipibibsbl_map(address_map &map) ATTR_COLD;

	void toaplan2_reset(int state);

	void batsugun_68k_mem(address_map &map) ATTR_COLD;
	void batsugunbl_68k_mem(address_map &map) ATTR_COLD;
	void dogyuun_68k_mem(address_map &map) ATTR_COLD;
	void dogyuunto_68k_mem(address_map &map) ATTR_COLD;
	void dogyuunto_sound_z80_mem(address_map &map) ATTR_COLD;
	void enmadaio_68k_mem(address_map &map) ATTR_COLD;
	void enmadaio_oki(address_map &map) ATTR_COLD;
	void fixeightbl_oki(address_map &map) ATTR_COLD;
	void hd647180_io_map(address_map &map) ATTR_COLD;
	void kbash2_68k_mem(address_map &map) ATTR_COLD;
	void kbash_68k_mem(address_map &map) ATTR_COLD;
	void kbash_v25_mem(address_map &map) ATTR_COLD;
	void pipibibi_bootleg_68k_mem(address_map &map) ATTR_COLD;
	void pipibibs_68k_mem(address_map &map) ATTR_COLD;
	void pipibibs_sound_z80_mem(address_map &map) ATTR_COLD;
	void snowbro2_68k_mem(address_map &map) ATTR_COLD;
	void snowbro2b3_68k_mem(address_map &map) ATTR_COLD;
	void tekipaki_68k_mem(address_map &map) ATTR_COLD;
	void v25_mem(address_map &map) ATTR_COLD;
	void vfive_68k_mem(address_map &map) ATTR_COLD;
	void vfive_v25_mem(address_map &map) ATTR_COLD;
};

// with paddle
class ghox_state : public toaplan2_state
{
public:
	ghox_state(const machine_config &mconfig, device_type type, const char *tag)
		: toaplan2_state(mconfig, type, tag)
		, m_io_pad(*this, "PAD%u", 1U)
	{ }

	void ghox(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_ioport_array<2> m_io_pad;

	s8 m_old_paddle_h[2] = {0};

	template<unsigned Which> u16 ghox_h_analog_r();

	void ghox_68k_mem(address_map &map) ATTR_COLD;
	void ghox_hd647180_mem_map(address_map &map) ATTR_COLD;
};

// with text layer
class truxton2_state : public toaplan2_state
{
public:
	truxton2_state(const machine_config &mconfig, device_type type, const char *tag)
		: toaplan2_state(mconfig, type, tag)
		, m_tx_videoram(*this, "tx_videoram")
		, m_tx_lineselect(*this, "tx_lineselect")
		, m_tx_linescroll(*this, "tx_linescroll")
		, m_tx_gfxram(*this, "tx_gfxram")
		, m_dma_space(*this, "dma_space")
		, m_audiobank(*this, "audiobank")
		, m_raizing_okibank{
			{ *this, "raizing_okibank0_%u", 0U },
			{ *this, "raizing_okibank1_%u", 0U } }
		, m_eepromout(*this, "EEPROMOUT")
	{ }

	void batrider(machine_config &config);
	void bbakraid(machine_config &config);
	void bgaregga(machine_config &config);
	void bgareggabl(machine_config &config);
	void fixeight(machine_config &config);
	void fixeightbl(machine_config &config);
	void mahoudai(machine_config &config);
	void nprobowl(machine_config &config);
	void shippumd(machine_config &config);
	void truxton2(machine_config &config);

	void init_batrider();
	void init_bbakraid();
	void init_bgaregga();
	void init_fixeight();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	required_shared_ptr<u16> m_tx_videoram;
	optional_shared_ptr<u16> m_tx_lineselect;
	optional_shared_ptr<u16> m_tx_linescroll;
	optional_shared_ptr<u16> m_tx_gfxram;

	optional_device<address_map_bank_device> m_dma_space;

	optional_memory_bank m_audiobank;
	optional_memory_bank_array<8> m_raizing_okibank[2];

	optional_ioport m_eepromout;

	u8 m_sndirq_line = 0;        /* IRQ4 for batrider, IRQ2 for bbakraid */
	u8 m_z80_busreq = 0;
	u16 m_gfxrom_bank[8]{};       /* Batrider object bank */

	tilemap_t *m_tx_tilemap = nullptr;    /* Tilemap for extra-text-layer */

	void shippumd_coin_w(u8 data);
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

	void install_raizing_okibank(int chip);

	TILE_GET_INFO_MEMBER(get_text_tile_info);
	DECLARE_MACHINE_RESET(bgaregga);
	DECLARE_VIDEO_START(truxton2);
	DECLARE_VIDEO_START(fixeightbl);
	DECLARE_VIDEO_START(bgaregga);
	DECLARE_VIDEO_START(bgareggabl);
	DECLARE_VIDEO_START(batrider);

	u32 screen_update_truxton2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(bbakraid_snd_interrupt);
	void create_tx_tilemap(int dx = 0, int dx_flipped = 0);

	void batrider_68k_mem(address_map &map) ATTR_COLD;
	void batrider_dma_mem(address_map &map) ATTR_COLD;
	void batrider_sound_z80_mem(address_map &map) ATTR_COLD;
	void batrider_sound_z80_port(address_map &map) ATTR_COLD;
	void bbakraid_68k_mem(address_map &map) ATTR_COLD;
	void bbakraid_sound_z80_mem(address_map &map) ATTR_COLD;
	void bbakraid_sound_z80_port(address_map &map) ATTR_COLD;
	void bgaregga_68k_mem(address_map &map) ATTR_COLD;
	void bgaregga_sound_z80_mem(address_map &map) ATTR_COLD;
	void fixeight_68k_mem(address_map &map) ATTR_COLD;
	void fixeight_v25_mem(address_map &map) ATTR_COLD;
	void fixeightbl_68k_mem(address_map &map) ATTR_COLD;
	void mahoudai_68k_mem(address_map &map) ATTR_COLD;
	void nprobowl_68k_mem(address_map &map) ATTR_COLD;
	template<unsigned Chip> void raizing_oki(address_map &map) ATTR_COLD;
	void raizing_sound_z80_mem(address_map &map) ATTR_COLD;
	void shippumd_68k_mem(address_map &map) ATTR_COLD;
	void truxton2_68k_mem(address_map &map) ATTR_COLD;
};


// with RTC, hopper
class pwrkick_state : public toaplan2_state
{
public:
	pwrkick_state(const machine_config &mconfig, device_type type, const char *tag)
		: toaplan2_state(mconfig, type, tag)
		, m_rtc(*this, "rtc")
		, m_hopper(*this, "hopper")
	{ }

	void othldrby(machine_config &config);
	void pwrkick(machine_config &config);

private:
	required_device<upd4992_device> m_rtc;
	optional_device<ticket_dispenser_device> m_hopper;

	void pwrkick_coin_w(u8 data);
	void pwrkick_coin_lockout_w(u8 data);

	void othldrby_68k_mem(address_map &map) ATTR_COLD;
	void pwrkick_68k_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_TOAPLAN_TOAPLAN2_H
