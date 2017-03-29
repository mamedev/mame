// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98034.h

    98034 module (HPIB interface)

*********************************************************************/

#pragma once

#ifndef _98034_H_
#define _98034_H_

#include "hp9845_io.h"
#include "cpu/nanoprocessor/nanoprocessor.h"
#include "bus/ieee488/ieee488.h"

class hp98034_io_card : public hp9845_io_card_device
{
public:
	// construction/destruction
	hp98034_io_card(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98034_io_card();

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_READ16_MEMBER(reg_r) override;
	virtual DECLARE_WRITE16_MEMBER(reg_w) override;

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

	IRQ_CALLBACK_MEMBER(irq_callback);

	DECLARE_WRITE_LINE_MEMBER(ieee488_ctrl_w);

private:
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

	void update_dc(void);
	void update_flg(void);
	void update_np_irq(void);
	void update_data_out(void);
	void update_ctrl_out(void);
	void update_clr_hpib(void);

};

// device type definition
extern const device_type HP98034_IO_CARD;

#endif /* _98034_H_ */
