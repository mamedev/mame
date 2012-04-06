/*************************************************************************

    Gyruss

*************************************************************************/

class gyruss_state : public driver_device
{
public:
	gyruss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	UINT8 *    m_flipscreen;

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
};


/*----------- defined in video/gyruss.c -----------*/


PALETTE_INIT( gyruss );
VIDEO_START( gyruss );
SCREEN_UPDATE_IND16( gyruss );
