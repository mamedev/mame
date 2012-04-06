#include "sound/samples.h"

#define TILEMAPS 0

class suna8_state : public driver_device
{
public:
	suna8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT8 m_rombank;
	UINT8 m_spritebank;
	UINT8 m_palettebank;
	UINT8 m_unknown;

	UINT8 m_protection_val;
	UINT8 m_nmi_enable;
	UINT8 m_spritebank_latch;
	UINT8 m_trash_prot;

	UINT8 *m_hardhead_ip;
	UINT8 *m_wram;
	UINT8 *m_spriteram;

	int m_text_dim; /* specifies format of text layer */

#if TILEMAPS
	tilemap_t *m_bg_tilemap;
	int m_tiles;
	int m_trombank;
	int m_page;
#endif

	INT16 *m_samplebuf;
	int m_sample;

	required_device<cpu_device> m_maincpu;
	DECLARE_READ8_MEMBER(hardhead_protection_r);
	DECLARE_WRITE8_MEMBER(hardhead_protection_w);
	DECLARE_READ8_MEMBER(hardhead_ip_r);
	DECLARE_WRITE8_MEMBER(hardhead_bankswitch_w);
	DECLARE_WRITE8_MEMBER(hardhead_flipscreen_w);
	DECLARE_WRITE8_MEMBER(rranger_bankswitch_w);
	DECLARE_READ8_MEMBER(rranger_soundstatus_r);
	DECLARE_WRITE8_MEMBER(sranger_prot_w);
	DECLARE_READ8_MEMBER(brickzn_c140_r);
	DECLARE_WRITE8_MEMBER(brickzn_palettebank_w);
	DECLARE_WRITE8_MEMBER(brickzn_spritebank_w);
	DECLARE_WRITE8_MEMBER(brickzn_unknown_w);
	DECLARE_WRITE8_MEMBER(brickzn_rombank_w);
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
};


/*----------- defined in audio/suna8.c -----------*/

WRITE8_DEVICE_HANDLER( suna8_play_samples_w );
WRITE8_DEVICE_HANDLER( rranger_play_samples_w );
WRITE8_DEVICE_HANDLER( suna8_samples_number_w );
SAMPLES_START( suna8_sh_start );


/*----------- defined in video/suna8.c -----------*/



VIDEO_START( suna8_textdim0 );
VIDEO_START( suna8_textdim8 );
VIDEO_START( suna8_textdim12 );
SCREEN_UPDATE_IND16( suna8 );
