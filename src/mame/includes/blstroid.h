// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Blasteroids hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class blstroid_state : public atarigen_state
{
public:
	enum
	{
		TIMER_IRQ_OFF = TID_ATARIGEN_LAST,
		TIMER_IRQ_ON
	};

	blstroid_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_playfield_tilemap(*this, "playfield"),
			m_jsa(*this, "jsa"),
			m_mob(*this, "mob"),
			m_priorityram(*this, "priorityram") { }

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<atari_jsa_i_device> m_jsa;
	required_device<atari_motion_objects_device> m_mob;
	required_shared_ptr<UINT16> m_priorityram;

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_WRITE16_MEMBER(blstroid_halt_until_hblank_0_w);
	DECLARE_DRIVER_INIT(blstroid);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(blstroid);
	DECLARE_MACHINE_RESET(blstroid);
	DECLARE_VIDEO_START(blstroid);
	UINT32 screen_update_blstroid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
