// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Aaron Giles
/*************************************************************************

    Atari Cloud 9 (prototype) hardware

*************************************************************************/

#include "cpu/m6502/m6502.h"
#include "machine/x2212.h"

class cloud9_state : public driver_device
{
public:
	cloud9_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_nvram(*this, "nvram") ,
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"){ }

	/* devices */
	required_device<m6502_device> m_maincpu;
	required_device<x2212_device> m_nvram;
	/* memory pointers */
	std::unique_ptr<UINT8[]>    m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	const UINT8 *m_syncprom;
	const UINT8 *m_wpprom;
	const UINT8 *m_priprom;
	bitmap_ind16 m_spritebitmap;
	double      m_rweights[3];
	double      m_gweights[3];
	double      m_bweights[3];
	UINT8       m_video_control[8];
	UINT8       m_bitmode_addr[2];

	/* misc */
	int         m_vblank_start;
	int         m_vblank_end;
	emu_timer   *m_irq_timer;
	UINT8       m_irq_state;

	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_WRITE8_MEMBER(cloud9_led_w);
	DECLARE_WRITE8_MEMBER(cloud9_coin_counter_w);
	DECLARE_READ8_MEMBER(leta_r);
	DECLARE_WRITE8_MEMBER(nvram_recall_w);
	DECLARE_WRITE8_MEMBER(nvram_store_w);
	DECLARE_WRITE8_MEMBER(cloud9_video_control_w);
	DECLARE_WRITE8_MEMBER(cloud9_paletteram_w);
	DECLARE_WRITE8_MEMBER(cloud9_videoram_w);
	DECLARE_READ8_MEMBER(cloud9_bitmode_r);
	DECLARE_WRITE8_MEMBER(cloud9_bitmode_w);
	DECLARE_WRITE8_MEMBER(cloud9_bitmode_addr_w);
	DECLARE_CUSTOM_INPUT_MEMBER(get_vblank);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_cloud9(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(clock_irq);
	inline void cloud9_write_vram( UINT16 addr, UINT8 data, UINT8 bitmd, UINT8 pixba );
	inline void bitmode_autoinc(  );
	inline void schedule_next_irq(int curscanline);
};
