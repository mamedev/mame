// license:BSD-3-Clause
// copyright-holders:R. Belmont,Curt Coder
/**********************************************************************

    Commodore MPS-1230 Dot Matrix Printer emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_MPS1230_H
#define MAME_BUS_CBMIEC_MPS1230_H

#pragma once

#include "cbmiec.h"

#include "bus/centronics/ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/bitmap_printer.h"
#include "machine/eepromser.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mps1230_device

class mps1230_device : public device_t, public device_cbm_iec_interface, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	mps1230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(local_changed);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;

	// device_cbm_iec_interface implementation
	virtual void cbm_iec_atn(int state) override;
	virtual void cbm_iec_clk(int state) override;
	virtual void cbm_iec_reset(int state) override;

	// device_centronics_peripheral_interface implementation
	virtual void input_strobe(int state) override;
	virtual void input_data0(int state) override { set_cent_data(0, state); }
	virtual void input_data1(int state) override { set_cent_data(1, state); }
	virtual void input_data2(int state) override { set_cent_data(2, state); }
	virtual void input_data3(int state) override { set_cent_data(3, state); }
	virtual void input_data4(int state) override { set_cent_data(4, state); }
	virtual void input_data5(int state) override { set_cent_data(5, state); }
	virtual void input_data6(int state) override { set_cent_data(6, state); }
	virtual void input_data7(int state) override { set_cent_data(7, state); }
	virtual void input_init(int state) override;

private:
	required_device<upd7810_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<bitmap_printer_device> m_bitmap_printer;
	required_device<stepper_device> m_pf_stepper;
	required_ioport m_panel;

	emu_timer *m_carriage_timer;

	u8 m_riloc;
	u8 m_pa;
	double m_position;
	double m_velocity;
	u8 m_sens0;
	u8 m_needle9;
	u8 m_cent_data;
	u8 m_cent_latch;
	int m_cent_strobe;
	int m_busin;

	u8 pa_r();
	void pa_w(u8 data);
	u8 pb_r();
	void pb_w(u8 data);
	u8 pc_r();
	void pc_w(u8 data);
	void busy_clear_w(u8 data);
	void set_cent_data(int bit, int state) { m_cent_data = (m_cent_data & ~(1 << bit)) | (state ? 1 << bit : 0); }
	bool has_iec() const { return m_bus != nullptr; }
	bool has_centronics() const { return device_centronics_peripheral_interface::m_slot != nullptr; }
	void needle9_w(int state);
	void fire_w(u8 data);

	TIMER_CALLBACK_MEMBER(carriage_tick);

	void mps1230_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MPS1230, mps1230_device)


#endif // MAME_BUS_CBMIEC_MPS1230_H
