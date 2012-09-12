#define RASTER_LINES 262
#define FIRST_VISIBLE_LINE 0
#define LAST_VISIBLE_LINE 223

class hyprduel_state : public driver_device
{
public:
	hyprduel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vram_2(*this, "vram_2"),
		m_paletteram(*this, "paletteram"),
		m_spriteram(*this, "spriteram"),
		m_tiletable(*this, "tiletable"),
		m_blitter_regs(*this, "blitter_regs"),
		m_window(*this, "window"),
		m_scroll(*this, "scroll"),
		m_irq_enable(*this, "irq_enable"),
		m_rombank(*this, "rombank"),
		m_screenctrl(*this, "screenctrl"),
		m_videoregs(*this, "videoregs"),
		m_sharedram1(*this, "sharedram1"),
		m_sharedram3(*this, "sharedram3"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_vram_0;
	required_shared_ptr<UINT16> m_vram_1;
	required_shared_ptr<UINT16> m_vram_2;
	required_shared_ptr<UINT16> m_paletteram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_tiletable;
	required_shared_ptr<UINT16> m_blitter_regs;
	required_shared_ptr<UINT16> m_window;
	required_shared_ptr<UINT16> m_scroll;
	required_shared_ptr<UINT16> m_irq_enable;
	required_shared_ptr<UINT16> m_rombank;
	required_shared_ptr<UINT16> m_screenctrl;
	required_shared_ptr<UINT16> m_videoregs;
	required_shared_ptr<UINT16> m_sharedram1;
	required_shared_ptr<UINT16> m_sharedram3;
	UINT16 *  m_tiletable_old;

	/* video-related */
	tilemap_t   *m_bg_tilemap[3];
	UINT8     *m_empty_tiles;
	UINT8     *m_dirtyindex;
	int       m_sprite_xoffs;
	int       m_sprite_yoffs;
	int       m_sprite_yoffs_sub;
	UINT8 *	  m_expanded_gfx1;

	/* misc */
	emu_timer *m_magerror_irq_timer;
	int       m_blitter_bit;
	int       m_requested_int;
	int       m_subcpu_resetline;
	int       m_cpu_trigger;
	int       m_int_num;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_subcpu;
	DECLARE_READ16_MEMBER(hyprduel_irq_cause_r);
	DECLARE_WRITE16_MEMBER(hyprduel_irq_cause_w);
	DECLARE_WRITE16_MEMBER(hyprduel_subcpu_control_w);
	DECLARE_READ16_MEMBER(hyprduel_cpusync_trigger1_r);
	DECLARE_WRITE16_MEMBER(hyprduel_cpusync_trigger1_w);
	DECLARE_READ16_MEMBER(hyprduel_cpusync_trigger2_r);
	DECLARE_WRITE16_MEMBER(hyprduel_cpusync_trigger2_w);
	DECLARE_READ16_MEMBER(hyprduel_bankedrom_r);
	DECLARE_WRITE16_MEMBER(hyprduel_blitter_w);
	DECLARE_WRITE16_MEMBER(hyprduel_paletteram_w);
	DECLARE_WRITE16_MEMBER(hyprduel_vram_0_w);
	DECLARE_WRITE16_MEMBER(hyprduel_vram_1_w);
	DECLARE_WRITE16_MEMBER(hyprduel_vram_2_w);
	DECLARE_WRITE16_MEMBER(hyprduel_window_w);
	DECLARE_WRITE16_MEMBER(hyprduel_scrollreg_w);
	DECLARE_WRITE16_MEMBER(hyprduel_scrollreg_init_w);
	void blt_write( address_space *space, const int tmap, const offs_t offs, const UINT16 data, const UINT16 mask );
	DECLARE_DRIVER_INIT(magerror);
	DECLARE_DRIVER_INIT(hyprduel);
	TILE_GET_INFO_MEMBER(get_tile_info_0_8bit);
	TILE_GET_INFO_MEMBER(get_tile_info_1_8bit);
	TILE_GET_INFO_MEMBER(get_tile_info_2_8bit);
};



/*----------- defined in video/hyprduel.c -----------*/



VIDEO_START( hyprduel_14220 );
VIDEO_START( magerror_14220 );
SCREEN_UPDATE_IND16( hyprduel );
