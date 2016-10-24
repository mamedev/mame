// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef M24_Z8000_H_
#define M24_Z8000_H_

#include "emu.h"
#include "cpu/z8000/z8000.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"

#define MCFG_M24_Z8000_HALT(_devcb) \
	devcb = &m24_z8000_device::set_halt_callback(*device, DEVCB_##_devcb);

class m24_z8000_device :  public device_t
{
public:
	m24_z8000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	template<class _Object> static devcb_base &set_halt_callback(device_t &device, _Object object) { return downcast<m24_z8000_device &>(device).m_halt_out.set_callback(object); }

	uint16_t pmem_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pmem_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dmem_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dmem_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t i86_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void i86_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void irqctl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void serctl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t handshake_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void handshake_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mo_w(int state);
	void timer_irq_w(int state);
	int int_cb(device_t &device, int irqline);
	bool halted() { return m_z8000_halt; }

	required_device<z8001_device> m_z8000;
protected:
	void device_start() override;
	void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	devcb_write_line m_halt_out;
	static const uint8_t pmem_table[16][4];
	static const uint8_t dmem_table[16][4];
	uint8_t m_handshake, m_irq;
	bool m_z8000_halt, m_z8000_mem, m_timer_irq;
};

extern const device_type M24_Z8000;

#endif /* M24_Z8000_H_ */
