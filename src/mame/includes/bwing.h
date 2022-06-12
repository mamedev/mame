// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Alex W. Jackson
/***************************************************************************

    B-Wings

***************************************************************************/

#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

#define BW_DEBUG 0

class bwing_state : public driver_device
{
public:
	bwing_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vrambank(*this, "vrambank"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_fgscrollram(*this, "fgscrollram"),
		m_bgscrollram(*this, "bgscrollram"),
		m_gfxram(*this, "gfxram") { }

	void init_bwing();
	void bwing(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(tilt_pressed);

private:
	/* device */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<address_map_bank_device> m_vrambank;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_fgscrollram;
	required_shared_ptr<uint8_t> m_bgscrollram;
	required_shared_ptr<uint8_t> m_gfxram;

	/* video-related */
	tilemap_t *m_charmap = nullptr;
	tilemap_t *m_fgmap = nullptr;
	tilemap_t *m_bgmap = nullptr;
	unsigned m_sreg[8]{};
	unsigned m_palatch = 0U;
	unsigned m_mapmask = 0U;

	/* sound-related */
	int m_bwp3_nmimask = 0;
	int m_bwp3_u8F_d = 0;

	void bwp3_u8F_w(uint8_t data);
	void bwp3_nmimask_w(uint8_t data);
	void bwp3_nmiack_w(uint8_t data);
	void bwp1_ctrl_w(offs_t offset, uint8_t data);
	void bwp2_ctrl_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void fgscrollram_w(offs_t offset, uint8_t data);
	void bgscrollram_w(offs_t offset, uint8_t data);
	void gfxram_w(offs_t offset, uint8_t data);
	void scrollreg_w(offs_t offset, uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_fgtileinfo);
	TILE_GET_INFO_MEMBER(get_bgtileinfo);
	TILE_GET_INFO_MEMBER(get_charinfo);
	TILEMAP_MAPPER_MEMBER(scan_cols);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void bwing_postload();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bmp, const rectangle &clip, uint8_t *ram, int pri );

	INTERRUPT_GEN_MEMBER(bwp3_interrupt);
	void bank_map(address_map &map);
	void bwp1_map(address_map &map);
	void bwp2_map(address_map &map);
	void bwp3_io_map(address_map &map);
	void bwp3_map(address_map &map);
};
