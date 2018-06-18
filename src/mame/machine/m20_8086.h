// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_MACHINE_M20_8086_H
#define MAME_MACHINE_M20_8086_H

#include "cpu/i86/i86.h"
#include "machine/pic8259.h"

class m20_8086_device :  public device_t
{
public:
	m20_8086_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER(z8000_io_r);
	DECLARE_WRITE16_MEMBER(z8000_io_w);
	DECLARE_WRITE_LINE_MEMBER(vi_w);
	DECLARE_WRITE_LINE_MEMBER(nvi_w);
	DECLARE_WRITE16_MEMBER(handshake_w);

	void halt() { m_8086->set_input_line(INPUT_LINE_HALT, ASSERT_LINE); }
	bool halted() const { return m_8086_halt; }

	void i86_io(address_map &map);
	void i86_prog(address_map &map);
protected:
	void device_start() override;
	void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<cpu_device> m_8086;
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	bool m_8086_halt;
	int m_nvi, m_vi;

	IRQ_CALLBACK_MEMBER(int_cb);
};

DECLARE_DEVICE_TYPE(M20_8086, m20_8086_device)

#endif // MAME_MACHINE_M20_8086_H
