// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, David Haywood
#ifdef MAME_DEBUG
#define LOGLEVEL  5
#else
#define LOGLEVEL  0
#endif
#define LOG(n,x)  do { if (LOGLEVEL >= n) logerror x; } while (0)

#include "machine/decocass_tape.h"
#include "cpu/mcs48/mcs48.h"

#define T1PROM 1
#define T1DIRECT 2
#define T1LATCH 4
#define T1LATCHINV 8

class decocass_state : public driver_device
{
public:
	decocass_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_mcu(*this, "mcu"),
			m_cassette(*this, "cassette"),
			m_rambase(*this, "rambase"),
			m_charram(*this, "charram"),
			m_fgvideoram(*this, "fgvideoram"),
			m_colorram(*this, "colorram"),
			m_tileram(*this, "tileram"),
			m_objectram(*this, "objectram"),
			m_paletteram(*this, "paletteram"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette")
	{
		m_type1_map = nullptr;
	}

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<upi41_cpu_device> m_mcu;
	required_device<decocass_tape_device> m_cassette;

	/* memory pointers */
	required_shared_ptr<UINT8> m_rambase;
	required_shared_ptr<UINT8> m_charram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_colorram;
	UINT8 *   m_bgvideoram; /* shares bits D0-3 with tileram! */
	required_shared_ptr<UINT8> m_tileram;
	required_shared_ptr<UINT8> m_objectram;
	required_shared_ptr<UINT8> m_paletteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	size_t    m_bgvideoram_size;

	/* video-related */
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_bg_tilemap_l;
	tilemap_t   *m_bg_tilemap_r;
	UINT8     m_empty_tile[16*16];
	INT32     m_watchdog_count;
	INT32     m_watchdog_flip;
	INT32     m_color_missiles;
	INT32     m_color_center_bot;
	INT32     m_mode_set;
	INT32     m_back_h_shift;
	INT32     m_back_vl_shift;
	INT32     m_back_vr_shift;
	INT32     m_part_h_shift;
	INT32     m_part_v_shift;
	INT32     m_center_h_shift_space;
	INT32     m_center_v_shift;
	rectangle m_bg_tilemap_l_clip;
	rectangle m_bg_tilemap_r_clip;

	/* sound-related */
	UINT8     m_sound_ack;  /* sound latches, ACK status bits and NMI timer */
	UINT8     m_audio_nmi_enabled;
	UINT8     m_audio_nmi_state;

	/* misc */
	INT32     m_firsttime;
	UINT8     m_latch1;
	UINT8     m_decocass_reset;
	INT32     m_de0091_enable;  /* DE-0091xx daughter board enable */
	UINT8     m_quadrature_decoder[4];  /* four inputs from the quadrature decoder (H1, V1, H2, V2) */
	int       m_showmsg;        // for debugging purposes

	/* i8041 */
	UINT8     m_i8041_p1;
	UINT8     m_i8041_p2;
	int       m_i8041_p1_write_latch;
	int       m_i8041_p1_read_latch;
	int       m_i8041_p2_write_latch;
	int       m_i8041_p2_read_latch;

	/* dongles-related */
	read8_delegate    m_dongle_r;
	write8_delegate   m_dongle_w;

	/* dongle type #1 */
	UINT32    m_type1_inmap;
	UINT32    m_type1_outmap;

	/* dongle type #2: status of the latches */
	INT32     m_type2_d2_latch; /* latched 8041-STATUS D2 value */
	INT32     m_type2_xx_latch; /* latched value (D7-4 == 0xc0) ? 1 : 0 */
	INT32     m_type2_promaddr; /* latched PROM address A0-A7 */

	/* dongle type #3: status and patches */
	INT32     m_type3_ctrs;     /* 12 bit counter stage */
	INT32     m_type3_d0_latch; /* latched 8041-D0 value */
	INT32     m_type3_pal_19;       /* latched 1 for PAL input pin-19 */
	INT32     m_type3_swap;

	/* dongle type #4: status */
	INT32     m_type4_ctrs;     /* latched PROM address (E5x0 LSB, E5x1 MSB) */
	INT32     m_type4_latch;        /* latched enable PROM (1100xxxx written to E5x1) */

	/* dongle type #5: status */
	INT32     m_type5_latch;        /* latched enable PROM (1100xxxx written to E5x1) */

	/* DS Telejan */
	UINT8     m_mux_data;

	DECLARE_DRIVER_INIT(decocass);
	DECLARE_DRIVER_INIT(decocrom);
	DECLARE_DRIVER_INIT(cdsteljn);
	TILEMAP_MAPPER_MEMBER(fgvideoram_scan_cols);
	TILEMAP_MAPPER_MEMBER(bgvideoram_scan_cols);
	TILE_GET_INFO_MEMBER(get_bg_l_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_r_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(decocass);
	DECLARE_MACHINE_RESET(ctsttape);
	DECLARE_MACHINE_RESET(cprogolfj);
	DECLARE_MACHINE_RESET(cdsteljn);
	DECLARE_MACHINE_RESET(cfishing);
	DECLARE_MACHINE_RESET(chwy);
	DECLARE_MACHINE_RESET(cterrani);
	DECLARE_MACHINE_RESET(castfant);
	DECLARE_MACHINE_RESET(csuperas);
	DECLARE_MACHINE_RESET(clocknch);
	DECLARE_MACHINE_RESET(cprogolf);
	DECLARE_MACHINE_RESET(cluckypo);
	DECLARE_MACHINE_RESET(ctisland);
	DECLARE_MACHINE_RESET(cexplore);
	DECLARE_MACHINE_RESET(cdiscon1);
	DECLARE_MACHINE_RESET(ctornado);
	DECLARE_MACHINE_RESET(cmissnx);
	DECLARE_MACHINE_RESET(cptennis);
	DECLARE_MACHINE_RESET(cbtime);
	DECLARE_MACHINE_RESET(cburnrub);
	DECLARE_MACHINE_RESET(cgraplop);
	DECLARE_MACHINE_RESET(cgraplop2);
	DECLARE_MACHINE_RESET(clapapa);
	DECLARE_MACHINE_RESET(cskater);
	DECLARE_MACHINE_RESET(cprobowl);
	DECLARE_MACHINE_RESET(cnightst);
	DECLARE_MACHINE_RESET(cpsoccer);
	DECLARE_MACHINE_RESET(csdtenis);
	DECLARE_MACHINE_RESET(czeroize);
	DECLARE_MACHINE_RESET(cppicf);
	DECLARE_MACHINE_RESET(cfghtice);
	DECLARE_MACHINE_RESET(type4);
	DECLARE_MACHINE_RESET(cbdash);
	DECLARE_MACHINE_RESET(cflyball);
	DECLARE_MACHINE_RESET(cmanhat);
	UINT32 screen_update_decocass(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(decocass_coin_counter_w);
	DECLARE_WRITE8_MEMBER(decocass_sound_command_w);
	DECLARE_READ8_MEMBER( decocass_sound_command_main_r );
	DECLARE_READ8_MEMBER(decocass_sound_data_r);
	DECLARE_READ8_MEMBER(decocass_sound_ack_r);
	DECLARE_WRITE8_MEMBER(decocass_sound_data_w);
	DECLARE_READ8_MEMBER(decocass_sound_command_r);
	DECLARE_WRITE8_MEMBER(decocass_sound_nmi_enable_w);
	DECLARE_READ8_MEMBER(decocass_sound_nmi_enable_r);
	DECLARE_READ8_MEMBER(decocass_sound_data_ack_reset_r);
	DECLARE_WRITE8_MEMBER(decocass_sound_data_ack_reset_w);
	DECLARE_WRITE8_MEMBER(decocass_nmi_reset_w);
	DECLARE_WRITE8_MEMBER(decocass_quadrature_decoder_reset_w);
	DECLARE_WRITE8_MEMBER(decocass_adc_w);
	DECLARE_READ8_MEMBER(decocass_input_r);

	DECLARE_WRITE8_MEMBER(decocass_reset_w);

	DECLARE_READ8_MEMBER(decocass_e5xx_r);
	DECLARE_WRITE8_MEMBER(decocass_e5xx_w);
	DECLARE_WRITE8_MEMBER(decocass_de0091_w);
	DECLARE_WRITE8_MEMBER(decocass_e900_w);


	DECLARE_WRITE8_MEMBER(i8041_p1_w);
	DECLARE_READ8_MEMBER(i8041_p1_r);
	DECLARE_WRITE8_MEMBER(i8041_p2_w);
	DECLARE_READ8_MEMBER(i8041_p2_r);

	void decocass_machine_state_save_init();

	DECLARE_WRITE8_MEMBER(decocass_paletteram_w);
	DECLARE_WRITE8_MEMBER(decocass_charram_w);
	DECLARE_WRITE8_MEMBER(decocass_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(decocass_colorram_w);
	DECLARE_WRITE8_MEMBER(decocass_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(decocass_tileram_w);
	DECLARE_WRITE8_MEMBER(decocass_objectram_w);

	DECLARE_WRITE8_MEMBER(decocass_watchdog_count_w);
	DECLARE_WRITE8_MEMBER(decocass_watchdog_flip_w);
	DECLARE_WRITE8_MEMBER(decocass_color_missiles_w);
	DECLARE_WRITE8_MEMBER(decocass_mode_set_w);
	DECLARE_WRITE8_MEMBER(decocass_color_center_bot_w);
	DECLARE_WRITE8_MEMBER(decocass_back_h_shift_w);
	DECLARE_WRITE8_MEMBER(decocass_back_vl_shift_w);
	DECLARE_WRITE8_MEMBER(decocass_back_vr_shift_w);
	DECLARE_WRITE8_MEMBER(decocass_part_h_shift_w);
	DECLARE_WRITE8_MEMBER(decocass_part_v_shift_w);
	DECLARE_WRITE8_MEMBER(decocass_center_h_shift_space_w);
	DECLARE_WRITE8_MEMBER(decocass_center_v_shift_w);

	void decocass_video_state_save_init();

	DECLARE_WRITE8_MEMBER(mirrorvideoram_w);
	DECLARE_WRITE8_MEMBER(mirrorcolorram_w);
	DECLARE_READ8_MEMBER(mirrorvideoram_r);
	DECLARE_READ8_MEMBER(mirrorcolorram_r);
	DECLARE_READ8_MEMBER(cdsteljn_input_r);
	DECLARE_WRITE8_MEMBER(cdsteljn_mux_w);
	TIMER_DEVICE_CALLBACK_MEMBER(decocass_audio_nmi_gen);
private:
	DECLARE_READ8_MEMBER(decocass_type1_r);
	DECLARE_READ8_MEMBER(decocass_type2_r);
	DECLARE_WRITE8_MEMBER(decocass_type2_w);
	DECLARE_READ8_MEMBER(decocass_type3_r);
	DECLARE_WRITE8_MEMBER(decocass_type3_w);
	DECLARE_READ8_MEMBER(decocass_type4_r);
	DECLARE_WRITE8_MEMBER(decocass_type4_w);
	DECLARE_READ8_MEMBER(decocass_type5_r);
	DECLARE_WRITE8_MEMBER(decocass_type5_w);
	DECLARE_READ8_MEMBER(decocass_nodong_r);

	UINT8* m_type1_map;
	void draw_edge(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, bool opaque);
	void draw_object(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_center(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mark_bg_tile_dirty(offs_t offset);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int color,
					int sprite_y_adjust, int sprite_y_adjust_flip_screen,
					UINT8 *sprite_ram, int interleave);

	void draw_missiles(bitmap_ind16 &bitmap, const rectangle &cliprect,
					int missile_y_adjust, int missile_y_adjust_flip_screen,
					UINT8 *missile_ram, int interleave);
	void decocass_fno( offs_t offset, UINT8 data );
};
