/***************************************************************************

  Galaxian hardware family (old)

  This include file is used by the following drivers:
    - dambustr.c
    - galaxold.c
    - scramble.c
    - scobra.c

***************************************************************************/

#ifndef __GALAXOLD_H__
#define __GALAXOLD_H__

/* star circuit */
#define STAR_COUNT  252
struct star
{
	int x, y, color;
};

class galaxold_state : public driver_device
{
public:
	galaxold_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	UINT8 *m_spriteram2;
	UINT8 *m_attributesram;
	UINT8 *m_bulletsram;
	UINT8 *m_rockclim_videoram;
	UINT8 *m_racknrol_tiles_bank;
	size_t m_spriteram_size;
	size_t m_spriteram2_size;
	size_t m_bulletsram_size;
	int m_irq_line;
	UINT8 m__4in1_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_rockclim_tilemap;
	int m_mooncrst_gfxextend;
	int m_spriteram2_present;
	UINT8 m_gfxbank[5];
	UINT8 m_flipscreen_x;
	UINT8 m_flipscreen_y;
	UINT8 m_color_mask;
	tilemap_t *m_dambustr_tilemap2;
	UINT8 *m_dambustr_videoram2;
	void (*m_modify_charcode)(running_machine &machine, UINT16 *code, UINT8 x);		/* function to call to do character banking */
	void (*m_modify_spritecode)(running_machine &machine, UINT8 *spriteram, int*, int*, int*, int);	/* function to call to do sprite banking */
	void (*m_modify_color)(UINT8 *color);	/* function to call to do modify how the color codes map to the PROM */
	void (*m_modify_ypos)(UINT8*);	/* function to call to do modify how vertical positioning bits are connected */

	UINT8 m_timer_adjusted;
	UINT8 m_darkplnt_bullet_color;
	void (*m_draw_bullets)(running_machine &,bitmap_ind16 &,const rectangle &, int, int, int);	/* function to call to draw a bullet */

	UINT8 m_background_enable;
	UINT8 m_background_red;
	UINT8 m_background_green;
	UINT8 m_background_blue;
	void (*m_draw_background)(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect);	/* function to call to draw the background */
	UINT16 m_rockclim_v;
	UINT16 m_rockclim_h;
	int m_dambustr_bg_split_line;
	int m_dambustr_bg_color_1;
	int m_dambustr_bg_color_2;
	int m_dambustr_bg_priority;
	int m_dambustr_char_bank;
	bitmap_ind16 *m_dambustr_tmpbitmap;

	void (*m_draw_stars)(running_machine &machine, bitmap_ind16 &, const rectangle &);		/* function to call to draw the star layer */
	int m_stars_colors_start;
	INT32 m_stars_scrollpos;
	UINT8 m_stars_on;
	UINT8 m_stars_blink_state;
	emu_timer *m_stars_blink_timer;
	emu_timer *m_stars_scroll_timer;
	struct star m_stars[STAR_COUNT];

