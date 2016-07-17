// license:BSD-3-Clause
// copyright-holders:Manuel Abadia

#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/k051316.h"
#include "video/konami_helper.h"

class ajax_state : public driver_device
{
public:
	ajax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_watchdog(*this, "watchdog"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_k051316(*this, "k051316"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* video-related */
	UINT8      m_priority;

	/* misc */
	int        m_firq_enable;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<k007232_device> m_k007232_1;
	required_device<k007232_device> m_k007232_2;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<k051316_device> m_k051316;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_READ8_MEMBER(ls138_f10_r);
	DECLARE_WRITE8_MEMBER(ls138_f10_w);
	DECLARE_WRITE8_MEMBER(bankswitch_2_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(lamps_w);
	DECLARE_WRITE8_MEMBER(k007232_extvol_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(volume_callback0);
	DECLARE_WRITE8_MEMBER(volume_callback1);
	K051316_CB_MEMBER(zoom_callback);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
};
