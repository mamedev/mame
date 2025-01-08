// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_OLIVETTI_M20_8086_H
#define MAME_OLIVETTI_M20_8086_H

#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
#include "machine/ram.h"

class m20_8086_device :  public device_t
{
public:
	template <typename T, typename U, typename V>
	m20_8086_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&maincpu_tag, U &&pic_tag, V &&ram_tag)
		: m20_8086_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_maincpu.set_tag(std::forward<T>(maincpu_tag));
		m_pic.set_tag(std::forward<U>(pic_tag));
		m_ram.set_tag(std::forward<V>(ram_tag));
	}

	m20_8086_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t z8000_io_r(offs_t offset, uint16_t mem_mask = ~0);
	void z8000_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vi_w(int state);
	void nvi_w(int state);
	void handshake_w(offs_t offset, uint16_t data);

	void halt() { m_8086->set_input_line(INPUT_LINE_HALT, ASSERT_LINE); }
	bool halted() const { return m_8086_halt; }

	void i86_io(address_map &map) ATTR_COLD;
	void i86_prog(address_map &map) ATTR_COLD;
protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cpu_device> m_8086;
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<ram_device> m_ram;
	bool m_8086_halt;
	int m_nvi = 0, m_vi = 0;

	IRQ_CALLBACK_MEMBER(int_cb);
};

DECLARE_DEVICE_TYPE(M20_8086, m20_8086_device)

#endif // MAME_OLIVETTI_M20_8086_H
