// license:BSD-3-Clause
// copyright-holders:Lee Taylor
/***************************************************************************

    Astro Fighter hardware

****************************************************************************/

#include "machine/timer.h"
#include "sound/samples.h"
#include "sound/sn76477.h"
#include "screen.h"

class astrof_state : public driver_device
{
public:
	astrof_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_astrof_color(*this, "astrof_color"),
		m_tomahawk_protection(*this, "tomahawk_prot"),
		m_fake_port(*this, "FAKE"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_sn(*this, "snsnd"),
		m_screen(*this, "screen") { }

	/* video-related */
	required_shared_ptr<uint8_t> m_videoram;

	std::unique_ptr<uint8_t[]>    m_colorram;
	required_shared_ptr<uint8_t> m_astrof_color;
	optional_shared_ptr<uint8_t> m_tomahawk_protection;
	optional_ioport m_fake_port;

	uint8_t      m_astrof_palette_bank;
	uint8_t      m_red_on;
	uint8_t      m_flipscreen;
	uint8_t      m_screen_off;
	uint16_t     m_abattle_count;

	/* sound-related */
	uint8_t      m_port_1_last;
	uint8_t      m_port_2_last;
	uint8_t      m_astrof_start_explosion;
	uint8_t      m_astrof_death_playing;
	uint8_t      m_astrof_bosskill_playing;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;  // astrof & abattle
	optional_device<sn76477_device> m_sn; // tomahawk
	required_device<screen_device> m_screen;
	DECLARE_READ8_MEMBER(irq_clear_r);
	DECLARE_WRITE8_MEMBER(astrof_videoram_w);
	DECLARE_WRITE8_MEMBER(tomahawk_videoram_w);
	DECLARE_WRITE8_MEMBER(video_control_1_w);
	DECLARE_WRITE8_MEMBER(astrof_video_control_2_w);
	DECLARE_WRITE8_MEMBER(spfghmk2_video_control_2_w);
	DECLARE_WRITE8_MEMBER(tomahawk_video_control_2_w);
	DECLARE_READ8_MEMBER(shoot_r);
	DECLARE_READ8_MEMBER(abattle_coin_prot_r);
	DECLARE_READ8_MEMBER(afire_coin_prot_r);
	DECLARE_READ8_MEMBER(tomahawk_protection_r);
	DECLARE_CUSTOM_INPUT_MEMBER(astrof_p1_controls_r);
	DECLARE_CUSTOM_INPUT_MEMBER(astrof_p2_controls_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tomahawk_controls_r);
	DECLARE_WRITE8_MEMBER(astrof_audio_1_w);
	DECLARE_WRITE8_MEMBER(astrof_audio_2_w);
	DECLARE_WRITE8_MEMBER(spfghmk2_audio_w);
	DECLARE_WRITE8_MEMBER(tomahawk_audio_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(service_coin_inserted);
	void init_afire();
	void init_abattle();
	void init_sstarbtl();
	void init_acombat3();
	virtual void video_start() override;
	DECLARE_MACHINE_START(astrof);
	DECLARE_MACHINE_START(abattle);
	DECLARE_MACHINE_RESET(abattle);
	DECLARE_MACHINE_START(spfghmk2);
	DECLARE_MACHINE_START(tomahawk);
	uint32_t screen_update_astrof(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tomahawk(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_callback);
	rgb_t make_pen( uint8_t data );
	void astrof_get_pens( pen_t *pens );
	void tomahawk_get_pens( pen_t *pens );
	void astrof_set_video_control_2( uint8_t data );
	void spfghmk2_set_video_control_2( uint8_t data );
	void tomahawk_set_video_control_2( uint8_t data );
	void video_update_common( bitmap_rgb32 &bitmap, const rectangle &cliprect, pen_t *pens, int num_pens );
	void base(machine_config &config);
	void astrof(machine_config &config);
	void abattle(machine_config &config);
	void spfghmk2(machine_config &config);
	void tomahawk(machine_config &config);
	void astrof_audio(machine_config &config);
	void spfghmk2_audio(machine_config &config);
	void tomahawk_audio(machine_config &config);
	void astrof_map(address_map &map);
	void spfghmk2_map(address_map &map);
	void tomahawk_map(address_map &map);
};
