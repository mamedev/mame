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

class hp98034_io_card_device : public hp9845_io_card_device
{
public:
	// construction/destruction
	hp98034_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98034_io_card_device();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual DECLARE_READ16_MEMBER(reg_r) override;
	virtual DECLARE_WRITE16_MEMBER(reg_w) override;

private:
	DECLARE_WRITE8_MEMBER(dc_w);
	DECLARE_READ8_MEMBER(dc_r);

	DECLARE_WRITE8_MEMBER(hpib_data_w);
	DECLARE_WRITE8_MEMBER(hpib_ctrl_w);
	DECLARE_READ8_MEMBER(hpib_ctrl_r);
	DECLARE_READ8_MEMBER(hpib_data_r);
	DECLARE_READ8_MEMBER(idr_r);
	DECLARE_WRITE8_MEMBER(odr_w);
	DECLARE_READ8_MEMBER(mode_reg_r);
	DECLARE_WRITE8_MEMBER(mode_reg_clear_w);
	DECLARE_READ8_MEMBER(switch_r);

	void np_io_map(address_map &map);
	void np_program_map(address_map &map);

	IRQ_CALLBACK_MEMBER(irq_callback);

	DECLARE_WRITE_LINE_MEMBER(ieee488_ctrl_w);

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

	// 488 bus state
	bool m_clr_hpib;
	uint8_t m_ctrl_out;
	uint8_t m_data_out;

	void update_dc();
	void update_flg();
	void update_np_irq();
	void update_data_out();
	void update_ctrl_out();
	void update_clr_hpib();

};

// device type definition
DECLARE_DEVICE_TYPE(HP98034_IO_CARD, hp98034_io_card_device)

#endif // MAME_BUS_HP9845_IO_98034_H
