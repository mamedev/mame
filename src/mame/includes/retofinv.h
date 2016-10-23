// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski, Andrea Mazzoleni

#include "machine/gen_latch.h"

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
		m_soundlatch(*this, "soundlatch"),
		m_fg_videoram(*this, "fg_videoram"),
		m_sharedram(*this, "sharedram"),
		m_bg_videoram(*this, "bg_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_68705;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_bg_videoram;

	uint8_t m_main_irq_mask;
	uint8_t m_sub_irq_mask;
	uint8_t m_cpu2_m6000;
	uint8_t m_from_main;
	uint8_t m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;
	uint8_t m_portA_in;
	uint8_t m_portA_out;
	uint8_t m_ddrA;
	uint8_t m_portB_in;
	uint8_t m_portB_out;
	uint8_t m_ddrB;
	uint8_t m_portC_in;
	uint8_t m_portC_out;
	uint8_t m_ddrC;
	int m_fg_bank;
	int m_bg_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	void cpu1_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpu2_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpu2_m6000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cpu0_mf800_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void soundcommand_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq0_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq1_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coincounter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coinlockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_portA_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_portA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_ddrA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_portB_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_portB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_ddrB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_portC_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_portC_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_ddrC_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gfx_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(retofinv);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap);

	INTERRUPT_GEN_MEMBER(main_vblank_irq);
	INTERRUPT_GEN_MEMBER(sub_vblank_irq);
};
