// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Ultraman

*************************************************************************/

#include "video/k051960.h"
#include "video/k051316.h"
#include "video/konami_helper.h"

class ultraman_state : public driver_device
{
public:
	ultraman_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k051316_1(*this, "k051316_1"),
		m_k051316_2(*this, "k051316_2"),
		m_k051316_3(*this, "k051316_3"),
		m_k051960(*this, "k051960") { }

	int        m_bank0;
	int        m_bank1;
	int        m_bank2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k051316_device> m_k051316_1;
	required_device<k051316_device> m_k051316_2;
	required_device<k051316_device> m_k051316_3;
	required_device<k051960_device> m_k051960;
	DECLARE_WRITE16_MEMBER(sound_cmd_w);
	DECLARE_WRITE16_MEMBER(sound_irq_trigger_w);
	DECLARE_WRITE16_MEMBER(ultraman_gfxctrl_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_ultraman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	K051316_CB_MEMBER(zoom_callback_1);
	K051316_CB_MEMBER(zoom_callback_2);
	K051316_CB_MEMBER(zoom_callback_3);
	K051960_CB_MEMBER(sprite_callback);
};
