/*************************************************************************

    Surprise Attack

*************************************************************************/

class surpratk_state : public driver_device
{
public:
	surpratk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ram(*this, "ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	/* misc */
	int        m_videobank;

	/* devices */
	device_t *m_maincpu;
	device_t *m_k052109;
	device_t *m_k053244;
	device_t *m_k053251;
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_WRITE8_MEMBER(surpratk_videobank_w);
	DECLARE_WRITE8_MEMBER(surpratk_5fc0_w);
};

/*----------- defined in video/surpratk.c -----------*/

extern void surpratk_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void surpratk_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);

SCREEN_UPDATE_IND16( surpratk );
