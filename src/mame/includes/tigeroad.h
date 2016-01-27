// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#include "video/bufsprite.h"
#include "sound/msm5205.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "cpu/m6805/m6805.h"
#include "video/tigeroad_spr.h"

class tigeroad_state : public driver_device
{
public:
	tigeroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_videoram(*this, "videoram"),
		m_ram16(*this, "ram16"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mcu(*this, "mcu"),
		m_spritegen(*this, "spritegen"),
		m_has_coinlock(1)
	{ }

	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_ram16;
	int m_bgcharbank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	DECLARE_WRITE16_MEMBER(f1dream_control_w);
	DECLARE_WRITE16_MEMBER(tigeroad_soundcmd_w);
	DECLARE_WRITE16_MEMBER(tigeroad_videoram_w);
	DECLARE_WRITE16_MEMBER(tigeroad_videoctrl_w);
	DECLARE_WRITE16_MEMBER(tigeroad_scroll_w);
	DECLARE_WRITE8_MEMBER(msm5205_w);
	DECLARE_DRIVER_INIT(f1dream);
	DECLARE_DRIVER_INIT(pushman);
	DECLARE_DRIVER_INIT(bballs);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(tigeroad_tilemap_scan);
	virtual void video_start() override;
	UINT32 screen_update_tigeroad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void f1dream_protection_w(address_space &space);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<cpu_device> m_mcu;
	required_device<tigeroad_spr_device> m_spritegen;

	UINT16     m_control[2];

	/* misc */
	UINT8      m_shared_ram[8];
	UINT16     m_latch;
	UINT16     m_new_latch;
	int m_has_coinlock;

	/* protection handling */
	DECLARE_READ16_MEMBER(pushman_68705_r);
	DECLARE_WRITE16_MEMBER(pushman_68705_w);
	DECLARE_READ16_MEMBER(bballs_68705_r);
	DECLARE_WRITE16_MEMBER(bballs_68705_w);
	DECLARE_READ8_MEMBER(pushman_68000_r);
	DECLARE_WRITE8_MEMBER(pushman_68000_w);
	DECLARE_MACHINE_RESET(pushman);
	DECLARE_MACHINE_RESET(bballs);

	virtual void machine_start() override;

};
