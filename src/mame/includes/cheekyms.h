/*************************************************************************

    Cheeky Mouse

*************************************************************************/


class cheekyms_state : public driver_device
{
public:
	cheekyms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_port_80(*this, "port_80"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_port_80;

	/* video-related */
	tilemap_t        *m_cm_tilemap;
	bitmap_ind16       *m_bitmap_buffer;

	/* devices */
	device_t *m_maincpu;
	dac_device *m_dac;

	UINT8          m_irq_mask;
	DECLARE_WRITE8_MEMBER(cheekyms_port_40_w);
	DECLARE_WRITE8_MEMBER(cheekyms_port_80_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	TILE_GET_INFO_MEMBER(cheekyms_get_tile_info);
};


/*----------- defined in video/cheekyms.c -----------*/

PALETTE_INIT( cheekyms );
VIDEO_START( cheekyms );
SCREEN_UPDATE_IND16( cheekyms );
