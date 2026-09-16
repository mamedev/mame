// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_CENTRONICS_NEOMANIA_ADAPTER_H
#define MAME_BUS_CENTRONICS_NEOMANIA_ADAPTER_H

#pragma once

#include "ctronics.h"

class neomania_adapter_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	neomania_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_strobe(int state) override;

	virtual void input_data0(int state) override { if (state) m_ddr |= 0x01; else m_ddr &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_ddr |= 0x02; else m_ddr &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_ddr |= 0x04; else m_ddr &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_ddr |= 0x08; else m_ddr &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_ddr |= 0x10; else m_ddr &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_ddr |= 0x20; else m_ddr &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_ddr |= 0x40; else m_ddr &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_ddr |= 0x80; else m_ddr &= ~0x80; }
//  virtual void input_ack(int state) override { printf("input_ack %d\n", state); }
//  virtual void input_busy(int state)  override { printf("input_busy %d\n", state); }
//  virtual void input_perror(int state) override { printf("input_perror %d\n", state); }
//  virtual void input_select(int state) override { printf("input_select %d\n", state); }
	virtual void input_autofd(int state) override;
//  virtual void input_fault(int state) override { printf("input_fault %d\n", state); }
	virtual void input_init(int state) override;
	virtual void input_select_in(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport_array<8> m_in;

	int m_select_in, m_init, m_strobe, m_autofd;
	bool m_has_inited;
	u8 m_ddr;

	void update_ack();
};


DECLARE_DEVICE_TYPE(NEOMANIA_ADAPTER, neomania_adapter_device)

#endif // MAME_BUS_CENTRONICS_NEOMANIA_ADAPTER_H
