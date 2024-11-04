// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_PARADISE_H
#define MAME_VIDEO_PC_VGA_PARADISE_H

#pragma once

#include "video/pc_vga.h"

class pvga1a_vga_device : public svga_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	pvga1a_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;
protected:
	pvga1a_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void gc_map(address_map &map) override ATTR_COLD;

	u8 m_video_select = 0;
	u8 m_crtc_lock = 0;
private:
	virtual u8 gc_data_r(offs_t offset) override;
	virtual void gc_data_w(offs_t offset, u8 data) override;

	u8 address_offset_r(offs_t offset);
	void address_offset_w(offs_t offset, u8 data);
	u8 memory_size_r(offs_t offset);
	void memory_size_w(offs_t offset, u8 data);
	u8 video_select_r(offs_t offset);
	void video_select_w(offs_t offset, u8 data);
	u8 crtc_lock_r(offs_t offset);
	void crtc_lock_w(offs_t offset, u8 data);
	u8 video_control_r(offs_t offset);
	void video_control_w(offs_t offset, u8 data);
	u8 ext_gc_status_r(offs_t offset);
	void ext_gc_unlock_w(offs_t offset, u8 data);

	u8 m_memory_size = 0;
	u8 m_video_control = 0;
	bool m_ext_gc_unlock = false;
	bool m_ega_compatible_mode = false;
};

class wd90c00_vga_device : public pvga1a_vga_device
{
public:
	wd90c00_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// TODO: backport to original PVGA1A
	// claims these as CNF(7)-CNF(4), which implies input sense read from GC register instead.
	auto read_cnf15_callback() { return m_cnf15_read_cb.bind(); }
	auto read_cnf14_callback() { return m_cnf14_read_cb.bind(); }
	auto read_cnf13_callback() { return m_cnf13_read_cb.bind(); }
	auto read_cnf12_callback() { return m_cnf12_read_cb.bind(); }

	// NOTE: these are internal shadows, for the input sense.
	ioport_value egasw4_r();
	ioport_value egasw3_r();
	ioport_value egasw2_r();
	ioport_value egasw1_r();
protected:
	wd90c00_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_reset() override ATTR_COLD;

	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void recompute_params() override;

	virtual bool get_interlace_mode() override { return m_interlace_mode; }

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	virtual u8 crtc_data_r(offs_t offset) override;
	virtual void crtc_data_w(offs_t offset, u8 data) override;

	u8 ext_crtc_status_r(offs_t offset);
	void ext_crtc_unlock_w(offs_t offset, u8 data);
	u8 egasw_r(offs_t offset);
	void egasw_w(offs_t offset, u8 data);
	u8 interlace_r(offs_t offset);
	void interlace_w(offs_t offset, u8 data);
	u8 misc_control_1_r(offs_t offset);
	void misc_control_1_w(offs_t offset, u8 data);

	bool m_ext_crtc_read_unlock = false;
	bool m_ext_crtc_write_unlock = false;
	u8 m_pr10_scratch = 0;
	u8 m_pr11 = 0;
	u8 m_interlace_start = 0;
	u8 m_interlace_end = 0;
	bool m_interlace_mode = 0;
	u8 m_pr15 = 0;

	devcb_read_line m_cnf15_read_cb;
	devcb_read_line m_cnf14_read_cb;
	devcb_read_line m_cnf13_read_cb;
	devcb_read_line m_cnf12_read_cb;
};

class wd90c11a_vga_device : public wd90c00_vga_device
{
public:
	wd90c11a_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	wd90c11a_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void sequencer_map(address_map &map) override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_view m_ext_seq_view;
private:
	virtual u8 sequencer_data_r(offs_t offset) override;
	virtual void sequencer_data_w(offs_t offset, u8 data) override;

	u8 ext_seq_status_r(offs_t offset);
	void ext_seq_unlock_w(offs_t offset, u8 data);
	u8 sys_if_control_r(offs_t offset);
	void sys_if_control_w(offs_t offset, u8 data);

	bool m_ext_seq_unlock = false;
	u8 m_pr31 = 0;
};

class wd90c30_vga_device : public wd90c11a_vga_device
{
public:
	wd90c30_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	wd90c30_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void sequencer_map(address_map &map) override ATTR_COLD;

	virtual void device_reset() override ATTR_COLD;
	virtual u16 line_compare_mask() override;
private:
	u8 vert_timing_overflow_r(offs_t offset);
	void vert_timing_overflow_w(offs_t offset, u8 data);

	u8 m_pr18 = 0;
};

class wd90c31_vga_device : public wd90c30_vga_device
{
public:
	wd90c31_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void ext_io_map(address_map &map) ATTR_COLD;

protected:
	wd90c31_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class wd90c33_vga_device : public wd90c31_vga_device
{
public:
	wd90c33_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void ext_io_map(address_map &map) override ATTR_COLD;
	void localbus_if_map(address_map &map) ATTR_COLD;

protected:
	wd90c33_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(PVGA1A, pvga1a_vga_device)
DECLARE_DEVICE_TYPE(WD90C00, wd90c00_vga_device)
DECLARE_DEVICE_TYPE(WD90C11A, wd90c11a_vga_device)
DECLARE_DEVICE_TYPE(WD90C30, wd90c30_vga_device)
DECLARE_DEVICE_TYPE(WD90C31, wd90c31_vga_device)
DECLARE_DEVICE_TYPE(WD90C33, wd90c33_vga_device)

#endif // MAME_VIDEO_PC_VGA_PARADISE_H
