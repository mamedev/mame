// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_VIDEO_PC_T1T_H
#define MAME_VIDEO_PC_T1T_H

#pragma once

#include "video/mc6845.h"
#include "machine/ram.h"
#include "machine/bankdev.h"
#include "machine/pic8259.h"
#include "emupal.h"

#define T1000_SCREEN_NAME   "screen"
#define T1000_MC6845_NAME   "mc6845_t1000"

class pc_t1t_device :  public device_t, public device_video_interface
{
public:
	DECLARE_WRITE_LINE_MEMBER( t1000_de_changed );
	uint8_t read(offs_t offset);

	virtual MC6845_UPDATE_ROW( crtc_update_row );
	MC6845_UPDATE_ROW( t1000_text_inten_update_row );
	MC6845_UPDATE_ROW( t1000_text_blink_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_4bpp_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_2bpp_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_2bpp_tga_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_1bpp_update_row );

	void vram_map(address_map &map);

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
	pc_t1t_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<mc6845_device> m_mc6845;
	uint8_t m_mode_control, m_color_select;
	uint8_t m_status;

	struct reg m_reg;

	uint16_t m_bank;

	int m_pc_framecnt;

	uint8_t *m_displayram;

	uint8_t  *m_chr_gen;
	uint8_t  m_chr_size;
	uint16_t m_ra_offset;

	uint8_t   m_address_data_ff;

	int     m_update_row_type;
	uint8_t   m_display_enable;
	uint8_t   m_vsync;
	uint8_t   m_palette_base;

	void pcjr_palette(palette_device &palette) const;

	int mode_control_r();
	void color_select_w(int data);
	int color_select_r();
	int status_r();
	void lightpen_strobe_w(int data);
	void vga_index_w(int data);
	int vga_data_r();
	int bank_r();

	required_device<palette_device> m_palette;
	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_vram;
};

class pcvideo_t1000_device :  public pc_t1t_device
{
public:
	// construction/destruction
	pcvideo_t1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER( disable_w );

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	void mode_switch();
	void vga_data_w(int data);
	void bank_w(int data);
	void mode_control_w(int data);
	bool m_disable;

	DECLARE_WRITE_LINE_MEMBER( t1000_vsync_changed );
};

DECLARE_DEVICE_TYPE(PCVIDEO_T1000, pcvideo_t1000_device)

class pcvideo_pcjr_device :  public pc_t1t_device
{
public:
	// construction/destruction
	pcvideo_pcjr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( de_changed );
	MC6845_UPDATE_ROW( pcjx_text_update_row );
	MC6845_UPDATE_ROW( pcjr_gfx_2bpp_high_update_row );

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	required_device<pic8259_device> m_pic8259;
	uint8_t   *m_jxkanji;

private:
	void pc_pcjr_mode_switch();
	void pc_pcjr_vga_data_w(int data);
	void pc_pcjr_bank_w(int data);
	void pc_pcjx_bank_w(int data);

	DECLARE_WRITE_LINE_MEMBER( pcjr_vsync_changed );

	virtual MC6845_UPDATE_ROW( crtc_update_row ) override;
};

DECLARE_DEVICE_TYPE(PCVIDEO_PCJR, pcvideo_pcjr_device)

#endif // MAME_VIDEO_PC_T1T_H
