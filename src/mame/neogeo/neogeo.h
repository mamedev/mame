// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Ernesto Corvi,Andrew Prime,Zsolt Vasvari
// thanks-to:Fuzz
/*************************************************************************

    Neo-Geo hardware

*************************************************************************/
#ifndef MAME_NEOGEO_NEOGEO_H
#define MAME_NEOGEO_NEOGEO_H

#pragma once

#include "ng_memcard.h"
#include "neogeo_spr.h"

#include "bus/neogeo/slot.h"
#include "bus/neogeo/carts.h"
#include "bus/neogeo_ctrl/ctrl.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/ymopn.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "machine/upd1990a.h"

#include "emupal.h"
#include "screen.h"


#define NEOGEO_MASTER_CLOCK                     (24000000)
#define NEOGEO_MAIN_CPU_CLOCK                   (NEOGEO_MASTER_CLOCK / 2)
#define NEOGEO_AUDIO_CPU_CLOCK                  (NEOGEO_MASTER_CLOCK / 6)
#define NEOGEO_YM2610_CLOCK                     (NEOGEO_MASTER_CLOCK / 3)
#define NEOGEO_PIXEL_CLOCK                      (NEOGEO_MASTER_CLOCK / 4)

// On scanline 224, /VBLANK goes low 56 mclks (14 pixels) from the rising edge of /HSYNC.
// Two mclks after /VBLANK goes low, the hardware sets a pending IRQ1 flip-flop.
#define NEOGEO_VBLANK_IRQ_HTIM (attotime::from_ticks(56+2, NEOGEO_MASTER_CLOCK))


class neogeo_base_state : public driver_device
{
public:
	ioport_value get_memcard_status();
	ioport_value get_audio_result();

protected:
	neogeo_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ym(*this, "ymsnd")
		, m_sprgen(*this, "spritegen")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_memcard(*this, "memcard")
		, m_systemlatch(*this, "systemlatch")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch2(*this, "soundlatch2")
		, m_region_maincpu(*this, "maincpu")
		, m_share_maincpu(*this, "maincpu")
		, m_region_sprites(*this, "sprites")
		, m_region_fixed(*this, "fixed")
		, m_region_fixedbios(*this, "fixedbios")
		, m_region_mainbios(*this, "mainbios")
		, m_region_audiobios(*this, "audiobios")
		, m_region_audiocpu(*this, "audiocpu")
		, m_bank_audio_main(*this, "audio_main")
		, m_bank_cartridge(*this, "cartridge")
		, m_edge(*this, "edge")
		, m_ctrl1(*this, "ctrl1")
		, m_ctrl2(*this, "ctrl2")
		, m_slots(*this, "cslot%u", 1U)
		, m_audionmi(*this, "audionmi")
	{ }

