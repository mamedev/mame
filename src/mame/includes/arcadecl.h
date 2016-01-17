// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Arcade Classics hardware (prototypes)

*************************************************************************/

#include "machine/atarigen.h"
#include "video/atarimo.h"

class arcadecl_state : public atarigen_state
{
public:
	arcadecl_state(const machine_config &mconfig, device_type type, std::string tag)
		: atarigen_state(mconfig, type, tag),
			m_mob(*this, "mob"),
			m_bitmap(*this, "bitmap") { }

	optional_device<atari_motion_objects_device> m_mob;
	required_shared_ptr<UINT16> m_bitmap;

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_WRITE16_MEMBER(latch_w);
	DECLARE_DRIVER_INIT(sparkz);
	DECLARE_MACHINE_START(arcadecl);
	DECLARE_MACHINE_RESET(arcadecl);
	DECLARE_VIDEO_START(arcadecl);
	UINT32 screen_update_arcadecl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void arcadecl_bitmap_render(bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
