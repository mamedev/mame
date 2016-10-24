// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    IBM-PC printer interface

***************************************************************************/

#ifndef __PC_LPT_H__
#define __PC_LPT_H__

#include "bus/centronics/ctronics.h"

#define MCFG_PC_LPT_IRQ_HANDLER(_devcb) \
	devcb = &pc_lpt_device::set_irq_handler(*device, DEVCB_##_devcb);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class pc_lpt_device : public device_t
{
public:
	pc_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<pc_lpt_device &>(device).m_irq_handler.set_callback(object); }

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void write_irq_enabled(int state);
	void write_centronics_ack(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	void update_irq();

	enum
	{
		CONTROL_STROBE = 1,
		CONTROL_AUTOFD = 2,
		CONTROL_INIT = 4,
		CONTROL_SELECT = 8,
		CONTROL_IRQ_ENABLED = 16,
		CONTROL_OUTPUT_ENABLED = 32
	};

	enum
	{
		STATUS_FAULT = 8,
		STATUS_SELECT = 16,
		STATUS_PERROR = 32,
		STATUS_ACK = 64,
		STATUS_BUSY = 128
	};

	// internal state

	int m_irq;
	uint8_t m_data;
	uint8_t m_control;
	int m_irq_enabled;
	int m_centronics_ack;

	devcb_write_line m_irq_handler;
	required_device<input_buffer_device> m_cent_data_in;
	required_device<output_latch_device> m_cent_data_out;
	required_device<input_buffer_device> m_cent_status_in;
	required_device<input_buffer_device> m_cent_ctrl_in;
	required_device<output_latch_device> m_cent_ctrl_out;
};

extern const device_type PC_LPT;

#endif /* __PC_LPT__ */
