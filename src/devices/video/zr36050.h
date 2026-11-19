// license:BSD-3-Clause
// copyright-holders:R. Belmont, Tomasz Slanina
/***************************************************************************

    Zoran ZR36050 JPEG Image Compression Processor

***************************************************************************/

#ifndef MAME_VIDEO_ZR36050_H
#define MAME_VIDEO_ZR36050_H

#pragma once

#include <vector>


class zr36050_device : public device_t
{
public:
	zr36050_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// compressed data source when the ZR36050 masters the code bus
	auto code_read() { return m_code_read_cb.bind(); }

	// /END, active high here
	auto end() { return m_end_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// compressed data from a code bus master
	void code_w(u8 data);

	// the last decoded image, as Y, Cb, Cr bytes per pixel
	u16 image_width() const { return m_width; }
	u16 image_height() const { return m_height; }
	const u8 *image_row(int y) const { return &m_image[size_t(y) * m_width * 3]; }

	// JFIF conversion, as done after the codec by the ZR36011/ZR36016
	static rgb_t ycbcr_to_rgb(u8 y, u8 cb, u8 cr);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	static constexpr u32 CODE_SIZE = 0x40000;

	// register addresses
	enum : offs_t
	{
		GO = 0x000,
		HARDWARE = 0x002,
		MODE = 0x003,
		STATUS_0 = 0x02e,
		STATUS_1 = 0x02f
	};

	// parser states for finding the end of the code stream
	enum : u8
	{
		CODE_IDLE,
		CODE_SOI,
		CODE_MARKER,
		CODE_MARKER_ID,
		CODE_LENGTH_HI,
		CODE_LENGTH_LO,
		CODE_SEGMENT,
		CODE_ENTROPY,
		CODE_ENTROPY_FF,
		CODE_DONE
	};

	devcb_read8 m_code_read_cb;
	devcb_write_line m_end_cb;
	memory_share_creator<u8> m_code;

	u8 m_regs[0x400];
	u32 m_code_length;
	u8 m_code_state;
	u8 m_code_marker;
	u16 m_code_remaining;
	bool m_running;

	std::vector<u8> m_image;
	u16 m_width;
	u16 m_height;

	void go();
	void code_byte(u8 data);
	void finish();
	void decode();
};

DECLARE_DEVICE_TYPE(ZR36050, zr36050_device)

#endif // MAME_VIDEO_ZR36050_H
