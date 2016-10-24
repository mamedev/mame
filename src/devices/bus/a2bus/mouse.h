// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    mouse.h

    Implemention of the Apple II Mouse Card

*********************************************************************/

#ifndef __A2BUS_MOUSE__
#define __A2BUS_MOUSE__

#include "emu.h"
#include "a2bus.h"
#include "machine/6821pia.h"
#include "cpu/m6805/m6805.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_mouse_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_mouse_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	a2bus_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	uint8_t pia_in_a(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pia_in_b(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pia_out_a(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_out_b(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_irqa_w(int state);
	void pia_irqb_w(int state);

	uint8_t mcu_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mcu_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mcu_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_timer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_timer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(address_space &space, uint8_t offset) override;
	virtual void write_c0nx(address_space &space, uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(address_space &space, uint8_t offset) override;

	required_device<pia6821_device> m_pia;
	required_device<m68705_device> m_mcu;
	required_ioport m_mouseb, m_mousex, m_mousey;

private:
	uint8_t *m_rom;
	bool m_started;
	int m_rom_bank;
	uint8_t m_ddr_a;
	uint8_t m_ddr_b;
	uint8_t m_ddr_c;
	uint8_t m_port_a_out;
	uint8_t m_port_b_out;
	uint8_t m_port_c_out;
	uint8_t m_port_a_in;
	uint8_t m_port_b_in;
	uint8_t m_port_c_in;
	uint8_t m_timer_cnt;
	uint8_t m_timer_ctl;
	uint8_t m_mask_option;
	int last_mx, last_my, count_x, count_y;
	emu_timer *m_timer, *m_read_timer;
};

// device type definition
extern const device_type A2BUS_MOUSE;

#endif /* __A2BUS_MOUSE__ */
