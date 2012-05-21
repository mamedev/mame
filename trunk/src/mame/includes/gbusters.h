/*************************************************************************

    Gangbusters

*************************************************************************/

class gbusters_state : public driver_device
{
public:
	gbusters_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ram(*this, "ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ram;

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;

	/* misc */
	int        m_palette_selected;
	int        m_priority;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k007232;
	device_t *m_k052109;
	device_t *m_k051960;
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_WRITE8_MEMBER(gbusters_1f98_w);
	DECLARE_WRITE8_MEMBER(gbusters_coin_counter_w);
	DECLARE_WRITE8_MEMBER(gbusters_unknown_w);
	DECLARE_WRITE8_MEMBER(gbusters_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
};

/*----------- defined in video/gbusters.c -----------*/

extern void gbusters_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags, int *priority);
extern void gbusters_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);

VIDEO_START( gbusters );
SCREEN_UPDATE_IND16( gbusters );
