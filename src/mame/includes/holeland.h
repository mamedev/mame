/*************************************************************************

    Hole Land

*************************************************************************/

class holeland_state : public driver_device
{
public:
	holeland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_palette_offset;
	int        m_po[2];
	DECLARE_WRITE8_MEMBER(holeland_videoram_w);
	DECLARE_WRITE8_MEMBER(holeland_colorram_w);
	DECLARE_WRITE8_MEMBER(holeland_pal_offs_w);
	DECLARE_WRITE8_MEMBER(holeland_scroll_w);
	DECLARE_WRITE8_MEMBER(holeland_flipscreen_w);
	TILE_GET_INFO_MEMBER(holeland_get_tile_info);
	TILE_GET_INFO_MEMBER(crzrally_get_tile_info);
	DECLARE_VIDEO_START(holeland);
	DECLARE_VIDEO_START(crzrally);
};


/*----------- defined in video/holeland.c -----------*/



SCREEN_UPDATE_IND16( holeland );
SCREEN_UPDATE_IND16( crzrally );

