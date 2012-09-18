class rocnrope_state : public driver_device
{
public:
	rocnrope_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram2(*this, "spriteram2"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	UINT8    m_irq_mask;
	DECLARE_WRITE8_MEMBER(rocnrope_interrupt_vector_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(rocnrope_videoram_w);
	DECLARE_WRITE8_MEMBER(rocnrope_colorram_w);
	DECLARE_WRITE8_MEMBER(rocnrope_flipscreen_w);
	DECLARE_DRIVER_INIT(rocnrope);
	DECLARE_DRIVER_INIT(rocnropk);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_rocnrope(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
};
