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
		, m_vdp(*this, "vdp")
		, m_vdp2(*this, "vdp2")
		, m_vdp3(*this, "vdp3")
		, m_screen(*this, "screen")
		, m_io_dsw(*this, "DSW%u", 0U)
		, m_io_in(*this, "IN%u", 0U)
		, m_audiobank(*this, "audiobank")
	{ }

	void i4100_config(machine_config &config);
	void i4100_config_360x224(machine_config &config);
	void i4220_config(machine_config &config);
	void i4220_config_320x240(machine_config &config);
	void i4220_config_304x224(machine_config &config);
	void i4300_config(machine_config &config);
	void i4300_config_384x224(machine_config &config);
	void i4300_config_320x240(machine_config &config);
	void balcube(machine_config &config);
	void bangball(machine_config &config);
	void batlbubl(machine_config &config);
	void daitoa(machine_config &config);
	void msgogo(machine_config &config);
	void puzzlet(machine_config &config);

	void init_balcube();
	void init_karatour();

protected:
	virtual void machine_start() override {}

	void ipl_w(u8 data);
	void coin_lockout_1word_w(u8 data);
	void coin_lockout_4words_w(offs_t offset, u16 data);
	u16 balcube_dsw_r(offs_t offset);
	void puzzlet_irq_enable_w(u8 data);
	void puzzlet_portb_w(u8 data);

	void ext_irq5_enable_w(int state);

	void vblank_irq(int state);
	INTERRUPT_GEN_MEMBER(periodic_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(bangball_scanline);
	void karatour_vblank_irq(int state);
	void puzzlet_vblank_irq(int state);

	void balcube_map(address_map &map) ATTR_COLD;
	void bangball_map(address_map &map) ATTR_COLD;
	void batlbubl_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;
	void daitoa_map(address_map &map) ATTR_COLD;
	void msgogo_map(address_map &map) ATTR_COLD;
	void puzzlet_map(address_map &map) ATTR_COLD;
	void ymf278_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<device_t> m_ymsnd; // TODO set correct type
	optional_device<imagetek_i4100_device> m_vdp;
	optional_device<imagetek_i4220_device> m_vdp2;
	optional_device<imagetek_i4300_device> m_vdp3;

	required_device<screen_device> m_screen;

	optional_ioport_array<2> m_io_dsw;
	optional_ioport_array<4> m_io_in;

	optional_memory_bank m_audiobank;

	bool m_ext_irq_enable = false;
};

// with Sound uPD7810
class metro_upd7810_state : public metro_state
{
public:
	metro_upd7810_state(const machine_config &mconfig, device_type type, const char *tag)
		: metro_state(mconfig, type, tag)
	{ }

	void metro_upd7810_sound(machine_config &config);
	void daitorid_upd7810_sound(machine_config &config);
	void daitorid(machine_config &config);
	void dharma(machine_config &config);
	void karatour(machine_config &config);
	void lastforg(machine_config &config);
	void lastfort(machine_config &config);
	void pangpoms(machine_config &config);
	void poitto(machine_config &config);
	void pururun(machine_config &config);
	void puzzli(machine_config &config);
	void puzzlia(machine_config &config);
	void sankokushi(machine_config &config);
	void skyalert(machine_config &config);
	void toride2g(machine_config &config);

	void init_dharmak();

	int custom_soundstatus_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void sound_data_w(u8 data);
	TIMER_CALLBACK_MEMBER(sound_data_sync);
	u8 soundstatus_r();
	void soundstatus_w(u8 data);
	template<int Mask> void upd7810_rombank_w(u8 data);
	u8 upd7810_porta_r();
	void upd7810_porta_w(u8 data);
	void upd7810_portb_w(u8 data);
	void daitorid_portb_w(u8 data);

	int rxd_r();

	void daitorid_map(address_map &map) ATTR_COLD;
	void dharma_map(address_map &map) ATTR_COLD;
	void karatour_map(address_map &map) ATTR_COLD;
	void kokushi_map(address_map &map) ATTR_COLD;
	void lastforg_map(address_map &map) ATTR_COLD;
	void lastfort_map(address_map &map) ATTR_COLD;
	void pangpoms_map(address_map &map) ATTR_COLD;
	void poitto_map(address_map &map) ATTR_COLD;
	void pururun_map(address_map &map) ATTR_COLD;
	void skyalert_map(address_map &map) ATTR_COLD;
	void toride2g_map(address_map &map) ATTR_COLD;
	void upd7810_map(address_map &map) ATTR_COLD;

