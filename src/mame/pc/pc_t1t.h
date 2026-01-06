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
protected:
	// construction/destruction
	pc_t1t_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock,
			uint8_t databits,
			uint8_t addrbits,
			unsigned chr_stride,
			unsigned ra_stride);

	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual uint32_t palette_entries() const noexcept override { return 16; }
	virtual uint32_t palette_indirect_entries() const noexcept override { return 32; }
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	pen_t palette_r(uint8_t index) const { return pen(!BIT(m_vga_addr, 4) ? (index & m_palette_mask) : (m_vga_addr & 0x0f)); }
	uint8_t chr_gen_r(uint8_t chr, uint8_t ra) const { return m_chr_gen[(uint32_t(chr) << m_chr_stride) | (uint32_t(ra) << m_ra_stride)]; }
	uint32_t row_base(uint8_t ra) const { return (m_display_base & m_base_mask) | ((uint32_t(ra) << m_ra_shift) & m_ra_mask); }

	void set_palette_base(uint8_t base);

	int status_r();
	void lightpen_strobe_w(int data);
	void vga_addr_w(uint8_t data);
	void vga_data_w(uint8_t data);
	void page_w(uint8_t data);

	virtual MC6845_UPDATE_ROW( crtc_update_row );
	MC6845_UPDATE_ROW( text_inten_update_row );
	MC6845_UPDATE_ROW( gfx_4bpp_update_row );
	MC6845_UPDATE_ROW( gfx_2bpp_high_update_row );

	void default_map(address_map &map) ATTR_COLD;

	const address_space_config m_vram_config;

	required_region_ptr<uint8_t> m_chr_gen;
	required_device<mc6845_device> m_mc6845;

	// character ROM addressing configuration
	unsigned const m_chr_stride, m_ra_stride;

	// VRAM addressing control
	uint32_t m_display_base, m_window_base;
	uint32_t m_base_mask, m_ra_mask, m_offset_mask;
	unsigned m_ra_shift;

	uint8_t m_status;

	uint8_t m_vga_addr;

	uint8_t m_palette_mask;
	uint8_t m_border_color;
	uint8_t m_palette_reg[16];

	uint8_t m_page;

	int m_pc_framecnt;

	int m_update_row_type;
	uint8_t m_display_enable;
	uint8_t m_vsync;

private:
	uint8_t m_palette_base;
};


class pcvideo_t1000_device : public pc_t1t_device, public device_gfx_interface
{
public:
	pcvideo_t1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(address_space &space, offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	uint8_t vram_window8_r(address_space &space, offs_t offset);
	uint16_t vram_window16_r(address_space &space, offs_t offset);
	void vram_window8_w(offs_t offset, uint8_t data);
	void vram_window16_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void disable_w(int state);

protected:
	pcvideo_t1000_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock,
			uint8_t databits,
			uint8_t addrbits);

	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	pen_t palette_cga_2bpp_r(uint8_t index) const
	{
		if (!index)
			return palette_r(BIT(m_color_sel, 0, 4));
		else if (!BIT(m_mode_sel, 2))
			return palette_r((BIT(m_color_sel, 4) << 3) | (index << 1) | BIT(m_color_sel, 5));
		else
			return palette_r((BIT(m_color_sel, 4) << 3) | bitswap<3>(index, 1, 0, 0));
	}

	void mode_switch();
	void vga_data_w(uint8_t data);
	void mode_select_w(uint8_t data);
	void color_select_w(uint8_t data);

	void de_changed(int state);
	void vsync_changed(int state);

	virtual MC6845_UPDATE_ROW( crtc_update_row ) override;
	MC6845_UPDATE_ROW( text_blink_update_row );
	MC6845_UPDATE_ROW( gfx_2bpp_update_row );
	MC6845_UPDATE_ROW( gfx_1bpp_update_row );

	uint8_t m_mode_sel;
	uint8_t m_color_sel;
	uint8_t m_mode_ctrl;

	bool m_disable;
};

DECLARE_DEVICE_TYPE(PCVIDEO_T1000, pcvideo_t1000_device)


class pcvideo_t1000x_device :  public pcvideo_t1000_device
{
public:
	static auto parent_rom_device_type() { return &PCVIDEO_T1000; }

	pcvideo_t1000x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write(offs_t offset, uint8_t data) override;

protected:
	pcvideo_t1000x_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock,
			uint8_t databits,
			uint8_t addrbits);

	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	void page_w(uint8_t data);
	void ext_page_w(uint8_t data);

	uint8_t m_ext_page;
};

DECLARE_DEVICE_TYPE(PCVIDEO_T1000X, pcvideo_t1000x_device)


class pcvideo_pcjr_device : public pc_t1t_device
{
public:
	// construction/destruction
	pcvideo_pcjr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto vsync_callback() { return m_vsync_cb.bind(); }
	template <typename T> pcvideo_pcjr_device &set_chr_gen_tag(T &&tag) { m_chr_gen.set_tag(std::forward<T>(tag)); return *this; }
	template <typename T> pcvideo_pcjr_device &set_kanji_tag(T &&tag) { m_jxkanji.set_tag(std::forward<T>(tag)); return *this; }

	// this needs to be called when the video RAM size is known
	void set_16bit(bool val) { m_16bit = val; }

	uint8_t read(address_space &space, offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t vram_window_r(offs_t offset);
	void vram_window_w(offs_t offset, uint8_t data);

	void de_changed(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	memory_access<17, 0, 0, ENDIANNESS_LITTLE>::cache m_vram;
	optional_region_ptr<uint8_t> m_jxkanji;
	devcb_write_line m_vsync_cb;
	bool m_16bit;

	uint8_t m_address_data_ff;
	uint8_t m_mode_ctrl_1, m_mode_ctrl_2;

private:
	void mode_switch();
	void vga_data_w(uint8_t data);

	void pcjr_vsync_changed(int state);

	virtual MC6845_UPDATE_ROW( crtc_update_row ) override;
	MC6845_UPDATE_ROW( text_inten_high_8bit_update_row );
	MC6845_UPDATE_ROW( text_blink_update_row );
	MC6845_UPDATE_ROW( text_blink_high_8bit_update_row );
	MC6845_UPDATE_ROW( pcjx_text_update_row );
	MC6845_UPDATE_ROW( gfx_4bpp_high_8bit_update_row );
	MC6845_UPDATE_ROW( gfx_2bpp_update_row );
	MC6845_UPDATE_ROW( gfx_2bpp_high_8bit_update_row );
	MC6845_UPDATE_ROW( gfx_1bpp_update_row );
};

DECLARE_DEVICE_TYPE(PCVIDEO_PCJR, pcvideo_pcjr_device)

#endif // MAME_PC_PC_T1T_H
