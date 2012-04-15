/*************************************************************************

    S.P.Y.

*************************************************************************/

class spy_state : public driver_device
{
public:
	spy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ram(*this, "ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ram;
	UINT8      m_pmcram[0x800];
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;

	/* misc */
	int        m_rambank;
	int        m_pmcbank;
	int        m_video_enable;
	int        m_old_3f90;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k007232_1;
	device_t *m_k007232_2;
	device_t *m_k052109;
	device_t *m_k051960;
	DECLARE_READ8_MEMBER(spy_bankedram1_r);
	DECLARE_WRITE8_MEMBER(spy_bankedram1_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(spy_3f90_w);
	DECLARE_WRITE8_MEMBER(spy_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
};


/*----------- defined in video/spy.c -----------*/

extern void spy_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void spy_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask,int *shadow);

VIDEO_START( spy );
SCREEN_UPDATE_IND16( spy );
