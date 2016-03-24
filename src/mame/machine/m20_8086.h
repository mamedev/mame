// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef M20_8086_H_
#define M20_8086_H_

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pic8259.h"

class m20_8086_device :  public device_t
{
public:
	m20_8086_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_READ16_MEMBER(z8000_io_r);
	DECLARE_WRITE16_MEMBER(z8000_io_w);
	DECLARE_WRITE_LINE_MEMBER(vi_w);
	DECLARE_WRITE_LINE_MEMBER(nvi_w);
	DECLARE_WRITE16_MEMBER(handshake_w);
	IRQ_CALLBACK_MEMBER(int_cb);
	bool halted() { return m_8086_halt; }
	required_device<cpu_device> m_8086;

protected:
	void device_start() override;
	void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	bool m_8086_halt;
	int m_nvi, m_vi;
};

extern const device_type M20_8086;

#endif /* M20_8086_H_ */
