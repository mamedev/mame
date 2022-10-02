// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/*************************************************************************

    Metro Games

*************************************************************************/
#ifndef MAME_METRO_METRO_H
#define MAME_METRO_METRO_H

#pragma once

#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/es8712.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "video/imagetek_i4100.h"
#include "video/k053936.h"

#include "screen.h"
#include "tilemap.h"

class metro_state : public driver_device
{
public:
	metro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_ymsnd(*this, "ymsnd")
		, m_essnd(*this, "essnd")
		, m_vdp(*this, "vdp")
		, m_vdp2(*this, "vdp2")
		, m_vdp3(*this, "vdp3")
		, m_k053936(*this, "k053936")
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_soundlatch(*this, "soundlatch")
		, m_input_sel(*this, "input_sel")
		, m_k053936_ram(*this, "k053936_ram")
		, m_audiobank(*this, "audiobank")
		, m_okibank(*this, "okibank")
	{ }

	void i4100_config(machine_config &config);
	void i4100_config_360x224(machine_config &config);
	void i4220_config(machine_config &config);
	void i4220_config_320x240(machine_config &config);
	void i4220_config_304x224(machine_config &config);
	void i4300_config(machine_config &config);
	void i4300_config_384x224(machine_config &config);
	void i4300_config_320x240(machine_config &config);
	void metro_upd7810_sound(machine_config &config);
	void daitorid_upd7810_sound(machine_config &config);
	void poitto(machine_config &config);
	void blzntrnd(machine_config &config);
	void sankokushi(machine_config &config);
	void mouja(machine_config &config);
	void toride2g(machine_config &config);
	void karatour(machine_config &config);
	void skyalert(machine_config &config);
	void gakusai(machine_config &config);
	void batlbubl(machine_config &config);
	void pururun(machine_config &config);
	void vmetal(machine_config &config);
	void daitorid(machine_config &config);
	void puzzli(machine_config &config);
	void puzzlia(machine_config &config);
	void pangpoms(machine_config &config);
	void dokyusp(machine_config &config);
	void dokyusei(machine_config &config);
	void daitoa(machine_config &config);
	void lastfort(machine_config &config);
	void puzzlet(machine_config &config);
	void gakusai2(machine_config &config);
	void balcube(machine_config &config);
	void msgogo(machine_config &config);
	void gstrik2(machine_config &config);
	void lastforg(machine_config &config);
	void bangball(machine_config &config);
	void dharma(machine_config &config);

	void init_karatour();
	void init_blzntrnd();
	void init_vmetal();
	void init_mouja();
	void init_balcube();
	void init_dharmak();
	void init_metro();
	void init_lastfortg();
	void init_puzzlet() { save_item(NAME(m_ext_irq_enable)); }

	DECLARE_READ_LINE_MEMBER(custom_soundstatus_r);

private:
	virtual void machine_start() override;

	void ipl_w(u8 data);
	void mouja_irq_timer_ctrl_w(u16 data);
	void sound_data_w(u8 data);
	TIMER_CALLBACK_MEMBER(sound_data_sync);
	u8 soundstatus_r();
	void soundstatus_w(u8 data);
	template<int Mask> void upd7810_rombank_w(u8 data);
	u8 upd7810_porta_r();
	void upd7810_porta_w(u8 data);
	void upd7810_portb_w(u8 data);
	void daitorid_portb_w(u8 data);
	void coin_lockout_1word_w(u8 data);
	void coin_lockout_4words_w(offs_t offset, u16 data);
	u16 balcube_dsw_r(offs_t offset);
	u16 gakusai_input_r();
	void blzntrnd_sh_bankswitch_w(u8 data);
	void puzzlet_irq_enable_w(u8 data);
	void puzzlet_portb_w(u16 data);
	void k053936_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void gakusai_oki_bank_hi_w(u8 data);
	void gakusai_oki_bank_lo_w(u8 data);
	u8 gakusai_eeprom_r();
	void gakusai_eeprom_w(u8 data);
	u8 dokyusp_eeprom_r();
	void dokyusp_eeprom_bit_w(u8 data);
	void dokyusp_eeprom_reset_w(u8 data);
	void mouja_sound_rombank_w(u8 data);

	// vmetal
	void vmetal_control_w(u8 data);
	void es8712_reset_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(vmetal_es8712_irq);

	TILE_GET_INFO_MEMBER(k053936_get_tile_info);
	TILE_GET_INFO_MEMBER(k053936_gstrik2_get_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_gstrik2);
	DECLARE_VIDEO_START(blzntrnd);
	DECLARE_VIDEO_START(gstrik2);
	uint32_t screen_update_psac_vdp2_mix(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(periodic_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(bangball_scanline);
	DECLARE_WRITE_LINE_MEMBER(karatour_vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(puzzlet_vblank_irq);
	DECLARE_READ_LINE_MEMBER(rxd_r);

	void balcube_map(address_map &map);
	void bangball_map(address_map &map);
	void batlbubl_map(address_map &map);
	void blzntrnd_map(address_map &map);
	void blzntrnd_sound_io_map(address_map &map);
	void blzntrnd_sound_map(address_map &map);
	void cpu_space_map(address_map &map);
	void daitoa_map(address_map &map);
	void daitorid_map(address_map &map);
	void dharma_map(address_map &map);
	void dokyusei_map(address_map &map);
	void dokyusp_map(address_map &map);
	void gakusai2_map(address_map &map);
	void gakusai_map(address_map &map);
	void karatour_map(address_map &map);
	void kokushi_map(address_map &map);
	void lastforg_map(address_map &map);
	void lastfort_map(address_map &map);
	void upd7810_map(address_map &map);
	void mouja_map(address_map &map);
	void mouja_okimap(address_map &map);
	void msgogo_map(address_map &map);
	void pangpoms_map(address_map &map);
	void poitto_map(address_map &map);
	void pururun_map(address_map &map);
	void puzzlet_io_map(address_map &map);
	void puzzlet_map(address_map &map);
	void skyalert_map(address_map &map);
	void toride2g_map(address_map &map);
	void vmetal_map(address_map &map);
	void ymf278_map(address_map &map);

	TIMER_CALLBACK_MEMBER(mouja_irq);

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<device_t> m_ymsnd; // TODO set correct type
	optional_device<es8712_device> m_essnd;
	optional_device<imagetek_i4100_device> m_vdp;
	optional_device<imagetek_i4220_device> m_vdp2;
	optional_device<imagetek_i4300_device> m_vdp3;

	optional_device<k053936_device> m_k053936;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	optional_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	optional_shared_ptr<u16> m_input_sel;
	optional_shared_ptr<u16> m_k053936_ram;

	optional_memory_bank m_audiobank;
	optional_memory_bank m_okibank;

	/* video-related */
	tilemap_t   *m_k053936_tilemap = nullptr;

	/* irq_related */
	emu_timer   *m_mouja_irq_timer = nullptr;

	/* sound related */
	u8          m_sound_data = 0;
	u16    m_soundstatus = 0;
	int         m_porta = 0;
	int         m_portb = 0;
	int         m_busy_sndcpu = 0;
	int         m_essnd_bank = 0;
	bool        m_essnd_gate = false;

	/* misc */
	int         m_gakusai_oki_bank_lo = 0;
	int         m_gakusai_oki_bank_hi = 0;

	void gakusai_oki_bank_set();

	DECLARE_WRITE_LINE_MEMBER(ext_irq5_enable_w);

	bool m_ext_irq_enable = false;
};

#endif // MAME_METRO_METRO_H
