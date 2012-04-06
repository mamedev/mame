/*************************************************************************

    Asterix

*************************************************************************/

class asterix_state : public driver_device
{
public:
	asterix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         m_sprite_colorbase;
	int         m_layer_colorbase[4];
	int         m_layerpri[3];
	UINT16      m_spritebank;
	int         m_tilebanks[4];
	int         m_spritebanks[4];

	/* misc */
	UINT8       m_cur_control2;
	UINT16      m_prot[2];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k053260;
	device_t *m_k056832;
	device_t *m_k053244;
	device_t *m_k053251;
	DECLARE_READ16_MEMBER(control2_r);
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_WRITE8_MEMBER(sound_arm_nmi_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_WRITE16_MEMBER(protection_w);
	DECLARE_WRITE16_MEMBER(asterix_spritebank_w);
};



/*----------- defined in video/asterix.c -----------*/

SCREEN_UPDATE_IND16( asterix );

extern void asterix_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
extern void asterix_sprite_callback(running_machine &machine, int *code, int *color, int *priority_mask);
