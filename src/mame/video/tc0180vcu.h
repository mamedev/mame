// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef __TC0180VCU_H__
#define __TC0180VCU_H__

class tc0180vcu_device : public device_t
{
public:
	tc0180vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tc0180vcu_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_bg_colorbase(device_t &device, int color) { downcast<tc0180vcu_device &>(device).m_bg_color_base = color; }
	static void set_fg_colorbase(device_t &device, int color) { downcast<tc0180vcu_device &>(device).m_fg_color_base = color; }
	static void set_tx_colorbase(device_t &device, int color) { downcast<tc0180vcu_device &>(device).m_tx_color_base = color; }

	uint8_t get_fb_page(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void set_fb_page(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t get_videoctrl(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint16_t ctrl_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t scroll_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, int plane);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint16_t         m_ctrl[0x10];

	std::unique_ptr<uint16_t[]>       m_ram;
	std::unique_ptr<uint16_t[]>       m_scrollram;

	tilemap_t      *m_tilemap[3];

	uint16_t         m_bg_rambank[2], m_fg_rambank[2], m_tx_rambank;
	uint8_t          m_framebuffer_page;
	uint8_t          m_video_control;

	int            m_bg_color_base;
	int            m_fg_color_base;
	int            m_tx_color_base;

	required_device<gfxdecode_device> m_gfxdecode;

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void video_control( uint8_t data );
};

extern const device_type TC0180VCU;

#define MCFG_TC0180VCU_BG_COLORBASE(_color) \
	tc0180vcu_device::set_bg_colorbase(*device, _color);

#define MCFG_TC0180VCU_FG_COLORBASE(_color) \
	tc0180vcu_device::set_fg_colorbase(*device, _color);

#define MCFG_TC0180VCU_TX_COLORBASE(_color) \
	tc0180vcu_device::set_tx_colorbase(*device, _color);

#define MCFG_TC0180VCU_GFXDECODE(_gfxtag) \
	tc0180vcu_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#endif
