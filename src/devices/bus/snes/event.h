// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SNES_EVENT_H
#define MAME_BUS_SNES_EVENT_H

#pragma once

#include "snes_slot.h"
#include "cpu/upd7725/upd7725.h"


// ======================> sns_pfest94_device

class sns_pfest94_device : public device_t,
						public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_pfest94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void speedup_addon_bios_access() override;

	// reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

	virtual uint32_t necdsp_prg_r(offs_t offset);
	virtual uint16_t necdsp_data_r(offs_t offset);

	TIMER_CALLBACK_MEMBER(event_tick);

private:
	required_device<upd7725_device> m_upd7725;
	required_ioport m_dsw;

	uint8_t m_base_bank;
	uint8_t m_mask;
	uint8_t m_status;
	uint32_t m_count;

	std::vector<uint32_t> m_dsp_prg;
	std::vector<uint16_t> m_dsp_data;

	emu_timer *pfest94_timer;

	void dsp_data_map_lorom(address_map &map) ATTR_COLD;
	void dsp_prg_map_lorom(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(SNS_PFEST94, sns_pfest94_device)

#endif // MAME_BUS_SNES_EVENT_H
