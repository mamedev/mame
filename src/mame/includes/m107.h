/*************************************************************************

    Irem M107 hardware

*************************************************************************/


struct pf_layer_info
{
	tilemap_t *     tmap;
	UINT16          vram_base;
	UINT16          control[4];
};

class m107_state : public driver_device
{
public:
	m107_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram"),
			m_vram_data(*this, "vram_data"),
			m_maincpu(*this, "maincpu"),
			m_soundcpu(*this, "soundcpu"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette") { }

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_vram_data;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	UINT8 m_irq_vectorbase;
	int m_sound_status;
	UINT8 m_spritesystem;
	UINT8 m_sprite_display;
	UINT16 m_raster_irq_position;
	pf_layer_info m_pf_layer[4];
	UINT16 m_control[0x10];
	UINT16 *m_buffered_spriteram;
	DECLARE_WRITE16_MEMBER(m107_coincounter_w);
	DECLARE_WRITE16_MEMBER(m107_bankswitch_w);
	DECLARE_WRITE16_MEMBER(m107_soundlatch_w);
	DECLARE_READ16_MEMBER(m107_sound_status_r);
	DECLARE_READ16_MEMBER(m107_soundlatch_r);
	DECLARE_WRITE16_MEMBER(m107_sound_irq_ack_w);
	DECLARE_WRITE16_MEMBER(m107_sound_status_w);
	DECLARE_WRITE16_MEMBER(m107_sound_reset_w);
	DECLARE_WRITE16_MEMBER(wpksoc_output_w);
	DECLARE_WRITE16_MEMBER(m107_vram_w);
	DECLARE_WRITE16_MEMBER(m107_control_w);
	DECLARE_WRITE16_MEMBER(m107_spritebuffer_w);
	DECLARE_DRIVER_INIT(firebarr);
	DECLARE_DRIVER_INIT(dsoccr94);
	DECLARE_DRIVER_INIT(wpksoc);
	TILE_GET_INFO_MEMBER(get_pf_tile_info);
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_m107(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(m107_scanline_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void m107_update_scroll_positions();
	void m107_tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int laynum, int category,int opaque);
	void m107_screenrefresh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