	uint16_t memcard_r(offs_t offset, uint16_t mem_mask = ~0);
	void memcard_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t audio_cpu_bank_select_r(offs_t offset);
	void audio_cpu_enable_nmi_w(offs_t offset, uint8_t data);
	uint16_t unmapped_r(address_space &space);
	uint16_t paletteram_r(offs_t offset);
	void paletteram_w(offs_t offset, uint16_t data);
	uint16_t video_register_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void video_register_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TIMER_CALLBACK_MEMBER(display_position_interrupt_callback);
	TIMER_CALLBACK_MEMBER(display_position_vblank_callback);
	TIMER_CALLBACK_MEMBER(vblank_interrupt_callback);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void io_control_w(offs_t offset, uint8_t data);
	void audio_command_w(uint8_t data);
	void set_use_cart_vectors(int state);
	void set_use_cart_audio(int state);
	uint16_t banked_vectors_r(offs_t offset);
	void write_banksel(uint16_t data);
	void write_bankprot(uint16_t data);
	void write_bankprot_pvc(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void write_bankprot_ms5p(offs_t offset, uint16_t data);
	void write_bankprot_kf2k3bl(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void write_bankprot_kof10th(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t read_lorom_kof10th(offs_t offset);

	void set_screen_shadow(int state);
	void set_palette_bank(int state);

	void neogeo_base(machine_config &config);
	void neogeo_stereo(machine_config &config);
	void neogeo_memcard(machine_config &config);

	void base_main_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
	void audio_map(address_map &map) ATTR_COLD;

	// device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void device_post_load() override;

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	// MVS-specific devices
	optional_device<ym2610_device> m_ym;
	required_device<neosprite_optimized_device> m_sprgen;

	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_device<ng_memcard_device> m_memcard;
	required_device<hc259_device> m_systemlatch;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	// memory
	optional_memory_region m_region_maincpu;
	optional_shared_ptr<uint16_t> m_share_maincpu;
	optional_memory_region m_region_sprites;
	optional_memory_region m_region_fixed;
	optional_memory_region m_region_fixedbios;
	optional_memory_region m_region_mainbios;
	optional_memory_region m_region_audiobios;
	optional_memory_region m_region_audiocpu;
	optional_memory_bank   m_bank_audio_main; // optional because of neocd
	memory_bank           *m_bank_audio_cart[4];
	memory_bank_creator    m_bank_cartridge;

	optional_device<neogeo_ctrl_edge_port_device> m_edge;
	optional_device<neogeo_control_port_device> m_ctrl1;
	optional_device<neogeo_control_port_device> m_ctrl2;

	// video hardware, including maincpu interrupts
	// TODO: make into a device
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

	const pen_t *m_bg_pen = nullptr;
	uint8_t      m_vblank_level = 0;
	uint8_t      m_raster_level = 0;

	uint8_t      m_use_cart_vectors = 0;
	uint8_t      m_use_cart_audio = 0;
	uint8_t      m_card_bank = 0;

	void set_slot_idx(int slot);

	// cart slots
	void init_cpu();
	void init_audio();
	void init_ym();
	void init_sprites();
	// temporary helper to restore memory banking while bankswitch is handled in the driver...
	uint32_t m_bank_base = 0;

	optional_device_array<neogeo_cart_slot_device, 6> m_slots;

	int m_curr_slot = 0;

private:
	void update_interrupts();
	void create_interrupt_timers();
	void start_interrupt_timers();
	void acknowledge_interrupt(uint16_t data);

	void adjust_display_position_interrupt_timer();
	void set_display_position_interrupt_control(uint16_t data);
	void set_display_counter_msb(uint16_t data);
	void set_display_counter_lsb(uint16_t data);
	void set_video_control(uint16_t data);

	void create_rgb_lookups();
	void set_pens();

	// internal state
	bool       m_recurse = false;

	emu_timer  *m_display_position_interrupt_timer = nullptr;
	emu_timer  *m_display_position_vblank_timer = nullptr;
	emu_timer  *m_vblank_interrupt_timer = nullptr;
	uint32_t     m_display_counter = 0;
	uint8_t      m_vblank_interrupt_pending = 0;
	uint8_t      m_display_position_interrupt_pending = 0;
	uint8_t      m_irq3_pending = 0;
	uint8_t      m_display_position_interrupt_control = 0;

	uint16_t get_video_control();

	required_device<input_merger_device> m_audionmi;

	// color/palette related
	std::vector<uint16_t> m_paletteram;
	uint8_t      m_palette_lookup[32][4]{};
	int          m_screen_shadow = 0;
	int          m_palette_bank = 0;
};


class ngarcade_base_state : public neogeo_base_state
{
public:
	ioport_value startsel_edge_joy_r();

protected:
	ngarcade_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: neogeo_base_state(mconfig, type, tag)
		, m_save_ram(*this, "saveram")
		, m_upd4990a(*this, "upd4990a")
		, m_dsw(*this, "DSW")
	{
	}

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void io_control_w(offs_t offset, uint8_t data) override;
	void set_save_ram_unlock(int state);
	void save_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t in0_edge_r();
	uint16_t in0_edge_joy_r();
	uint16_t in1_edge_r();
	uint16_t in1_edge_joy_r();

	void neogeo_arcade(machine_config &config);
	void neogeo_mono(machine_config &config);

	void neogeo_main_map(address_map &map) ATTR_COLD;

private:
	required_shared_ptr<uint16_t> m_save_ram;
	required_device<upd4990a_device> m_upd4990a;
	required_ioport m_dsw;

	uint8_t m_save_ram_unlocked = 0;
};


class aes_base_state : public neogeo_base_state
{
public:
	DECLARE_INPUT_CHANGED_MEMBER(aes_jp1);

protected:
	aes_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: neogeo_base_state(mconfig, type, tag)
		, m_io_in2(*this, "IN2")
	{
	}

	uint16_t aes_in2_r();

	virtual void machine_start() override ATTR_COLD;

	void aes_base_main_map(address_map &map) ATTR_COLD;

private:
	required_ioport m_io_in2;
};


/*----------- defined in drivers/neogeo.c -----------*/

INPUT_PORTS_EXTERN(neogeo);
INPUT_PORTS_EXTERN(aes);

#endif // MAME_NEOGEO_NEOGEO_H
