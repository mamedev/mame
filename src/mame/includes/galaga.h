// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/discrete.h"
#include "sound/namco.h"
#include "sound/samples.h"

class galaga_state : public driver_device
{
public:
	galaga_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_galaga_ram1(*this, "galaga_ram1"),
		m_galaga_ram2(*this, "galaga_ram2"),
		m_galaga_ram3(*this, "galaga_ram3"),
		m_galaga_starcontrol(*this, "starcontrol"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_namco_sound(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	optional_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_galaga_ram1;
	optional_shared_ptr<UINT8> m_galaga_ram2;
	optional_shared_ptr<UINT8> m_galaga_ram3;
	optional_shared_ptr<UINT8> m_galaga_starcontrol;    // 6 addresses
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	required_device<namco_device> m_namco_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	emu_timer *m_cpu3_interrupt_timer;
	UINT8 m_custom_mod;

	/* machine state */
	UINT32 m_stars_scrollx;
	UINT32 m_stars_scrolly;

	UINT32 m_galaga_gfxbank; // used by catsbee

	/* devices */

	/* bank support */

	/* shared */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	UINT8 m_sub2_nmi_mask;
	DECLARE_READ8_MEMBER(bosco_dsw_r);
	DECLARE_WRITE8_MEMBER(galaga_flip_screen_w);
	DECLARE_WRITE8_MEMBER(bosco_latch_w);
	DECLARE_WRITE8_MEMBER(galaga_videoram_w);
	DECLARE_WRITE8_MEMBER(gatsbee_bank_w);
	DECLARE_WRITE8_MEMBER(out_0);
	DECLARE_WRITE8_MEMBER(out_1);
	DECLARE_READ8_MEMBER(namco_52xx_rom_r);
	DECLARE_READ8_MEMBER(namco_52xx_si_r);
	DECLARE_READ8_MEMBER(custom_mod_r);
	DECLARE_DRIVER_INIT(galaga);
	DECLARE_DRIVER_INIT(gatsbee);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_MACHINE_START(galaga);
	DECLARE_MACHINE_RESET(galaga);
	DECLARE_VIDEO_START(galaga);
	DECLARE_PALETTE_INIT(galaga);
	UINT32 screen_update_galaga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_galaga(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(main_vblank_irq);
	INTERRUPT_GEN_MEMBER(sub_vblank_irq);
	TIMER_CALLBACK_MEMBER(cpu3_interrupt_callback);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void bosco_latch_reset();
	struct star
	{
		UINT16 x,y;
		UINT8 col,set;
	};

	static struct star m_star_seed_tab[];
};

class xevious_state : public galaga_state
{
public:
	xevious_state(const machine_config &mconfig, device_type type, std::string tag)
		: galaga_state(mconfig, type, tag),
		m_xevious_sr1(*this, "xevious_sr1"),
		m_xevious_sr2(*this, "xevious_sr2"),
		m_xevious_sr3(*this, "xevious_sr3"),
		m_xevious_fg_colorram(*this, "fg_colorram"),
		m_xevious_bg_colorram(*this, "bg_colorram"),
		m_xevious_fg_videoram(*this, "fg_videoram"),
		m_xevious_bg_videoram(*this, "bg_videoram"),
		m_samples(*this, "samples"),
		m_subcpu3(*this, "sub3") { }

	required_shared_ptr<UINT8> m_xevious_sr1;
	required_shared_ptr<UINT8> m_xevious_sr2;
	required_shared_ptr<UINT8> m_xevious_sr3;
	required_shared_ptr<UINT8> m_xevious_fg_colorram;
	required_shared_ptr<UINT8> m_xevious_bg_colorram;
	required_shared_ptr<UINT8> m_xevious_fg_videoram;
	required_shared_ptr<UINT8> m_xevious_bg_videoram;
	optional_device<samples_device> m_samples;

	INT32 m_xevious_bs[2];
	DECLARE_DRIVER_INIT(xevious);
	DECLARE_DRIVER_INIT(xevios);
	DECLARE_DRIVER_INIT(battles);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_VIDEO_START(xevious);
	DECLARE_PALETTE_INIT(xevious);
	DECLARE_MACHINE_RESET(xevios);
	DECLARE_PALETTE_INIT(battles);
	DECLARE_MACHINE_RESET(battles);
	UINT32 screen_update_xevious(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(battles_interrupt_4);
	TIMER_DEVICE_CALLBACK_MEMBER(battles_nmi_generate);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER( xevious_fg_videoram_w );
	DECLARE_WRITE8_MEMBER( xevious_fg_colorram_w );
	DECLARE_WRITE8_MEMBER( xevious_bg_videoram_w );
	DECLARE_WRITE8_MEMBER( xevious_bg_colorram_w );
	DECLARE_WRITE8_MEMBER( xevious_vh_latch_w );
	DECLARE_WRITE8_MEMBER( xevious_bs_w );
	DECLARE_READ8_MEMBER( xevious_bb_r );

	// Custom I/O
	void battles_customio_init();

	DECLARE_READ8_MEMBER( battles_customio0_r );
	DECLARE_READ8_MEMBER( battles_customio_data0_r );
	DECLARE_READ8_MEMBER( battles_customio3_r );
	DECLARE_READ8_MEMBER( battles_customio_data3_r );
	DECLARE_READ8_MEMBER( battles_input_port_r );

	DECLARE_WRITE8_MEMBER( battles_customio0_w );
	DECLARE_WRITE8_MEMBER( battles_customio_data0_w );
	DECLARE_WRITE8_MEMBER( battles_customio3_w );
	DECLARE_WRITE8_MEMBER( battles_customio_data3_w );
	DECLARE_WRITE8_MEMBER( battles_CPU4_coin_w );
	DECLARE_WRITE8_MEMBER( battles_noise_sound_w );

	UINT8 m_customio[16];
	char m_battles_customio_command;
	char m_battles_customio_prev_command;
	char m_battles_customio_command_count;
	char m_battles_customio_data;
	char m_battles_sound_played;

	optional_device<cpu_device> m_subcpu3;
};


class bosco_state : public galaga_state
{
public:
	bosco_state(const machine_config &mconfig, device_type type, std::string tag)
		: galaga_state(mconfig, type, tag),
			m_bosco_radarattr(*this, "bosco_radarattr"),
			m_bosco_starcontrol(*this, "starcontrol"),
			m_bosco_starblink(*this, "bosco_starblink") { }

	required_shared_ptr<UINT8> m_bosco_radarattr;

	required_shared_ptr<UINT8> m_bosco_starcontrol;
	required_shared_ptr<UINT8> m_bosco_starblink;

	UINT8 *m_bosco_radarx;
	UINT8 *m_bosco_radary;

	UINT8 *m_spriteram;
	UINT8 *m_spriteram2;
	UINT32 m_spriteram_size;
	DECLARE_WRITE8_MEMBER(bosco_flip_screen_w);
	TILEMAP_MAPPER_MEMBER(fg_tilemap_scan);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);
	DECLARE_VIDEO_START(bosco);
	DECLARE_PALETTE_INIT(bosco);
	UINT32 screen_update_bosco(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_bosco(screen_device &screen, bool state);

	inline void get_tile_info_bosco(tile_data &tileinfo,int tile_index,int ram_offs);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	DECLARE_WRITE8_MEMBER( bosco_videoram_w );
	DECLARE_WRITE8_MEMBER( bosco_scrollx_w );
	DECLARE_WRITE8_MEMBER( bosco_scrolly_w );
	DECLARE_WRITE8_MEMBER( bosco_starclr_w );
};

class digdug_state : public galaga_state
{
public:
	digdug_state(const machine_config &mconfig, device_type type, std::string tag)
		: galaga_state(mconfig, type, tag),
		m_digdug_objram(*this, "digdug_objram"),
		m_digdug_posram(*this, "digdug_posram"),
		m_digdug_flpram(*this, "digdug_flpram")     { }

	required_shared_ptr<UINT8> m_digdug_objram;
	required_shared_ptr<UINT8> m_digdug_posram;
	required_shared_ptr<UINT8> m_digdug_flpram;

	UINT8 m_bg_select;
	UINT8 m_tx_color_mode;
	UINT8 m_bg_disable;
	UINT8 m_bg_color_bank;
	DECLARE_CUSTOM_INPUT_MEMBER(shifted_port_r);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	DECLARE_VIDEO_START(digdug);
	DECLARE_PALETTE_INIT(digdug);
	UINT32 screen_update_digdug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER( digdug_videoram_w );
	DECLARE_WRITE8_MEMBER( digdug_PORT_w );
};

/*----------- defined in audio/galaga.c -----------*/

DISCRETE_SOUND_EXTERN( bosco );
DISCRETE_SOUND_EXTERN( galaga );
