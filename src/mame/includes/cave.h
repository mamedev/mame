/***************************************************************************

    Cave hardware

***************************************************************************/

struct sprite_cave
{
	int priority, flags;

	const UINT8 *pen_data;	/* points to top left corner of tile data */
	int line_offset;

	pen_t base_pen;
	int tile_width, tile_height;
	int total_width, total_height;	/* in screen coordinates */
	int x, y, xcount0, ycount0;
	int zoomx_re, zoomy_re;
};

#define MAX_PRIORITY        4
#define MAX_SPRITE_NUM      0x400

class cave_state : public driver_device
{
public:
	cave_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag), m_int_timer(*this, "int_timer") { }

	/* memory pointers */
	UINT16 *     m_videoregs;
	UINT16 *     m_vram[4];
	UINT16 *     m_vctrl[4];
	UINT16 *     m_spriteram;
	UINT16 *     m_spriteram_2;
	UINT16 *     m_paletteram;
	size_t       m_spriteram_size;
	size_t       m_paletteram_size;

	/* video-related */
	struct sprite_cave *m_sprite;
	struct sprite_cave *m_sprite_table[MAX_PRIORITY][MAX_SPRITE_NUM + 1];

	struct
	{
		int    clip_left, clip_right, clip_top, clip_bottom;
		UINT8  *baseaddr;
		int    line_offset;
		UINT8  *baseaddr_zbuf;
		int    line_offset_zbuf;
	} m_blit;


	void (*m_get_sprite_info)(running_machine &machine);
	void (*m_sprite_draw)(running_machine &machine, int priority);

	tilemap_t    *m_tilemap[4];
	int          m_tiledim[4];
	int          m_old_tiledim[4];

	bitmap_ind16 m_sprite_zbuf;
	UINT16       m_sprite_zbuf_baseval;

	int          m_num_sprites;

	int          m_spriteram_bank;
	int          m_spriteram_bank_delay;

	UINT16       *m_palette_map;

	int          m_layers_offs_x;
	int          m_layers_offs_y;
	int          m_row_effect_offs_n;
	int          m_row_effect_offs_f;
	int          m_background_color;

	int          m_spritetype[2];
	int          m_kludge;


	/* misc */
	int          m_time_vblank_irq;
	UINT8        m_irq_level;
	UINT8        m_vblank_irq;
	UINT8        m_sound_irq;
	UINT8        m_unknown_irq;
	UINT8        m_agallet_vblank_irq;

	/* sound related */
	int          m_soundbuf_len;
	UINT8        m_soundbuf_data[32];
	//UINT8        m_sound_flag1;
	//UINT8        m_sound_flag2;

	/* eeprom-related */
	int          m_region_byte;

	/* game specific */
	// sailormn
	int          m_sailormn_tilebank;
	UINT8        *m_mirror_ram;
	// korokoro
	UINT16       m_leds[2];
	int          m_hopper;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	required_device<timer_device> m_int_timer;
	int m_rasflag;
	int m_old_rasflag;
	DECLARE_READ16_MEMBER(cave_irq_cause_r);
	DECLARE_READ8_MEMBER(soundflags_r);
	DECLARE_READ16_MEMBER(soundflags_ack_r);
	DECLARE_WRITE16_MEMBER(sound_cmd_w);
	DECLARE_READ8_MEMBER(soundlatch_lo_r);
	DECLARE_READ8_MEMBER(soundlatch_hi_r);
	DECLARE_READ16_MEMBER(soundlatch_ack_r);
	DECLARE_WRITE8_MEMBER(soundlatch_ack_w);
	DECLARE_WRITE16_MEMBER(gaia_coin_lsb_w);
	DECLARE_READ16_MEMBER(donpachi_videoregs_r);
	DECLARE_WRITE16_MEMBER(korokoro_leds_w);
	DECLARE_WRITE16_MEMBER(pwrinst2_vctrl_0_w);
	DECLARE_WRITE16_MEMBER(pwrinst2_vctrl_1_w);
	DECLARE_WRITE16_MEMBER(pwrinst2_vctrl_2_w);
	DECLARE_WRITE16_MEMBER(pwrinst2_vctrl_3_w);
	DECLARE_READ16_MEMBER(sailormn_input0_r);
	DECLARE_WRITE16_MEMBER(tjumpman_leds_w);
	DECLARE_WRITE16_MEMBER(pacslot_leds_w);
	DECLARE_WRITE8_MEMBER(hotdogst_rombank_w);
	DECLARE_WRITE8_MEMBER(hotdogst_okibank_w);
	DECLARE_WRITE8_MEMBER(mazinger_rombank_w);
	DECLARE_WRITE8_MEMBER(metmqstr_rombank_w);
	DECLARE_WRITE8_MEMBER(metmqstr_okibank0_w);
	DECLARE_WRITE8_MEMBER(metmqstr_okibank1_w);
	DECLARE_WRITE8_MEMBER(pwrinst2_rombank_w);
	DECLARE_READ8_MEMBER(mirror_ram_r);
	DECLARE_WRITE8_MEMBER(mirror_ram_w);
	DECLARE_WRITE8_MEMBER(sailormn_rombank_w);
	DECLARE_WRITE8_MEMBER(sailormn_okibank0_w);
	DECLARE_WRITE8_MEMBER(sailormn_okibank1_w);
	DECLARE_WRITE16_MEMBER(donpachi_videoregs_w);
	DECLARE_WRITE16_MEMBER(cave_vram_0_w);
	DECLARE_WRITE16_MEMBER(cave_vram_1_w);
	DECLARE_WRITE16_MEMBER(cave_vram_2_w);
	DECLARE_WRITE16_MEMBER(cave_vram_3_w);
	DECLARE_WRITE16_MEMBER(cave_vram_0_8x8_w);
	DECLARE_WRITE16_MEMBER(cave_vram_1_8x8_w);
	DECLARE_WRITE16_MEMBER(cave_vram_2_8x8_w);
	DECLARE_WRITE16_MEMBER(cave_vram_3_8x8_w);
};

/*----------- defined in video/cave.c -----------*/



PALETTE_INIT( cave );
PALETTE_INIT( ddonpach );
PALETTE_INIT( dfeveron );
PALETTE_INIT( mazinger );
PALETTE_INIT( sailormn );
PALETTE_INIT( pwrinst2 );
PALETTE_INIT( korokoro );

VIDEO_START( cave_1_layer );
VIDEO_START( cave_2_layers );
VIDEO_START( cave_3_layers );
VIDEO_START( cave_4_layers );

VIDEO_START( sailormn_3_layers );

SCREEN_UPDATE_IND16( cave );

void cave_get_sprite_info(running_machine &machine);
void sailormn_tilebank_w(running_machine &machine, int bank);
