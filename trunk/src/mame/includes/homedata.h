
class homedata_state : public driver_device
{
public:
	homedata_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_vreg;
	UINT8 *  m_videoram;

	/* video-related */
	tilemap_t *m_bg_tilemap[2][4];
	int      m_visible_page;
	int      m_priority;
	UINT8    m_reikaids_which;
	int      m_flipscreen;
	UINT8	   m_gfx_bank[2];	// pteacher only uses the first one
	UINT8	   m_blitter_bank;
	int      m_blitter_param_count;
	UINT8	   m_blitter_param[4];		/* buffers last 4 writes to 0x8006 */


	/* misc */
	int      m_vblank;
	int      m_sndbank;
	int      m_keyb;
	int      m_snd_command;
	int      m_upd7807_porta;
	int      m_upd7807_portc;
	int      m_to_cpu;
	int      m_from_cpu;

	/* device */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_dac;
	device_t *m_ym;
	device_t *m_sn;
	UINT8 m_prot_data;
};



/*----------- defined in video/homedata.c -----------*/

WRITE8_HANDLER( mrokumei_videoram_w );
WRITE8_HANDLER( reikaids_videoram_w );
WRITE8_HANDLER( reikaids_gfx_bank_w );
WRITE8_HANDLER( pteacher_gfx_bank_w );
WRITE8_HANDLER( homedata_blitter_param_w );
WRITE8_HANDLER( mrokumei_blitter_bank_w );
WRITE8_HANDLER( reikaids_blitter_bank_w );
WRITE8_HANDLER( pteacher_blitter_bank_w );
WRITE8_HANDLER( mrokumei_blitter_start_w );
WRITE8_HANDLER( reikaids_blitter_start_w );
WRITE8_HANDLER( pteacher_blitter_start_w );

PALETTE_INIT( mrokumei );
PALETTE_INIT( reikaids );
PALETTE_INIT( pteacher );
PALETTE_INIT( mirderby );

VIDEO_START( mrokumei );
VIDEO_START( reikaids );
VIDEO_START( pteacher );
VIDEO_START( lemnangl );
VIDEO_START( mirderby );
SCREEN_UPDATE( mrokumei );
SCREEN_UPDATE( reikaids );
SCREEN_UPDATE( pteacher );
SCREEN_UPDATE( mirderby );
SCREEN_EOF( homedata );
