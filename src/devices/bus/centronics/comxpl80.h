// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX PL-80 plotter emulation

**********************************************************************/

#pragma once

#ifndef __COMX_PL80__
#define __COMX_PL80__

#include "emu.h"
#include "cpu/m6805/m6805.h"
#include "bus/centronics/ctronics.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_pl80_device

class comx_pl80_device :  public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	comx_pl80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// not really public
	void pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void input_data0(int state) override { if (state) m_data |= 0x01; else m_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_data |= 0x02; else m_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_data |= 0x04; else m_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_data |= 0x08; else m_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_data |= 0x10; else m_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_data |= 0x20; else m_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_data |= 0x40; else m_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_data |= 0x80; else m_data &= ~0x80; }

private:
	required_memory_region m_plotter;
	required_ioport m_font;
	required_ioport m_sw;

	// PL-80 plotter state
	uint16_t m_font_addr;         // font ROM pack address latch
	uint8_t m_x_motor_phase;      // X motor phase
	uint8_t m_y_motor_phase;      // Y motor phase
	uint8_t m_z_motor_phase;      // Z motor phase
	uint8_t m_plotter_data;       // plotter data bus
	int m_plotter_ack;          // plotter acknowledge
	int m_plotter_online;       // online LED

	uint8_t m_data;
};


// device type definition
extern const device_type COMX_PL80;



#endif
