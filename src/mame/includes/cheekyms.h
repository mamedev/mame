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
	cpu_device *m_maincpu;
	dac_device *m_dac;

	UINT8          m_irq_mask;
	DECLARE_WRITE8_MEMBER(cheekyms_port_40_w);
	DECLARE_WRITE8_MEMBER(cheekyms_port_80_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	TILE_GET_INFO_MEMBER(cheekyms_get_tile_info);
	virtual void machine_start();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_cheekyms(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
};
