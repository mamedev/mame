// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore D9060/D9090 Hard Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_D9060_H
#define MAME_BUS_IEEE488_D9060_H

#pragma once

#include "ieee488.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/mos6530.h"
#include "bus/scsi/scsi.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> d9060_device_base

class d9060_device_base : public device_t, public device_ieee488_interface
{
protected:
	enum
	{
		TYPE_9060,
		TYPE_9090
	};

	// construction/destruction
	d9060_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_ieee488_interface overrides
	void ieee488_atn(int state) override;
	void ieee488_ifc(int state) override;

private:
	inline void update_ieee_signals();

	uint8_t dio_r();
	void dio_w(uint8_t data);
	uint8_t riot1_pa_r();
	void riot1_pa_w(uint8_t data);
	uint8_t riot1_pb_r();
	void riot1_pb_w(uint8_t data);
	void via_pb_w(uint8_t data);
	void ack_w(int state);
	void enable_w(int state);
	void scsi_data_w(uint8_t data);

	required_device<m6502_device> m_maincpu;
	required_device<m6502_device> m_hdccpu;
	required_device<mos6532_device> m_riot0;
	required_device<mos6532_device> m_riot1;
	required_device<via6522_device> m_via;
	required_device<scsi_port_device> m_sasibus;
	required_device<output_latch_device> m_sasi_data_out;
	required_ioport m_address;
	output_finder<3> m_leds;

	// IEEE-488 bus
	int m_rfdo;                         // not ready for data output
	int m_daco;                         // not data accepted output
	int m_atna;                         // attention acknowledge
	int m_ifc;

	// SASI bus
	int m_enable;
	uint8_t m_data;

	int m_variant;
	void hdc_mem(address_map &map) ATTR_COLD;
	void main_mem(address_map &map) ATTR_COLD;
};


// ======================> d9060_device

class d9060_device : public d9060_device_base
{
public:
	// construction/destruction
	d9060_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> d9090_device

class d9090_device : public d9060_device_base
{
public:
	// construction/destruction
	d9090_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(D9060, d9060_device)
DECLARE_DEVICE_TYPE(D9090, d9090_device)

#endif // MAME_BUS_IEEE488_D9060_H
