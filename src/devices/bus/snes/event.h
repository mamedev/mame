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
	static constexpr device_timer_id TIMER_EVENT = 0;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void speedup_addon_bios_access() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_READ8_MEMBER(chip_read) override;
	virtual DECLARE_WRITE8_MEMBER(chip_write) override;

	virtual DECLARE_READ32_MEMBER(necdsp_prg_r);
	virtual DECLARE_READ16_MEMBER(necdsp_data_r);

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

	void dsp_data_map_lorom(address_map &map);
	void dsp_prg_map_lorom(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(SNS_PFEST94, sns_pfest94_device)

#endif // MAME_BUS_SNES_EVENT_H
