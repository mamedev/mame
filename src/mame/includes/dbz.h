/*************************************************************************

    Dragonball Z

*************************************************************************/

class dbz_state : public driver_device
{
public:
	dbz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg1_videoram(*this, "bg1_videoram"),
		m_bg2_videoram(*this, "bg2_videoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_bg1_videoram;
	required_shared_ptr<UINT16> m_bg2_videoram;
//  UINT16 *      m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *m_bg1_tilemap;
	tilemap_t    *m_bg2_tilemap;
	int          m_layer_colorbase[6];
	int          m_layerpri[5];
	int          m_sprite_colorbase;

	/* misc */
	int           m_control;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_k053246;
	device_t *m_k053251;
	device_t *m_k056832;
	device_t *m_k053936_1;
	device_t *m_k053936_2;
	DECLARE_READ16_MEMBER(dbzcontrol_r);
	DECLARE_WRITE16_MEMBER(dbzcontrol_w);
	DECLARE_WRITE16_MEMBER(dbz_sound_command_w);
	DECLARE_WRITE16_MEMBER(dbz_sound_cause_nmi);
	DECLARE_WRITE16_MEMBER(dbz_bg2_videoram_w);
	DECLARE_WRITE16_MEMBER(dbz_bg1_videoram_w);
	DECLARE_WRITE_LINE_MEMBER(dbz_irq2_ack_w);
	DECLARE_DRIVER_INIT(dbza);
	DECLARE_DRIVER_INIT(dbz);
	DECLARE_DRIVER_INIT(dbz2);
	TILE_GET_INFO_MEMBER(get_dbz_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_dbz_bg1_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_dbz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/dbz.c -----------*/

extern void dbz_sprite_callback(running_machine &machine, int *code, int *color, int *priority_mask);
extern void dbz_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);




