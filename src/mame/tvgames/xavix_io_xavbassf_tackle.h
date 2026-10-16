// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_XAVBASSF_TACKLE_H
#define MAME_TVGAMES_XAVIX_IO_XAVBASSF_TACKLE_H

#pragma once

#include "cpu/mcs51/nrf24e1.h"
#include "machine/eepromspi.h"

class xavix_bassfishing_device : public device_t
{
public:
	xavix_bassfishing_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	nrf2401_device &radio() { return subdevice<nrf24e1_device>("mcu")->radio(); }

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;

private:
	u8 tackle_port_r();
	u16 tackle_adc_r(offs_t channel);
	void tackle_port_w(offs_t offset, u8 data, u8 mem_mask);
	void update_reaction();
	TIMER_CALLBACK_MEMBER(sample_inputs);

	required_device<nrf24e1_device> m_mcu;
	required_device<eeprom_spi_25320_device> m_eeprom;
	required_ioport m_controls;
	required_ioport m_rod;
	required_ioport m_reel;
	required_ioport m_power;
	required_ioport m_reaction_battery;
	output_finder<> m_motor_drive;
	output_finder<> m_reaction;
	emu_timer *m_input_timer;

	u8 m_last_reel;
	u8 m_reel_phase;
	s16 m_reel_pending;
	bool m_powered;
};

DECLARE_DEVICE_TYPE(XAVIX_BASSFISHING, xavix_bassfishing_device)

#endif // MAME_TVGAMES_XAVIX_IO_XAVBASSF_TACKLE_H
