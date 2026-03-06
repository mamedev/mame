// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 CRTC186 emulation

*********************************************************************/

#ifndef MAME_BUS_MM2_CRTC186_H
#define MAME_BUS_MM2_CRTC186_H

#pragma once

#include "exp.h"
#include "mm2kb.h"
#include "screen.h"
#include "machine/74259.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "machine/timer.h"
#include "video/crt9007.h"
#include "video/crt9212.h"

DECLARE_DEVICE_TYPE(NOKIA_CRTC186, crtc186_device)

class crtc186_device : public device_t, public device_mikromikko2_expansion_bus_card_interface
{
public:
	crtc186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void device_start() override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;

	void int_w(int state) { m_bus->ir5_w(state); }

	memory_share_creator<uint16_t> m_video_ram;
	required_memory_region m_char_rom;
	required_memory_region m_attr_rom;
	required_device<crt9007_device> m_vpac;
	required_device<crt9212_device> m_drb0;
	required_device<crt9212_device> m_drb1;
	required_device<timer_device> m_timer_vidldsh;
	required_device<i8251_device> m_sio;
	required_device<mm2_keyboard_device> m_kb;
	required_device<ls259_device> m_ctrl;
	required_device<screen_device> m_screen;

	void vpac_mem(address_map &map) ATTR_COLD;

	bitmap_rgb32 m_bitmap;

	uint16_t vpac_r(offs_t offset, uint16_t mem_mask);
	void vpac_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void cpl_w(int state) { m_cpl = state; }
	void blc_w(int state) { m_blc = state; }
	void mode_w(int state) { m_mode = state; set_vidldsh_timer(); }
	void modeg_w(int state) { m_modeg = state; set_vidldsh_timer(); }
	void c70_50_w(int state) { m_c70_50 = state; }
	void cru_w(int state) { m_cru = state; }
	void crb_w(int state) { m_crb = state; }

	uint8_t videoram_r(offs_t offset);
	void vpac_vlt_w(int state);
	void vpac_drb_w(int state);
	void vpac_wben_w(int state);
	void vpac_cblank_w(int state);
	void vpac_slg_w(int state);
	void vpac_sld_w(int state);
	void vidla_w(uint8_t data);
	void drb_attr_w(uint8_t data);
	void set_vidldsh_timer();

	TIMER_DEVICE_CALLBACK_MEMBER( vidldsh_tick );

	u8 m_vidla;
	bool m_cpl;
	bool m_blc;
	bool m_mode;
	bool m_modeg;
	bool m_c70_50;
	bool m_cru;
	bool m_crb;
	int m_cursor_x;
	int m_cursor_y;

	static constexpr rgb_t halflit() { return rgb_t(0x7f, 0x7f, 0x7f); }
};

#endif // MAME_BUS_MM2_CRTC186_H
