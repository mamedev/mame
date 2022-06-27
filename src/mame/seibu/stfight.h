// license:BSD-3-Clause
// copyright-holders:Mark McDougall
#ifndef MAME_INCLUDES_STFIGHT_H
#define MAME_INCLUDES_STFIGHT_H

#pragma once

#include "cpu/m6805/m68705.h"
#include "sound/ymopn.h"
#include "sound/msm5205.h"
#include "stfight_dev.h"
#include "airraid_dev.h"

class stfight_state : public driver_device
{
public:
	stfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_coin_mech(*this, "COIN")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_mcu(*this, "mcu")
		, m_msm(*this, "msm")
		, m_ym(*this, "ym%u", 0)
		, m_main_bank(*this, "mainbank")
		, m_samples(*this, "adpcm")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_coin_state(0)
		, m_fm_data(0)
		, m_cpu_to_mcu_empty(true)
		, m_cpu_to_mcu_data(0x0f)
		, m_port_a_out(0xff)
		, m_port_c_out(0xff)
		, m_vck2(false)
		, m_adpcm_reset(true)
		, m_adpcm_data_offs(0x0000)
	{
	}

	void stfight_base(machine_config &config);
	void stfight(machine_config &config);
	void cshooter(machine_config &config);

	void init_stfight();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	TIMER_CALLBACK_MEMBER(rst08_tick);

private:
	DECLARE_WRITE_LINE_MEMBER(stfight_adpcm_int);

	void stfight_io_w(uint8_t data);
	uint8_t stfight_coin_r();
	void stfight_coin_w(uint8_t data);
	void stfight_fm_w(uint8_t data);
	void stfight_mcu_w(uint8_t data);

	void stfight_bank_w(uint8_t data);

	uint8_t stfight_fm_r();

	INTERRUPT_GEN_MEMBER(stfight_vb_interrupt);

	// MCU specifics
	uint8_t stfight_68705_port_b_r();
	void stfight_68705_port_a_w(uint8_t data);
	void stfight_68705_port_b_w(uint8_t data);
	void stfight_68705_port_c_w(uint8_t data);

	void cpu1_map(address_map &map);
	void cpu2_map(address_map &map);
	void cshooter_cpu1_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
	void stfight_cpu1_map(address_map &map);

	required_ioport                  m_coin_mech;

	required_device<cpu_device>      m_maincpu;
	required_device<cpu_device>      m_audiocpu;
	required_device<m68705p5_device> m_mcu;
	required_device<msm5205_device>  m_msm;
	required_device_array<ym2203_device, 2> m_ym;

	required_memory_bank             m_main_bank;

	required_region_ptr<uint8_t>     m_samples;
	optional_shared_ptr<uint8_t>     m_decrypted_opcodes;

	uint8_t     m_coin_state = 0;

	uint8_t     m_fm_data = 0;

	bool        m_cpu_to_mcu_empty = false;
	uint8_t     m_cpu_to_mcu_data = 0;
	uint8_t     m_port_a_out = 0;
	uint8_t     m_port_c_out = 0;

	bool        m_vck2 = false;
	bool        m_adpcm_reset = false;
	uint16_t    m_adpcm_data_offs = 0;

	emu_timer   *m_int1_timer = nullptr;
};

#endif // MAME_INCLUDES_STFIGHT_H
