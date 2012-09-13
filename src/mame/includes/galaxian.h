/***************************************************************************

    Galaxian hardware family

***************************************************************************/

#include "machine/i8255.h"

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
		  m_ppi8255_0(*this, "ppi8255_0"),
		  m_ppi8255_1(*this, "ppi8255_1"),
		  m_ppi8255_2(*this, "ppi8255_2"),
		  m_spriteram(*this, "spriteram"),
		  m_videoram(*this, "videoram"){ }

	optional_device<i8255_device>  m_ppi8255_0;
	optional_device<i8255_device>  m_ppi8255_1;
	optional_device<i8255_device>  m_ppi8255_2;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;

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
	DECLARE_CUSTOM_INPUT_MEMBER(scramble_protection_alt_r);
	DECLARE_CUSTOM_INPUT_MEMBER(gmgalax_port_r);
	DECLARE_CUSTOM_INPUT_MEMBER(azurian_port_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kingball_muxbit_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kingball_noise_r);
	DECLARE_CUSTOM_INPUT_MEMBER(moonwar_dial_r);
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(start_lamp_w);
	DECLARE_WRITE8_MEMBER(coin_lock_w);
	DECLARE_WRITE8_MEMBER(coin_count_0_w);
	DECLARE_WRITE8_MEMBER(coin_count_1_w);
	DECLARE_READ8_MEMBER(konami_ay8910_r);
	DECLARE_WRITE8_MEMBER(konami_ay8910_w);
	DECLARE_WRITE8_MEMBER(konami_sound_filter_w);
	DECLARE_READ8_MEMBER(theend_ppi8255_r);
	DECLARE_WRITE8_MEMBER(theend_ppi8255_w);
	DECLARE_WRITE8_MEMBER(explorer_sound_control_w);
	DECLARE_READ8_MEMBER(sfx_sample_io_r);
	DECLARE_WRITE8_MEMBER(sfx_sample_io_w);
	DECLARE_READ8_MEMBER(monsterz_protection_r);
	DECLARE_READ8_MEMBER(frogger_ppi8255_r);
	DECLARE_WRITE8_MEMBER(frogger_ppi8255_w);
	DECLARE_READ8_MEMBER(frogger_ay8910_r);
	DECLARE_WRITE8_MEMBER(frogger_ay8910_w);
	DECLARE_WRITE8_MEMBER(froggrmc_sound_control_w);
	DECLARE_READ8_MEMBER(frogf_ppi8255_r);
	DECLARE_WRITE8_MEMBER(frogf_ppi8255_w);
	DECLARE_READ8_MEMBER(turtles_ppi8255_0_r);
	DECLARE_READ8_MEMBER(turtles_ppi8255_1_r);
	DECLARE_WRITE8_MEMBER(turtles_ppi8255_0_w);
	DECLARE_WRITE8_MEMBER(turtles_ppi8255_1_w);
	DECLARE_READ8_MEMBER(scorpion_ay8910_r);
	DECLARE_WRITE8_MEMBER(scorpion_ay8910_w);
	DECLARE_READ8_MEMBER(scorpion_digitalker_intr_r);
	DECLARE_WRITE8_MEMBER(zigzag_bankswap_w);
	DECLARE_WRITE8_MEMBER(zigzag_ay8910_w);
	DECLARE_WRITE8_MEMBER(kingball_speech_dip_w);
	DECLARE_WRITE8_MEMBER(kingball_sound1_w);
	DECLARE_WRITE8_MEMBER(kingball_sound2_w);
	DECLARE_WRITE8_MEMBER(mshuttle_ay8910_cs_w);
	DECLARE_WRITE8_MEMBER(mshuttle_ay8910_control_w);
	DECLARE_WRITE8_MEMBER(mshuttle_ay8910_data_w);
	DECLARE_READ8_MEMBER(mshuttle_ay8910_data_r);
	DECLARE_READ8_MEMBER(jumpbug_protection_r);
	DECLARE_WRITE8_MEMBER(checkman_sound_command_w);
	DECLARE_READ8_MEMBER(checkmaj_protection_r);
	DECLARE_READ8_MEMBER(dingo_3000_r);
	DECLARE_READ8_MEMBER(dingo_3035_r);
	DECLARE_READ8_MEMBER(dingoe_3001_r);
	DECLARE_WRITE8_MEMBER(tenspot_unk_6000_w);
	DECLARE_WRITE8_MEMBER(tenspot_unk_8000_w);
	DECLARE_WRITE8_MEMBER(tenspot_unk_e000_w);
	DECLARE_WRITE8_MEMBER(artic_gfxbank_w);
	DECLARE_READ8_MEMBER(tenspot_dsw_read);
	DECLARE_INPUT_CHANGED_MEMBER(gmgalax_game_changed);
	DECLARE_WRITE8_MEMBER(konami_sound_control_w);
	DECLARE_READ8_MEMBER(konami_sound_timer_r);
	DECLARE_WRITE8_MEMBER(konami_portc_0_w);
	DECLARE_WRITE8_MEMBER(konami_portc_1_w);
	DECLARE_WRITE8_MEMBER(theend_coin_counter_w);
	DECLARE_WRITE8_MEMBER(scramble_protection_w);
	DECLARE_READ8_MEMBER(scramble_protection_r);
	DECLARE_READ8_MEMBER(explorer_sound_latch_r);
	DECLARE_WRITE8_MEMBER(sfx_sample_control_w);
	DECLARE_WRITE8_MEMBER(monsterz_porta_1_w);
	DECLARE_WRITE8_MEMBER(monsterz_portb_1_w);
	DECLARE_WRITE8_MEMBER(monsterz_portc_1_w);
	DECLARE_READ8_MEMBER(frogger_sound_timer_r);
	DECLARE_READ8_MEMBER(scorpion_protection_r);
	DECLARE_WRITE8_MEMBER(scorpion_protection_w);
	DECLARE_WRITE8_MEMBER(scorpion_digitalker_control_w);
	DECLARE_WRITE8_MEMBER(kingball_dac_w);
	DECLARE_WRITE8_MEMBER(moonwar_port_select_w);
	DECLARE_DRIVER_INIT(galaxian);
	DECLARE_DRIVER_INIT(nolock);
	DECLARE_DRIVER_INIT(azurian);
	DECLARE_DRIVER_INIT(gmgalax);
	DECLARE_DRIVER_INIT(pisces);
	DECLARE_DRIVER_INIT(batman2);
	DECLARE_DRIVER_INIT(frogg);
	DECLARE_DRIVER_INIT(mooncrst);
	DECLARE_DRIVER_INIT(mooncrsu);
	DECLARE_DRIVER_INIT(mooncrgx);
	DECLARE_DRIVER_INIT(moonqsr);
	DECLARE_DRIVER_INIT(pacmanbl);
	DECLARE_DRIVER_INIT(tenspot);
	DECLARE_DRIVER_INIT(devilfsg);
	DECLARE_DRIVER_INIT(zigzag);
	DECLARE_DRIVER_INIT(jumpbug);
	DECLARE_DRIVER_INIT(checkman);
	DECLARE_DRIVER_INIT(checkmaj);
	DECLARE_DRIVER_INIT(dingo);
	DECLARE_DRIVER_INIT(dingoe);
	DECLARE_DRIVER_INIT(skybase);
	DECLARE_DRIVER_INIT(kong);
	DECLARE_DRIVER_INIT(mshuttle);
	DECLARE_DRIVER_INIT(mshuttlj);
	DECLARE_DRIVER_INIT(fantastc);
	DECLARE_DRIVER_INIT(kingball);
	DECLARE_DRIVER_INIT(scorpnmc);
	DECLARE_DRIVER_INIT(thepitm);
	DECLARE_DRIVER_INIT(theend);
	DECLARE_DRIVER_INIT(scramble);
	DECLARE_DRIVER_INIT(explorer);
	DECLARE_DRIVER_INIT(sfx);
	DECLARE_DRIVER_INIT(atlantis);
	DECLARE_DRIVER_INIT(scobra);
	DECLARE_DRIVER_INIT(losttomb);
	DECLARE_DRIVER_INIT(frogger);
	DECLARE_DRIVER_INIT(froggrmc);
	DECLARE_DRIVER_INIT(froggers);
	DECLARE_DRIVER_INIT(turtles);
	DECLARE_DRIVER_INIT(scorpion);
	DECLARE_DRIVER_INIT(anteater);
	DECLARE_DRIVER_INIT(anteateruk);
	DECLARE_DRIVER_INIT(superbon);
	DECLARE_DRIVER_INIT(calipso);
	DECLARE_DRIVER_INIT(moonwar);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	virtual void video_start();
	virtual void palette_init();
	DECLARE_PALETTE_INIT(moonwar);
	void tenspot_set_game_bank(running_machine& machine, int bank, int from_game);
};


/*----------- defined in video/galaxian.c -----------*/





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
