/***************************************************************************

    B-Wings

***************************************************************************/

#define BW_DEBUG 0

class bwing_state : public driver_device
{
public:
	bwing_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bwp1_sharedram1(*this, "bwp1_sharedram1"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_bwp2_sharedram1(*this, "bwp2_sharedram1"),
		m_bwp3_rombase(*this, "bwp3_rombase"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_bwp1_sharedram1;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_bwp2_sharedram1;
	required_shared_ptr<UINT8> m_bwp3_rombase;

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
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

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
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(tilt_pressed);
	DECLARE_DRIVER_INIT(bwing);
	TILE_GET_INFO_MEMBER(get_fgtileinfo);
	TILE_GET_INFO_MEMBER(get_bgtileinfo);
	TILE_GET_INFO_MEMBER(get_charinfo);
	TILEMAP_MAPPER_MEMBER(bwing_scan_cols);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_bwing(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(bwp3_interrupt);
	void fill_srxlat( int *xlat );
	void draw_sprites( bitmap_ind16 &bmp, const rectangle &clip, UINT8 *ram, int pri );
	void fix_bwp3(  );
};

/*----------- defined in video/bwing.c -----------*/
extern const gfx_layout bwing_tilelayout;
