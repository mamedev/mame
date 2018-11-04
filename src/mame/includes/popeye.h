#include "machine/eepromser.h"

// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Couriersud
// thanks-to: Marc Lafontaine
class tnx1_state : public driver_device
{
public:
	tnx1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_background_pos(*this, "background_pos"),
		m_palettebank(*this, "palettebank"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_color_prom(*this, "proms"),
		m_color_prom_spr(*this, "sprpal") { }

	DECLARE_CUSTOM_INPUT_MEMBER(dsw1_read);
	DECLARE_CUSTOM_INPUT_MEMBER(pop_field_r);
	virtual void config(machine_config &config);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_background_pos;
	required_shared_ptr<uint8_t> m_palettebank;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_region_ptr<uint8_t> m_color_prom;
	required_region_ptr<uint8_t> m_color_prom_spr;

	uint8_t m_background_ram[0x1000];
	std::unique_ptr<bitmap_ind16> m_sprite_bitmap;
	tilemap_t *m_fg_tilemap;
	uint8_t m_last_palette;
	int   m_field;
	uint8_t m_prot0;
	uint8_t m_prot1;
	uint8_t m_prot_shift;
	uint8_t m_dswbit;

	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_WRITE8_MEMBER(popeye_videoram_w);
	DECLARE_WRITE8_MEMBER(popeye_colorram_w);
	virtual DECLARE_WRITE8_MEMBER(background_w);
	DECLARE_WRITE8_MEMBER(popeye_portB_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void driver_start() override;
	virtual void video_start() override;
	virtual DECLARE_PALETTE_INIT(palette_init);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(popeye_interrupt);
	void update_palette();
	virtual void decrypt_rom();
	virtual void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_field(bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void maincpu_program_map(address_map &map);
	void maincpu_io_map(address_map &map);
};

class tpp1_state : public tnx1_state
{
	using tnx1_state::tnx1_state;
protected:
	virtual DECLARE_PALETTE_INIT(palette_init) override;
	virtual void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect) override;
};

class popeyebl_state : public tpp1_state
{
	using tpp1_state::tpp1_state;
protected:
	virtual void decrypt_rom() override;
	virtual void maincpu_program_map(address_map &map) override;
};

class tpp2_state : public tpp1_state
{
	using tpp1_state::tpp1_state;
public:
	virtual void config(machine_config &config) override;
protected:
	virtual void maincpu_program_map(address_map &map) override;
	virtual void decrypt_rom() override;
	virtual void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual DECLARE_WRITE8_MEMBER(background_w) override;
};

class tpp2np_state : public tpp2_state
{
	using tpp2_state::tpp2_state;
protected:
	virtual void maincpu_program_map(address_map &map) override;
};
