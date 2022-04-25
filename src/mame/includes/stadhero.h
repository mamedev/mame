// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

#include "machine/gen_latch.h"
#include "video/decbac06.h"
#include "video/decmxc06.h"
#include "screen.h"
#include "tilemap.h"

class stadhero_state : public driver_device
{
public:
	stadhero_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_tilegen(*this, "tilegen"),
		m_spritegen(*this, "spritegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch"),
		m_screen(*this, "screen"),
		m_spriteram(*this, "spriteram"),
		m_pf1_data(*this, "pf1_data"),
		m_coin(*this, "COIN")
	{
	}

	void stadhero(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco_bac06_device> m_tilegen;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_pf1_data;

	required_ioport m_coin;

	tilemap_t *m_pf1_tilemap = nullptr;

	void int_ack_w(uint16_t data);
	void pf1_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t mystery_r();

	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void audio_map(address_map &map);
	void main_map(address_map &map);
};
