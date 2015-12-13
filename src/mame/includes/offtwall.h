// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class offtwall_state : public atarigen_state
{
public:
	offtwall_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_vad(*this, "vad"),
			m_bankrom_base(*this, "bankrom_base") { }

	required_device<atari_jsa_iii_device> m_jsa;
	required_device<atari_vad_device> m_vad;

	UINT16 *m_bankswitch_base;
	required_shared_ptr<UINT16> m_bankrom_base;
	UINT32 m_bank_offset;

	UINT16 *m_spritecache_count;
	UINT16 *m_unknown_verify_base;
	virtual void update_interrupts() override;
	DECLARE_WRITE16_MEMBER(io_latch_w);
	DECLARE_READ16_MEMBER(bankswitch_r);
	DECLARE_READ16_MEMBER(bankrom_r);
	DECLARE_READ16_MEMBER(spritecache_count_r);
	DECLARE_READ16_MEMBER(unknown_verify_r);
	DECLARE_DRIVER_INIT(offtwall);
	DECLARE_DRIVER_INIT(offtwalc);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(offtwall);
	DECLARE_MACHINE_RESET(offtwall);
	DECLARE_VIDEO_START(offtwall);
	UINT32 screen_update_offtwall(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
