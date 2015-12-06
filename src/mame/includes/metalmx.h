// license:BSD-3-Clause
// copyright-holders:Philip Bennett
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/dsp32/dsp32.h"

class metalmx_state : public driver_device
{
public:
	metalmx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_gsp(*this, "gsp"),
			m_adsp(*this, "adsp"),
			m_dsp32c_1(*this, "dsp32c_1"),
			m_dsp32c_2(*this, "dsp32c_2"),
			m_cage(*this, "cage"),
		m_adsp_internal_program_ram(*this, "adsp_intprog"),
		m_gsp_dram(*this, "gsp_dram"),
		m_gsp_vram(*this, "gsp_vram"){ }

	required_device<m68ec020_device> m_maincpu;
	required_device<tms34020_device> m_gsp;
	required_device<adsp2105_device> m_adsp;
	required_device<dsp32c_device> m_dsp32c_1;
	required_device<dsp32c_device> m_dsp32c_2;
	required_device<atari_cage_device> m_cage;

	required_shared_ptr<UINT32> m_adsp_internal_program_ram;
	required_shared_ptr<UINT16> m_gsp_dram;
	required_shared_ptr<UINT16> m_gsp_vram;

	DECLARE_READ32_MEMBER(unk_r);
	DECLARE_READ32_MEMBER(watchdog_r);
	DECLARE_WRITE32_MEMBER(shifter_w);
	DECLARE_WRITE32_MEMBER(motor_w);
	DECLARE_WRITE32_MEMBER(reset_w);
	DECLARE_READ32_MEMBER(sound_data_r);
	DECLARE_WRITE32_MEMBER(sound_data_w);
	DECLARE_WRITE32_MEMBER(dsp32c_1_w);
	DECLARE_READ32_MEMBER(dsp32c_1_r);
	DECLARE_WRITE32_MEMBER(dsp32c_2_w);
	DECLARE_READ32_MEMBER(dsp32c_2_r);
	DECLARE_WRITE32_MEMBER(host_gsp_w);
	DECLARE_READ32_MEMBER(host_gsp_r);
	DECLARE_READ32_MEMBER(host_dram_r);
	DECLARE_WRITE32_MEMBER(host_dram_w);
	DECLARE_READ32_MEMBER(host_vram_r);
	DECLARE_WRITE32_MEMBER(host_vram_w);
	DECLARE_WRITE32_MEMBER(timer_w);
	DECLARE_DRIVER_INIT(metalmx);
	DECLARE_WRITE8_MEMBER(cage_irq_callback);
	DECLARE_WRITE_LINE_MEMBER(tms_interrupt);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_metalmx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
