// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1526/MPS-802/4023 Printer emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_C1526_H
#define MAME_BUS_CBMIEC_C1526_H

#pragma once

#include "cbmiec.h"
#include "bus/ieee488/ieee488.h"
#include "cpu/m6502/m6504.h"
#include "machine/mos6530.h"
#include "machine/6522via.h"
#include "machine/bitmap_printer.h"
#include "machine/input_merger.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1526_device_base

class c1526_device_base : public device_t
{
protected:
	// construction/destruction
	c1526_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override;
	void c1526_mem(address_map &map) ATTR_COLD;
	void add_common_mconfig(machine_config &config) ATTR_COLD;

	uint8_t riot1_pa_r();
	void riot1_pa_w(uint8_t data);
	void riot1_pb_w(uint8_t data);

	void via0_pa_w(uint8_t data);
	void via0_pb_w(uint8_t data);
	void fire_needles();

	TIMER_CALLBACK_MEMBER(cr_sensor_tick);
	void set_t_signal(int state);
	void update_cr_sensors(bool stepped);

	required_device<m6504_device> m_maincpu;
	required_device<mos6532_device> m_riot0;
	required_device<mos6532_device> m_riot1;
	required_device<via6522_device> m_via0;
	required_device<input_merger_device> m_irqs;
	required_device<bitmap_printer_device> m_bitmap_printer;
	required_device<stepper_device> m_pf_stepper;
	required_ioport m_paper_adv;

	emu_timer *m_cr_sensor_timer = nullptr;
	attotime m_last_cr_step = attotime::zero;

	uint8_t m_u5d_pa_out = 0x00;
	uint8_t m_u5d_pb_out = 0x00;
	uint8_t m_u6d_pa_out = 0x00;
	uint8_t m_u6d_pb_out = 0x01;
	int m_t_signal = 1;
	int m_home = 0;
};


// ======================> c1526_device

class c1526_device : public c1526_device_base, public device_cbm_iec_interface
{
public:
	// construction/destruction
	c1526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_cbm_iec_interface overrides
	void cbm_iec_atn(int state) override;
	void cbm_iec_data(int state) override;
	void cbm_iec_reset(int state) override;

private:
	uint8_t riot0_pa_r();
	void riot0_pa_w(uint8_t data);
	uint8_t riot0_pb_r();

	void update_iec_data();

	int m_attn_ack = 0;
	int m_nrfd = 0;
};


// ======================> c4023_device

class c4023_device : public c1526_device_base, public device_ieee488_interface
{
public:
	// construction/destruction
	c4023_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_ieee488_interface overrides
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ifc(int state) override;

private:
	uint8_t riot0_pa_r();
	void riot0_pa_w(uint8_t data);
	uint8_t riot0_pb_dio_r();
	void riot0_pb_dio_w(uint8_t data);

	void update_ieee();

	int m_rfd_flag = 0;
	int m_dac_flag = 0;
};


// device type definition
DECLARE_DEVICE_TYPE(C1526, c1526_device)
DECLARE_DEVICE_TYPE(GPIB_C4023, c4023_device)


#endif // MAME_BUS_CBMIEC_C1526_H
