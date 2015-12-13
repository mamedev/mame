// license:???
// copyright-holders:Frank Palazzolo, Stefan Jokisch
/*
 *  The schematics don't seem to make a lot of sense when it
 *  comes to the video timing chain::
 *
 *    * there are clear typos -- what is H132???
 *    * there are two HBLANK/HSYNC periods as the horizontal
 *      chain is drawn, which would give an alternating long
 *      line followed by a much shorter one.  This cannot be right
 *    * the carry-out/load circuit on LS161@J8 is superflous
 *
 *  These values also give a frame rate of about 45Hz, which is
 *  probably too low.  I suspect that screen is not really
 *  512 pixels wide -- most likely 384, which would give 60Hz
 *
 *  Some of the graphics, like the starfield, is clocked with the
 *  12MHz signal, effecitvely doubling the horizontal resolution
 */

#include "sound/discrete.h"


#define STARSHP1_MASTER_CLOCK       (12096000)
#define STARSHP1_CPU_CLOCK          (STARSHP1_MASTER_CLOCK / 16)
#define STARSHP1_PIXEL_CLOCK        (STARSHP1_MASTER_CLOCK / 2)
#define STARSHP1_HTOTAL             (0x200)
#define STARSHP1_HBEND              (0x000)
#define STARSHP1_HBSTART            (0x200)
#define STARSHP1_VTOTAL             (0x106)
#define STARSHP1_VBEND              (0x000)
#define STARSHP1_VBSTART            (0x0f0)


class starshp1_state : public driver_device
{
public:
	starshp1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_hpos_ram(*this, "hpos_ram"),
		m_vpos_ram(*this, "vpos_ram"),
		m_obj_ram(*this, "obj_ram"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	int m_analog_in_select;
	int m_attract;
	required_shared_ptr<UINT8> m_playfield_ram;
	required_shared_ptr<UINT8> m_hpos_ram;
	required_shared_ptr<UINT8> m_vpos_ram;
	required_shared_ptr<UINT8> m_obj_ram;
	int m_ship_explode;
	int m_ship_picture;
	int m_ship_hoffset;
	int m_ship_voffset;
	int m_ship_size;
	int m_circle_hpos;
	int m_circle_vpos;
	int m_circle_size;
	int m_circle_mod;
	int m_circle_kill;
	int m_phasor;
	int m_collision_latch;
	int m_starfield_kill;
	int m_mux;
	int m_inverse;
	UINT16 *m_LSFR;
	bitmap_ind16 m_helper;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(starshp1_collision_reset_w);
	DECLARE_WRITE8_MEMBER(starshp1_analog_in_w);
	DECLARE_WRITE8_MEMBER(starshp1_misc_w);
	DECLARE_READ8_MEMBER(starshp1_rng_r);
	DECLARE_WRITE8_MEMBER(starshp1_ssadd_w);
	DECLARE_WRITE8_MEMBER(starshp1_sspic_w);
	DECLARE_WRITE8_MEMBER(starshp1_playfield_w);
	DECLARE_CUSTOM_INPUT_MEMBER(starshp1_analog_r);
	DECLARE_CUSTOM_INPUT_MEMBER(collision_latch_r);
	DECLARE_WRITE8_MEMBER(starshp1_audio_w);
	DECLARE_WRITE8_MEMBER(starshp1_analog_out_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(starshp1);
	UINT32 screen_update_starshp1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_starshp1(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(starshp1_interrupt);
	void set_pens();
	void draw_starfield(bitmap_ind16 &bitmap);
	int get_sprite_hpos(int i);
	int get_sprite_vpos(int i);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_spaceship(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_phasor(bitmap_ind16 &bitmap);
	int get_radius();
	int get_circle_hpos();
	int get_circle_vpos();
	void draw_circle_line(bitmap_ind16 &bitmap, int x, int y, int l);
	void draw_circle(bitmap_ind16 &bitmap);
	int spaceship_collision(bitmap_ind16 &bitmap, const rectangle &rect);
	int point_in_circle(int x, int y, int center_x, int center_y, int r);
	int circle_collision(const rectangle &rect);
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

/*----------- defined in audio/starshp1.c -----------*/

DISCRETE_SOUND_EXTERN( starshp1 );

/* Discrete Sound Input Nodes */
#define STARSHP1_NOISE_AMPLITUDE    NODE_01
#define STARSHP1_TONE_PITCH         NODE_02
#define STARSHP1_MOTOR_SPEED        NODE_03
#define STARSHP1_NOISE_FREQ         NODE_04
#define STARSHP1_MOLVL              NODE_05
#define STARSHP1_SL2                NODE_06
#define STARSHP1_SL1                NODE_07
#define STARSHP1_KICKER             NODE_08
#define STARSHP1_PHASOR_ON          NODE_09
#define STARSHP1_ATTRACT            NODE_10
