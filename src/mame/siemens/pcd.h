// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_SIEMENS_PCD_H
#define MAME_SIEMENS_PCD_H

#pragma once

#include "machine/pic8259.h"
#include "machine/timer.h"
#include "video/scn2674.h"
#include "emupal.h"


class pcdx_video_device : public device_t, public device_gfx_interface
{
public:
	virtual void map(address_map &map) = 0;
	uint8_t detect_r();
	void detect_w(uint8_t data);

protected:
	pcdx_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void pcdx_palette(palette_device &palette) const;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<pic8259_device> m_pic2;
};

class pcd_video_device : public pcdx_video_device
{
public:
	pcd_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;
	void vram_sw_w(uint8_t data);
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<scn2674_device> m_crtc;
	required_ioport m_mouse_btn;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;

	std::vector<uint8_t> m_vram;
	std::vector<uint8_t> m_charram;
	uint8_t m_vram_sw, m_t1, m_p2;

	struct
	{
		int phase;
		int x;
		int y;
		int prev_x;
		int prev_y;
		int xa;
		int xb;
		int ya;
		int yb;
	} m_mouse;

	uint8_t t1_r();
	uint8_t p1_r();
	void p2_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(mouse_timer);

	SCN2674_DRAW_CHARACTER_MEMBER(display_pixels);
};

class pcx_video_device : public pcdx_video_device
{
public:
	pcx_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	auto txd_handler() { return m_txd_handler.bind(); }
	void rx_w(int state);

	virtual void map(address_map &map) override ATTR_COLD;
	uint8_t term_r(offs_t offset);
	void term_w(offs_t offset, uint8_t data);
	uint8_t term_mcu_r(offs_t offset);
	void term_mcu_w(offs_t offset, uint8_t data);
	uint8_t unk_r();
	void p1_w(uint8_t data);

	void pcx_vid_io(address_map &map) ATTR_COLD;
	void pcx_vid_map(address_map &map) ATTR_COLD;
	void pcx_char_ram(address_map &map) ATTR_COLD;
	void pcx_attr_ram(address_map &map) ATTR_COLD;
protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<scn2672_device> m_crtc;

	required_region_ptr<uint8_t> m_charrom;
	devcb_write_line m_txd_handler;
	uint8_t m_term_key, m_term_char, m_term_stat, m_p1, m_p3;

	uint8_t p3_r();
	void p3_w(uint8_t data);

	SCN2672_DRAW_CHARACTER_MEMBER(display_pixels);
};

DECLARE_DEVICE_TYPE(PCD_VIDEO, pcd_video_device)
DECLARE_DEVICE_TYPE(PCX_VIDEO, pcx_video_device)

#endif // MAME_SIEMENS_PCD_H
