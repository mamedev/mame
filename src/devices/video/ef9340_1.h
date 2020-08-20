// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/***************************************************************************

    Thomson EF9340 + EF9341 teletext graphics chips

***************************************************************************/

#ifndef MAME_VIDEO_EF9340_1_H
#define MAME_VIDEO_EF9340_1_H

#pragma once


class ef9340_1_device : public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	template <typename T>
	ef9340_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: ef9340_1_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}

	// configuration helpers
	ef9340_1_device &set_offsets(int x, int y) { m_offset_x = x; m_offset_y = y; return *this; } // when used with overlay chip
	auto write_exram() { return m_write_exram.bind(); } // ADR0-ADR3 in a0-a3, B in a4-a11, A in a12-a19
	auto read_exram() { return m_read_exram.bind(); } // "

	ef9340_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	inline bitmap_ind16 *get_bitmap() { return &m_tmp_bitmap; }
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ef9341_write( uint8_t command, uint8_t b, uint8_t data );
	uint8_t ef9341_read( uint8_t command, uint8_t b );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	inline uint16_t ef9340_get_c_addr(uint8_t x, uint8_t y);
	inline void ef9340_inc_c();

	void ef9340_scanline(int vpos);

	/* timers */
	static constexpr device_timer_id TIMER_LINE = 0;
	static constexpr device_timer_id TIMER_BLINK = 1;

	emu_timer *m_line_timer;
	emu_timer *m_blink_timer;

	required_region_ptr<uint8_t> m_charset;

	bitmap_ind16 m_tmp_bitmap;

	int m_offset_x = 0;
	int m_offset_y = 0;

	struct
	{
		uint8_t TA;
		uint8_t TB;
		bool busy;
	} m_ef9341;

	struct
	{
		uint8_t X;
		uint8_t Y;
		uint8_t Y0;
		uint8_t R;
		uint8_t M;
		bool blink;
		int blink_prescaler;
		bool h_parity;
	} m_ef9340;

	uint8_t m_ram_a[0x400];
	uint8_t m_ram_b[0x400];

	devcb_write8 m_write_exram;
	devcb_read8 m_read_exram;
};


// device type definition
DECLARE_DEVICE_TYPE(EF9340_1, ef9340_1_device)

#endif // MAME_VIDEO_EF9340_1_H