	// sound related
	u8   m_sound_data = 0;
	u16  m_soundstatus = 0;
	u8   m_porta = 0;
	u8   m_portb = 0;
	bool m_busy_sndcpu = false;
};

// with Mahjong input
class gakusai_state : public metro_state
{
public:
	gakusai_state(const machine_config &mconfig, device_type type, const char *tag)
		: metro_state(mconfig, type, tag)
		, m_eeprom(*this, "eeprom")
		, m_input_sel(*this, "input_sel")
		, m_io_key(*this, "KEY%u", 0U)
	{ }

	void dokyusei(machine_config &config);
	void dokyusp(machine_config &config);
	void gakusai2(machine_config &config);
	void gakusai(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u16 input_r();
	void oki_bank_hi_w(u8 data);
	void oki_bank_lo_w(u8 data);
	u8 gakusai_eeprom_r();
	void gakusai_eeprom_w(u8 data);
	u8 dokyusp_eeprom_r();
	void dokyusp_eeprom_bit_w(u8 data);
	void dokyusp_eeprom_reset_w(u8 data);

	void oki_bank_set();

	void dokyusei_map(address_map &map) ATTR_COLD;
	void dokyusp_map(address_map &map) ATTR_COLD;
	void gakusai2_map(address_map &map) ATTR_COLD;
	void gakusai_map(address_map &map) ATTR_COLD;

	// devices
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	// memory pointers
	required_shared_ptr<u16> m_input_sel;

	required_ioport_array<5> m_io_key;

	// misc
	u8 m_oki_bank_lo = 0;
	u8 m_oki_bank_hi = 0;
};

// with ES8712+MSM6585 sound
class vmetal_state : public metro_state
{
public:
	vmetal_state(const machine_config &mconfig, device_type type, const char *tag)
		: metro_state(mconfig, type, tag)
		, m_essnd(*this, "essnd")
	{ }

	void vmetal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// vmetal
	void vmetal_control_w(u8 data);
	void es8712_reset_w(u8 data);
	void es8712_irq(int state);

	void main_map(address_map &map) ATTR_COLD;

	// devices
	required_device<es8712_device> m_essnd;

	// sound related
	u8   m_essnd_bank = 0;
	bool m_essnd_gate = false;
};

// with Configurable timer
class mouja_state : public metro_state
{
public:
	mouja_state(const machine_config &mconfig, device_type type, const char *tag)
		: metro_state(mconfig, type, tag)
		, m_okibank(*this, "okibank")
	{ }

	void mouja(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void irq_timer_ctrl_w(u16 data);
	void sound_rombank_w(u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(mouja_irq);

	required_memory_bank m_okibank;

	// irq_related
	emu_timer *m_mouja_irq_timer = nullptr;
};

// with K053936 PSAC2
class blzntrnd_state : public metro_state
{
public:
	blzntrnd_state(const machine_config &mconfig, device_type type, const char *tag)
		: metro_state(mconfig, type, tag)
		, m_gfxdecode(*this, "gfxdecode")
		, m_soundlatch(*this, "soundlatch")
		, m_k053936(*this, "k053936")
		, m_k053936_ram(*this, "k053936_ram")
	{ }

	void blzntrnd(machine_config &config);
	void gstrik2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void audiobank_w(u8 data);
	void k053936_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(k053936_get_tile_info);
	TILE_GET_INFO_MEMBER(k053936_gstrik2_get_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_gstrik2);
	DECLARE_VIDEO_START(blzntrnd);
	DECLARE_VIDEO_START(gstrik2);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// devices
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<k053936_device> m_k053936;

	// memory pointers
	required_shared_ptr<u16> m_k053936_ram;

	// video-related
	tilemap_t *m_k053936_tilemap = nullptr;
};

#endif // MAME_METRO_METRO_H
