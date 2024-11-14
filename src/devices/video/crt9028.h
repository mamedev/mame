// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Standard Microsystems CRT9028/9128 Video Terminal Logic Controller

***********************************************************************
                           _____    _____
                  DA8   1 |*    \__/     | 40  DA7
                  DA9   2 |              | 39  DA6
                 DA10   3 |              | 38  DA5
                  GND   4 |              | 37  DA4
                XTAL2   5 |              | 36  DA3
                XTAL1   6 |              | 35  DA2
               /VIDEO   7 |              | 34  DA1
               INTOUT   8 |              | 33  DA0
                 /DWR   9 |              | 32  DB7
                  DD0  10 |   CRT 9028   | 31  DB6
                  DD1  11 |   CRT 9128   | 30  DB5
                  DD2  12 |              | 29  DB4
                  DD3  13 |              | 28  DB3
                  DD4  14 |              | 27  DB2
                  DD5  15 |              | 26  DB1
                  DD6  16 |              | 25  DB0
                  DD7  17 |              | 24  A/D
                HSYNC  18 |              | 23  /RD or /DS
                VSYNC  19 |              | 22  /WR or R/W
                CSYNC  20 |______________| 21  VCC

**********************************************************************/

#ifndef MAME_VIDEO_CRT9028_H
#define MAME_VIDEO_CRT9028_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> crt9028_device

class crt9028_device : public device_t, public device_memory_interface, public device_video_interface
{
public:
	// device configuration
	auto hsync_callback() { return m_hsync_callback.bind(); }
	auto vsync_callback() { return m_hsync_callback.bind(); }

	// read/write handlers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// screen  update method
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// base type constructor
	crt9028_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
							int dots_per_char, int chars_per_row, int horiz_blanking, int hsync_delay, int hsync_width, bool hsync_active,
							int char_rows, int scans_per_char, bool vsync_active,
							int vert_blanking, int vsync_delay, int vsync_width,
							int alt_vert_blanking, int alt_vsync_delay, int alt_vsync_width,
							int csync_delay, int csync_width, int underline,
							u16 wide_gfx_seg1_4, u16 wide_gfx_seg2_5, u16 wide_gfx_seg3_6, u8 wide_gfx_pattern,
							u16 thin_gfx_seg1, u16 thin_gfx_seg2, u16 thin_gfx_seg3, u16 thin_gfx_seg4,
							u8 thin_gfx_dots1, u8 thin_gfx_dots2, u8 thin_gfx_dots3, u8 thin_gfx_dots4);

	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

private:
	// register helpers
	void chip_reset();
	u8 status_r();
	void filadd_w(u8 data);
	void tosadd_w(u8 data);
	void curlo_w(u8 data);
	void curhi_w(u8 data);
	void attdat_w(u8 data);
	void mode_w(u8 data);
	u8 character_r();
	void character_w(u8 data);

	// address space for display memory
	const address_space_config m_space_config;
	address_space *m_space;

	// internal character set
	required_region_ptr<u8> m_charset;

	// timing callbacks
	devcb_write_line m_hsync_callback;
	devcb_write_line m_vsync_callback;

	// mask parameters
	const int m_dots_per_char;
	const int m_chars_per_row;
	const int m_horiz_blanking;
	const int m_hsync_delay;
	const int m_hsync_width;
	const bool m_hsync_active;
	const int m_char_rows;
	const int m_scans_per_char;
	const bool m_vsync_active;
	const int m_vert_blanking[2];
	const int m_vsync_delay[2];
	const int m_vsync_width[2];
	const int m_csync_delay;
	const int m_csync_width;
	const int m_underline;
	const u16 m_wide_gfx_seg[3];
	const u8 m_wide_gfx_pattern;
	const u16 m_thin_gfx_seg[4];
	const u8 m_thin_gfx_dots[4];

	// internal state
	u8 m_address_register;
};

// ======================> crt9028_000_device

class crt9028_000_device : public crt9028_device
{
public:
	// device constructor
	crt9028_000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-specific overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type declarations
DECLARE_DEVICE_TYPE(CRT9028_000, crt9028_000_device)

#endif // MAME_VIDEO_CRT9028_H
