// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_KONAMI_NEMESIS_H
#define MAME_KONAMI_NEMESIS_H

#pragma once

#include "machine/timer.h"
#include "sound/flt_rc.h"
#include "sound/k007232.h"
#include "sound/k005289.h"
#include "sound/vlm5030.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

// base class
class gx400_base_state : public driver_device
{
protected:
	gx400_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_charram(*this, "charram"),
		m_xscroll(*this, "xscroll%u", 1U),
		m_yscroll(*this, "yscroll%u", 1U),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram%u", 1U),
		m_spriteram(*this, "spriteram"),
		m_io_in3(*this, "IN3"),
		m_io_wheel(*this, "WHEEL")
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_charram;
	required_shared_ptr_array<uint16_t, 2> m_xscroll;
	required_shared_ptr_array<uint16_t, 2> m_yscroll;
	required_shared_ptr_array<uint16_t, 2> m_videoram;
	required_shared_ptr_array<uint16_t, 2> m_colorram;
	required_shared_ptr<uint16_t> m_spriteram;

	optional_ioport m_io_in3;
	optional_ioport m_io_wheel;

	/* video-related */
	tilemap_t *m_tilemap[2]{};
	uint32_t  m_spriteram_words = 0;
	uint32_t  m_tilemap_flip = 0;
	bool      m_flipscreen = false;
	uint8_t   m_blank_tile[8*8]{};

	/* misc */
	bool      m_irq_on = false;
	bool      m_irq2_on = false;

	void irq2_enable_w(int state);
	uint16_t konamigt_input_word_r();
	template <unsigned Which> void videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <unsigned Which> void colorram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void charram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <unsigned Which> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nemesis_vblank_irq(int state);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_screen_raw_params(machine_config &config);
};

// uses bottom board only
class salamand_state : public gx400_base_state
{
public:
	salamand_state(const machine_config &mconfig, device_type type, const char *tag) :
		gx400_base_state(mconfig, type, tag),
		m_k007232(*this, "k007232")
	{ }

	void nyanpani(machine_config &config) ATTR_COLD;
	void salamand(machine_config &config) ATTR_COLD;
	void blkpnthr(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	/* devices */
	required_device<k007232_device> m_k007232;

	/* misc */
	uint8_t   m_irq_port_last = 0;

	void control_port_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void speech_start_w(uint8_t data);
	uint8_t speech_busy_r();
	void city_sound_bank_w(uint8_t data);
	void blkpnthr_vblank_irq(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(hcrash_interrupt);
	void volume_callback(uint8_t data);

	void blkpnthr_map(address_map &map) ATTR_COLD;
	void blkpnthr_sound_map(address_map &map) ATTR_COLD;
	void city_sound_map(address_map &map) ATTR_COLD;
	void nyanpani_map(address_map &map) ATTR_COLD;
	void sal_sound_map(address_map &map) ATTR_COLD;
	void salamand_map(address_map &map) ATTR_COLD;
	void salamand_vlm_map(address_map &map) ATTR_COLD;
};

// above with accel
class hcrash_state : public salamand_state
{
public:
	hcrash_state(const machine_config &mconfig, device_type type, const char *tag) :
		salamand_state(mconfig, type, tag),
		m_io_accel(*this, "ACCEL")
	{ }

	void hcrash(machine_config &config) ATTR_COLD;
	void citybomb(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_ioport m_io_accel;

	uint8_t   m_selected_ip = 0; // needed for Hyper Crash

	void citybomb_control_port_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void selected_ip_w(uint8_t data);
	uint8_t selected_ip_r();

	TIMER_DEVICE_CALLBACK_MEMBER(hcrash_interrupt);

	void citybomb_map(address_map &map) ATTR_COLD;
	void hcrash_map(address_map &map) ATTR_COLD;
};

// for GX400 and directly derived hardwares
class gx400_state : public gx400_base_state
{
public:
	gx400_state(const machine_config &mconfig, device_type type, const char *tag) :
		gx400_base_state(mconfig, type, tag),
		m_filter(*this, "filter%u", 1U),
		m_k005289(*this, "k005289"),
		m_paletteram(*this, "paletteram"),
		m_sound_shared_ram(*this, "sound_shared")
	{ }

	void konamigt(machine_config &config) ATTR_COLD;
	void rf2_gx400(machine_config &config) ATTR_COLD;
	void gx400(machine_config &config) ATTR_COLD;
	void nemesis(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	/* devices */
	required_device_array<filter_rc_device, 4> m_filter;
	required_device<k005289_device> m_k005289;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint8_t> m_sound_shared_ram;

	/* video-related */
	uint8_t   m_palette_lookup[32]{};

	/* misc */
	bool      m_irq1_on = false;
	bool      m_irq4_on = false;
	//int32_t   m_gx400_irq1_cnt = 0;
	uint32_t  m_speech_offset = 0;

	void irq_enable_w(int state);
	void irq1_enable_w(int state);
	void irq4_enable_w(int state);
	void coin1_lockout_w(int state);
	void coin2_lockout_w(int state);
	void sound_irq_w(int state);
	void sound_nmi_w(int state);
	uint16_t sound_sharedram_word_r(offs_t offset);
	void sound_sharedram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gfx_flipx_w(int state);
	void gfx_flipy_w(int state);
	void palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nemesis_filter_w(offs_t offset, uint8_t data);
	void speech_w(offs_t offset, uint8_t data);
	uint8_t nemesis_portA_r();

	TIMER_DEVICE_CALLBACK_MEMBER(konamigt_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(gx400_interrupt);
	void create_palette_lookups() ATTR_COLD;

	void gx400_map(address_map &map) ATTR_COLD;
	void gx400_sound_map(address_map &map) ATTR_COLD;
	void gx400_vlm_map(address_map &map) ATTR_COLD;
	void konamigt_map(address_map &map) ATTR_COLD;
	void nemesis_map(address_map &map) ATTR_COLD;
	void rf2_gx400_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

// for Bubble System
class bubsys_state : public gx400_state
{
public:
	bubsys_state(const machine_config &mconfig, device_type type, const char *tag) :
		gx400_state(mconfig, type, tag),
		m_bubsys_shared_ram(*this, "bubsys_shared"),
		m_bubsys_control_ram(*this, "bubsys_control"),
		m_bubblememory_region(*this, "bubblememory")
	{ }

	void bubsys(machine_config &config) ATTR_COLD;

	void bubsys_init() ATTR_COLD;
	void bubsys_twinbeeb_init() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_bubsys_shared_ram;
	required_shared_ptr<uint16_t> m_bubsys_control_ram;
	required_memory_region m_bubblememory_region;

	uint16_t  m_scanline_counter = 0;

	void bubsys_mcu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bubsys_vblank_irq(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(bubsys_interrupt);

	void main_map(address_map &map) ATTR_COLD;
};

#endif // MAME_KONAMI_NEMESIS_H
