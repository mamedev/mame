// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert
#ifndef MAME_ATARI_ATARISTB_H
#define MAME_ATARI_ATARISTB_H

#pragma once

class st_blitter_device : public device_t
{
	static constexpr uint8_t SKEW_NFSR   = 0x40;
	static constexpr uint8_t SKEW_FXSR   = 0x80;

	static constexpr uint8_t CTRL_SMUDGE = 0x20;
	static constexpr uint8_t CTRL_HOG    = 0x40;
	static constexpr uint8_t CTRL_BUSY   = 0x80;

public:
	st_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(tag, spacenum); }
	auto int_callback() { return m_int_callback.bind(); }

	uint16_t halftone_r(offs_t offset);
	uint16_t src_inc_x_r();
	uint16_t src_inc_y_r();
	uint16_t src_r(offs_t offset);
	uint16_t end_mask_r(offs_t offset);
	uint16_t dst_inc_x_r();
	uint16_t dst_inc_y_r();
	uint16_t dst_r(offs_t offset);
	uint16_t count_x_r();
	uint16_t count_y_r();
	uint16_t op_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t ctrl_r(offs_t offset, uint16_t mem_mask = ~0);

	void halftone_w(offs_t offset, uint16_t data);
	void src_inc_x_w(uint16_t data);
	void src_inc_y_w(uint16_t data);
	void src_w(offs_t offset, uint16_t data);
	void end_mask_w(offs_t offset, uint16_t data);
	void dst_inc_x_w(uint16_t data);
	void dst_inc_y_w(uint16_t data);
	void dst_w(offs_t offset, uint16_t data);
	void count_x_w(uint16_t data);
	void count_y_w(uint16_t data);
	void op_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void blitter_source();
	uint16_t blitter_hop();
	void blitter_op(uint16_t s, uint32_t dstaddr, uint16_t mask);
	TIMER_CALLBACK_MEMBER(blitter_tick);

	required_address_space m_space;
	devcb_write_line m_int_callback;

	// timers
	emu_timer *m_blitter_timer;

	// blitter state
	uint16_t m_halftone[16]{};
	int16_t m_src_inc_x = 0;
	int16_t m_src_inc_y = 0;
	int16_t m_dst_inc_x = 0;
	int16_t m_dst_inc_y = 0;
	uint32_t m_src = 0U;
	uint32_t m_dst = 0U;
	uint16_t m_endmask1 = 0U;
	uint16_t m_endmask2 = 0U;
	uint16_t m_endmask3 = 0U;
	uint16_t m_xcount = 0U;
	uint16_t m_ycount = 0U;
	uint16_t m_xcountl = 0U;
	uint8_t m_hop = 0U;
	uint8_t m_op = 0U;
	uint8_t m_ctrl = 0U;
	uint8_t m_skew = 0U;
	uint32_t m_srcbuf = 0U;
};

// device type declaration
DECLARE_DEVICE_TYPE(ST_BLITTER, st_blitter_device)

#endif // MAME_ATARI_ATARISTB_H
