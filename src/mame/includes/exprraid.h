/*************************************************************************

    Express Raider

*************************************************************************/


class exprraid_state : public driver_device
{
public:
	exprraid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_main_ram(*this, "main_ram"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_main_ram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	int            m_bg_index[4];

	/* misc */
	//int          m_coin;    // used in the commented out INTERRUPT_GEN - can this be removed?

	/* devices */
	device_t *m_maincpu;
	device_t *m_slave;
	DECLARE_READ8_MEMBER(exprraid_protection_r);
	DECLARE_WRITE8_MEMBER(sound_cpu_command_w);
	DECLARE_READ8_MEMBER(vblank_r);
	DECLARE_WRITE8_MEMBER(exprraid_videoram_w);
	DECLARE_WRITE8_MEMBER(exprraid_colorram_w);
	DECLARE_WRITE8_MEMBER(exprraid_flipscreen_w);
	DECLARE_WRITE8_MEMBER(exprraid_bgselect_w);
	DECLARE_WRITE8_MEMBER(exprraid_scrollx_w);
	DECLARE_WRITE8_MEMBER(exprraid_scrolly_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_deco16);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_nmi);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	DECLARE_DRIVER_INIT(exprraid);
	DECLARE_DRIVER_INIT(wexpressb);
	DECLARE_DRIVER_INIT(wexpressb2);
	DECLARE_DRIVER_INIT(wexpressb3);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
};


/*----------- defined in video/exprraid.c -----------*/


extern VIDEO_START( exprraid );
extern SCREEN_UPDATE_IND16( exprraid );
