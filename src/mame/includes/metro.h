// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/*************************************************************************

    Metro Games

*************************************************************************/
#ifndef MAME_INCLUDES_METRO_H
#define MAME_INCLUDES_METRO_H

#pragma once

#include "sound/okim6295.h"
#include "sound/ym2151.h"
#include "sound/es8712.h"
#include "video/k053936.h"
#include "video/imagetek_i4100.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
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
		, m_irq_enable(*this, "irq_enable")
		, m_irq_levels(*this, "irq_levels")
		, m_irq_vectors(*this, "irq_vectors")
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
	void init_gakusai();
	void init_dharmak();
	void init_puzzlet();
	void init_metro();
	void init_lastfortg();

	DECLARE_CUSTOM_INPUT_MEMBER(custom_soundstatus_r);

private:
	enum
	{
		TIMER_KARATOUR_IRQ,
		TIMER_MOUJA_IRQ
	};

	u8 irq_cause_r(offs_t offset);
	void irq_cause_w(offs_t offset, u8 data);
	uint8_t irq_vector_r(offs_t offset);
	DECLARE_WRITE16_MEMBER(mouja_irq_timer_ctrl_w);
	DECLARE_WRITE8_MEMBER(soundlatch_w);
	DECLARE_READ8_MEMBER(soundstatus_r);
	DECLARE_WRITE8_MEMBER(soundstatus_w);
	template<int Mask> DECLARE_WRITE8_MEMBER(upd7810_rombank_w);
	DECLARE_READ8_MEMBER(upd7810_porta_r);
	DECLARE_WRITE8_MEMBER(upd7810_porta_w);
	DECLARE_WRITE8_MEMBER(upd7810_portb_w);
	DECLARE_WRITE8_MEMBER(daitorid_portb_w);
	DECLARE_WRITE8_MEMBER(coin_lockout_1word_w);
	DECLARE_WRITE16_MEMBER(coin_lockout_4words_w);
	DECLARE_READ16_MEMBER(balcube_dsw_r);
	DECLARE_READ16_MEMBER(gakusai_input_r);
	DECLARE_WRITE8_MEMBER(blzntrnd_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(puzzlet_irq_enable_w);
	DECLARE_WRITE16_MEMBER(puzzlet_portb_w);
	DECLARE_WRITE16_MEMBER(k053936_w);
	DECLARE_WRITE8_MEMBER(gakusai_oki_bank_hi_w);
	DECLARE_WRITE8_MEMBER(gakusai_oki_bank_lo_w);
	DECLARE_READ8_MEMBER(gakusai_eeprom_r);
	DECLARE_WRITE8_MEMBER(gakusai_eeprom_w);
	DECLARE_READ8_MEMBER(dokyusp_eeprom_r);
	DECLARE_WRITE8_MEMBER(dokyusp_eeprom_bit_w);
	DECLARE_WRITE8_MEMBER(dokyusp_eeprom_reset_w);
	DECLARE_WRITE8_MEMBER(mouja_sound_rombank_w);
	DECLARE_WRITE_LINE_MEMBER(vdp_blit_end_w);

	// vmetal
	DECLARE_WRITE8_MEMBER(vmetal_control_w);
	DECLARE_WRITE8_MEMBER(es8712_reset_w);
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

	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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
	optional_shared_ptr<uint16_t> m_irq_enable;
	optional_shared_ptr<uint16_t> m_irq_levels;
	optional_shared_ptr<uint16_t> m_irq_vectors;
	optional_shared_ptr<uint16_t> m_input_sel;
	optional_shared_ptr<uint16_t> m_k053936_ram;

	optional_memory_bank m_audiobank;
	optional_memory_bank m_okibank;

	/* video-related */
	tilemap_t   *m_k053936_tilemap;

	/* irq_related */
	int         m_vblank_bit;
	int         m_blitter_bit;
	int         m_irq_line;
	uint8_t     m_requested_int[8];
	emu_timer   *m_mouja_irq_timer;
	emu_timer   *m_karatour_irq_timer;

	/* sound related */
	uint16_t      m_soundstatus;
	int         m_porta;
	int         m_portb;
	int         m_busy_sndcpu;
	int         m_essnd_bank;
	bool        m_essnd_gate;

	/* misc */
	int         m_gakusai_oki_bank_lo;
	int         m_gakusai_oki_bank_hi;

	void update_irq_state();
	void metro_common();
	void gakusai_oki_bank_set();
};

#endif // MAME_INCLUDES_METRO_H
