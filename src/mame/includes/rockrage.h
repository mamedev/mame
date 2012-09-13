/*************************************************************************

    Rock'n Rage

*************************************************************************/

class rockrage_state : public driver_device
{
public:
	rockrage_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_paletteram(*this, "paletteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_paletteram;

	/* video-related */
	int        m_layer_colorbase[2];
	int        m_vreg;

	/* devices */
	cpu_device *m_audiocpu;
	device_t *m_k007342;
	device_t *m_k007420;
	DECLARE_WRITE8_MEMBER(rockrage_bankswitch_w);
	DECLARE_WRITE8_MEMBER(rockrage_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(rockrage_vreg_w);
	DECLARE_READ8_MEMBER(rockrage_VLM5030_busy_r);
	DECLARE_WRITE8_MEMBER(rockrage_speech_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
};


/*----------- defined in video/rockrage.c -----------*/


SCREEN_UPDATE_IND16( rockrage );


void rockrage_tile_callback(running_machine &machine, int layer, int bank, int *code, int *color, int *flags);
void rockrage_sprite_callback(running_machine &machine, int *code, int *color);
