// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SPC1000_FDD_H__
#define __SPC1000_FDD_H__

#include "exp.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/upd765.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spc1000_fdd_exp_device

class spc1000_fdd_exp_device : public device_t,
						public device_spc1000_card_interface
{
public:
	// construction/destruction
	spc1000_fdd_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

public:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	uint8_t tc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8255_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8255_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8255_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	// internal state
	required_device<z80_device> m_cpu;
	required_device<upd765a_device> m_fdc;
	required_device<i8255_device> m_pio;

	floppy_image_device *m_fd0;
	floppy_image_device *m_fd1;

	emu_timer *m_timer_tc;

	uint8_t m_i8255_0_pc;
	uint8_t m_i8255_1_pc;
	uint8_t m_i8255_portb;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	static const device_timer_id TIMER_TC = 0;
};


// device type definition
extern const device_type SPC1000_FDD_EXP;

#endif  /* __SPC1000_FDD_H__ */
