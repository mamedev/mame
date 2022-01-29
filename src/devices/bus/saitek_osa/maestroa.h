// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Saitek OSA Module: Kasparov Maestro A

***************************************************************************/

#ifndef MAME_BUS_SAITEKOSA_MAESTROA_H
#define MAME_BUS_SAITEKOSA_MAESTROA_H

#pragma once

#include "expansion.h"

DECLARE_DEVICE_TYPE(OSA_MAESTROA, saitekosa_maestroa_device)


class saitekosa_maestroa_device : public device_t, public device_saitekosa_expansion_interface
{
public:
	// construction/destruction
	saitekosa_maestroa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(switch_cpu_freq) { set_cpu_freq(); }

	// from host
	virtual u8 data_r() override;
	virtual void nmi_w(int state) override;
	virtual void ack_w(int state) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;

	void main_map(address_map &map);

	u8 rts_r();
	u8 xdata_r();
	void xdata_w(u8 data);
	u8 ack_r();
	void control_w(u8 data);

	void set_cpu_freq();

	u8 m_latch = 0xff;
	bool m_latch_enable = false;
};


#endif // MAME_BUS_SAITEKOSA_MAESTROA_H
