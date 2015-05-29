// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "sound/msm5205.h"
#include "video/huc6260.h"
#include "video/huc6270.h"
#include "video/huc6202.h"

#define MAIN_CLOCK      21477270

class battlera_state : public driver_device
{
public:
	battlera_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_screen(*this, "screen"),
		m_huc6260(*this, "huc6260")
		{ }

	int m_control_port_select;
	int m_msm5205next;
	int m_toggle;
	int m_inc_value;
	int m_irq_enable;
	int m_rcr_enable;
	int m_sb_enable;
	int m_bb_enable;
	int m_bldwolf_vblank;

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE8_MEMBER(battlera_sound_w);
	DECLARE_WRITE8_MEMBER(control_data_w);
	DECLARE_READ8_MEMBER(control_data_r);
	DECLARE_WRITE8_MEMBER(battlera_adpcm_data_w);

	DECLARE_WRITE8_MEMBER(battlera_adpcm_reset_w);


	DECLARE_WRITE_LINE_MEMBER(battlera_adpcm_int);
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<screen_device> m_screen;
	optional_device<huc6260_device> m_huc6260;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(pce_irq_changed);
};
