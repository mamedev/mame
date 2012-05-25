/*************************************************************************

    Wild West C.O.W.boys of Moo Mesa / Bucky O'Hare

*************************************************************************/

class moo_state : public driver_device
{
public:
	moo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_workram(*this, "workram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	optional_shared_ptr<UINT16> m_workram;
	required_shared_ptr<UINT16> m_spriteram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         m_sprite_colorbase;
	int         m_layer_colorbase[4];
	int         m_layerpri[3];
	int         m_alpha_enabled;

	/* misc */
	int         m_game_type;
	UINT16      m_protram[16];
	UINT16      m_cur_control2;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k054539;
	device_t *m_k053246;
	device_t *m_k053251;
	device_t *m_k056832;
	device_t *m_k054338;

    emu_timer *m_dmaend_timer;
	DECLARE_READ16_MEMBER(control2_r);
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_WRITE16_MEMBER(sound_cmd1_w);
	DECLARE_WRITE16_MEMBER(sound_cmd2_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_READ16_MEMBER(K053247_scattered_word_r);
	DECLARE_WRITE16_MEMBER(K053247_scattered_word_w);
	DECLARE_WRITE16_MEMBER(moo_prot_w);
	DECLARE_WRITE16_MEMBER(moobl_oki_bank_w);
};



/*----------- defined in video/moo.c -----------*/

extern void moo_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
extern void moo_sprite_callback(running_machine &machine, int *code, int *color, int *priority_mask);

VIDEO_START(moo);
SCREEN_UPDATE_RGB32(moo);
