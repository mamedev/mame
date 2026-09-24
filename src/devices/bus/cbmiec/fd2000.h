// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD FD-2000/FD-4000 disk drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_FD2000_H
#define MAME_BUS_CBMIEC_FD2000_H

#pragma once

#include "cbmiec.h"
#include "cpu/m6502/w65c02.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/ds1215.h"
#include "machine/upd765.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fd2000_device

class fd2000_device :  public device_t,
					   public device_cbm_iec_interface
{
public:
	// construction/destruction
	fd2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t via_pa_r();
	void via_pa_w(uint8_t data);
	uint8_t via_pb_r();
	void via_pb_w(uint8_t data);
	uint8_t rtc_r(offs_t offset);
	void via_cb1_w(int state);
	void via_cb2_w(int state);

	static void floppy_formats(format_registration &fr);

protected:
	fd2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_cbm_iec_interface overrides
	void cbm_iec_srq(int state) override;
	void cbm_iec_atn(int state) override;
	void cbm_iec_data(int state) override;
	void cbm_iec_reset(int state) override;

	void add_common_devices(machine_config &config);

	required_device<w65c02_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<ds1216e_device> m_rtc;
	required_memory_region m_rom;
	required_device<upd765_family_device> m_fdc;
	output_finder<3> m_leds;
	required_ioport m_pb;

private:
	enum
	{
		LED_PWR = 0,
		LED_ACT,
		LED_ERR
	};

	void fd2000_mem(address_map &map) ATTR_COLD;

	bool m_fst_dir = 0;
	bool m_fst_clk = 0;
	bool m_fst_data = 0;
	bool m_atn_ack = 1;
	bool m_iec_atn = 1;
	bool m_iec_clk = 1;
	bool m_iec_data = 1;

	emu_timer *m_iec_sync_timer;
	TIMER_CALLBACK_MEMBER(iec_sync_tick);
};


// ======================> fd4000_device

class fd4000_device :  public fd2000_device
{
public:
	// construction/destruction
	fd4000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void mtr0_w(int state);

	void fd4000_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(FD2000, fd2000_device)
DECLARE_DEVICE_TYPE(FD4000, fd4000_device)


#endif // MAME_BUS_CBMIEC_FD2000_H
