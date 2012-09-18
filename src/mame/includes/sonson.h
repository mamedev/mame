/*************************************************************************

    Son Son

*************************************************************************/

class sonson_state : public driver_device
{
public:
	sonson_state(const machine_config &mconfig, device_type type, const char *tag)
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

	/* misc */
	int        m_last_irq;

	/* devices */
	cpu_device *m_audiocpu;
	DECLARE_WRITE8_MEMBER(sonson_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(sonson_coin1_counter_w);
	DECLARE_WRITE8_MEMBER(sonson_coin2_counter_w);
	DECLARE_WRITE8_MEMBER(sonson_videoram_w);
	DECLARE_WRITE8_MEMBER(sonson_colorram_w);
	DECLARE_WRITE8_MEMBER(sonson_scrollx_w);
	DECLARE_WRITE8_MEMBER(sonson_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_sonson(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
