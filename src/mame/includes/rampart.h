// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Rampart hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "sound/okim6295.h"
#include "video/atarimo.h"

class rampart_state : public atarigen_state
{
public:
	rampart_state(const machine_config &mconfig, device_type type, std::string tag)
		: atarigen_state(mconfig, type, tag),
			m_mob(*this, "mob"),
			m_oki(*this, "oki"),
			m_bitmap(*this, "bitmap") { }

	required_device<atari_motion_objects_device> m_mob;
	required_device<okim6295_device> m_oki;

	required_shared_ptr<UINT16> m_bitmap;

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_WRITE16_MEMBER(latch_w);
	DECLARE_DRIVER_INIT(rampart);
	DECLARE_MACHINE_START(rampart);
	DECLARE_MACHINE_RESET(rampart);
	DECLARE_VIDEO_START(rampart);
	UINT32 screen_update_rampart(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void rampart_bitmap_render(bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
