// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SNS_EVENT_H
#define __SNS_EVENT_H

#include "snes_slot.h"
#include "cpu/upd7725/upd7725.h"


// ======================> sns_pfest94_device

class sns_pfest94_device : public device_t,
						public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_pfest94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	required_device<upd7725_device> m_upd7725;
	required_ioport m_dsw;

	virtual void speedup_addon_bios_access() override;

	// reading and writing
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t chip_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual uint32_t necdsp_prg_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	virtual uint16_t necdsp_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

private:
	uint8_t m_base_bank;
	uint8_t m_mask;
	uint8_t m_status;
	uint32_t m_count;

	std::vector<uint32_t> m_dsp_prg;
	std::vector<uint16_t> m_dsp_data;

	static const device_timer_id TIMER_EVENT = 0;
	emu_timer *pfest94_timer;
};


// device type definition
extern const device_type SNS_PFEST94;

#endif
