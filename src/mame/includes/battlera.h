// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "sound/msm5205.h"
#include "video/huc6260.h"
#include "video/huc6270.h"

#define MAIN_CLOCK      21477270

class battlera_state : public driver_device
{
public:
	battlera_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_screen(*this, "screen"),
		m_huc6260(*this, "huc6260")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<screen_device> m_screen;
	required_device<huc6260_device> m_huc6260;

	int m_control_port_select;
	int m_msm5205next;
	int m_toggle;

	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(control_data_w);
	DECLARE_READ8_MEMBER(control_data_r);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE8_MEMBER(adpcm_reset_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	virtual void machine_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
