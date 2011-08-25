/*************************************************************************

    SNK/Alpha 68000 based games

*************************************************************************/

class alpha68k_state : public driver_device
{
public:
	alpha68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_videoram;
	UINT16 *    m_spriteram;
	UINT16 *    m_shared_ram;
	UINT16 *    m_paletteram;

	/* video-related */
	tilemap_t     *m_fix_tilemap;
	int         m_bank_base;
	int			m_flipscreen;
	int			m_last_bank;
	int         m_buffer_28;
	int			m_buffer_60;
	int			m_buffer_68;

	/* misc */
	int         m_invert_controls;
	int         m_microcontroller_id;
	int         m_coin_id;
	unsigned    m_trigstate;
	unsigned	m_deposits1;
	unsigned	m_deposits2;
	unsigned	m_credits;
	unsigned    m_coinvalue;
	unsigned    m_microcontroller_data;
	int         m_latch;
	unsigned    m_game_id;	// see below

	/* devices */
	device_t *m_audiocpu;
};

/* game_id - used to deal with a few game specific situations */
enum
{
	ALPHA68K_BTLFIELDB = 1,		// used in alpha_II_trigger_r
	ALPHA68K_JONGBOU,			// used in kyros_alpha_trigger_r & kyros_draw_sprites
	ALPHA68K_KYROS			// used in kyros_draw_sprites
};


/*----------- defined in video/alpha68k.c -----------*/

PALETTE_INIT( kyros );
PALETTE_INIT( paddlem );

VIDEO_START( alpha68k );

SCREEN_UPDATE( kyros );
SCREEN_UPDATE( sstingry );
SCREEN_UPDATE( alpha68k_I );
SCREEN_UPDATE( alpha68k_II );
SCREEN_UPDATE( alpha68k_V );
SCREEN_UPDATE( alpha68k_V_sb );

void alpha68k_V_video_bank_w(running_machine &machine, int bank);
void alpha68k_flipscreen_w(running_machine &machine, int flip);

WRITE16_HANDLER( alpha68k_paletteram_w );
WRITE16_HANDLER( alpha68k_videoram_w );
WRITE16_HANDLER( alpha68k_II_video_bank_w );
WRITE16_HANDLER( alpha68k_V_video_control_w );
