// license:BSD-3-Clause
// copyright-holders:Devin Acker

#ifndef MAME_BUS_CENTRONICS_PC6022_H
#define MAME_BUS_CENTRONICS_PC6022_H

#pragma once

#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/input_merger.h"

class pc6022_device : public device_t,
					public device_centronics_peripheral_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	// construction/destruction
	pc6022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ack_w(int state);
	void pen_ctrl_w(int state);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void input_strobe(int state) override;
	virtual void input_data0(int state) override { if (state) m_data |= 0x01; else m_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_data |= 0x02; else m_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_data |= 0x04; else m_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_data |= 0x08; else m_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_data |= 0x10; else m_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_data |= 0x20; else m_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_data |= 0x40; else m_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_data |= 0x80; else m_data &= ~0x80; }

private:
	required_device<upd7801_device> m_cpu;
	required_device<input_merger_any_high_device> m_busy;

	void io_map(address_map &map) ATTR_COLD;

	u8 data_r();

	u8 m_data;
	u8 m_pen_ctrl;
	u8 m_pen_down;
	u8 m_ack;
	u8 m_strobe;
};

// device type definition
DECLARE_DEVICE_TYPE(PC6022, pc6022_device)

#endif // MAME_BUS_CENTRONICS_PC6022_H
