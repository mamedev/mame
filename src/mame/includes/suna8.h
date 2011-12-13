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
};


/*----------- defined in audio/suna8.c -----------*/

WRITE8_DEVICE_HANDLER( suna8_play_samples_w );
WRITE8_DEVICE_HANDLER( rranger_play_samples_w );
WRITE8_DEVICE_HANDLER( suna8_samples_number_w );
SAMPLES_START( suna8_sh_start );


/*----------- defined in video/suna8.c -----------*/

WRITE8_HANDLER( suna8_spriteram_w );			// for debug
WRITE8_HANDLER( suna8_banked_spriteram_w );	// for debug

READ8_HANDLER( suna8_banked_paletteram_r );
READ8_HANDLER( suna8_banked_spriteram_r );

WRITE8_HANDLER( brickzn_banked_paletteram_w );

VIDEO_START( suna8_textdim0 );
VIDEO_START( suna8_textdim8 );
VIDEO_START( suna8_textdim12 );
SCREEN_UPDATE( suna8 );
