
#define PIXMAP_COLOR_BASE  (16 + 32)
#define BITMAPRAM_SIZE      0x6000


class dogfgt_state : public driver_device
{
public:
	dogfgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_bgvideoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_sharedram;
//  UINT8 *    m_paletteram;  // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	bitmap_ind16 m_pixbitmap;
	tilemap_t   *m_bg_tilemap;
	UINT8     *m_bitmapram;
	int       m_bm_plane;
	int       m_pixcolor;
	int       m_scroll[4];
	int       m_lastflip;
	int       m_lastpixcolor;

	/* sound-related */
	int       m_soundlatch;
	int       m_last_snd_ctrl;

	/* devices */
	device_t *m_subcpu;
	DECLARE_READ8_MEMBER(sharedram_r);
	DECLARE_WRITE8_MEMBER(sharedram_w);
	DECLARE_WRITE8_MEMBER(subirqtrigger_w);
	DECLARE_WRITE8_MEMBER(sub_irqack_w);
	DECLARE_WRITE8_MEMBER(dogfgt_soundlatch_w);
	DECLARE_WRITE8_MEMBER(dogfgt_soundcontrol_w);
	DECLARE_WRITE8_MEMBER(dogfgt_plane_select_w);
	DECLARE_READ8_MEMBER(dogfgt_bitmapram_r);
	DECLARE_WRITE8_MEMBER(internal_bitmapram_w);
	DECLARE_WRITE8_MEMBER(dogfgt_bitmapram_w);
	DECLARE_WRITE8_MEMBER(dogfgt_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(dogfgt_scroll_w);
	DECLARE_WRITE8_MEMBER(dogfgt_1800_w);
};


/*----------- defined in video/dogfgt.c -----------*/


PALETTE_INIT( dogfgt );
VIDEO_START( dogfgt );
SCREEN_UPDATE_IND16( dogfgt );
