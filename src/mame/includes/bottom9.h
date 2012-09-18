/*************************************************************************

    Bottom of the Ninth

*************************************************************************/

class bottom9_state : public driver_device
{
public:
	bottom9_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_zoom_colorbase;

	/* misc */
	int        m_video_enable;
	int        m_zoomreadroms;
	int        m_k052109_selected;
	int        m_nmienable;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_k007232_1;
	device_t *m_k007232_2;
	device_t *m_k052109;
	device_t *m_k051960;
	device_t *m_k051316;
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
	DECLARE_READ8_MEMBER(bottom9_bankedram1_r);
	DECLARE_WRITE8_MEMBER(bottom9_bankedram1_w);
	DECLARE_READ8_MEMBER(bottom9_bankedram2_r);
	DECLARE_WRITE8_MEMBER(bottom9_bankedram2_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(bottom9_1f90_w);
	DECLARE_WRITE8_MEMBER(bottom9_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_bottom9(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(bottom9_interrupt);
	INTERRUPT_GEN_MEMBER(bottom9_sound_interrupt);
};

/*----------- defined in video/bottom9.c -----------*/

extern void bottom9_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void bottom9_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);
extern void bottom9_zoom_callback(running_machine &machine, int *code,int *color,int *flags);
