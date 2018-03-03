// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino

#include "machine/taito68705interface.h"

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "screen.h"

class xain_state : public driver_device
{
public:
	xain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_subcpu(*this, "sub")
		, m_mcu(*this, "mcu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_charram(*this, "charram")
		, m_bgram0(*this, "bgram0")
		, m_bgram1(*this, "bgram1")
		, m_spriteram(*this, "spriteram")
		, m_char_tilemap(nullptr)
		, m_bgram0_tilemap(nullptr)
		, m_bgram1_tilemap(nullptr)
	{
	}

	DECLARE_WRITE8_MEMBER(cpuA_bankswitch_w);
	DECLARE_WRITE8_MEMBER(cpuB_bankswitch_w);
	DECLARE_WRITE8_MEMBER(main_irq_w);
	DECLARE_WRITE8_MEMBER(irqA_assert_w);
	DECLARE_WRITE8_MEMBER(irqB_clear_w);
	DECLARE_READ8_MEMBER(mcu_comm_reset_r);
	DECLARE_WRITE8_MEMBER(bgram0_w);
	DECLARE_WRITE8_MEMBER(bgram1_w);
	DECLARE_WRITE8_MEMBER(charram_w);
	DECLARE_WRITE8_MEMBER(scrollxP0_w);
	DECLARE_WRITE8_MEMBER(scrollyP0_w);
	DECLARE_WRITE8_MEMBER(scrollxP1_w);
	DECLARE_WRITE8_MEMBER(scrollyP1_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);

	DECLARE_CUSTOM_INPUT_MEMBER(vblank_r);
	DECLARE_CUSTOM_INPUT_MEMBER(mcu_status_r);

	TILEMAP_MAPPER_MEMBER(back_scan);
	TILE_GET_INFO_MEMBER(get_bgram0_tile_info);
	TILE_GET_INFO_MEMBER(get_bgram1_tile_info);
	TILE_GET_INFO_MEMBER(get_char_tile_info);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void xsleenab(machine_config &config);
	void xsleena(machine_config &config);
	void bootleg_map(address_map &map);
	void cpu_map_B(address_map &map);
	void main_map(address_map &map);
	void sound_map(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	inline int scanline_to_vcount(int scanline);

	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	required_device<cpu_device>             m_maincpu;
	required_device<cpu_device>             m_audiocpu;
	required_device<cpu_device>             m_subcpu;
	optional_device<taito68705_mcu_device>  m_mcu;
	required_device<gfxdecode_device>       m_gfxdecode;
	required_device<screen_device>          m_screen;
	required_device<palette_device>         m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<u8> m_charram;
	required_shared_ptr<u8> m_bgram0;
	required_shared_ptr<u8> m_bgram1;
	required_shared_ptr<u8> m_spriteram;

	tilemap_t   *m_char_tilemap;
	tilemap_t   *m_bgram0_tilemap;
	tilemap_t   *m_bgram1_tilemap;

	int     m_vblank;

	u8      m_pri;
	u8      m_scrollxP0[2];
	u8      m_scrollyP0[2];
	u8      m_scrollxP1[2];
	u8      m_scrollyP1[2];
};
