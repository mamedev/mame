// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
#include "sound/msm5205.h"

class lucky74_state : public driver_device
{
public:
	lucky74_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram"),
		m_fg_colorram(*this, "fg_colorram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_colorram(*this, "bg_colorram"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode") { }

	uint8_t m_ym2149_portb;
	uint8_t m_usart_8251;
	uint8_t m_copro_sm7831;
	int m_adpcm_pos;
	int m_adpcm_end;
	int m_adpcm_data;
	uint8_t m_adpcm_reg[6];
	uint8_t m_adpcm_busy_line;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_fg_colorram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_bg_colorram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	uint8_t custom_09R81P_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void custom_09R81P_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t usart_8251_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void usart_8251_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t copro_sm7831_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void copro_sm7831_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lucky74_fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lucky74_fg_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lucky74_bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lucky74_bg_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ym2149_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamps_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamps_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	virtual void machine_reset() override;
	virtual void sound_start() override;
	void palette_init_lucky74(palette_device &palette);
	uint32_t screen_update_lucky74(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmi_interrupt(device_t &device);
	void lucky74_adpcm_int(int state);
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
};
