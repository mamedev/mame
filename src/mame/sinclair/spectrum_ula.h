// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECTRUM_ULA_H
#define MAME_SINCLAIR_SPECTRUM_ULA_H

#pragma once

#include "cpu/z80/z80.h"
#include "screen.h"

class spectrum_ula_device : public device_t
{
public:
	spectrum_ula_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_z80(T &&cpu_tag) { m_maincpu.set_tag(std::forward<T>(cpu_tag)); }
	template <typename T> void set_screen(T &&screen_tag, rectangle screen_area) { m_screen.set_tag(std::forward<T>(screen_tag)); m_screen_area = screen_area; }
	virtual INPUT_CHANGED_MEMBER(on_contention_changed) {};

	virtual void nomem_rq(offs_t offset, u8 data) {}
	virtual void m1(offs_t offset) {}
	virtual void io_r(offs_t offset) {}
	virtual void io_w(offs_t offset) {}
	virtual void data_r(offs_t offset) {}
	virtual void data_w(offs_t offset) {}
	virtual void ula_r(offs_t offset) {}
	virtual void ula_w(offs_t offset) {}

	virtual void on_irq() {}
	void bank3_pg_w(u8 bank3_page) { m_bank3_page = bank3_page; }

	virtual bool is_snow_possible(u16 addr) { return false; }
	virtual bool is_in_contended_area() { return false; }
	virtual u64 get_video_line_clocks() { assert(false); return -1; } // video line clocks data is undefined for uncontended case and must be guarded with "if" condition
	virtual u64 get_raster_line_clocks() { assert(false); return -1; } // raster line clocks data is undefined
	virtual u64 get_irq_at() { assert(false); return -1; } // irq time is undefined
	virtual u8 get_irq_ext_length() { return 0; }
	virtual u8 get_border_chunk_size() { return 1; }
	virtual u8 get_btime() { return 0; }
	virtual u8 get_atime_left() { return 0; }
	virtual u8 get_atime_right() { return 0; }
	virtual s8 get_raster_contention_offset() { return 0; }

protected:
	spectrum_ula_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_reset() override ATTR_COLD {}
	virtual void device_start() override ATTR_COLD;

	optional_device<z80_device> m_maincpu;
	optional_device<screen_device> m_screen;

	rectangle m_screen_area;
	u8 m_bank3_page;
};


class spectrum_ula_contended_device : public spectrum_ula_device
{
public:
	virtual INPUT_CHANGED_MEMBER(on_contention_changed) override;

	virtual void nomem_rq(offs_t offset, u8 data) override;
	virtual void m1(offs_t offset) override;
	virtual void io_r(offs_t offset) override;
	virtual void io_w(offs_t offset) override;
	virtual void ula_r(offs_t offset) override;
	virtual void ula_w(offs_t offset) override;
	virtual void data_r(offs_t offset) override;
	virtual void data_w(offs_t offset) override;

	virtual void on_irq() override;

	virtual bool is_snow_possible(u16 addr) override;
	virtual bool is_in_contended_area() override;
	virtual u64 get_video_line_clocks() override { return m_video_line_clocks; }
	virtual u64 get_raster_line_clocks() override { return m_raster_line_clocks; }
	virtual u64 get_irq_at() override { return m_int_at; }
	virtual u8 get_irq_ext_length() override { return m_is_timings_late; };
	virtual u8 get_border_chunk_size() override { return 8; }
	virtual u8 get_btime() override { return m_btime; }
	virtual u8 get_atime_left() override { return m_atime_left; }
	virtual u8 get_atime_right() override { return m_atime_right; }
	virtual s8 get_raster_contention_offset() override { return m_base_offset + m_is_timings_late; }

protected:
	spectrum_ula_contended_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual bool is_contended(offs_t offset) = 0;

	u8 m_pattern[8];
	u8 m_btime; // Pixel offset in 16px chunk (8T) when current [b]order attribute is applied.
	u8 m_atime_left; // Pixel offset when [a]ttribute is applied on the left side.
	u8 m_atime_right; // Pixel offset when [a]ttribute is applied on the right side.
	s8 m_base_offset; // Defines offset in CPU cycles from screen left side. Early model (48/128/+2) typically use -1, later (+2A/+3) +1

	u64 m_video_line_clocks;
	u64 m_raster_line_clocks;
	bool m_is_timings_late;
	u64 m_int_at;
	bool m_is_m1_rd_contended;

private:
	void content_early(s8 shift = 0);
	void content_late();

};

class spectrum_ula_48k_device : public spectrum_ula_contended_device
{
public:
	spectrum_ula_48k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual bool is_contended(offs_t offset) override;
};


class spectrum_ula_128k_device : public spectrum_ula_contended_device
{
public:
	spectrum_ula_128k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual bool is_contended(offs_t offset) override;
};


class spectrum_ula_plus2a_device : public spectrum_ula_contended_device
{
public:
	spectrum_ula_plus2a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual INPUT_CHANGED_MEMBER(on_contention_changed) override;

protected:
	virtual bool is_contended(offs_t offset) override;
};


DECLARE_DEVICE_TYPE(SPECTRUM_ULA_UNCONTENDED, spectrum_ula_device)
DECLARE_DEVICE_TYPE(SPECTRUM_ULA_48K, spectrum_ula_48k_device)
DECLARE_DEVICE_TYPE(SPECTRUM_ULA_128K, spectrum_ula_128k_device)
DECLARE_DEVICE_TYPE(SPECTRUM_ULA_PLUS2A, spectrum_ula_plus2a_device)
#endif // MAME_SINCLAIR_SPECTRUM_ULA_H
