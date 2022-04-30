// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Glass

*************************************************************************/

#include "machine/74259.h"
#include "emupal.h"
#include "tilemap.h"

class glass_state : public driver_device
{
public:
	glass_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram"),
		m_bmap(*this, "bmap"),
		m_okibank(*this, "okibank"),
		m_pant{ nullptr, nullptr },
		m_blitter_command(0)
	{ }

	void glass(machine_config &config);
	void glass_ds5002fp(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_outlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_shareram;
	required_region_ptr<uint8_t>  m_bmap;

	required_memory_bank m_okibank;

	/* video-related */
	tilemap_t     *m_pant[2]{};
	std::unique_ptr<bitmap_ind16> m_screen_bitmap;

	/* misc */
	int         m_current_bit = 0;
	int         m_cause_interrupt = 0;
	int         m_blitter_command = 0;

	void shareram_w(offs_t offset, uint8_t data);
	uint8_t shareram_r(offs_t offset);
	void clr_int_w(uint16_t data);
	void oki_bankswitch_w(uint8_t data);
	void coin_w(offs_t offset, uint16_t data);
	void blitter_w(uint16_t data);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	DECLARE_WRITE_LINE_MEMBER(coin1_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_counter_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void glass_map(address_map &map);
	void mcu_hostmem_map(address_map &map);
	void oki_map(address_map &map);
};
