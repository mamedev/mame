// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/nvram.h"
#include "sound/dac.h"

class seicross_state : public driver_device
{
public:
	seicross_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_dac(*this, "dac"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_debug_port(*this, "DEBUG"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_row_scroll(*this, "row_scroll"),
		m_spriteram2(*this, "spriteram2"),
		m_colorram(*this, "colorram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<dac_byte_interface> m_dac;
	optional_device<nvram_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_ioport m_debug_port;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_row_scroll;
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	uint8_t m_portb;
	tilemap_t *m_bg_tilemap;
	uint8_t m_irq_mask;

	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t portB_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void portB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void vblank_irq(device_t &device);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_seicross(palette_device &palette);
	void init_friskytb();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	void nvram_init(nvram_device &nvram, void *data, size_t size);

	void dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};
