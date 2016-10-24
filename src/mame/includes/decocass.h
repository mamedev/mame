// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, David Haywood
#ifdef MAME_DEBUG
#define LOGLEVEL  5
#else
#define LOGLEVEL  0
#endif
#define LOG(n,x)  do { if (LOGLEVEL >= n) logerror x; } while (0)

#include "machine/decocass_tape.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "cpu/mcs48/mcs48.h"

#define T1PROM 1
#define T1DIRECT 2
#define T1LATCH 4
#define T1LATCHINV 8

class decocass_state : public driver_device
{
public:
	decocass_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_mcu(*this, "mcu"),
			m_watchdog(*this, "watchdog"),
			m_cassette(*this, "cassette"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_soundlatch(*this, "soundlatch"),
			m_soundlatch2(*this, "soundlatch2"),
			m_rambase(*this, "rambase"),
			m_charram(*this, "charram"),
			m_fgvideoram(*this, "fgvideoram"),
			m_colorram(*this, "colorram"),
			m_tileram(*this, "tileram"),
			m_objectram(*this, "objectram"),
			m_paletteram(*this, "paletteram")
	{
		m_type1_map = nullptr;
	}

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<upi41_cpu_device> m_mcu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<decocass_tape_device> m_cassette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_rambase;
	required_shared_ptr<uint8_t> m_charram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_colorram;
	uint8_t *   m_bgvideoram; /* shares bits D0-3 with tileram! */
	required_shared_ptr<uint8_t> m_tileram;
	required_shared_ptr<uint8_t> m_objectram;
	required_shared_ptr<uint8_t> m_paletteram;

	size_t    m_bgvideoram_size;

	/* video-related */
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_bg_tilemap_l;
	tilemap_t   *m_bg_tilemap_r;
	uint8_t     m_empty_tile[16*16];
	int32_t     m_watchdog_count;
	int32_t     m_watchdog_flip;
	int32_t     m_color_missiles;
	int32_t     m_color_center_bot;
	int32_t     m_mode_set;
	int32_t     m_back_h_shift;
	int32_t     m_back_vl_shift;
	int32_t     m_back_vr_shift;
	int32_t     m_part_h_shift;
	int32_t     m_part_v_shift;
	int32_t     m_center_h_shift_space;
	int32_t     m_center_v_shift;
	rectangle m_bg_tilemap_l_clip;
	rectangle m_bg_tilemap_r_clip;

	/* sound-related */
	uint8_t     m_sound_ack;  /* sound latches, ACK status bits and NMI timer */
	uint8_t     m_audio_nmi_enabled;
	uint8_t     m_audio_nmi_state;

	/* misc */
	int32_t     m_firsttime;
	uint8_t     m_latch1;
	uint8_t     m_decocass_reset;
	int32_t     m_de0091_enable;  /* DE-0091xx daughter board enable */
	uint8_t     m_quadrature_decoder[4];  /* four inputs from the quadrature decoder (H1, V1, H2, V2) */
	int       m_showmsg;        // for debugging purposes

	/* i8041 */
	uint8_t     m_i8041_p1;
	uint8_t     m_i8041_p2;
	int       m_i8041_p1_write_latch;
	int       m_i8041_p1_read_latch;
	int       m_i8041_p2_write_latch;
	int       m_i8041_p2_read_latch;

	/* dongles-related */
	read8_delegate    m_dongle_r;
	write8_delegate   m_dongle_w;

	/* dongle type #1 */
	uint32_t    m_type1_inmap;
	uint32_t    m_type1_outmap;

	/* dongle type #2: status of the latches */
	int32_t     m_type2_d2_latch; /* latched 8041-STATUS D2 value */
	int32_t     m_type2_xx_latch; /* latched value (D7-4 == 0xc0) ? 1 : 0 */
	int32_t     m_type2_promaddr; /* latched PROM address A0-A7 */

	/* dongle type #3: status and patches */
	int32_t     m_type3_ctrs;     /* 12 bit counter stage */
	int32_t     m_type3_d0_latch; /* latched 8041-D0 value */
	int32_t     m_type3_pal_19;       /* latched 1 for PAL input pin-19 */
	int32_t     m_type3_swap;

	/* dongle type #4: status */
	int32_t     m_type4_ctrs;     /* latched PROM address (E5x0 LSB, E5x1 MSB) */
	int32_t     m_type4_latch;        /* latched enable PROM (1100xxxx written to E5x1) */

	/* dongle type #5: status */
	int32_t     m_type5_latch;        /* latched enable PROM (1100xxxx written to E5x1) */

	/* DS Telejan */
	uint8_t     m_mux_data;

	void init_decocass();
	void init_decocrom();
	void init_cdsteljn();
	tilemap_memory_index fgvideoram_scan_cols(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index bgvideoram_scan_cols(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bg_l_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_r_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_decocass(palette_device &palette);
	void machine_reset_ctsttape();
	void machine_reset_cprogolfj();
	void machine_reset_cdsteljn();
	void machine_reset_cfishing();
	void machine_reset_chwy();
	void machine_reset_cterrani();
	void machine_reset_castfant();
	void machine_reset_csuperas();
	void machine_reset_clocknch();
	void machine_reset_cprogolf();
	void machine_reset_cluckypo();
	void machine_reset_ctisland();
	void machine_reset_cexplore();
	void machine_reset_cdiscon1();
	void machine_reset_ctornado();
	void machine_reset_cmissnx();
	void machine_reset_cptennis();
	void machine_reset_cbtime();
	void machine_reset_cburnrub();
	void machine_reset_cgraplop();
	void machine_reset_cgraplop2();
	void machine_reset_clapapa();
	void machine_reset_cskater();
	void machine_reset_cprobowl();
	void machine_reset_cnightst();
	void machine_reset_cpsoccer();
	void machine_reset_csdtenis();
	void machine_reset_czeroize();
	void machine_reset_cppicf();
	void machine_reset_cfghtice();
	void machine_reset_type4();
	void machine_reset_cbdash();
	void machine_reset_cflyball();
	void machine_reset_cmanhat();
	void machine_reset_cocean1a(); /* 10 */
	uint32_t screen_update_decocass(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void decocass_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t decocass_sound_command_main_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t decocass_sound_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t decocass_sound_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void decocass_sound_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t decocass_sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void decocass_sound_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t decocass_sound_nmi_enable_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t decocass_sound_data_ack_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void decocass_sound_data_ack_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_nmi_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_quadrature_decoder_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_adc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t decocass_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void decocass_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t decocass_e5xx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void decocass_e5xx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_de0091_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_e900_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);


	void i8041_p1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8041_p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8041_p2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8041_p2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void decocass_machine_state_save_init();

	void decocass_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_charram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_tileram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_objectram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void decocass_watchdog_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_watchdog_flip_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_color_missiles_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_mode_set_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_color_center_bot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_back_h_shift_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_back_vl_shift_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_back_vr_shift_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_part_h_shift_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_part_v_shift_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_center_h_shift_space_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_center_v_shift_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void decocass_video_state_save_init();

	void mirrorvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mirrorcolorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mirrorvideoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mirrorcolorram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t cdsteljn_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cdsteljn_mux_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decocass_audio_nmi_gen(timer_device &timer, void *ptr, int32_t param);
private:
	uint8_t decocass_type1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t decocass_type2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void decocass_type2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t decocass_type3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void decocass_type3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t decocass_type4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void decocass_type4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t decocass_type5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void decocass_type5_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t decocass_nodong_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint8_t* m_type1_map;
	void draw_edge(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, bool opaque);
	void draw_object(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_center(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mark_bg_tile_dirty(offs_t offset);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int color,
					int sprite_y_adjust, int sprite_y_adjust_flip_screen,
					uint8_t *sprite_ram, int interleave);

	void draw_missiles(bitmap_ind16 &bitmap, const rectangle &cliprect,
					int missile_y_adjust, int missile_y_adjust_flip_screen,
					uint8_t *missile_ram, int interleave);
	void decocass_fno( offs_t offset, uint8_t data );
};
