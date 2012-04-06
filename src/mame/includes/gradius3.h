/*************************************************************************

    Gradius 3

*************************************************************************/

class gradius3_state : public driver_device
{
public:
	gradius3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_gfxram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         m_layer_colorbase[3];
	int         m_sprite_colorbase;

	/* misc */
	int         m_priority;
	int         m_irqAen;
	int         m_irqBmask;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_subcpu;
	device_t *m_k007232;
	device_t *m_k052109;
	device_t *m_k051960;
	DECLARE_READ16_MEMBER(k052109_halfword_r);
	DECLARE_WRITE16_MEMBER(k052109_halfword_w);
	DECLARE_READ16_MEMBER(k051937_halfword_r);
	DECLARE_WRITE16_MEMBER(k051937_halfword_w);
	DECLARE_READ16_MEMBER(k051960_halfword_r);
	DECLARE_WRITE16_MEMBER(k051960_halfword_w);
	DECLARE_WRITE16_MEMBER(cpuA_ctrl_w);
	DECLARE_WRITE16_MEMBER(cpuB_irqenable_w);
	DECLARE_WRITE16_MEMBER(cpuB_irqtrigger_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_READ16_MEMBER(gradius3_gfxrom_r);
	DECLARE_WRITE16_MEMBER(gradius3_gfxram_w);
};

/*----------- defined in video/gradius3.c -----------*/

extern void gradius3_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask,int *shadow);
extern void gradius3_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);


VIDEO_START( gradius3 );
SCREEN_UPDATE_IND16( gradius3 );
