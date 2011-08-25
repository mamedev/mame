/*************************************************************************

    Karnov - Wonder Planet - Chelnov

*************************************************************************/

class karnov_state : public driver_device
{
public:
	karnov_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_videoram;
	UINT16 *    m_ram;
	UINT16 *    m_pf_data;
//  UINT16 *    m_spriteram;  // currently this uses generic buffered spriteram

	/* video-related */
	bitmap_t    *m_bitmap_f;
	tilemap_t     *m_fix_tilemap;
	int         m_flipscreen;
	UINT16      m_scroll[2];

	/* misc */
	UINT16      m_i8751_return;
	UINT16      m_i8751_needs_ack;
	UINT16      m_i8751_coin_pending;
	UINT16      m_i8751_command_queue;
	int         m_i8751_level;	// needed by chelnov
	int         m_microcontroller_id;
	int         m_coin_mask;
	int         m_latch;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};

enum {
	KARNOV = 0,
	KARNOVJ,
	CHELNOV,
	CHELNOVU,
	CHELNOVJ,
	WNDRPLNT
};


/*----------- defined in video/karnov.c -----------*/

WRITE16_HANDLER( karnov_playfield_swap_w );
WRITE16_HANDLER( karnov_videoram_w );

void karnov_flipscreen_w(running_machine &machine, int data);

PALETTE_INIT( karnov );
VIDEO_START( karnov );
VIDEO_START( wndrplnt );
SCREEN_UPDATE( karnov );
