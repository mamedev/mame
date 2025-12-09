// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_BUS_NUBUS_SUPERMAC_H
#define MAME_BUS_NUBUS_SUPERMAC_H

#pragma once


class supermac_spec_crtc
{
public:
	void register_save(device_t &device);

	void reset() noexcept;

	void write(device_t &device, offs_t offset, u32 data);

	bool valid(device_t &device) const;

	u32 h_sync(u32 scale) const noexcept
	{ return (m_hsync + 1) * scale; }
	u32 h_total(u32 scale) const noexcept
	{ return (m_htotal + 1) * scale; }
	u32 h_active(u32 scale) const noexcept
	{ return (m_hend - m_hstart) * scale; }
	u32 h_start(u32 scale) const noexcept
	{ return (m_hstart + 1) * scale; }
	u32 h_end(u32 scale) const noexcept
	{ return (m_hend + 1) * scale; }

	u16 v_sync() const noexcept
	{ return m_vsync + 1; }
	u16 v_total() const noexcept
	{ return m_vtotal + 1; }
	u32 v_active() const noexcept
	{ return (m_vend - m_vstart); }
	u16 v_start() const noexcept
	{ return m_vstart + 1; }
	u16 v_end() const noexcept
	{ return m_vend + 1; }

	attoseconds_t frame_time(u32 scale, XTAL const &osc) const noexcept
	{ return attotime::from_ticks((m_htotal + 1) * (m_vtotal + 1) * scale, osc).attoseconds(); }

private:
	u16 m_hsync, m_hstart, m_hend, m_htotal;
	u16 m_vsync, m_vstart, m_vend, m_vtotal;
};


class supermac_spec_shift_reg
{
public:
	void register_save(device_t &device);

	void reset() noexcept;

	void write_control(device_t &device, u32 data);
	void write_data(device_t &device, u32 data);

	bool ready() const noexcept { return m_shift_count == 10; }
	u8 select() const noexcept { return (m_shift_data >> 6) & 0x03; }
	u8 value() const noexcept { return (m_shift_data >> 8) & 0xff; }

private:
	u16 m_shift_data;
	u8 m_shift_count;
};

#endif // MAME_BUS_NUBUS_SUPERMAC_H
