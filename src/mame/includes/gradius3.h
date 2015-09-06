// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Gradius 3

*************************************************************************/
#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/konami_helper.h"

class gradius3_state : public driver_device
{
public:
	gradius3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_gfxram(*this, "k052109"),
		m_gfxrom(*this, "k051960"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_k007232(*this, "k007232"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_gfxram;
	required_region_ptr<UINT8> m_gfxrom;

	/* misc */
	int         m_priority;
	int         m_irqAen;
	int         m_irqBmask;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;

	DECLARE_READ16_MEMBER(k052109_halfword_r);
	DECLARE_WRITE16_MEMBER(k052109_halfword_w);
	DECLARE_READ16_MEMBER(k051937_halfword_r);
	DECLARE_WRITE16_MEMBER(k051937_halfword_w);
	DECLARE_READ16_MEMBER(k051960_halfword_r);
	DECLARE_WRITE16_MEMBER(k051960_halfword_w);
	DECLARE_WRITE16_MEMBER(cpuA_ctrl_w);
	DECLARE_WRITE16_MEMBER(cpuB_irqenable_w);
	DECLARE_WRITE16_MEMBER(cpuB_irqtrigger_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_READ16_MEMBER(gradius3_gfxrom_r);
	DECLARE_WRITE16_MEMBER(gradius3_gfxram_w);
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_gradius3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cpuA_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(gradius3_sub_scanline);
	void gradius3_postload();
	DECLARE_WRITE8_MEMBER(volume_callback);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
};
