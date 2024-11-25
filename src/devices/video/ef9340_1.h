// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/***************************************************************************

    Thomson EF9340 + EF9341 teletext graphics chips

***************************************************************************/

#ifndef MAME_VIDEO_EF9340_1_H
#define MAME_VIDEO_EF9340_1_H

#pragma once


// pinout reference

/*
            ______   ______                                   ______   ______
    Vss  1 |*     \_/      | 40 A4    10*address      Vss  1 |*     \_/      | 40 A7
     A5  2 |               | 39 A3    ------------>  ADR0  2 |               | 39 A6
     A6  3 |               | 38 A2          |        ADR1  3 |               | 38 A5
    Vss  4 |               | 37 A1          |        ADR2  4 |               | 37 A4
    CLK  5 |               | 36 A0          v        ADR3  5 |               | 36 A3
    _VE  6 |               | 35 ADR0   ___________  R/_WI  6 |               | 35 A2
   C/_T  7 |               | 34 ADR1  |           |   _SG  7 |               | 34 A1
      B  8 |               | 33 ADR2  |           |   _SM  8 |               | 33 A0
      G  9 |               | 32 ADR3  |           |    B7  9 |               | 32 D0
      R 10 |     EF9340    | 31 ADR4  | 2*1KB RAM |    B6 10 |     EF9341    | 31 D1
      I 11 |     (VIN)     | 30 ADR5  |           |    B5 11 |     (GEN)     | 30 D2
    _SM 12 |               | 29 ADR6  |           |    B4 12 |               | 29 D3
    _SG 13 |               | 28 ADR7  |           |    B3 13 |               | 28 D4
    _ST 14 |               | 27 ADR8  |___________|    B2 14 |               | 27 D5
  R/_WI 15 |               | 26 ADR9                   B1 15 |               | 26 D6
     TT 16 |               | 25 TL          ^          B0 16 |               | 25 D7
    SYT 17 |               | 24 TEST        |         _ST 17 |               | 24 C/_T
     B7 18 |               | 23 _RES        v         _VE 18 |               | 23 R/_W
     B6 19 |               | 22 A7    <----------->   _CS 19 |               | 22 B/_A
     B5 20 |_______________| 21 Vcc   16*data           E 20 |_______________| 21 Vcc

*/


class ef9340_1_device : public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	template <typename T>
	ef9340_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag)
		: ef9340_1_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}

	// configuration helpers
	ef9340_1_device &set_offsets(int x, int y) { m_offset_x = x; m_offset_y = y; return *this; } // when used with overlay chip
	auto write_exram() { return m_write_exram.bind(); } // ADR0-ADR3 in a0-a3, B in a4-a11, A in a12-a19
	auto read_exram() { return m_read_exram.bind(); } // "

	ef9340_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	inline bitmap_ind16 *get_bitmap() { return &m_tmp_bitmap; }
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ef9341_write(u8 command, u8 b, u8 data);
	u8 ef9341_read(u8 command, u8 b);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	inline u16 ef9340_get_c_addr(u8 x, u8 y);
	inline void ef9340_inc_c();

	TIMER_CALLBACK_MEMBER(draw_scanline);
	TIMER_CALLBACK_MEMBER(blink_update);

	// timers
	emu_timer *m_line_timer;
	emu_timer *m_blink_timer;

	required_region_ptr<u8> m_charset;

	bitmap_ind16 m_tmp_bitmap;

	int m_offset_x;
	int m_offset_y;

	struct
	{
		u8 TA;
		u8 TB;
		bool busy;
	} m_ef9341;

	struct
	{
		u8 X;
		u8 Y;
		u8 Y0;
		u8 R;
		u8 M;
		bool blink;
		int blink_prescaler;
		bool h_parity;
	} m_ef9340;

	u8 m_ram_a[0x400];
	u8 m_ram_b[0x400];

	devcb_write8 m_write_exram;
	devcb_read8 m_read_exram;
};


// device type definition
DECLARE_DEVICE_TYPE(EF9340_1, ef9340_1_device)

#endif // MAME_VIDEO_EF9340_1_H
