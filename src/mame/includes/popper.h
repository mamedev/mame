/***************************************************************************

    Popper

***************************************************************************/

class popper_state : public driver_device
{
public:
	popper_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ol_videoram(*this, "ol_videoram"),
		m_videoram(*this, "videoram"),
		m_ol_attribram(*this, "ol_attribram"),
		m_attribram(*this, "attribram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ol_videoram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_ol_attribram;
	required_shared_ptr<UINT8> m_attribram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t *m_p123_tilemap;
	tilemap_t *m_p0_tilemap;
	tilemap_t *m_ol_p123_tilemap;
	tilemap_t *m_ol_p0_tilemap;
	INT32 m_flipscreen;
	INT32 m_e002;
	INT32 m_gfx_bank;
	rectangle m_tilemap_clip;

	/* devices */
	device_t *m_audiocpu;

	UINT8 m_nmi_mask;
	DECLARE_READ8_MEMBER(popper_input_ports_r);
	DECLARE_READ8_MEMBER(popper_soundcpu_nmi_r);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(popper_ol_videoram_w);
	DECLARE_WRITE8_MEMBER(popper_videoram_w);
	DECLARE_WRITE8_MEMBER(popper_ol_attribram_w);
	DECLARE_WRITE8_MEMBER(popper_attribram_w);
	DECLARE_WRITE8_MEMBER(popper_flipscreen_w);
	DECLARE_WRITE8_MEMBER(popper_e002_w);
	DECLARE_WRITE8_MEMBER(popper_gfx_bank_w);
	TILE_GET_INFO_MEMBER(get_popper_p123_tile_info);
	TILE_GET_INFO_MEMBER(get_popper_p0_tile_info);
	TILE_GET_INFO_MEMBER(get_popper_ol_p123_tile_info);
	TILE_GET_INFO_MEMBER(get_popper_ol_p0_tile_info);
};


/*----------- defined in video/popper.c -----------*/


PALETTE_INIT( popper );
VIDEO_START( popper );
SCREEN_UPDATE_IND16( popper );

