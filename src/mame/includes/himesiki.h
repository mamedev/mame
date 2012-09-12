/*************************************************************************

    Himeshikibu

*************************************************************************/

class himesiki_state : public driver_device
{
public:
	himesiki_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg_ram(*this, "bg_ram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_bg_ram;
	required_shared_ptr<UINT8> m_spriteram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int 	     m_scrollx[2];
	int        m_flipscreen;

	/* devices */
	cpu_device *m_subcpu;
	DECLARE_WRITE8_MEMBER(himesiki_rombank_w);
	DECLARE_WRITE8_MEMBER(himesiki_sound_w);
	DECLARE_WRITE8_MEMBER(himesiki_bg_ram_w);
	DECLARE_WRITE8_MEMBER(himesiki_scrollx_w);
	DECLARE_WRITE8_MEMBER(himesiki_flip_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


/*----------- defined in video/himesiki.c -----------*/

VIDEO_START( himesiki );
SCREEN_UPDATE_IND16( himesiki );

