
class simpsons_state : public driver_device
{
public:
	simpsons_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_ram;
	UINT8 *    m_xtraram;
	UINT16 *   m_spriteram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_sprite_colorbase;
	int        m_layer_colorbase[3];
	int        m_layerpri[3];

	/* misc */
	int        m_firq_enabled;
	int        m_video_bank;
	//int        m_nmi_enabled;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k053260;
	device_t *m_k052109;
	device_t *m_k053246;
	device_t *m_k053251;
	DECLARE_WRITE8_MEMBER(z80_bankswitch_w);
	DECLARE_WRITE8_MEMBER(z80_arm_nmi_w);
	DECLARE_WRITE8_MEMBER(simpsons_eeprom_w);
	DECLARE_WRITE8_MEMBER(simpsons_coin_counter_w);
	DECLARE_READ8_MEMBER(simpsons_sound_interrupt_r);
	DECLARE_READ8_MEMBER(simpsons_k052109_r);
	DECLARE_WRITE8_MEMBER(simpsons_k052109_w);
	DECLARE_READ8_MEMBER(simpsons_k053247_r);
	DECLARE_WRITE8_MEMBER(simpsons_k053247_w);
};

/*----------- defined in machine/simpsons.c -----------*/

READ8_DEVICE_HANDLER( simpsons_sound_r );
MACHINE_RESET( simpsons );
MACHINE_START( simpsons );

/*----------- defined in video/simpsons.c -----------*/

void simpsons_video_banking( running_machine &machine, int select );
SCREEN_UPDATE_IND16( simpsons );

extern void simpsons_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void simpsons_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
