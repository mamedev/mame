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
	m24_z8000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	template<class _Object> static devcb_base &set_halt_callback(device_t &device, _Object object) { return downcast<m24_z8000_device &>(device).m_halt_out.set_callback(object); }

	DECLARE_READ16_MEMBER(pmem_r);
	DECLARE_WRITE16_MEMBER(pmem_w);
	DECLARE_READ16_MEMBER(dmem_r);
	DECLARE_WRITE16_MEMBER(dmem_w);
	DECLARE_READ16_MEMBER(i86_io_r);
	DECLARE_WRITE16_MEMBER(i86_io_w);
	DECLARE_WRITE8_MEMBER(irqctl_w);
	DECLARE_WRITE8_MEMBER(serctl_w);
	DECLARE_READ8_MEMBER(handshake_r);
	DECLARE_WRITE8_MEMBER(handshake_w);
	DECLARE_WRITE_LINE_MEMBER(mo_w);
	DECLARE_WRITE_LINE_MEMBER(timer_irq_w);
	IRQ_CALLBACK_MEMBER(int_cb);
	bool halted() { return m_z8000_halt; }

	required_device<z8001_device> m_z8000;
protected:
	void device_start();
	void device_reset();

private:
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	devcb_write_line m_halt_out;
	static const UINT8 pmem_table[16][4];
	static const UINT8 dmem_table[16][4];
	UINT8 m_handshake, m_irq;
	bool m_z8000_halt, m_z8000_mem, m_timer_irq;
};

extern const device_type M24_Z8000;

#endif /* M24_Z8000_H_ */
