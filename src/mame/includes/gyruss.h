/*************************************************************************

    Gyruss

*************************************************************************/

class gyruss_state : public driver_device
{
public:
	gyruss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_flipscreen(*this, "flipscreen"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_flipscreen;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_tilemap;

	/* devices */
	cpu_device *m_audiocpu;
	cpu_device *m_audiocpu_2;

	UINT8	   m_master_nmi_mask;
	UINT8      m_slave_irq_mask;
	DECLARE_WRITE8_MEMBER(gyruss_irq_clear_w);
	DECLARE_WRITE8_MEMBER(gyruss_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(gyruss_i8039_irq_w);
	DECLARE_WRITE8_MEMBER(master_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(slave_irq_mask_w);
	DECLARE_WRITE8_MEMBER(gyruss_spriteram_w);
	DECLARE_READ8_MEMBER(gyruss_scanline_r);
	DECLARE_READ8_MEMBER(gyruss_portA_r);
	DECLARE_WRITE8_MEMBER(gyruss_dac_w);
	DECLARE_WRITE8_MEMBER(gyruss_filter0_w);
	DECLARE_WRITE8_MEMBER(gyruss_filter1_w);
};


/*----------- defined in video/gyruss.c -----------*/


PALETTE_INIT( gyruss );
VIDEO_START( gyruss );
SCREEN_UPDATE_IND16( gyruss );
