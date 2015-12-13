// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Food Fight hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "machine/x2212.h"

class foodf_state : public atarigen_state
{
public:
	foodf_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_nvram(*this, "nvram"),
			m_playfield_tilemap(*this, "playfield"),
			m_spriteram(*this, "spriteram") { }

	required_device<x2212_device> m_nvram;
	required_device<tilemap_device> m_playfield_tilemap;

	double          m_rweights[3];
	double          m_gweights[3];
	double          m_bweights[2];
	UINT8           m_playfield_flip;

	UINT8           m_whichport;
	required_shared_ptr<UINT16> m_spriteram;
	virtual void update_interrupts() override;
	DECLARE_WRITE16_MEMBER(nvram_recall_w);
	DECLARE_WRITE8_MEMBER(digital_w);
	DECLARE_READ16_MEMBER(analog_r);
	DECLARE_WRITE16_MEMBER(analog_w);
	DECLARE_WRITE16_MEMBER(foodf_paletteram_w);
	void foodf_set_flip(int flip);
	DECLARE_READ8_MEMBER(pot_r);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(foodf);
	DECLARE_MACHINE_RESET(foodf);
	DECLARE_VIDEO_START(foodf);
	UINT32 screen_update_foodf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update_timer);
};
