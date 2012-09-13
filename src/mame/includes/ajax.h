
class ajax_state : public driver_device
{
public:
	ajax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_zoom_colorbase;
	UINT8      m_priority;

	/* misc */
	int        m_firq_enable;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	cpu_device *m_subcpu;
	device_t *m_k007232_1;
	device_t *m_k007232_2;
	device_t *m_k052109;
	device_t *m_k051960;
	device_t *m_k051316;
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_READ8_MEMBER(ajax_ls138_f10_r);
	DECLARE_WRITE8_MEMBER(ajax_ls138_f10_w);
	DECLARE_WRITE8_MEMBER(ajax_bankswitch_2_w);
	DECLARE_WRITE8_MEMBER(ajax_bankswitch_w);
	DECLARE_WRITE8_MEMBER(ajax_lamps_w);
	DECLARE_WRITE8_MEMBER(k007232_extvol_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};


/*----------- defined in machine/ajax.c -----------*/



INTERRUPT_GEN( ajax_interrupt );

/*----------- defined in video/ajax.c -----------*/


SCREEN_UPDATE_IND16( ajax );

extern void ajax_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void ajax_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);
extern void ajax_zoom_callback(running_machine &machine, int *code,int *color,int *flags);
