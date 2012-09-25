/*************************************************************************

    Chequered Flag

*************************************************************************/

class chqflag_state : public driver_device
{
public:
	chqflag_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_ram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_zoom_colorbase[2];
	int        m_sprite_colorbase;

	/* misc */
	int        m_k051316_readroms;
	int        m_last_vreg;
	int        m_analog_ctrl;
	int        m_accel;
	int        m_wheel;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_k007232_1;
	device_t *m_k007232_2;
	device_t *m_k051960;
	device_t *m_k051316_1;
	device_t *m_k051316_2;
	DECLARE_WRITE8_MEMBER(chqflag_bankswitch_w);
	DECLARE_WRITE8_MEMBER(chqflag_vreg_w);
	DECLARE_WRITE8_MEMBER(select_analog_ctrl_w);
	DECLARE_READ8_MEMBER(analog_read_r);
	DECLARE_WRITE8_MEMBER(chqflag_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(k007232_bankswitch_w);
	DECLARE_WRITE8_MEMBER(k007232_extvolume_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_chqflag(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(chqflag_scanline);
};

/*----------- defined in video/chqflag.c -----------*/

extern void chqflag_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);
extern void chqflag_zoom_callback_0(running_machine &machine, int *code,int *color,int *flags);
extern void chqflag_zoom_callback_1(running_machine &machine, int *code,int *color,int *flags);
