// license:BSD-3-Clause
// copyright-holders:Lee Taylor, Couriersud
/***************************************************************************

    IREM M-10,M-11 and M-15 based hardware

****************************************************************************/
#ifndef MAME_INCLUDES_M10_H
#define MAME_INCLUDES_M10_H

#pragma once

#include "sound/samples.h"
#include "machine/74123.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#define IREMM10_MASTER_CLOCK        (12500000)

#define IREMM10_CPU_CLOCK       (IREMM10_MASTER_CLOCK/16)
#define IREMM10_PIXEL_CLOCK     (IREMM10_MASTER_CLOCK/2)
#define IREMM10_HTOTAL          (360)   /* (0x100-0xd3)*8 */
#define IREMM10_HBSTART         (248)
#define IREMM10_HBEND           (8)
#define IREMM10_VTOTAL          (281)   /* (0x200-0xe7) */
#define IREMM10_VBSTART         (240)
#define IREMM10_VBEND           (16)

#define IREMM15_MASTER_CLOCK    (11730000)

#define IREMM15_CPU_CLOCK       (IREMM15_MASTER_CLOCK/10)
#define IREMM15_PIXEL_CLOCK     (IREMM15_MASTER_CLOCK/2)
#define IREMM15_HTOTAL          (372)
#define IREMM15_HBSTART         (256)
#define IREMM15_HBEND           (0)
#define IREMM15_VTOTAL          (262)
#define IREMM15_VBSTART         (240)
#define IREMM15_VBEND           (16)

class m10_state : public driver_device
{
public:
	m10_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_memory(*this, "memory"),
		m_rom(*this, "rom"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_chargen(*this, "chargen"),
		m_maincpu(*this, "maincpu"),
		m_ic8j1(*this, "ic8j1"),
		m_ic8j2(*this, "ic8j2"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void m15(machine_config &config);
	void headoni(machine_config &config);
	void m10(machine_config &config);
	void m11(machine_config &config);

	void init_andromed();
	void init_ipminva1();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	enum
	{
		TIMER_INTERRUPT
	};

	/* memory pointers */
	required_shared_ptr<uint8_t> m_memory;
	required_shared_ptr<uint8_t> m_rom;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_chargen;

	/* video-related */
	tilemap_t *         m_tx_tilemap;
	gfx_element *       m_back_gfx;

	/* this is currently unused, because it is needed by gfx_layout (which has no machine) */
	uint32_t              extyoffs[32 * 8];

	/* video state */
	uint8_t               m_bottomline;
	uint8_t               m_flip;

	/* misc */
	int                 m_last;
	emu_timer *m_interrupt_timer;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<ttl74123_device> m_ic8j1;
	optional_device<ttl74123_device> m_ic8j2;
	required_device<samples_device> m_samples;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(m10_ctrl_w);
	DECLARE_WRITE8_MEMBER(m11_ctrl_w);
	DECLARE_WRITE8_MEMBER(m15_ctrl_w);
	DECLARE_WRITE8_MEMBER(m10_a500_w);
	DECLARE_WRITE8_MEMBER(m11_a100_w);
	DECLARE_WRITE8_MEMBER(m15_a100_w);
	DECLARE_READ8_MEMBER(m10_a700_r);
	DECLARE_READ8_MEMBER(m11_a700_r);
	DECLARE_WRITE8_MEMBER(m10_colorram_w);
	DECLARE_WRITE8_MEMBER(m10_chargen_w);
	DECLARE_WRITE8_MEMBER(m15_chargen_w);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_MACHINE_START(m10);
	DECLARE_MACHINE_RESET(m10);
	DECLARE_VIDEO_START(m10);
	void m10_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(m15);
	uint32_t screen_update_m10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_m15(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(m15_interrupt);
	TIMER_CALLBACK_MEMBER(interrupt_callback);
	DECLARE_WRITE_LINE_MEMBER(ic8j1_output_changed);
	DECLARE_WRITE_LINE_MEMBER(ic8j2_output_changed);
	inline void plot_pixel_m10( bitmap_ind16 &bm, int x, int y, int col );

	void m10_main(address_map &map);
	void m11_main(address_map &map);
	void m15_main(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_M10_H
