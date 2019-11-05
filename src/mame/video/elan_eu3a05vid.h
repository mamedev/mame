// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_VIDEO_ELAN_EU3A05VID_H
#define MAME_VIDEO_ELAN_EU3A05VID_H

#include "elan_eu3a05commonvid.h"
#include "cpu/m6502/m6502.h"
#include "machine/bankdev.h"

class elan_eu3a05vid_device : public elan_eu3a05commonvid_device
{
public:
	elan_eu3a05vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_addrbank(T &&tag) { m_bank.set_tag(std::forward<T>(tag)); }

	// VIDEO
	// tile bases
	DECLARE_WRITE8_MEMBER(tile_gfxbase_lo_w);
	DECLARE_WRITE8_MEMBER(tile_gfxbase_hi_w);
	DECLARE_READ8_MEMBER(tile_gfxbase_lo_r);
	DECLARE_READ8_MEMBER(tile_gfxbase_hi_r);
	// sprite tile bases
	DECLARE_WRITE8_MEMBER(sprite_gfxbase_lo_w);
	DECLARE_WRITE8_MEMBER(sprite_gfxbase_hi_w);
	DECLARE_READ8_MEMBER(sprite_gfxbase_lo_r);
	DECLARE_READ8_MEMBER(sprite_gfxbase_hi_r);

	DECLARE_READ8_MEMBER(elan_eu3a05_vidctrl_r);
	DECLARE_WRITE8_MEMBER(elan_eu3a05_vidctrl_w);

	DECLARE_READ8_MEMBER(tile_scroll_r);
	DECLARE_WRITE8_MEMBER(tile_scroll_w);

	DECLARE_READ8_MEMBER(splitpos_r);
	DECLARE_WRITE8_MEMBER(splitpos_w);

	uint16_t get_scroll(int which);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<m6502_device> m_cpu;
	required_device<address_map_bank_device> m_bank;

	uint8_t m_vidctrl;

	uint8_t m_tile_gfxbase_lo_data;
	uint8_t m_tile_gfxbase_hi_data;

	uint8_t m_sprite_gfxbase_lo_data;
	uint8_t m_sprite_gfxbase_hi_data;

	uint8_t m_tile_scroll[4*2];

	uint8_t m_splitpos[2];

	bool get_tile_data(int base, int drawpri, int& tile, int &attr, int &unk2);
	void draw_tilemaps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int drawpri);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t read_spriteram(int offset);
	uint8_t read_vram(int offset);

};

DECLARE_DEVICE_TYPE(ELAN_EU3A05_VID, elan_eu3a05vid_device)

#endif // MAME_VIDEO_ELAN_EU3A05VID_H
