// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#include "sound/sn76496.h"

class spcforce_state : public driver_device
{
public:
	spcforce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sn1(*this, "sn1"),
		m_sn2(*this, "sn2"),
		m_sn3(*this, "sn3"),
		m_scrollram(*this, "scrollram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<sn76496_device> m_sn1;
	required_device<sn76496_device> m_sn2;
	required_device<sn76496_device> m_sn3;

	required_shared_ptr<UINT8> m_scrollram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	int m_sn76496_latch;
	int m_sn76496_select;
	int m_sn1_ready;
	int m_sn2_ready;
	int m_sn3_ready;
	UINT8 m_irq_mask;

	DECLARE_WRITE8_MEMBER(SN76496_latch_w);
	DECLARE_READ8_MEMBER(SN76496_select_r);
	DECLARE_WRITE8_MEMBER(SN76496_select_w);
	DECLARE_WRITE_LINE_MEMBER(write_sn1_ready);
	DECLARE_WRITE_LINE_MEMBER(write_sn2_ready);
	DECLARE_WRITE_LINE_MEMBER(write_sn3_ready);
	DECLARE_READ8_MEMBER(t0_r);
	DECLARE_WRITE8_MEMBER(soundtrigger_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);

	virtual void machine_start() override;
	DECLARE_PALETTE_INIT(spcforce);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
};
