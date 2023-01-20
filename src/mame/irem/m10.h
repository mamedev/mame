// license:BSD-3-Clause
// copyright-holders:Lee Taylor, Couriersud
/***************************************************************************

    IREM M-10,M-11 and M-15 based hardware

****************************************************************************/
#ifndef MAME_INCLUDES_M10_H
#define MAME_INCLUDES_M10_H

#pragma once

#include "machine/74123.h"
#include "machine/rescap.h"
#include "sound/samples.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#define IREMM10_MASTER_CLOCK    12.5_MHz_XTAL

#define IREMM10_CPU_CLOCK       (IREMM10_MASTER_CLOCK/16)
#define IREMM10_PIXEL_CLOCK     (IREMM10_MASTER_CLOCK/2)
#define IREMM10_HTOTAL          (360)   /* (0x100-0xd3)*8 */
#define IREMM10_HBSTART         (248)
#define IREMM10_HBEND           (8)
#define IREMM10_VTOTAL          (281)   /* (0x200-0xe7) */
#define IREMM10_VBSTART         (240)
#define IREMM10_VBEND           (16)

#define IREMM11_MASTER_CLOCK    11.73_MHz_XTAL

#define IREMM11_CPU_CLOCK       (IREMM11_MASTER_CLOCK/16)
#define IREMM11_PIXEL_CLOCK     (IREMM11_MASTER_CLOCK/2)
#define IREMM11_HTOTAL          (372)
#define IREMM11_HBSTART         (256)
#define IREMM11_HBEND           (0)
#define IREMM11_VTOTAL          (262)
#define IREMM11_VBSTART         (240)
#define IREMM11_VBEND           (16)

class m1x_state : public driver_device
{
public:
	m1x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_chargen(*this, "chargen"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_cab(*this, "CAB")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void colorram_w(offs_t offset, uint8_t data);

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_chargen;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_ioport m_cab;

	// video-related
	tilemap_t * m_tx_tilemap;

	// video state
	uint8_t m_flip = 0;

	// misc
	int m_last = 0;

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
};

class m10_state : public m1x_state
{
public:
	m10_state(const machine_config &mconfig, device_type type, const char *tag) :
		m1x_state(mconfig, type, tag),
		m_ic8j1(*this, "ic8j1"),
		m_ic8j2(*this, "ic8j2")
	{ }

	void m10(machine_config &config);
	void m11(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(set_vr1) { m_ic8j2->set_resistor_value(RES_K(10 + newval / 5.0)); }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<ttl74123_device> m_ic8j1;
	required_device<ttl74123_device> m_ic8j2;

	gfx_element *       m_back_gfx = nullptr;
	int                 m_back_color[4];
	int                 m_back_xpos[4];
	uint8_t             m_bottomline = 0U;

	void m10_ctrl_w(uint8_t data);
	void m11_ctrl_w(uint8_t data);
	void m10_a500_w(uint8_t data);
	void m11_a100_w(uint8_t data);
	uint8_t clear_74123_r();
	void chargen_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void plot_pixel( bitmap_ind16 &bm, int x, int y, int col );

	void m10_main(address_map &map);
	void m11_main(address_map &map);
};

class m15_state : public m1x_state
{
public:
	m15_state(const machine_config &mconfig, device_type type, const char *tag) :
		m1x_state(mconfig, type, tag)
	{ }

	void m15(machine_config &config);

protected:
	virtual void video_start() override;

private:
	void ctrl_w(uint8_t data);
	void a100_w(uint8_t data);
	void chargen_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);

	void m15_main(address_map &map);
};

#endif // MAME_INCLUDES_M10_H
