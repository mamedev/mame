// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/k053252.h"

class hexion_state : public driver_device
{
public:
	hexion_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_k053252(*this, "k053252"),
		m_gfxdecode(*this, "gfxdecode")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<k053252_device> m_k053252;
	required_device<gfxdecode_device> m_gfxdecode;

	uint8_t *m_vram[2];
	uint8_t *m_unkram;
	int m_bankctrl;
	int m_rambank;
	int m_pmcbank;
	int m_gfxrom_select;
	tilemap_t *m_bg_tilemap[2];

	void coincntr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bankedram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bankedram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gfxrom_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_ack_w(int state);
	void nmi_ack_w(int state);

	void get_tile_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void scanline(timer_device &timer, void *ptr, int32_t param);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tile_info(tile_data &tileinfo,int tile_index,uint8_t *ram);
};
