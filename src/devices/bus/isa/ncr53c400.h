// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_ISA_NCR53C400_H
#define MAME_BUS_ISA_NCR53C400_H

#pragma once

#include "isa.h"
#include "machine/ncr5380.h"


class isa8_rt1000b_device : public device_t, public device_isa8_card_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::DISK; }

	isa8_rt1000b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
private:
	void scsic_config(device_t *device);
	void irq_w(int state);
	void drq_w(int state);

	required_device<ncr53c80_device> m_scsic;
	required_ioport m_config;

	u8 m_irq_line;
	u32 m_config_address;

	u8 m_control;
	u8 m_block_counter;
	std::unique_ptr<u8[]> m_internal_sram;
	std::unique_ptr<u8[]> m_external_sram;
	int m_irq;
	int m_drq;

	u8 control_r(offs_t offset);
	void control_w(offs_t offset, u8 data);
	u8 block_counter_r(offs_t offset);
	void block_counter_w(offs_t offset, u8 data);
	u8 switch_r(offs_t offset);
	void resume_xfer_w(offs_t offset, u8 data);
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_RT1000B, isa8_rt1000b_device)

#endif // MAME_BUS_ISA_NCR53C400_H
