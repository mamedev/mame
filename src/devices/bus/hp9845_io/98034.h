// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98034.h

    98034 module (HPIB interface)

*********************************************************************/

#ifndef MAME_BUS_HP9845_IO_98034_H
#define MAME_BUS_HP9845_IO_98034_H

#pragma once

#include "hp9845_io.h"
#include "cpu/nanoprocessor/nanoprocessor.h"
#include "bus/ieee488/ieee488.h"

class hp98034_io_card_device : public device_t, public device_hp9845_io_interface
{
public:
	// construction/destruction
	hp98034_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98034_io_card_device();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint16_t reg_r(address_space &space, offs_t offset) override;
	virtual void reg_w(address_space &space, offs_t offset, uint16_t data) override;

private:
	void dc_w(uint8_t data);
	uint8_t dc_r();
	uint8_t int_ack_r();

	void hpib_data_w(uint8_t data);
	void hpib_ctrl_w(uint8_t data);
	uint8_t hpib_ctrl_r();
	uint8_t hpib_data_r();
	uint8_t idr_r();
	void odr_w(uint8_t data);
	uint8_t mode_reg_r();
	void mode_reg_clear_w(uint8_t data);
	uint8_t switch_r();

	void np_io_map(address_map &map) ATTR_COLD;
	void np_program_map(address_map &map) ATTR_COLD;

	void ieee488_ctrl_w(int state);

	required_device<hp_nanoprocessor_device> m_cpu;
	required_ioport m_sw1;
	required_device<ieee488_device> m_ieee488;

	// DC lines
	uint8_t m_dc;

	// Interface state
	uint8_t m_idr;  // Input Data Register
	uint8_t m_odr;  // Output Data Register
	bool m_force_flg;
	uint8_t m_mode_reg;
	bool m_flg;

	// 488 bus state
	bool m_clr_hpib;
	uint8_t m_ctrl_out;
	uint8_t m_data_out;

	void update_dc();
	bool update_flg();
	void update_np_irq();
	void update_data_out();
	void update_ctrl_out();
	void update_clr_hpib();

};

// device type definition
DECLARE_DEVICE_TYPE(HP98034_IO_CARD, hp98034_io_card_device)

#endif // MAME_BUS_HP9845_IO_98034_H
