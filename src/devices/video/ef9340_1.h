// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    ef9340_1.h

    Thomson EF9340 + EF9341 teletext graphics chips with 1KB external
    character ram.

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

	ef9340_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	inline bitmap_ind16 *get_bitmap() { return &m_tmp_bitmap; }

	void ef9341_write( uint8_t command, uint8_t b, uint8_t data );
	uint8_t ef9341_read( uint8_t command, uint8_t b );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	inline uint16_t ef9340_get_c_addr(uint8_t x, uint8_t y);
	inline void ef9340_inc_c();

	// Calculate the external chargen address for a character and slice
	inline uint16_t external_chargen_address(uint8_t b, uint8_t slice);

	void ef9340_scanline(int vpos);

	/* timers */
	static constexpr device_timer_id TIMER_LINE = 0;

	emu_timer *m_line_timer;

	required_region_ptr<uint8_t> m_charset;

	bitmap_ind16 m_tmp_bitmap;

	struct
	{
		uint8_t   TA;
		uint8_t   TB;
		uint8_t   busy;
	} m_ef9341;
	struct
	{
		uint8_t   X;
		uint8_t   Y;
		uint8_t   Y0;
		uint8_t   R;
		uint8_t   M;
		int     max_vpos;
	} m_ef9340;
	uint8_t   m_ef934x_ram_a[1024];
	uint8_t   m_ef934x_ram_b[1024];
	uint8_t   m_ef934x_ext_char_ram[2048];   /* The G7400 has 2KB of external ram hooked up. The datasheet only describes how to hookup 1KB. */
};


// device type definition
DECLARE_DEVICE_TYPE(EF9340_1, ef9340_1_device)

#endif // MAME_VIDEO_EF9340_1_H
