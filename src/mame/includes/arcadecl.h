/*************************************************************************

    Atari Arcade Classics hardware (prototypes)

*************************************************************************/


#include "machine/atarigen.h"

class arcadecl_state : public atarigen_state
{
public:
	arcadecl_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_bitmap(*this, "bitmap") { }

	required_shared_ptr<UINT16> m_bitmap;
	UINT8           m_has_mo;
	virtual void update_interrupts();
	virtual void scanline_update(screen_device &screen, int scanline);
	DECLARE_WRITE16_MEMBER(latch_w);
	DECLARE_DRIVER_INIT(sparkz);
	DECLARE_MACHINE_START(arcadecl);
	DECLARE_MACHINE_RESET(arcadecl);
	DECLARE_VIDEO_START(arcadecl);
	UINT32 screen_update_arcadecl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
