/*************************************************************************

    Atari Subs hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define SUBS_SONAR1_EN			NODE_01
#define SUBS_SONAR2_EN			NODE_02
#define SUBS_LAUNCH_DATA		NODE_03
#define SUBS_CRASH_DATA			NODE_04
#define SUBS_CRASH_EN			NODE_05
#define SUBS_EXPLODE_EN			NODE_06
#define SUBS_NOISE_RESET		NODE_07


class subs_state : public driver_device
{
public:
	subs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_discrete(*this, "discrete"){ }

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_device<discrete_device> m_discrete;
	int m_steering_buf1;
	int m_steering_buf2;
	int m_steering_val1;
	int m_steering_val2;
	int m_last_val_1;
	int m_last_val_2;
	DECLARE_WRITE8_MEMBER(subs_steer_reset_w);
	DECLARE_READ8_MEMBER(subs_control_r);
	DECLARE_READ8_MEMBER(subs_coin_r);
	DECLARE_READ8_MEMBER(subs_options_r);
	DECLARE_WRITE8_MEMBER(subs_lamp1_w);
	DECLARE_WRITE8_MEMBER(subs_lamp2_w);
	DECLARE_WRITE8_MEMBER(subs_invert1_w);
	DECLARE_WRITE8_MEMBER(subs_invert2_w);
	virtual void machine_reset();
	virtual void palette_init();
	UINT32 screen_update_subs_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_subs_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(subs_interrupt);
	DECLARE_WRITE8_MEMBER(subs_sonar1_w);
	DECLARE_WRITE8_MEMBER(subs_sonar2_w);
	DECLARE_WRITE8_MEMBER(subs_crash_w);
	DECLARE_WRITE8_MEMBER(subs_explode_w);
	DECLARE_WRITE8_MEMBER(subs_noise_reset_w);
};

/*----------- defined in audio/subs.c -----------*/

DISCRETE_SOUND_EXTERN( subs );
