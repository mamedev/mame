// license:BSD-3-Clause
// copyright-holders:AJR,Curt Coder
/**********************************************************************

    Commodore MPS-1200 & MPS-1250 printers

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_MPS1200_H
#define MAME_BUS_CBMIEC_MPS1200_H

#pragma once

#include "cbmiec.h"
#include "bus/centronics/ctronics.h"
#include "cpu/m6502/m50734.h"
#include "machine/citizen120d.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mps1200_device

class mps1200_device : public device_t, public device_cbm_iec_interface
{
public:
	// construction/destruction
	mps1200_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	mps1200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_cbm_iec_interface overrides
	void cbm_iec_atn(int state) override;
	void cbm_iec_clk(int state) override;
	void cbm_iec_reset(int state) override;

	// the SW1 bit the firmware turns into device 4 or 5
	virtual u16 device_number_mask() const { return 0x0080; } // SW1:1

	// the front end IC14 and IC11 share, and the _INT1 sources the MPS-1250 multiplexes
	virtual u8 p1_r();
	virtual u8 p4_r();
	virtual void p0_w(u8 data);
	virtual void update_int1();

	required_device<m50734_device> m_mpscpu;
	required_ioport m_sw;
	required_ioport m_panel;
	required_device<citizen_120d_device> m_mech;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	u8 p0_r();
	u8 p2_r();
	void p2_w(u8 data);
	void p3_w(u8 data);
	u8 hsen_r();
	u8 vmon_r();
	u8 hthm_r();

	void dsck_w(int state);
	int dsdt_r();
	void sio_out_w(int state);

	TIMER_CALLBACK_MEMBER(eoi_timeout);

	emu_timer *m_eoi_timer = nullptr;

	u16 m_dip_shift = 0;
	bool m_dip_load = false;

	u8 m_shift = 0;
	u8 m_bit_count = 0;
	bool m_byte_ready = false;
	bool m_eoi = false;
	bool m_data_out = false;
	int m_clk_state = 1;

	// bits shifted out on DSDT; HDLD/MDLD latch the low 9/8 on their rising edge
	u16 m_mech_shift = 0;
	u8 m_p3_data = 0;
};


// ======================> mps1250_device

class mps1250_device : public mps1200_device, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	mps1250_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_centronics_peripheral_interface overrides
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

	virtual u16 device_number_mask() const override { return 0x0008; } // SW1:5

	virtual u8 p1_r() override;
	virtual u8 p4_r() override;
	virtual void p0_w(u8 data) override;
	virtual void update_int1() override;

private:
	// SW1:1 also reaches the board as IFSEL, which is what selects the front end
	bool ifsel() const { return !BIT(m_sw->read(), 7); }
	bool has_centronics() const { return device_centronics_peripheral_interface::m_slot != nullptr; }
	void set_cent_data(int bit, int state) { m_cent_data = (m_cent_data & ~(1 << bit)) | (state ? 1 << bit : 0); }

	u8 m_cent_data = 0xff;
	bool m_cent_strobe = true;
	bool m_cent_byte = false;
};


// device type declarations
DECLARE_DEVICE_TYPE(MPS1200, mps1200_device)
DECLARE_DEVICE_TYPE(MPS1250, mps1250_device)


#endif // MAME_BUS_CBMIEC_MPS1200_H
