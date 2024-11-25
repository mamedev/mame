// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System 32/Multi 32 hardware

***************************************************************************/
#ifndef MAME_SEGA_SEGAS32_H
#define MAME_SEGA_SEGAS32_H

#pragma once

#include "sound/multipcm.h"
#include "s32comm.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class segas32_state : public device_t
{
public:
	segas32_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void init_alien3();
	void init_arescue(int m_hasdsp);
	void init_arabfgt();
	void init_brival();
	void init_darkedge();
	void init_dbzvrvs();
	void init_f1en();
	void init_f1lap();
	void init_f1lapt();
	void init_ga2();
	void init_harddunk();
	void init_holo();
	void init_jpark();
	void init_orunners();
	void init_radm();
	void init_radr();
	void init_scross();
	void init_slipstrm();
	void init_sonic();
	void init_sonicp();
	void init_spidman();
	void init_svf();
	void init_jleague();
	void init_titlef();

	cpu_device* maincpu() { return m_maincpu; }

	uint32_t screen_update_system32(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_multi32_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_multi32_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ym3438_irq_handler(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(signal_v60_irq_callback);
	INTERRUPT_GEN_MEMBER(start_of_vblank_int);

	void misc_output_0_w(uint8_t data);
	void misc_output_1_w(uint8_t data);
	void sw2_output_0_w(uint8_t data);
	void sw2_output_1_w(uint8_t data);
	template<int Which> void display_enable_w(int state);
	void tilebank_external_w(uint8_t data);

protected:
	segas32_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool is_multi32);

	typedef void (segas32_state::*sys32_output_callback)(int which, uint16_t data);

	struct layer_info
	{
		bitmap_ind16 bitmap;
		std::unique_ptr<uint8_t[]> transparent;
		int num = 0;
	};

	struct extents_list
	{
		uint8_t                   scan_extent[256]{};
		uint16_t                  extent[32][16]{};
	};


	struct cache_entry
	{
		cache_entry *             next = nullptr;
		tilemap_t *               tmap = nullptr;
		uint8_t                   page = 0;
		uint8_t                   bank = 0;
	};

	void sonic_level_load_protection(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t brival_protection_r(offs_t offset, uint16_t mem_mask = ~0);
	void brival_protection_w(offs_t offset, uint16_t data);
	void darkedge_protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t darkedge_protection_r(offs_t offset, uint16_t mem_mask = ~0);
	void dbzvrvs_protection_w(address_space &space, uint16_t data);
	uint16_t dbzvrvs_protection_r();
	void jleague_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t arescue_dsp_r(offs_t offset);
	void arescue_dsp_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Which> uint16_t paletteram_r(offs_t offset);
	template<int Which> void paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t videoram_r(offs_t offset);
	void videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t sprite_control_r(offs_t offset);
	void sprite_control_w(offs_t offset, uint8_t data);
	uint16_t spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Which> uint16_t mixer_r(offs_t offset);
	template<int Which> void mixer_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t int_control_r(offs_t offset);
	void int_control_w(offs_t offset, uint8_t data);

	void random_number_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t random_number_r();
	uint8_t shared_ram_r(offs_t offset);
	void shared_ram_w(offs_t offset, uint8_t data);
	void sound_int_control_lo_w(offs_t offset, uint8_t data);
	void sound_int_control_hi_w(offs_t offset, uint8_t data);
	void sound_bank_lo_w(uint8_t data);
	void sound_bank_hi_w(uint8_t data);
	uint8_t sound_dummy_r();
	void sound_dummy_w(uint8_t data);

	void multipcm_bank_w(uint8_t data);
	void scross_bank_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);

	TIMER_CALLBACK_MEMBER(end_of_vblank_int);
	TIMER_CALLBACK_MEMBER(update_sprites);

	void system32_set_vblank(int state);
	inline uint16_t xBBBBBGGGGGRRRRR_to_xBGRBBBBGGGGRRRR(uint16_t value);
	inline uint16_t xBGRBBBBGGGGRRRR_to_xBBBBBGGGGGRRRRR(uint16_t value);
	inline void update_color(int offset, uint16_t data);
	tilemap_t *find_cache_entry(int page, int bank);
	inline void get_tilemaps(int bgnum, tilemap_t **tilemaps);
	uint8_t update_tilemaps(screen_device &screen, const rectangle &cliprect);
	void sprite_erase_buffer();
	void sprite_swap_buffers();
	int draw_one_sprite(uint16_t const *data, int xoffs, int yoffs, const rectangle &clipin, const rectangle &clipout);
	void sprite_render_list();
	inline uint8_t compute_color_offsets(int which, int layerbit, int layerflag);
	inline uint16_t compute_sprite_blend(uint8_t encoding);
	inline uint16_t *get_layer_scanline(int layer, int scanline);
	void mix_all_layers(int which, int xoffs, bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t enablemask);
	void print_mixer_data(int which);
	uint32_t multi32_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int index);
	void update_irq_state();
	void signal_v60_irq(int which);
	void update_sound_irq_state();
	void segas32_common_init();
	void radm_sw1_output( int which, uint16_t data );
	void radm_sw2_output( int which, uint16_t data );
	void radr_sw2_output( int which, uint16_t data );
	void alien3_sw1_output( int which, uint16_t data );
	void arescue_sw1_output( int which, uint16_t data );
	void f1lap_sw1_output( int which, uint16_t data );
	void jpark_sw1_output( int which, uint16_t data );
	void orunners_sw1_output( int which, uint16_t data );
	void orunners_sw2_output( int which, uint16_t data );
	void harddunk_sw1_output( int which, uint16_t data );
	void harddunk_sw2_output( int which, uint16_t data );
	void harddunk_sw3_output( int which, uint16_t data );
	void titlef_sw1_output( int which, uint16_t data );
	void titlef_sw2_output( int which, uint16_t data );
	void scross_sw1_output( int which, uint16_t data );
	void scross_sw2_output( int which, uint16_t data );
	int compute_clipping_extents(screen_device &screen, int enable, int clipout, int clipmask, const rectangle &cliprect, extents_list *list);
	void compute_tilemap_flips(int bgnum, int &flipx, int &flipy);
	void update_tilemap_zoom(screen_device &screen, layer_info &layer, const rectangle &cliprect, int bgnum);
	void update_tilemap_rowscroll(screen_device &screen, layer_info &layer, const rectangle &cliprect, int bgnum);
	void update_tilemap_text(screen_device &screen, layer_info &layer, const rectangle &cliprect);
	void update_bitmap(screen_device &screen, layer_info &layer, const rectangle &cliprect);
	void update_background(layer_info &layer, const rectangle &cliprect);

	void signal_sound_irq(int which);
	void clear_sound_irq(int which);
	void darkedge_fd1149_vblank();
	void f1lap_fd1149_vblank();

	void ga2_main_map(address_map &map) ATTR_COLD;
	void multi32_6player_map(address_map &map) ATTR_COLD;
	void multi32_map(address_map &map) ATTR_COLD;
	void multi32_sound_map(address_map &map) ATTR_COLD;
	void multi32_sound_portmap(address_map &map) ATTR_COLD;
	void multipcm_map(address_map &map) ATTR_COLD;
	void rf5c68_map(address_map &map) ATTR_COLD;
	void system32_4player_map(address_map &map) ATTR_COLD;
	void system32_analog_map(address_map &map) ATTR_COLD;
	void system32_cd_map(address_map &map) ATTR_COLD;
	void system32_map(address_map &map) ATTR_COLD;
	void system32_sound_map(address_map &map) ATTR_COLD;
	void system32_sound_portmap(address_map &map) ATTR_COLD;
	void upd7725_data_map(address_map &map) ATTR_COLD;
	void upd7725_prg_map(address_map &map) ATTR_COLD;
	void v25_map(address_map &map) ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_shared_ptr<uint8_t> m_z80_shared_ram;
	optional_shared_ptr<uint16_t> m_system32_workram;
	memory_share_creator<uint16_t> m_videoram;
	memory_share_creator<uint16_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_soundram;
	memory_share_array_creator<uint16_t, 2> m_paletteram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<multipcm_device> m_multipcm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_device<timer_device> m_irq_timer_0;
	required_device<timer_device> m_irq_timer_1;
	optional_device<s32comm_device> m_s32comm;

	required_region_ptr<uint32_t> m_sprite_region;
	required_memory_region m_maincpu_region;
	required_memory_bank m_soundrom_bank;
	optional_memory_bank m_multipcm_bank_hi;
	optional_memory_bank m_multipcm_bank_lo;

	const bool m_is_multi32 = false;

	// internal states
	uint8_t m_v60_irq_control[0x10]{};
	timer_device *m_v60_irq_timer[2]{};
	uint8_t m_sound_irq_control[4]{};
	uint8_t m_sound_irq_input = 0;
	uint8_t m_sound_dummy_value = 0;
	uint16_t m_sound_bank = 0;
	sys32_output_callback m_sw1_output;
	sys32_output_callback m_sw2_output;
	sys32_output_callback m_sw3_output;

	// hardware specific
	std::unique_ptr<uint16_t[]> m_system32_protram;
	uint16_t m_arescue_dsp_io[6]{};

	// video-related
	uint16_t m_system32_displayenable[2]{};
	uint16_t m_system32_tilebank_external = 0;
	std::unique_ptr<cache_entry[]> m_tmap_cache;
	cache_entry *m_cache_head = nullptr;
	layer_info m_layer_data[11];
	uint16_t m_mixer_control[2][0x40]{};
	std::unique_ptr<uint16_t[]> m_solid_0000;
	std::unique_ptr<uint16_t[]> m_solid_ffff;
	uint8_t m_sprite_render_count = 0;
	uint8_t m_sprite_control_latched[8]{};
	uint8_t m_sprite_control[8]{};
	std::unique_ptr<uint32_t[]> m_spriteram_32bit;
	std::unique_ptr<int32_t[]> m_prev_bgstartx;
	std::unique_ptr<int32_t[]> m_prev_bgendx;
	std::unique_ptr<int32_t[]> m_bgcolor_line;
	typedef void (segas32_state::*prot_vblank_func)();
	prot_vblank_func m_system32_prot_vblank;
	int m_print_count = 0;
	emu_timer *m_vblank_end_int_timer = nullptr;
	emu_timer *m_update_sprites_timer = nullptr;
};

