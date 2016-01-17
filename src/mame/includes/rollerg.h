// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Rollergames

*************************************************************************/
#include "machine/k053252.h"
#include "video/k051316.h"
#include "video/konami_helper.h"
#include "video/k053244_k053245.h"

class rollerg_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI
	};

	rollerg_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k053244(*this, "k053244"),
		m_k051316(*this, "k051316"),
		m_k053252(*this, "k053252")
		{ }

	/* misc */
	int        m_readzoomroms;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k05324x_device> m_k053244;
	required_device<k051316_device> m_k051316;
	required_device<k053252_device> m_k053252;
	DECLARE_WRITE8_MEMBER(rollerg_0010_w);
	DECLARE_READ8_MEMBER(rollerg_k051316_r);
	DECLARE_WRITE8_MEMBER(soundirq_w);
	DECLARE_WRITE8_MEMBER(sound_arm_nmi_w);
	DECLARE_READ8_MEMBER(pip_r);
	DECLARE_WRITE_LINE_MEMBER(rollerg_irq_ack_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_rollerg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	K05324X_CB_MEMBER(sprite_callback);
	K051316_CB_MEMBER(zoom_callback);
	DECLARE_WRITE8_MEMBER(banking_callback);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
