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
			m_mainram(*this, "mainram"),
			m_bankrom_base(*this, "bankrom_base") { }

	required_device<atari_jsa_iii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_shared_ptr<uint16_t> m_mainram;

	uint16_t *m_bankswitch_base;
	required_shared_ptr<uint16_t> m_bankrom_base;
	uint32_t m_bank_offset;

	uint16_t *m_spritecache_count;
	uint16_t *m_unknown_verify_base;
	virtual void update_interrupts() override;
	DECLARE_WRITE16_MEMBER(io_latch_w);
	DECLARE_READ16_MEMBER(bankswitch_r);
	DECLARE_READ16_MEMBER(bankrom_r);
	DECLARE_READ16_MEMBER(spritecache_count_r);
	DECLARE_READ16_MEMBER(unknown_verify_r);
	void init_offtwall();
	void init_offtwalc();
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	void machine_start_offtwall();
	void machine_reset_offtwall();
	void video_start_offtwall();
	uint32_t screen_update_offtwall(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
