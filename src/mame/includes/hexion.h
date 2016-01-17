// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/k053252.h"

class hexion_state : public driver_device
{
public:
	hexion_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_k053252(*this, "k053252"),
		m_gfxdecode(*this, "gfxdecode")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<k053252_device> m_k053252;
	required_device<gfxdecode_device> m_gfxdecode;

	UINT8 *m_vram[2];
	UINT8 *m_unkram;
	int m_bankctrl;
	int m_rambank;
	int m_pmcbank;
	int m_gfxrom_select;
	tilemap_t *m_bg_tilemap[2];

	DECLARE_WRITE8_MEMBER(coincntr_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_WRITE8_MEMBER(bankctrl_w);
	DECLARE_WRITE8_MEMBER(gfxrom_select_w);
	DECLARE_WRITE_LINE_MEMBER(irq_ack_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_ack_w);

	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tile_info(tile_data &tileinfo,int tile_index,UINT8 *ram);
};
