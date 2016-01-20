// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "video/decbac06.h"
#include "video/decmxc06.h"

class stadhero_state : public driver_device
{
public:
	stadhero_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_tilegen1(*this, "tilegen1"),
		m_spritegen(*this, "spritegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spriteram(*this, "spriteram"),
		m_pf1_data(*this, "pf1_data") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco_bac06_device> m_tilegen1;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_pf1_data;

	tilemap_t *m_pf1_tilemap;

	DECLARE_WRITE16_MEMBER(stadhero_control_w);
	DECLARE_WRITE16_MEMBER(stadhero_pf1_data_w);

	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	UINT32 screen_update_stadhero(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
