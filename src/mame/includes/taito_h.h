// license:BSD-3-Clause
// copyright-holders:Yochizo
/*************************************************************************

    Taito H system

*************************************************************************/
#include "machine/taitoio.h"
#include "video/tc0080vco.h"


class taitoh_state : public driver_device
{
public:
	taitoh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_m68000_mainram(*this, "m68000_mainram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_tc0080vco(*this, "tc0080vco"),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_m68000_mainram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<tc0080vco_device> m_tc0080vco;
	required_device<tc0220ioc_device> m_tc0220ioc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t syvalion_input_bypass_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_syvalion(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_recordbr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dleague(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void syvalion_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void recordbr_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	void dleague_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	void taitoh_log_vram();
};
