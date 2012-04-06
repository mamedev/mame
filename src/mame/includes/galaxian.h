/***************************************************************************

    Galaxian hardware family

***************************************************************************/

/* we scale horizontally by 3 to render stars correctly */
#define GALAXIAN_XSCALE			3

/* master clocks */
#define GALAXIAN_MASTER_CLOCK	(18432000)
#define GALAXIAN_PIXEL_CLOCK	(GALAXIAN_XSCALE*GALAXIAN_MASTER_CLOCK/3)

/* H counts from 128->511, HBLANK starts at 130 and ends at 250 */
/* we normalize this here so that we count 0->383 with HBLANK */
/* from 264-383 */
#define GALAXIAN_HTOTAL			(384*GALAXIAN_XSCALE)
#define GALAXIAN_HBEND			(0*GALAXIAN_XSCALE)
//#define GALAXIAN_H0START      (6*GALAXIAN_XSCALE)
//#define GALAXIAN_HBSTART      (264*GALAXIAN_XSCALE)
#define GALAXIAN_H0START		(0*GALAXIAN_XSCALE)
#define GALAXIAN_HBSTART		(256*GALAXIAN_XSCALE)

#define GALAXIAN_VTOTAL			(264)
#define GALAXIAN_VBEND			(16)
#define GALAXIAN_VBSTART		(224+16)

/* video extension callbacks */
typedef void (*galaxian_extend_tile_info_func)(running_machine &machine, UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x);
typedef void (*galaxian_extend_sprite_info_func)(running_machine &machine, const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color);
typedef void (*galaxian_draw_bullet_func)(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
typedef void (*galaxian_draw_background_func)(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect);


class galaxian_state : public driver_device
{
public:
	galaxian_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	UINT8 *m_videoram;
	int m_bullets_base;
	int m_numspritegens;
	int m_counter_74ls161[2];
	int m_direction[2];
	UINT8 m_gmgalax_selected_game;
	UINT8 m_zigzag_ay8910_latch;
	UINT8 m_kingball_speech_dip;
	UINT8 m_kingball_sound;
	UINT8 m_mshuttle_ay8910_cs;
	UINT16 m_protection_state;
	UINT8 m_protection_result;
	UINT8 m_konami_sound_control;
	UINT8 m_sfx_sample_control;
	UINT8 m_moonwar_port_select;
	UINT8 m_irq_enabled;
	int m_irq_line;
	int m_tenspot_current_game;
	UINT8 m_frogger_adjust;
	UINT8 m_sfx_tilemap;
	galaxian_extend_tile_info_func m_extend_tile_info_ptr;
	galaxian_extend_sprite_info_func m_extend_sprite_info_ptr;
	galaxian_draw_bullet_func m_draw_bullet_ptr;
	galaxian_draw_background_func m_draw_background_ptr;
	tilemap_t *m_bg_tilemap;
	UINT8 m_flipscreen_x;
	UINT8 m_flipscreen_y;
	UINT8 m_background_enable;
	UINT8 m_background_red;
	UINT8 m_background_green;
	UINT8 m_background_blue;
	UINT32 m_star_rng_origin;
	UINT32 m_star_rng_origin_frame;
	rgb_t m_star_color[64];
	UINT8 *m_stars;
	UINT8 m_stars_enabled;
	UINT8 m_stars_blink_state;
	rgb_t m_bullet_color[8];
	UINT8 m_gfxbank[5];
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(galaxian_videoram_w);
	DECLARE_WRITE8_MEMBER(galaxian_objram_w);
	DECLARE_WRITE8_MEMBER(galaxian_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(galaxian_flip_screen_y_w);
	DECLARE_WRITE8_MEMBER(galaxian_flip_screen_xy_w);
	DECLARE_WRITE8_MEMBER(galaxian_stars_enable_w);
	DECLARE_WRITE8_MEMBER(scramble_background_enable_w);
	DECLARE_WRITE8_MEMBER(scramble_background_red_w);
	DECLARE_WRITE8_MEMBER(scramble_background_green_w);
	DECLARE_WRITE8_MEMBER(scramble_background_blue_w);
	DECLARE_WRITE8_MEMBER(galaxian_gfxbank_w);
};


/*----------- defined in video/galaxian.c -----------*/

PALETTE_INIT( galaxian );
PALETTE_INIT( moonwar );

VIDEO_START( galaxian );
SCREEN_UPDATE_RGB32( galaxian );





TIMER_DEVICE_CALLBACK( galaxian_stars_blink_timer );

/* special purpose background rendering */
void galaxian_draw_background(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect);
void frogger_draw_background(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect);
//void amidar_draw_background(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect);
void turtles_draw_background(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect);
void scramble_draw_background(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect);
void anteater_draw_background(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect);
void jumpbug_draw_background(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect);

/* special purpose bullet rendering */
void galaxian_draw_bullet(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
void mshuttle_draw_bullet(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
void scramble_draw_bullet(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
void theend_draw_bullet(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);

/* generic extensions */
void upper_extend_tile_info(running_machine &machine, UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x);
void upper_extend_sprite_info(running_machine &machine, const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color);

/* Frogger extensions */
void frogger_extend_tile_info(running_machine &machine, UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x);
void frogger_extend_sprite_info(running_machine &machine, const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color);

/* Ghostmuncher Galaxian extensions */
void gmgalax_extend_tile_info(running_machine &machine, UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x);
void gmgalax_extend_sprite_info(running_machine &machine, const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color);

/* Pisces extensions */
void pisces_extend_tile_info(running_machine &machine, UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x);
void pisces_extend_sprite_info(running_machine &machine, const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color);

/* Batman Part 2 extensions */
void batman2_extend_tile_info(running_machine &machine, UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x);

/* Moon Cresta extensions */
void mooncrst_extend_tile_info(running_machine &machine, UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x);
void mooncrst_extend_sprite_info(running_machine &machine, const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color);

/* Moon Quasar extensions */
void moonqsr_extend_tile_info(running_machine &machine, UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x);
void moonqsr_extend_sprite_info(running_machine &machine, const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color);

/* Moon Shuttle extensions */
void mshuttle_extend_tile_info(running_machine &machine, UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x);
void mshuttle_extend_sprite_info(running_machine &machine, const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color);

/* Calipso extensions */
void calipso_extend_sprite_info(running_machine &machine, const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color);

/* Jumpbug extensions */
void jumpbug_extend_tile_info(running_machine &machine, UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x);
void jumpbug_extend_sprite_info(running_machine &machine, const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color);

/*----------- defined in drivers/galaxian.c -----------*/

/* Ten Spot extensions */
void tenspot_set_game_bank(running_machine &machine, int bank, int from_game);

