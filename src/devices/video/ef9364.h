// license:BSD-3-Clause
// copyright-holders:Jean-Francois DEL NERO
/*********************************************************************

    ef9364.h

    Thomson EF9364 video controller

*********************************************************************/

#pragma once

#ifndef __EF9364_H__
#define __EF9364_H__

#define EF9364_NB_OF_COLUMNS 64
#define EF9364_NB_OF_ROWS 16

#define EF9364_TXTPLANE_MAX_SIZE ( EF9364_NB_OF_COLUMNS * EF9364_NB_OF_ROWS )
#define EF9364_MAX_TXTPLANES  2

#define MCFG_EF9364_PALETTE(_palette_tag) \
	ef9364_device::static_set_palette_tag(*device, "^" _palette_tag);

#define MCFG_EF9364_PAGES_CNT(_pages_number) \
	ef9364_device::static_set_nb_of_pages(*device,_pages_number);

#define MCFG_EF9364_IRQ_HANDLER(_devcb) \
	devcb = &ef9364_device::set_irq_handler(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ef9364_device

class ef9364_device :   public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	ef9364_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void static_set_nb_of_pages(device_t &device, int nb_bitplanes );

	// device interface

	void update_scanline(UINT16 scanline);
	void set_color_entry( int index, UINT8 r, UINT8 g, UINT8 b );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void char_latch_w(UINT8 data);
	void command_w(UINT8 cmd);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// address space configurations
	const address_space_config      m_space_config;

	// inline helper

private:
	void screen_scanning( int force_clear );
	void set_video_mode(void);
	void draw_border(UINT16 line);

	// internal state

	required_region_ptr<UINT8> m_charset;
	address_space *m_textram;

	UINT8 x_curs_pos;
	UINT8 y_curs_pos;
	UINT8 char_latch;

	UINT8 m_border[80];                     //border color

	rgb_t palette[2];
	int   nb_of_pages;
	int   bitplane_xres;
	int   bitplane_yres;
	int   vsync_scanline_pos;
	int   cursor_cnt;
	int   cursor_state;

	UINT32 clock_freq;
	bitmap_rgb32 m_screen_out;

	required_device<palette_device> m_palette;
};

// device type definition
extern const device_type EF9364;

#endif
