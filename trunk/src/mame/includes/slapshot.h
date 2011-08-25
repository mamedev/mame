/*************************************************************************

    Slapshot / Operation Wolf 3

*************************************************************************/

struct slapshot_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class slapshot_state : public driver_device
{
public:
	slapshot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_color_ram;
	UINT16 *    m_spriteram;
	UINT16 *    m_spriteext;
	UINT16 *    m_spriteram_buffered;
	UINT16 *    m_spriteram_delayed;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling
	size_t      m_spriteext_size;
	size_t      m_spriteram_size;

	/* video-related */
	struct      slapshot_tempsprite *m_spritelist;
	INT32       m_sprites_disabled;
	INT32       m_sprites_active_area;
	INT32       m_sprites_master_scrollx;
	INT32       m_sprites_master_scrolly;
	int         m_sprites_flipscreen;
	int         m_prepare_sprites;
	int         m_dislayer[5];

	UINT16      m_spritebank[8];

	/* misc */
	INT32      m_banknum;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_tc0140syt;
	device_t *m_tc0480scp;
	device_t *m_tc0360pri;
	device_t *m_tc0640fio;
};


/*----------- defined in video/slapshot.c -----------*/

VIDEO_START( slapshot );
SCREEN_UPDATE( slapshot );
SCREEN_EOF( taito_no_buffer );
