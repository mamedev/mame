/***************************************************************************

    B-Wings

***************************************************************************/

#define BW_DEBUG 0

class bwing_state : public driver_device
{
public:
	bwing_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_paletteram;
	UINT8 *    m_bwp1_sharedram1;
	UINT8 *    m_bwp2_sharedram1;
	UINT8 *    m_bwp3_rombase;
	size_t     m_bwp3_romsize;

	/* video-related */
	tilemap_t *m_charmap;
	tilemap_t *m_fgmap;
	tilemap_t *m_bgmap;
	UINT8 *m_srbase[4];
	UINT8 *m_fgdata;
	UINT8 *m_bgdata;
	int *m_srxlat;
	unsigned m_sreg[8];
	unsigned m_palatch;
	unsigned m_srbank;
	unsigned m_mapmask;
	unsigned m_mapflip;

	/* sound-related */
	int m_bwp3_nmimask;
	int m_bwp3_u8F_d;

	/* misc */
	UINT8 *m_bwp123_membase[3];

	/* device */
	device_t *m_maincpu;
	device_t *m_subcpu;
	device_t *m_audiocpu;
	DECLARE_WRITE8_MEMBER(bwp12_sharedram1_w);
	DECLARE_WRITE8_MEMBER(bwp3_u8F_w);
	DECLARE_WRITE8_MEMBER(bwp3_nmimask_w);
	DECLARE_WRITE8_MEMBER(bwp3_nmiack_w);
	DECLARE_READ8_MEMBER(bwp1_io_r);
	DECLARE_WRITE8_MEMBER(bwp1_ctrl_w);
	DECLARE_WRITE8_MEMBER(bwp2_ctrl_w);
	DECLARE_WRITE8_MEMBER(bwing_spriteram_w);
	DECLARE_WRITE8_MEMBER(bwing_videoram_w);
	DECLARE_READ8_MEMBER(bwing_scrollram_r);
	DECLARE_WRITE8_MEMBER(bwing_scrollram_w);
	DECLARE_WRITE8_MEMBER(bwing_scrollreg_w);
	DECLARE_WRITE8_MEMBER(bwing_paletteram_w);
};


/*----------- defined in video/bwing.c -----------*/

extern const gfx_layout bwing_tilelayout;


VIDEO_START( bwing );
SCREEN_UPDATE_IND16( bwing );
