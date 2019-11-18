// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

/***************************************************************************

    Blockout

***************************************************************************/

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"

class blockout_state : public driver_device
{
public:
	blockout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram", 16),
		m_frontvideoram(*this, "frontvideoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u16> m_frontvideoram;

	/* video-related */
	bitmap_ind16 m_tmpbitmap;
	u16   m_color;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE_LINE_MEMBER(irq_handler);
	DECLARE_WRITE16_MEMBER(blockout_irq6_ack_w);
	DECLARE_WRITE16_MEMBER(blockout_irq5_ack_w);
	void frontcolor_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 videoram_r(offs_t offset);
	void videoram_w(offs_t offset, u8 data);
	void init_agress();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(blockout_scanline);
	static rgb_t blockout_xBGR_444(u32 raw);
	void blockout(machine_config &config);
	void agress(machine_config &config);
	void agress_map(address_map &map);
	void audio_map(address_map &map);
	void main_map(address_map &map);
};
