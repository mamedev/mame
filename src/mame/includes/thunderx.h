/*************************************************************************

    Super Contra / Thunder Cross

*************************************************************************/

class thunderx_state : public driver_device
{
public:
	thunderx_state(const machine_config &mconfig, device_type type, const char *tag)
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
	int        m_priority;
	UINT8      m_1f98_data;
	int        m_palette_selected;
	int        m_rambank;
	int        m_pmcbank;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_k007232;
	device_t *m_k052109;
	device_t *m_k051960;
	DECLARE_READ8_MEMBER(scontra_bankedram_r);
	DECLARE_WRITE8_MEMBER(scontra_bankedram_w);
	DECLARE_READ8_MEMBER(thunderx_bankedram_r);
	DECLARE_WRITE8_MEMBER(thunderx_bankedram_w);
	DECLARE_READ8_MEMBER(thunderx_1f98_r);
	DECLARE_WRITE8_MEMBER(thunderx_1f98_w);
	DECLARE_WRITE8_MEMBER(scontra_bankswitch_w);
	DECLARE_WRITE8_MEMBER(thunderx_videobank_w);
	DECLARE_WRITE8_MEMBER(thunderx_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
	DECLARE_WRITE8_MEMBER(scontra_snd_bankswitch_w);
	virtual void video_start();
	DECLARE_MACHINE_START(scontra);
	DECLARE_MACHINE_RESET(scontra);
	DECLARE_MACHINE_START(thunderx);
	DECLARE_MACHINE_RESET(thunderx);
};


/*----------- defined in video/thunderx.c -----------*/

extern void thunderx_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void thunderx_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask,int *shadow);


SCREEN_UPDATE_IND16( scontra );
