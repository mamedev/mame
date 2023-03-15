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

	void write(device_t &device, offs_t offset, uint32_t data);

	bool valid(device_t &device) const;

	uint32_t h_sync(uint32_t scale) const noexcept
	{ return (m_hsync + 1) * scale; }
	uint32_t h_total(uint32_t scale) const noexcept
	{ return (m_htotal + 1) * scale; }
	uint32_t h_active(uint32_t scale) const noexcept
	{ return (m_hend - m_hstart) * scale; }
	uint32_t h_start(uint32_t scale) const noexcept
	{ return (m_hstart + 1) * scale; }
	uint32_t h_end(uint32_t scale) const noexcept
	{ return (m_hend + 1) * scale; }

	uint16_t v_sync() const noexcept
	{ return m_vsync + 1; }
	uint16_t v_total() const noexcept
	{ return m_vtotal + 1; }
	uint32_t v_active() const noexcept
	{ return (m_vend - m_vstart); }
	uint16_t v_start() const noexcept
	{ return m_vstart + 1; }
	uint16_t v_end() const noexcept
	{ return m_vend + 1; }

	attoseconds_t frame_time(uint32_t scale, XTAL const &osc) const noexcept
	{ return attotime::from_ticks((m_htotal + 1) * (m_vtotal + 1) * scale, osc).attoseconds(); }

private:
	uint16_t m_hsync, m_hstart, m_hend, m_htotal;
	uint16_t m_vsync, m_vstart, m_vend, m_vtotal;
};


class supermac_spec_shift_reg
{
public:
	void register_save(device_t &device);

	void reset() noexcept;

	void write_control(device_t &device, uint32_t data);
	void write_data(device_t &device, uint32_t data);

	bool ready() const noexcept { return m_shift_count == 10; }
	uint8_t select() const noexcept { return (m_shift_data >> 6) & 0x03; }
	uint8_t value() const noexcept { return (m_shift_data >> 8) & 0xff; }

private:
	uint16_t m_shift_data;
	uint8_t m_shift_count;
};

#endif // MAME_BUS_NUBUS_SUPERMAC_H