class segas32_regular_state : public segas32_state
{
public:
	segas32_regular_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class segas32_analog_state : public segas32_state
{
public:
	segas32_analog_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	segas32_analog_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class segas32_trackball_state : public segas32_state
{
public:
	segas32_trackball_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void system32_trackball_map(address_map &map) ATTR_COLD;
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class segas32_4player_state : public segas32_state
{
public:
	segas32_4player_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	segas32_4player_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class segas32_v25_state : public segas32_4player_state
{
public:
	segas32_v25_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static const uint8_t arf_opcode_table[256];
	static const uint8_t ga2_opcode_table[256];

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void decrypt_protrom();
};

class segas32_upd7725_state : public segas32_analog_state
{
public:
	segas32_upd7725_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class segas32_cd_state : public segas32_state
{
public:
	segas32_cd_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void lamps1_w(uint8_t data);
	void lamps2_w(uint8_t data);
	void scsi_irq_w(int state);
	void scsi_drq_w(int state);

	static void cdrom_config(device_t *device);
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	output_finder<16> m_lamps;
};

class sega_multi32_state : public segas32_state
{
public:
	sega_multi32_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	sega_multi32_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class sega_multi32_analog_state : public sega_multi32_state
{
public:
	sega_multi32_analog_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	ioport_value in2_analog_read();
	ioport_value in3_analog_read();
	void analog_bank_w(uint8_t data);

	void multi32_analog_map(address_map &map) ATTR_COLD;
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	optional_ioport_array<8> m_analog_ports;
	uint8_t m_analog_bank = 0;
};

class sega_multi32_6player_state : public sega_multi32_state
{
public:
	sega_multi32_6player_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SEGA_S32_PCB, segas32_state)

#endif // MAME_SEGA_SEGAS32_H
