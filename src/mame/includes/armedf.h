



class armedf_state : public driver_device
{
public:
	armedf_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  m_text_videoram;
	UINT16 *  m_bg_videoram;
	UINT16 *  m_fg_videoram;
	UINT16 *  m_legion_cmd;	// legion only!
//  UINT16 *  m_spriteram;    // currently this uses generic buffered_spriteram
//  UINT16 *  m_paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	tilemap_t  *m_tx_tilemap;
	UINT16   m_scroll_msb;
	UINT16   m_vreg;
	UINT16   m_fg_scrollx;
	UINT16   m_fg_scrolly;
	UINT16   m_bg_scrollx;
	UINT16   m_bg_scrolly;
	int      m_scroll_type;
	int      m_sprite_offy;
	int      m_mcu_mode;
	int      m_old_mcu_mode;
	int      m_waiting_msb;
	int      m_oldmode;
};


/*----------- defined in video/armedf.c -----------*/

SCREEN_UPDATE( armedf );
SCREEN_EOF( armedf );
VIDEO_START( armedf );

WRITE16_HANDLER( armedf_bg_videoram_w );
WRITE16_HANDLER( armedf_fg_videoram_w );
WRITE16_HANDLER( armedf_text_videoram_w );
WRITE16_HANDLER( terraf_fg_scrollx_w );
WRITE16_HANDLER( terraf_fg_scrolly_w );
WRITE16_HANDLER( terraf_fg_scroll_msb_arm_w );
WRITE16_HANDLER( armedf_fg_scrollx_w );
WRITE16_HANDLER( armedf_fg_scrolly_w );
WRITE16_HANDLER( armedf_bg_scrollx_w );
WRITE16_HANDLER( armedf_bg_scrolly_w );
WRITE16_HANDLER( armedf_mcu_cmd );
