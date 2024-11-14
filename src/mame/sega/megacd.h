// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega CD / Mega CD */
#ifndef MAME_SEGA_MEGACD_H
#define MAME_SEGA_MEGACD_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/lc89510.h"
#include "megacdcd.h"
#include "sound/rf5c68.h"
#include "screen.h"
#include "tilemap.h"

class sega_segacd_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	template <typename T> void set_hostcpu(T &&tag) { m_hostcpu.set_tag(std::forward<T>(tag)); }

	// set some variables at start, depending on region (shall be moved to a device interface?)
	void set_framerate(int rate) { m_framerate = rate; }
	void set_total_scanlines(int total) { m_base_total_scanlines = total; }     // this gets set at start only
	void update_total_scanlines(bool mode3) { m_total_scanlines = mode3 ? (m_base_total_scanlines * 2) : m_base_total_scanlines; }  // this gets set at each EOF
	double get_framerate() { return has_screen() ? screen().frame_period().as_hz() : double(m_framerate); }

	uint16_t segacd_dmaaddr_r();
	void segacd_dmaaddr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void scd_a12000_halt_reset_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t scd_a12000_halt_reset_r();
	uint16_t scd_a12002_memory_mode_r();
	void scd_a12002_memory_mode_w_8_15(u8 data);
	void scd_a12002_memory_mode_w_0_7(u8 data);
	void scd_a12002_memory_mode_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_sub_memory_mode_r();
	void segacd_sub_memory_mode_w_8_15(u8 data);
	void segacd_sub_memory_mode_w_0_7(u8 data);
	void segacd_sub_memory_mode_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t segacd_comms_flags_r();
	void segacd_comms_flags_subcpu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void segacd_comms_flags_maincpu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t scd_4m_prgbank_ram_r(offs_t offset);
	void scd_4m_prgbank_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_comms_main_part1_r(offs_t offset);
	void segacd_comms_main_part1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_comms_main_part2_r(offs_t offset);
	void segacd_comms_main_part2_w(uint16_t data);
	uint16_t segacd_comms_sub_part1_r(offs_t offset);
	void segacd_comms_sub_part1_w(uint16_t data);
	uint16_t segacd_comms_sub_part2_r(offs_t offset);
	void segacd_comms_sub_part2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);



	uint16_t segacd_main_dataram_part1_r(offs_t offset);
	void segacd_main_dataram_part1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t scd_hint_vector_r(offs_t offset);
	uint16_t scd_a12006_hint_register_r();
	void scd_a12006_hint_register_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);


	void segacd_stopwatch_timer_w(uint16_t data);
	uint16_t segacd_stopwatch_timer_r();
	uint16_t segacd_sub_led_ready_r(offs_t offset, uint16_t mem_mask = ~0);
	void segacd_sub_led_ready_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_sub_dataram_part1_r(offs_t offset);
	void segacd_sub_dataram_part1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_sub_dataram_part2_r(offs_t offset);
	void segacd_sub_dataram_part2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);


	uint16_t segacd_stampsize_r();
	void segacd_stampsize_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER( get_stampmap_16x16_1x1_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_32x32_1x1_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_16x16_16x16_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_32x32_16x16_tile_info );

	void segacd_trace_vector_base_address_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_imagebuffer_vdot_size_r();
	void segacd_imagebuffer_vdot_size_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_stampmap_base_address_r();
	void segacd_stampmap_base_address_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_imagebuffer_start_address_r();
	void segacd_imagebuffer_start_address_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_imagebuffer_offset_r();
	void segacd_imagebuffer_offset_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_imagebuffer_vcell_size_r();
	void segacd_imagebuffer_vcell_size_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_imagebuffer_hdot_size_r();
	void segacd_imagebuffer_hdot_size_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_irq3timer_r();
	void segacd_irq3timer_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t backupram_r(offs_t offset);
	void backupram_w(offs_t offset, uint8_t data);
	uint8_t font_color_r();
	void font_color_w(uint8_t data);
	uint16_t font_converted_r(offs_t offset);

	void segacd_map(address_map &map) ATTR_COLD;
	void segacd_pcm_map(address_map &map) ATTR_COLD;
