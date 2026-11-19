// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II/12/16/6000 video/keyboard interface emulation

**********************************************************************/

#ifndef MAME_BUS_TRS80M2_VIDKB_H
#define MAME_BUS_TRS80M2_VIDKB_H

#pragma once

#include "slot.h"

#include "bus/trs80m2kbd/kbd.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"


class trs80m2_vidkb_device_base : public device_t, public device_trs80m2_card_interface
{
protected:
	trs80m2_vidkb_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool has_mirror);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual int access_wait(attotime const &now) = 0;

	int cclk_wait(attotime const &now);
	int blank_wait(attotime const &now);

	required_ioport m_config;

private:
	void map(address_map &map) ATTR_COLD;

	MC6845_UPDATE_ROW( crtc_update_row );
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t keyboard_r();
	void crtc_address_w(uint8_t data);
	void crtc_register_w(uint8_t data);
	uint8_t rtc_r();
	uint8_t status_r();
	void latch_w(uint8_t data);
	uint8_t video_ram_r(offs_t offset);
	void video_ram_w(offs_t offset, uint8_t data);
	void de_w(int state);
	void vsync_w(int state);
	void kb_clock_w(int state);
	void update_nmi();
	void update_kbirq();

	required_device<screen_device> m_screen;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<trs80m2_keyboard_port_device> m_kb;
	required_region_ptr<uint8_t> m_char_rom;
	memory_share_creator<uint8_t> m_video_ram;

	bool const m_has_mirror;
	bool m_installed;
	uint8_t m_crtc_address;
	uint8_t m_hdisp;
	attotime m_de_start;
	uint8_t m_de;

	uint8_t m_key_data;
	uint8_t m_key_bit;
	uint8_t m_kbclk;
	uint8_t m_kbdata;
	uint8_t m_kbirq;

	uint8_t m_blnkvid;
	uint8_t m_80_40_char_en;
	uint8_t m_enable_rtc_int;
	uint8_t m_rtc_div;
	uint8_t m_rtc_int;
};


class trs80m2_vidkb_device : public trs80m2_vidkb_device_base
{
public:
	trs80m2_vidkb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual int access_wait(attotime const &now) override;
};


class trs80m12_vidkb_device : public trs80m2_vidkb_device_base
{
public:
	trs80m12_vidkb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual int access_wait(attotime const &now) override;
};


DECLARE_DEVICE_TYPE(TRS80M2_VIDKB, trs80m2_vidkb_device)
DECLARE_DEVICE_TYPE(TRS80M12_VIDKB, trs80m12_vidkb_device)

#endif // MAME_BUS_TRS80M2_VIDKB_H
