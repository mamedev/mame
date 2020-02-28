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
	std::unique_ptr<uint8_t[]>        m_otherram;

	/* video-related */
	tilemap_t      *m_bg_tilemap;
	tilemap_t      *m_fg_tilemap;
	uint8_t          m_write_mask;
	uint8_t          m_gfxbank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(metlclsh_cause_irq);
	DECLARE_WRITE8_MEMBER(metlclsh_ack_nmi);
	DECLARE_WRITE8_MEMBER(metlclsh_cause_nmi2);
	DECLARE_WRITE8_MEMBER(metlclsh_ack_irq2);
	DECLARE_WRITE8_MEMBER(metlclsh_ack_nmi2);
	DECLARE_WRITE8_MEMBER(metlclsh_flipscreen_w);
	DECLARE_WRITE8_MEMBER(metlclsh_rambank_w);
	DECLARE_WRITE8_MEMBER(metlclsh_gfxbank_w);
	DECLARE_WRITE8_MEMBER(metlclsh_bgram_w);
	DECLARE_WRITE8_MEMBER(metlclsh_fgram_w);
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
