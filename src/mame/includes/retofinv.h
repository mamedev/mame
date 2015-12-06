// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski, Andrea Mazzoleni
class retofinv_state : public driver_device
{
public:
	retofinv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_68705(*this, "68705"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_fg_videoram(*this, "fg_videoram"),
		m_sharedram(*this, "sharedram"),
		m_bg_videoram(*this, "bg_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_68705;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_sharedram;
	required_shared_ptr<UINT8> m_bg_videoram;

	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	UINT8 m_cpu2_m6000;
	UINT8 m_from_main;
	UINT8 m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;
	UINT8 m_portA_in;
	UINT8 m_portA_out;
	UINT8 m_ddrA;
	UINT8 m_portB_in;
	UINT8 m_portB_out;
	UINT8 m_ddrB;
	UINT8 m_portC_in;
	UINT8 m_portC_out;
	UINT8 m_ddrC;
	int m_fg_bank;
	int m_bg_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	DECLARE_WRITE8_MEMBER(cpu1_reset_w);
	DECLARE_WRITE8_MEMBER(cpu2_reset_w);
	DECLARE_WRITE8_MEMBER(mcu_reset_w);
	DECLARE_WRITE8_MEMBER(cpu2_m6000_w);
	DECLARE_READ8_MEMBER(cpu0_mf800_r);
	DECLARE_WRITE8_MEMBER(soundcommand_w);
	DECLARE_WRITE8_MEMBER(irq0_ack_w);
	DECLARE_WRITE8_MEMBER(irq1_ack_w);
	DECLARE_WRITE8_MEMBER(coincounter_w);
	DECLARE_WRITE8_MEMBER(coinlockout_w);
	DECLARE_READ8_MEMBER(mcu_portA_r);
	DECLARE_WRITE8_MEMBER(mcu_portA_w);
	DECLARE_WRITE8_MEMBER(mcu_ddrA_w);
	DECLARE_READ8_MEMBER(mcu_portB_r);
	DECLARE_WRITE8_MEMBER(mcu_portB_w);
	DECLARE_WRITE8_MEMBER(mcu_ddrB_w);
	DECLARE_READ8_MEMBER(mcu_portC_r);
	DECLARE_WRITE8_MEMBER(mcu_portC_w);
	DECLARE_WRITE8_MEMBER(mcu_ddrC_w);
	DECLARE_WRITE8_MEMBER(mcu_w);
	DECLARE_READ8_MEMBER(mcu_r);
	DECLARE_READ8_MEMBER(mcu_status_r);
	DECLARE_WRITE8_MEMBER(bg_videoram_w);
	DECLARE_WRITE8_MEMBER(fg_videoram_w);
	DECLARE_WRITE8_MEMBER(gfx_ctrl_w);

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(retofinv);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap);

	INTERRUPT_GEN_MEMBER(main_vblank_irq);
	INTERRUPT_GEN_MEMBER(sub_vblank_irq);
};
