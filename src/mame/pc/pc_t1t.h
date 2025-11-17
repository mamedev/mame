// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_PC_PC_T1T_H
#define MAME_PC_PC_T1T_H

#pragma once

#include "video/mc6845.h"


#define T1000_SCREEN_NAME   "screen"

class pc_t1t_device :
		public device_t,
		public device_video_interface,
		public device_palette_interface,
		public device_memory_interface
{
public:
	// configuration
	template <typename T> pc_t1t_device &set_chr_gen_tag(T &&tag) { m_chr_gen.set_tag(std::forward<T>(tag)); return *this; }

	void t1000_de_changed(int state);
	uint8_t read(offs_t offset);

	virtual MC6845_UPDATE_ROW( crtc_update_row );
	MC6845_UPDATE_ROW( t1000_text_inten_update_row );
	MC6845_UPDATE_ROW( t1000_text_blink_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_4bpp_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_2bpp_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_2bpp_tga_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_1bpp_update_row );

protected:
	// used in tandy1000hx; used in pcjr???
	struct reg
	{
		reg()
		{
			index = 0;
			memset(&data, 0, sizeof(data));
		}

		uint8_t index;
		uint8_t data[0x20];
		/* see vgadoc
		   0 mode control 1
		   1 palette mask
		   2 border color
		   3 mode control 2
		   4 reset
		   0x10-0x1f palette registers
		*/
	};

	// construction/destruction
	pc_t1t_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t addrbits);

	virtual void device_start() override ATTR_COLD;
	virtual uint32_t palette_entries() const noexcept override { return 32; }
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	int mode_control_r();
	void color_select_w(int data);
	int color_select_r();
	int status_r();
	void lightpen_strobe_w(int data);
	void vga_index_w(int data);
	int vga_data_r();
	int bank_r();

	void default_map(address_map &map) ATTR_COLD;

	const address_space_config m_vram_config;

	required_region_ptr<uint8_t> m_chr_gen;
	required_device<mc6845_device> m_mc6845;

	uint32_t m_display_base, m_window_base;

	uint8_t m_mode_control, m_color_select;
	uint8_t m_status;

	struct reg m_reg;

	uint16_t m_bank;

	int m_pc_framecnt;

	uint8_t m_chr_size;
	uint16_t m_ra_offset;

	uint8_t m_address_data_ff;

	int m_update_row_type;
	uint8_t m_display_enable;
	uint8_t m_vsync;
	uint8_t m_palette_base;
};


class pcvideo_t1000_device :  public pc_t1t_device
{
public:
	pcvideo_t1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);

	uint8_t vram_window8_r(address_space &space, offs_t offset);
	uint16_t vram_window16_r(address_space &space, offs_t offset);
	void vram_window8_w(offs_t offset, uint8_t data);
	void vram_window16_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void disable_w(int state);

protected:
	pcvideo_t1000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t addrbits);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void mode_switch();
	void vga_data_w(int data);
	void bank_w(uint16_t data);
	void mode_control_w(int data);

	void t1000_vsync_changed(int state);

	bool m_disable;
};

DECLARE_DEVICE_TYPE(PCVIDEO_T1000, pcvideo_t1000_device)


class pcvideo_pcjr_device :  public pc_t1t_device
{
public:
	// construction/destruction
	pcvideo_pcjr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto vsync_callback() { return m_vsync_cb.bind(); }
	template <typename T> pcvideo_pcjr_device &set_kanji_tag(T &&tag) { m_jxkanji.set_tag(std::forward<T>(tag)); return *this; }

	void write(offs_t offset, uint8_t data);

	uint8_t vram_window_r(offs_t offset);
	void vram_window_w(offs_t offset, uint8_t data);

	void de_changed(int state);
	MC6845_UPDATE_ROW( pcjx_text_update_row );
	MC6845_UPDATE_ROW( pcjr_gfx_2bpp_high_update_row );

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	memory_access<17, 0, 0, ENDIANNESS_LITTLE>::cache m_vram;
	optional_region_ptr<uint8_t> m_jxkanji;
	devcb_write_line m_vsync_cb;

private:
	void pc_pcjr_mode_switch();
	void pc_pcjr_vga_data_w(int data);
	void pc_pcjr_bank_w(uint8_t data);

	void pcjr_vsync_changed(int state);

	virtual MC6845_UPDATE_ROW( crtc_update_row ) override;
};

DECLARE_DEVICE_TYPE(PCVIDEO_PCJR, pcvideo_pcjr_device)

#endif // MAME_PC_PC_T1T_H
