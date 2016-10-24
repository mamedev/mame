// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Luca Elia

#include "machine/i8255.h"

typedef device_delegate<uint16_t (uint16_t)> igs017_igs031_palette_scramble_delegate;

#define MCFG_PALETTE_SCRAMBLE_CB( _class, _method) \
	igs017_igs031_device::set_palette_scramble_cb(*device, igs017_igs031_palette_scramble_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));

#define MCFG_REVERSE_TEXT_BITS \
	igs017_igs031_device::static_set_text_reverse_bits(*device);

class igs017_igs031_device : public device_t,
							public device_gfx_interface,
							public device_video_interface,
							public device_memory_interface
{
	//static const gfx_layout tilelayout, spritelayout;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

public:
	igs017_igs031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


	static void set_palette_scramble_cb(device_t &device,igs017_igs031_palette_scramble_delegate newtilecb);

	static void static_set_text_reverse_bits(device_t &device)
	{
		igs017_igs031_device &dev = downcast<igs017_igs031_device &>(device);
		dev.m_revbits = 1;
	}

	uint16_t palette_callback_straight(uint16_t bgr);

	igs017_igs031_palette_scramble_delegate m_palette_scramble_cb;

	DECLARE_ADDRESS_MAP(map, 8);

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// the gfx roms were often hooked up with the bits backwards, allow us to handle it here to save doing it in every driver.
	int m_revbits;

	int m_toggle;
	int m_debug_addr;
	int m_debug_width;
	uint8_t m_video_disable;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	std::unique_ptr<uint8_t[]> m_sprites_gfx;
	int m_sprites_gfx_size;

	int get_nmi_enable() { return m_nmi_enable; }
	int get_irq_enable() { return m_irq_enable; }


	int m_nmi_enable;
	int m_irq_enable;


	void palram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8255_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void video_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);


	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void fg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void space_w(int offset, uint8_t data);
	uint8_t space_r(int offset);

	void expand_sprites();
	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int sx, int sy, int dimx, int dimy, int flipx, int flipy, int color, int addr);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int debug_viewer(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_igs017(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void video_start();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	address_space_config        m_space_config;

public:
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_palram;
	optional_device<i8255_device> m_i8255;
	required_device<palette_device> m_palette;

private:
};

extern const device_type IGS017_IGS031;