	UINT8 m_nmi_mask; /* Harem per-game specific */
	DECLARE_READ8_MEMBER(drivfrcg_port0_r);
	DECLARE_READ8_MEMBER(scramb2_protection_r);
	DECLARE_READ8_MEMBER(scramb2_port0_r);
	DECLARE_READ8_MEMBER(scramb2_port1_r);
	DECLARE_READ8_MEMBER(scramb2_port2_r);
	DECLARE_WRITE8_MEMBER(harem_nmi_mask_w);
	DECLARE_READ8_MEMBER(hexpoola_data_port_r);
	DECLARE_WRITE8_MEMBER(galaxold_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(galaxold_coin_lockout_w);
	DECLARE_WRITE8_MEMBER(galaxold_coin_counter_w);
	DECLARE_WRITE8_MEMBER(galaxold_coin_counter_1_w);
	DECLARE_WRITE8_MEMBER(galaxold_coin_counter_2_w);
	DECLARE_WRITE8_MEMBER(galaxold_leds_w);
	DECLARE_WRITE8_MEMBER(zigzag_sillyprotection_w);
	DECLARE_READ8_MEMBER(scramblb_protection_1_r);
	DECLARE_READ8_MEMBER(scramblb_protection_2_r);
	DECLARE_WRITE8_MEMBER(_4in1_bank_w);
	DECLARE_READ8_MEMBER(checkmaj_protection_r);
	DECLARE_READ8_MEMBER(dingo_3000_r);
	DECLARE_READ8_MEMBER(dingo_3035_r);
	DECLARE_READ8_MEMBER(dingoe_3001_r);
	DECLARE_WRITE8_MEMBER(racknrol_tiles_bank_w);
	DECLARE_WRITE8_MEMBER(galaxold_videoram_w);
	DECLARE_READ8_MEMBER(galaxold_videoram_r);
	DECLARE_WRITE8_MEMBER(galaxold_attributesram_w);
	DECLARE_WRITE8_MEMBER(galaxold_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(galaxold_flip_screen_y_w);
	DECLARE_WRITE8_MEMBER(gteikob2_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(gteikob2_flip_screen_y_w);
	DECLARE_WRITE8_MEMBER(hotshock_flip_screen_w);
	DECLARE_WRITE8_MEMBER(scrambold_background_enable_w);
	DECLARE_WRITE8_MEMBER(scrambold_background_red_w);
	DECLARE_WRITE8_MEMBER(scrambold_background_green_w);
	DECLARE_WRITE8_MEMBER(scrambold_background_blue_w);
	DECLARE_WRITE8_MEMBER(galaxold_stars_enable_w);
	DECLARE_WRITE8_MEMBER(darkplnt_bullet_color_w);
	DECLARE_WRITE8_MEMBER(galaxold_gfxbank_w);
	DECLARE_WRITE8_MEMBER(rockclim_videoram_w);
	DECLARE_WRITE8_MEMBER(rockclim_scroll_w);
	DECLARE_READ8_MEMBER(rockclim_videoram_r);
	DECLARE_WRITE8_MEMBER(dambustr_bg_split_line_w);
	DECLARE_WRITE8_MEMBER(dambustr_bg_color_w);
};


/*----------- defined in video/galaxold.c -----------*/

PALETTE_INIT( galaxold );
PALETTE_INIT( scrambold );
PALETTE_INIT( darkplnt );
PALETTE_INIT( rescue );
PALETTE_INIT( minefld );
PALETTE_INIT( stratgyx );
PALETTE_INIT( mariner );
PALETTE_INIT( rockclim );
PALETTE_INIT( dambustr );
PALETTE_INIT( turtles );

VIDEO_START( dambustr );





VIDEO_START( galaxold_plain );
VIDEO_START( galaxold );
VIDEO_START( mooncrst );
VIDEO_START( pisces );
VIDEO_START( batman2 );
VIDEO_START( dkongjrm );
VIDEO_START( scrambold );
VIDEO_START( darkplnt );
VIDEO_START( rescue );
VIDEO_START( minefld );
VIDEO_START( stratgyx );
VIDEO_START( mimonkey );
VIDEO_START( mariner );
VIDEO_START( ckongs );
VIDEO_START( rockclim );
VIDEO_START( drivfrcg );
VIDEO_START( bongo );
VIDEO_START( scorpion );
VIDEO_START( racknrol );
VIDEO_START( ad2083 );

void galaxold_init_stars(running_machine &machine, int colors_offset);
void galaxold_draw_stars(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect);

SCREEN_UPDATE_IND16( galaxold );
SCREEN_UPDATE_IND16( dambustr );



/*----------- defined in machine/galaxold.c -----------*/

TIMER_DEVICE_CALLBACK( galaxold_interrupt_timer );

WRITE_LINE_DEVICE_HANDLER( galaxold_7474_9m_2_q_callback );
WRITE_LINE_DEVICE_HANDLER( galaxold_7474_9m_1_callback );

DRIVER_INIT( 4in1 );
DRIVER_INIT( ladybugg );


MACHINE_RESET( galaxold );
MACHINE_RESET( devilfsg );
MACHINE_RESET( hunchbkg );

#define galaxold_coin_counter_0_w galaxold_coin_counter_w


CUSTOM_INPUT( _4in1_fake_port_r );

INTERRUPT_GEN( hunchbks_vh_interrupt );

#endif
