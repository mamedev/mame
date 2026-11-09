// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_VIDEO_I82750DB_H
#define MAME_VIDEO_I82750DB_H

#pragma once

#include "cpu/i750/i750.h"

class i82750db_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	i82750db_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_pixel_processor(T &&tag) { m_pb.set_tag(std::forward<T>(tag)); }
	auto shutdown_callback() { return m_shutdown_cb.bind(); }

	void reset_w(int state);
	void vblank_w(int state);
	void draw(bitmap_rgb32 &bitmap, const rectangle &cliprect, const rectangle &visible);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr unsigned REG_COUNT = 0x60;
	static constexpr unsigned CLUT_SIZE = 256;

	u32 pointer(unsigned index) const;
	s16 pitch(unsigned index) const;
	void load_registers();
	rgb_t color(u8 y, u8 v, u8 u) const;
	u8 chroma(u32 address, unsigned x, unsigned y, unsigned samples, unsigned rows, unsigned step, s32 pitch);

	required_device<i82750pb_device> m_pb;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_vram;
	devcb_write_line m_shutdown_cb;

	u32 m_reg[REG_COUNT];
	u32 m_clut[CLUT_SIZE];
	u32 m_y;
	u32 m_vu;
	s16 m_y_pitch;
	s16 m_vu_pitch;
	bool m_reset;
	bool m_load;
	bool m_enabled;
	bool m_shutdown;
};

DECLARE_DEVICE_TYPE(I82750DB, i82750db_device)

#endif // MAME_VIDEO_I82750DB_H
