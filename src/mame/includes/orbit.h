// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Orbit hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define ORBIT_NOTE_FREQ       NODE_01
#define ORBIT_ANOTE1_AMP      NODE_02
#define ORBIT_ANOTE2_AMP      NODE_03
#define ORBIT_NOISE1_AMP      NODE_04
#define ORBIT_NOISE2_AMP      NODE_05
#define ORBIT_WARNING_EN      NODE_06
#define ORBIT_NOISE_EN        NODE_07

class orbit_state : public driver_device
{
public:
	orbit_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_playfield_ram;
	required_shared_ptr<UINT8> m_sprite_ram;

	required_device<discrete_device> m_discrete;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int        m_flip_screen;

	/* misc */
	UINT8      m_misc_flags;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(orbit_misc_w);
	DECLARE_WRITE8_MEMBER(orbit_playfield_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_orbit(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(orbit_interrupt);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_32v);
	DECLARE_WRITE8_MEMBER(orbit_note_w);
	DECLARE_WRITE8_MEMBER(orbit_note_amp_w);
	DECLARE_WRITE8_MEMBER(orbit_noise_amp_w);
	DECLARE_WRITE8_MEMBER(orbit_noise_rst_w);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void update_misc_flags(address_space &space, UINT8 val);
};
/*----------- defined in audio/orbit.c -----------*/

DISCRETE_SOUND_EXTERN( orbit );
