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

	required_shared_ptr<uint32_t> m_adsp_internal_program_ram;
	required_shared_ptr<uint16_t> m_gsp_dram;
	required_shared_ptr<uint16_t> m_gsp_vram;

	uint32_t unk_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t watchdog_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void shifter_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void motor_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void reset_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t sound_data_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void sound_data_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void dsp32c_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t dsp32c_1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dsp32c_2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t dsp32c_2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void host_gsp_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t host_gsp_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t host_dram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void host_dram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t host_vram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void host_vram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void timer_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void init_metalmx();
	void cage_irq_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_metalmx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
