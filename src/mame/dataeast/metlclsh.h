// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    Metal Clash

*************************************************************************/

#include "emupal.h"
#include "tilemap.h"

class metlclsh_state : public driver_device
{
public:
	metlclsh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram"),
		m_bgram(*this, "bgram"),
		m_scrollx(*this, "scrollx"),
		m_rambank(*this, "rambank"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	void metlclsh(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_scrollx;
	required_memory_bank m_rambank;
	std::unique_ptr<uint8_t[]>        m_otherram;

	/* video-related */
	tilemap_t      *m_bg_tilemap = nullptr;
	tilemap_t      *m_fg_tilemap = nullptr;
	uint8_t          m_write_mask = 0;
	uint8_t          m_gfxbank = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void metlclsh_cause_irq(uint8_t data);
	void metlclsh_ack_nmi(uint8_t data);
	void metlclsh_cause_nmi2(uint8_t data);
	void metlclsh_ack_irq2(uint8_t data);
	void metlclsh_ack_nmi2(uint8_t data);
	void metlclsh_flipscreen_w(uint8_t data);
	void metlclsh_rambank_w(uint8_t data);
	void metlclsh_gfxbank_w(uint8_t data);
	void metlclsh_bgram_w(offs_t offset, uint8_t data);
	void metlclsh_fgram_w(offs_t offset, uint8_t data);
	TILEMAP_MAPPER_MEMBER(metlclsh_bgtilemap_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_metlclsh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void metlclsh_master_map(address_map &map);
	void metlclsh_slave_map(address_map &map);
};
