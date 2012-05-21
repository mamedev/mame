/*************************************************************************

    Blades of Steel

*************************************************************************/

class bladestl_state : public driver_device
{
public:
	bladestl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_paletteram(*this, "paletteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_paletteram;

	/* video-related */
	int        m_spritebank;
	int        m_layer_colorbase[2];

	/* misc */
	int        m_last_track[4];

	/* devices */
	device_t *m_audiocpu;
	device_t *m_k007342;
	device_t *m_k007420;
	DECLARE_READ8_MEMBER(trackball_r);
	DECLARE_WRITE8_MEMBER(bladestl_bankswitch_w);
	DECLARE_WRITE8_MEMBER(bladestl_sh_irqtrigger_w);
};



/*----------- defined in video/bladestl.c -----------*/

PALETTE_INIT( bladestl );

SCREEN_UPDATE_IND16( bladestl );

void bladestl_tile_callback(running_machine &machine, int layer, int bank, int *code, int *color, int *flags);
void bladestl_sprite_callback(running_machine &machine, int *code, int *color);
