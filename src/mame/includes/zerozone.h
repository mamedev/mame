// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/*************************************************************************

    Zero Zone

*************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

class zerozone_state : public driver_device
{
public:
	zerozone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_vram(*this, "videoram"),
			m_gfxdecode(*this, "gfxdecode")
	{ }

	// in drivers/zerozone.c
	DECLARE_WRITE16_MEMBER(sound_w);

	// in video/zerozone.c
	DECLARE_WRITE16_MEMBER(tilemap_w);
	DECLARE_WRITE16_MEMBER(tilebank_w);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;

	// shared pointers
	required_shared_ptr<UINT16> m_vram;
	// currently this driver uses generic palette handling

	required_device<gfxdecode_device> m_gfxdecode;
	// state
	// video-related
	UINT16         m_tilebank;
	tilemap_t     *m_zz_tilemap;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
	TILE_GET_INFO_MEMBER(get_zerozone_tile_info);
};
