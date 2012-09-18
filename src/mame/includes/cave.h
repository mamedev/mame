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
		: driver_device(mconfig, type, tag),
		  m_videoregs(*this, "videoregs"),
		  m_vram(*this, "vram"),
		  m_vctrl(*this, "vctrl"),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram_2(*this, "spriteram_2"),
		  m_paletteram(*this, "paletteram"),
		  m_mirror_ram(*this, "mirror_ram"),
		  m_int_timer(*this, "int_timer") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_videoregs;
	optional_shared_ptr_array<UINT16, 4> m_vram;
	optional_shared_ptr_array<UINT16, 4> m_vctrl;
	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_spriteram_2;
	required_shared_ptr<UINT16> m_paletteram;

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
	optional_shared_ptr<UINT8> m_mirror_ram;
	// korokoro
	UINT16       m_leds[2];
	int          m_hopper;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
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
	DECLARE_CUSTOM_INPUT_MEMBER(korokoro_hopper_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tjumpman_hopper_r);
	DECLARE_WRITE16_MEMBER(cave_eeprom_msb_w);
	DECLARE_WRITE16_MEMBER(sailormn_eeprom_msb_w);
	DECLARE_WRITE16_MEMBER(hotdogst_eeprom_msb_w);
	DECLARE_WRITE16_MEMBER(cave_eeprom_lsb_w);
	DECLARE_WRITE16_MEMBER(metmqstr_eeprom_msb_w);
	DECLARE_WRITE16_MEMBER(korokoro_eeprom_msb_w);
	DECLARE_READ16_MEMBER(pwrinst2_eeprom_r);
	DECLARE_WRITE16_MEMBER(tjumpman_eeprom_lsb_w);
	DECLARE_DRIVER_INIT(uopoko);
	DECLARE_DRIVER_INIT(donpachi);
	DECLARE_DRIVER_INIT(mazinger);
	DECLARE_DRIVER_INIT(gaia);
	DECLARE_DRIVER_INIT(pwrinst2);
	DECLARE_DRIVER_INIT(ddonpach);
	DECLARE_DRIVER_INIT(agallet);
	DECLARE_DRIVER_INIT(hotdogst);
	DECLARE_DRIVER_INIT(tjumpman);
	DECLARE_DRIVER_INIT(korokoro);
	DECLARE_DRIVER_INIT(esprade);
	DECLARE_DRIVER_INIT(pwrinst2j);
	DECLARE_DRIVER_INIT(guwange);
	DECLARE_DRIVER_INIT(feversos);
	DECLARE_DRIVER_INIT(sailormn);
	DECLARE_DRIVER_INIT(dfeveron);
	DECLARE_DRIVER_INIT(metmqstr);
	TILE_GET_INFO_MEMBER(sailormn_get_tile_info_2);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);
	TILE_GET_INFO_MEMBER(get_tile_info_3);
	DECLARE_MACHINE_START(cave);
	DECLARE_MACHINE_RESET(cave);
	DECLARE_VIDEO_START(cave_2_layers);
	DECLARE_PALETTE_INIT(dfeveron);
	DECLARE_VIDEO_START(cave_3_layers);
	DECLARE_PALETTE_INIT(ddonpach);
	DECLARE_PALETTE_INIT(cave);
	DECLARE_VIDEO_START(cave_1_layer);
	DECLARE_PALETTE_INIT(korokoro);
	DECLARE_PALETTE_INIT(mazinger);
	DECLARE_VIDEO_START(cave_4_layers);
	DECLARE_PALETTE_INIT(pwrinst2);
	DECLARE_VIDEO_START(sailormn_3_layers);
	DECLARE_PALETTE_INIT(sailormn);
	UINT32 screen_update_cave(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cave_interrupt);
};

/*----------- defined in video/cave.c -----------*/
void cave_get_sprite_info(running_machine &machine);
void sailormn_tilebank_w(running_machine &machine, int bank);
