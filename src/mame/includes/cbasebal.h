// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Capcom Baseball

*************************************************************************/

#include "emupal.h"
#include "tilemap.h"

class cbasebal_state : public driver_device
{
public:
	cbasebal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	void init_cbasebal();
	void cbasebal(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_fg_tilemap = nullptr;
	tilemap_t    *m_bg_tilemap = nullptr;
	std::unique_ptr<uint8_t[]>    m_textram;
	std::unique_ptr<uint8_t[]>      m_scrollram;
	std::unique_ptr<uint8_t[]>    m_decoded;
	uint8_t      m_scroll_x[2]{};
	uint8_t      m_scroll_y[2]{};
	int        m_tilebank = 0;
	int        m_spritebank = 0;
	int        m_text_on = 0;
	int        m_bg_on = 0;
	int        m_obj_on = 0;
	int        m_flipscreen = 0;

	/* misc */
	uint8_t      m_rambank = 0U;
	void cbasebal_bankswitch_w(uint8_t data);
	uint8_t bankedram_r(offs_t offset);
	void bankedram_w(offs_t offset, uint8_t data);
	void cbasebal_coinctrl_w(uint8_t data);
	void cbasebal_textram_w(offs_t offset, uint8_t data);
	uint8_t cbasebal_textram_r(offs_t offset);
	void cbasebal_scrollram_w(offs_t offset, uint8_t data);
	uint8_t cbasebal_scrollram_r(offs_t offset);
	void cbasebal_gfxctrl_w(uint8_t data);
	void cbasebal_scrollx_w(offs_t offset, uint8_t data);
	void cbasebal_scrolly_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_cbasebal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void cbasebal_map(address_map &map);
	void cbasebal_portmap(address_map &map);
	void decrypted_opcodes_map(address_map &map);
};
