// license:BSD-3-Clause
// copyright-holders:Golden Child
/**********************************************************************

    Epson RX-80 dot matrix printer emulation (skeleton)

**********************************************************************/

#ifndef MAME_BUS_CENTRONICS_EPSON_RX80_H
#define MAME_BUS_CENTRONICS_EPSON_RX80_H

#pragma once

#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> epson_rx80_device

class epson_rx80_device :  public device_t, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_rx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	/* Centronics stuff */
	virtual void input_init(int state) override;
	virtual void input_strobe(int state) override;
	virtual void input_data0(int state) override { if (state) m_centronics_data |= 0x01; else m_centronics_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_centronics_data |= 0x02; else m_centronics_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_centronics_data |= 0x04; else m_centronics_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_centronics_data |= 0x08; else m_centronics_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_centronics_data |= 0x10; else m_centronics_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_centronics_data |= 0x20; else m_centronics_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_centronics_data |= 0x40; else m_centronics_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_centronics_data |= 0x80; else m_centronics_data &= ~0x80; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual bool supports_pin35_5v() override { return true; }

private:
	uint8_t porta_r();
	uint8_t portb_r();
	uint8_t portc_r();
	uint8_t portd_r();
	uint8_t portf_r();
	void porta_w(uint8_t data);
	void portb_w(uint8_t data);
	void portc_w(uint8_t data);
	void portd_w(uint8_t data);
	void portf_w(uint8_t data);
	uint8_t an0_r();
	uint8_t an1_r();
	uint8_t an2_r();
	uint8_t an3_r();
	uint8_t an4_r();
	uint8_t an5_r();
	uint8_t an6_r();
	uint8_t an7_r();

	uint8_t centronics_data_r(offs_t offset) { return m_centronics_data; };

	void epson_rx80_mem(address_map &map) ATTR_COLD;

	required_device<upd7810_device> m_maincpu;

	uint8_t m_centronics_data;
};

// device type definition
DECLARE_DEVICE_TYPE(EPSON_RX80, epson_rx80_device)

#endif // MAME_BUS_CENTRONICS_EPSON_RX80_H