protected:
	sega_segacd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<cpu_device> m_scdcpu;
	required_device<cpu_device> m_hostcpu;
	required_device<rf5c164_device> m_rfsnd;
	required_device<lc89510_temp_device> m_lc89510_temp;
	required_device<timer_device> m_stopwatch_timer;
	required_device<timer_device> m_stamp_timer;
	required_device<timer_device> m_irq3_timer;
	required_device<timer_device> m_dma_timer;
	//required_device<timer_device> m_hock_timer;

	required_shared_ptr<uint16_t> m_prgram;
	required_shared_ptr<uint16_t> m_dataram;
	required_shared_ptr<uint16_t> m_font_bits;

	output_finder<> m_red_led;
	output_finder<> m_green_led;

	// can't use a memshare because it's 8-bit RAM in a 16-bit address space
	std::vector<uint8_t> m_backupram;

	uint8_t m_font_color = 0;

	uint16_t scd_rammode = 0;
	uint32_t scd_mode_dmna_ret_flags = 0;

	tilemap_t    *segacd_stampmap[4]{};


	uint8_t segacd_ram_writeprotect_bits = 0;
	int segacd_4meg_prgbank = 0; // which bank the MainCPU can see of the SubCPU PrgRAM
	int segacd_memory_priority_mode = 0;
	int segacd_stampsize = 0;

	uint16_t segacd_hint_register = 0;
	uint16_t segacd_imagebuffer_vdot_size = 0;
	uint16_t segacd_imagebuffer_vcell_size = 0;
	uint16_t segacd_imagebuffer_hdot_size = 0;

	int segacd_conversion_active = 0;
	uint16_t segacd_stampmap_base_address = 0;
	uint16_t segacd_imagebuffer_start_address = 0;
	uint16_t segacd_imagebuffer_offset = 0;


	uint16_t segacd_comms_flags = 0;
	uint16_t segacd_comms_part1[0x8]{};
	uint16_t segacd_comms_part2[0x8]{};

	int segacd_redled = 0;
	int segacd_greenled = 0;
	int segacd_ready = 1; // actually set 100ms after startup?
	uint8_t m_irq3_timer_reg = 0;


	inline void write_pixel(uint8_t pix, int pixeloffset);
	uint16_t segacd_1meg_mode_word_read(offs_t offset);
	void segacd_1meg_mode_word_write(offs_t offset, uint16_t data, uint16_t mem_mask, int use_pm);

	uint16_t m_dmaaddr = 0;

	uint16_t m_a12000_halt_reset_reg = 0;

	int m_framerate = 0;
	int m_base_total_scanlines = 0;
	int m_total_scanlines = 0;

	void segacd_mark_tiles_dirty(int offset);
	int segacd_get_active_stampmap_tilemap(void);

	void SCD_GET_TILE_INFO_16x16_1x1( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_32x32_1x1( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_16x16_16x16( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_32x32_16x16( int& tile_region, int& tileno, int tile_index );

	uint8_t get_stampmap_16x16_1x1_tile_info_pixel(int xpos, int ypos);
	uint8_t get_stampmap_32x32_1x1_tile_info_pixel(int xpos, int ypos);
	uint8_t get_stampmap_16x16_16x16_tile_info_pixel(int xpos, int ypos);
	uint8_t get_stampmap_32x32_16x16_tile_info_pixel(int xpos, int ypos);

	uint8_t read_pixel_from_stampmap(bitmap_ind16* srcbitmap, int x, int y);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER( dma_timer_callback );
	IRQ_CALLBACK_MEMBER(segacd_sub_int_callback);

	TIMER_DEVICE_CALLBACK_MEMBER( irq3_timer_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( stamp_timer_callback );

	void SegaCD_CDC_Do_DMA( int &dmacount, uint8_t *CDC_BUFFER, uint16_t &dma_addrc, uint16_t &destination );
};


class sega_segacd_us_device : public sega_segacd_device
{
public:
	sega_segacd_us_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class sega_segacd_japan_device : public sega_segacd_device
{
public:
	sega_segacd_japan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class sega_segacd_europe_device : public sega_segacd_device
{
public:
	sega_segacd_europe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(SEGA_SEGACD_US,     sega_segacd_us_device)
DECLARE_DEVICE_TYPE(SEGA_SEGACD_JAPAN,  sega_segacd_japan_device)
DECLARE_DEVICE_TYPE(SEGA_SEGACD_EUROPE, sega_segacd_europe_device)

#endif // MAME_SEGA_MEGACD_H
