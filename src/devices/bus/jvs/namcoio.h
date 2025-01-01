// license:BSD-3-Clause
// copyright-holders:smf

#ifndef MAME_BUS_JVS_NAMCOIO_H
#define MAME_BUS_JVS_NAMCOIO_H

#pragma once

#include "jvs.h"
#include "bus/rs232/rs232.h"
#include "cpu/f2mc16/mb90610a.h"
#include "cpu/h8/h83337.h"

class namco_amc_device :
	public device_t,
	public device_jvs_interface,
	public device_rs232_port_interface
{
public:
	namco_amc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void clear_output(int state);
	int unknown_output();

	static constexpr DEVICE_INPUT_DEFAULTS_START(dsw_rs232)
		DEVICE_INPUT_DEFAULTS("DSW", 0x08, 0x00)
	DEVICE_INPUT_DEFAULTS_END

protected:
	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_jvs_interface
	virtual void rxd(int state) override;

	// device_rs232_port_interface
	virtual void input_txd(int state) override;

	virtual void iocpu_program_map(address_map &map);

	template<int N> uint16_t adc_r();
	void update_analog_output0();
	template<unsigned N> void analog_output_w(uint8_t data);

	required_device<mb90611a_device> m_iocpu;
	std::array<uint8_t, 2> m_analog_output;
	uint8_t m_clear_output;
	uint8_t m_output;
	uint8_t m_ppg_duty;
	uint8_t m_ppg_flags;
	uint8_t m_sense_latch;
	uint8_t m_unknown_output;
};


DECLARE_DEVICE_TYPE(NAMCO_AMC, namco_amc_device)
DECLARE_DEVICE_TYPE(NAMCO_ASCA1, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_ASCA3, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_ASCA3A, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_ASCA5, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_CSZ1, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_EMIO102, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_EMPRI101, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_FCA10, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_FCA11, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_FCB, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_TSSIO, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_XMIU1, device_jvs_interface)

#endif
