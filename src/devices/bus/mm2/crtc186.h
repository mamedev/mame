// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 CRTC186 emulation

*********************************************************************/

#ifndef MAME_BUS_MM2_CRTC186_H
#define MAME_BUS_MM2_CRTC186_H

#pragma once

#include "exp.h"
#include "emupal.h"
#include "mm2kb.h"
#include "screen.h"
#include "machine/clock.h"
#include "machine/i8251.h"
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
    virtual void device_reset() override ATTR_COLD;

private:
    void map(address_map &map) ATTR_COLD;

	void int_w(int state) { m_bus->ir5_w(state); }

	memory_share_creator<uint16_t> m_video_ram;
	required_memory_region m_char_rom;
	required_memory_region m_attr_rom;
    required_device<crt9007_device> m_vpac;
	required_device<crt9212_device> m_drb0;
	required_device<crt9212_device> m_drb1;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_timer_vidldsh;
	required_device<i8251_device> m_sio;
	required_device<mm2_keyboard_device> m_kb;
	required_device<screen_device> m_screen;

	void palette(palette_device &palette) const;

	void vpac_mem(address_map &map) ATTR_COLD;

    bitmap_rgb32 m_bitmap;

	uint16_t vpac_r(offs_t offset, uint16_t mem_mask);
	void vpac_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void cpl_w(offs_t offset, uint8_t data) { m_cpl = BIT(data, 0); }
	void blc_w(offs_t offset, uint8_t data) { m_blc = BIT(data, 0); }
	void mode_w(offs_t offset, uint8_t data) { m_mode = BIT(data, 0); set_vidldsh_timer(); }
	void modeg_w(offs_t offset, uint8_t data) { m_modeg = BIT(data, 0); set_vidldsh_timer(); }
	void c70_50_w(offs_t offset, uint8_t data) { m_c70_50 = BIT(data, 0); }
	void cru_w(offs_t offset, uint8_t data) { m_cru = BIT(data, 0); }
	void crb_w(offs_t offset, uint8_t data) { m_crb = BIT(data, 0); }

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
};

#endif // MAME_BUS_MM2_CRTC186_H
