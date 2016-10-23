// license:BSD-3-Clause
// copyright-holders:Luca Elia

#include "machine/gen_latch.h"
#include "video/decmxc06.h"


class thedeep_state : public driver_device
{
public:
	thedeep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_scroll(*this, "scroll"),
		m_scroll2(*this, "scroll2")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_vram_0;
	required_shared_ptr<uint8_t> m_vram_1;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_scroll2;

	int m_nmi_enable;
	uint8_t m_protection_command;
	uint8_t m_protection_data;
	int m_protection_index;
	int m_protection_irq;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	uint8_t m_mcu_p3_reg;

	void nmi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t e004_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void e100_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t from_main_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void to_main_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vram_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows_back);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(thedeep);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(mcu_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
};
