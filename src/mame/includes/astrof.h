// license:BSD-3-Clause
// copyright-holders:Lee Taylor
/***************************************************************************

    Astro Fighter hardware

****************************************************************************/

#include "sound/samples.h"
#include "sound/sn76477.h"

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
	uint8_t irq_clear_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void astrof_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tomahawk_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_control_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void astrof_video_control_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spfghmk2_video_control_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tomahawk_video_control_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t shoot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t abattle_coin_prot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t afire_coin_prot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tomahawk_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	ioport_value astrof_p1_controls_r(ioport_field &field, void *param);
	ioport_value astrof_p2_controls_r(ioport_field &field, void *param);
	ioport_value tomahawk_controls_r(ioport_field &field, void *param);
	void astrof_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void astrof_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spfghmk2_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tomahawk_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void service_coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void init_afire();
	void init_abattle();
	void init_sstarbtl();
	void init_acombat3();
	virtual void video_start() override;
	void machine_start_astrof();
	void machine_start_abattle();
	void machine_reset_abattle();
	void machine_start_spfghmk2();
	void machine_start_tomahawk();
	uint32_t screen_update_astrof(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tomahawk(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void irq_callback(timer_device &timer, void *ptr, int32_t param);
	rgb_t make_pen( uint8_t data );
	void astrof_get_pens( pen_t *pens );
	void tomahawk_get_pens( pen_t *pens );
	void astrof_set_video_control_2( uint8_t data );
	void spfghmk2_set_video_control_2( uint8_t data );
	void tomahawk_set_video_control_2( uint8_t data );
	void video_update_common( bitmap_rgb32 &bitmap, const rectangle &cliprect, pen_t *pens, int num_pens );
};

/*----------- defined in audio/astrof.c -----------*/

MACHINE_CONFIG_EXTERN( astrof_audio );

MACHINE_CONFIG_EXTERN( spfghmk2_audio );

MACHINE_CONFIG_EXTERN( tomahawk_audio );
