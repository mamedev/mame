// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Pandora's Palace

*************************************************************************/

class pandoras_state : public driver_device
{
public:
	pandoras_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	tilemap_t     *m_layer0;
	int         m_flipscreen;

	int m_irq_enable_a;
	int m_irq_enable_b;
	int m_firq_old_data_a;
	int m_firq_old_data_b;
	int m_i8039_status;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(pandoras_int_control_w);
	DECLARE_WRITE8_MEMBER(pandoras_cpua_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(pandoras_cpub_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(pandoras_i8039_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(i8039_irqen_and_status_w);
	DECLARE_WRITE8_MEMBER(pandoras_z80_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(pandoras_vram_w);
	DECLARE_WRITE8_MEMBER(pandoras_cram_w);
	DECLARE_WRITE8_MEMBER(pandoras_scrolly_w);
	DECLARE_WRITE8_MEMBER(pandoras_flipscreen_w);
	DECLARE_READ8_MEMBER(pandoras_portA_r);
	DECLARE_READ8_MEMBER(pandoras_portB_r);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(pandoras);
	UINT32 screen_update_pandoras(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pandoras_master_interrupt);
	INTERRUPT_GEN_MEMBER(pandoras_slave_interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8* sr );
};
