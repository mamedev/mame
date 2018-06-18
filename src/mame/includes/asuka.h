// license:BSD-3-Clause
// copyright-holders:David Graves, Brian Troha
/*************************************************************************

    Asuka & Asuka  (+ Taito/Visco games on similar hardware)

*************************************************************************/

#ifndef MAME_INCLUDES_ASUKA_H
#define MAME_INCLUDES_ASUKA_H

#pragma once


#include "machine/taitocchip.h"
#include "machine/taitoio.h"

#include "sound/msm5205.h"
#include "machine/74157.h"
#include "video/pc090oj.h"
#include "video/tc0100scn.h"
#include "video/tc0110pcr.h"
#include "machine/timer.h"


class asuka_state : public driver_device
{
public:
	enum
	{
		TIMER_CADASH_INTERRUPT5
	};

	asuka_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cadash_shared_ram(*this, "sharedram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_cchip(*this, "cchip")
		, m_msm(*this, "msm")
		, m_adpcm_select(*this, "adpcm_select")
		, m_sound_data(*this, "ymsnd")
		, m_pc090oj(*this, "pc090oj")
		, m_tc0100scn(*this, "tc0100scn")
		, m_tc0110pcr(*this, "tc0110pcr")
		, m_tc0220ioc(*this, "tc0220ioc")
		, m_cchip_irq_clear(*this, "cchip_irq_clear")
		, m_audiobank(*this, "audiobank")
	{ }

	DECLARE_WRITE8_MEMBER(coin_control_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(asuka_msm5205_address_w);
	DECLARE_READ16_MEMBER(cadash_share_r);
	DECLARE_WRITE16_MEMBER(cadash_share_w);
	DECLARE_WRITE16_MEMBER(asuka_spritectrl_w);
	DECLARE_WRITE8_MEMBER(asuka_msm5205_start_w);
	DECLARE_WRITE8_MEMBER(asuka_msm5205_stop_w);
	DECLARE_WRITE8_MEMBER(counters_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_bonzeadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_asuka(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_asuka);
	INTERRUPT_GEN_MEMBER(cadash_interrupt);
	void init_cadash();
	INTERRUPT_GEN_MEMBER(bonze_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(cchip_irq_clear_cb);

	DECLARE_WRITE_LINE_MEMBER(asuka_msm5205_vck);

	void mofflott(machine_config &config);
	void asuka(machine_config &config);
	void cadash(machine_config &config);
	void eto(machine_config &config);
	void bonzeadv(machine_config &config);
	void asuka_map(address_map &map);
	void bonzeadv_map(address_map &map);
	void bonzeadv_z80_map(address_map &map);
	void cadash_map(address_map &map);
	void cadash_sub_io(address_map &map);
	void cadash_sub_map(address_map &map);
	void cadash_z80_map(address_map &map);
	void eto_map(address_map &map);
	void z80_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	/* video-related */
	u16         m_video_ctrl;
	u16         m_video_mask;

	/* misc */
	u16         m_adpcm_pos;
	bool        m_adpcm_ff;

	emu_timer *m_cadash_int5_timer;

	optional_shared_ptr<uint8_t> m_cadash_shared_ram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito_cchip_device> m_cchip;
	optional_device<msm5205_device> m_msm;
	optional_device<ls157_device> m_adpcm_select;
	optional_region_ptr<u8> m_sound_data;
	required_device<pc090oj_device> m_pc090oj;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0110pcr_device> m_tc0110pcr;
	optional_device<tc0220ioc_device> m_tc0220ioc;

	optional_device<timer_device> m_cchip_irq_clear;

	required_memory_bank m_audiobank;
};

#endif // MAME_INCLUDES_ASUKA_H
