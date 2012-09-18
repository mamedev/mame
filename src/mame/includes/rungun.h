/*************************************************************************

    Run and Gun / Slam Dunk

*************************************************************************/

class rungun_state : public driver_device
{
public:
	rungun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_sysreg(*this, "sysreg"),
		m_936_videoram(*this, "936_videoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_sysreg;
	required_shared_ptr<UINT16> m_936_videoram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *m_ttl_tilemap;
	tilemap_t   *m_936_tilemap;
	UINT16      m_ttl_vram[0x1000];
	int         m_ttl_gfx_index;
	int         m_sprite_colorbase;

	/* misc */
	int         m_z80_control;
	int         m_sound_status;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_k054539_1;
	device_t *m_k054539_2;
	device_t *m_k053936;
	device_t *m_k055673;
	device_t *m_k053252;
	DECLARE_READ16_MEMBER(rng_sysregs_r);
	DECLARE_WRITE16_MEMBER(rng_sysregs_w);
	DECLARE_WRITE16_MEMBER(sound_cmd1_w);
	DECLARE_WRITE16_MEMBER(sound_cmd2_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_READ16_MEMBER(sound_status_msb_r);
	DECLARE_WRITE8_MEMBER(sound_status_w);
	DECLARE_WRITE8_MEMBER(z80ctrl_w);
	DECLARE_READ16_MEMBER(rng_ttl_ram_r);
	DECLARE_WRITE16_MEMBER(rng_ttl_ram_w);
	DECLARE_WRITE16_MEMBER(rng_936_videoram_w);
	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	TILE_GET_INFO_MEMBER(get_rng_936_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_rng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
/*----------- defined in video/rungun.c -----------*/

extern void rng_sprite_callback(running_machine &machine, int *code, int *color, int *priority_mask);
