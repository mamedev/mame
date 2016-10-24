// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "video/huc6260.h"
#include "video/huc6270.h"

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
		m_huc6260(*this, "huc6260"),
		m_soundlatch(*this, "soundlatch")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<screen_device> m_screen;
	required_device<huc6260_device> m_huc6260;
	required_device<generic_latch_8_device> m_soundlatch;

	int m_control_port_select;
	int m_msm5205next;
	int m_toggle;

	void sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void control_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t control_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void adpcm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_int(int state);

	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
