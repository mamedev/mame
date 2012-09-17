#include "sound/samples.h"

#define TILEMAPS 0

class suna8_state : public driver_device
{
public:
	suna8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_hardhead_ip(*this, "hardhead_ip"),
		m_spriteram(*this, "spriteram"),
		m_wram(*this, "wram")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<UINT8> m_hardhead_ip;
	optional_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_wram;

	UINT8 m_rombank;
	UINT8 m_spritebank;
	UINT8 m_palettebank;
	UINT8 m_paletteram_enab;
	UINT8 m_prot2;
	UINT8 m_prot2_prev;

	UINT8 m_protection_val;
	UINT8 m_nmi_enable;
	UINT8 m_spritebank_latch;
	UINT8 m_trash_prot;


	int m_text_dim; /* specifies format of text layer */

#if TILEMAPS
	tilemap_t *m_bg_tilemap;
	int m_tiles;
	int m_trombank;
	int m_page;

	TILE_GET_INFO_MEMBER(get_tile_info);
#endif

	INT16 *m_samplebuf;
	int m_sample;

	DECLARE_READ8_MEMBER(hardhead_protection_r);
	DECLARE_WRITE8_MEMBER(hardhead_protection_w);
	DECLARE_READ8_MEMBER(hardhead_ip_r);
	DECLARE_WRITE8_MEMBER(hardhead_bankswitch_w);
	DECLARE_WRITE8_MEMBER(hardhead_flipscreen_w);
	DECLARE_WRITE8_MEMBER(rranger_bankswitch_w);
	DECLARE_READ8_MEMBER(rranger_soundstatus_r);
	DECLARE_WRITE8_MEMBER(sranger_prot_w);

	// brickzn
	DECLARE_READ8_MEMBER(brickzn_cheats_r);
	DECLARE_WRITE8_MEMBER(brickzn_multi_w);
	DECLARE_WRITE8_MEMBER(brickzn_prot_w);
	DECLARE_WRITE8_MEMBER(brickzn_prot2_w);
	DECLARE_WRITE8_MEMBER(brickzn_rombank_w);
	DECLARE_WRITE8_MEMBER(brickzn_enab_palram_w);
	DECLARE_WRITE8_MEMBER(brickzn_disab_palram_w);

	DECLARE_WRITE8_MEMBER(hardhea2_nmi_w);
	DECLARE_WRITE8_MEMBER(hardhea2_flipscreen_w);
	DECLARE_WRITE8_MEMBER(hardhea2_leds_w);
	DECLARE_WRITE8_MEMBER(hardhea2_spritebank_w);
	DECLARE_WRITE8_MEMBER(hardhea2_rombank_w);
	DECLARE_WRITE8_MEMBER(hardhea2_spritebank_0_w);
	DECLARE_WRITE8_MEMBER(hardhea2_spritebank_1_w);
	DECLARE_WRITE8_MEMBER(hardhea2_rambank_0_w);
	DECLARE_WRITE8_MEMBER(hardhea2_rambank_1_w);
	DECLARE_WRITE8_MEMBER(starfigh_spritebank_latch_w);
	DECLARE_WRITE8_MEMBER(starfigh_spritebank_w);
	DECLARE_WRITE8_MEMBER(sparkman_cmd_prot_w);
	DECLARE_WRITE8_MEMBER(suna8_wram_w);
	DECLARE_WRITE8_MEMBER(sparkman_flipscreen_w);
	DECLARE_WRITE8_MEMBER(sparkman_leds_w);
	DECLARE_WRITE8_MEMBER(sparkman_coin_counter_w);
	DECLARE_WRITE8_MEMBER(sparkman_spritebank_w);
	DECLARE_WRITE8_MEMBER(sparkman_rombank_w);
	DECLARE_READ8_MEMBER(sparkman_c0a3_r);
	DECLARE_WRITE8_MEMBER(sparkman_en_trash_w);
	DECLARE_WRITE8_MEMBER(brickzn_pcm_w);

	DECLARE_READ8_MEMBER(banked_paletteram_r);
	DECLARE_WRITE8_MEMBER( brickzn_banked_paletteram_w );
	DECLARE_READ8_MEMBER(suna8_banked_spriteram_r);
	DECLARE_WRITE8_MEMBER(suna8_spriteram_w);
	DECLARE_WRITE8_MEMBER(suna8_banked_spriteram_w);
	DECLARE_DRIVER_INIT(brickznv4);
	DECLARE_DRIVER_INIT(starfigh);
	DECLARE_DRIVER_INIT(hardhea2);
	DECLARE_DRIVER_INIT(hardhedb);
	DECLARE_DRIVER_INIT(sparkman);
	DECLARE_DRIVER_INIT(brickzn);
	DECLARE_DRIVER_INIT(hardhead);
	DECLARE_DRIVER_INIT(suna8);
	DECLARE_VIDEO_START(suna8_textdim12);
	DECLARE_VIDEO_START(suna8_textdim8);
	DECLARE_MACHINE_RESET(brickzn);
	DECLARE_VIDEO_START(suna8_textdim0);
	DECLARE_MACHINE_RESET(hardhea2);
	UINT32 screen_update_suna8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in audio/suna8.c -----------*/

DECLARE_WRITE8_DEVICE_HANDLER( suna8_play_samples_w );
DECLARE_WRITE8_DEVICE_HANDLER( rranger_play_samples_w );
DECLARE_WRITE8_DEVICE_HANDLER( suna8_samples_number_w );
SAMPLES_START( suna8_sh_start );


/*----------- defined in video/suna8.c -----------*/







