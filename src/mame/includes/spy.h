// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    S.P.Y.

*************************************************************************/
#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/konami_helper.h"

class spy_state : public driver_device
{
public:
	spy_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ram;
	UINT8      m_pmcram[0x800];
	std::vector<UINT8> m_paletteram;

	/* misc */
	int        m_rambank;
	int        m_pmcbank;
	int        m_video_enable;
	int        m_old_3f90;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007232_device> m_k007232_1;
	required_device<k007232_device> m_k007232_2;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<palette_device> m_palette;
	DECLARE_READ8_MEMBER(spy_bankedram1_r);
	DECLARE_WRITE8_MEMBER(spy_bankedram1_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(spy_3f90_w);
	DECLARE_WRITE8_MEMBER(spy_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_spy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(spy_interrupt);
	void spy_collision(  );
	DECLARE_WRITE8_MEMBER(volume_callback0);
	DECLARE_WRITE8_MEMBER(volume_callback1);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
};
