// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore D9060/D9090 Hard Disk Drive emulation

**********************************************************************/

#pragma once

#ifndef __D9060__
#define __D9060__

#include "emu.h"
#include "ieee488.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/mos6530n.h"
#include "bus/scsi/scsi.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> d9060_base_t

class d9060_base_t :  public device_t,
						public device_ieee488_interface
{
public:
	enum
	{
		TYPE_9060,
		TYPE_9090
	};

	// construction/destruction
	d9060_base_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, uint32_t variant, const char *shortname, const char *source);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// not really public
	uint8_t dio_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t riot1_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void riot1_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t riot1_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void riot1_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void via_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ack_w(int state);
	void enable_w(int state);
	void scsi_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_ieee488_interface overrides
	void ieee488_atn(int state) override;
	void ieee488_ifc(int state) override;

private:
	inline void update_ieee_signals();

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_hdccpu;
	required_device<mos6532_t> m_riot0;
	required_device<mos6532_t> m_riot1;
	required_device<via6522_device> m_via;
	required_device<SCSI_PORT_DEVICE> m_sasibus;
	required_device<output_latch_device> m_sasi_data_out;
	required_ioport m_address;

	// IEEE-488 bus
	int m_rfdo;                         // not ready for data output
	int m_daco;                         // not data accepted output
	int m_atna;                         // attention acknowledge
	int m_ifc;

	// SASI bus
	int m_enable;
	uint8_t m_data;

	int m_variant;
};


// ======================> d9060_t

class d9060_t :  public d9060_base_t
{
public:
	// construction/destruction
	d9060_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> d9090_t

class d9090_t :  public d9060_base_t
{
public:
	// construction/destruction
	d9090_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
extern const device_type D9060;
extern const device_type D9090;



#endif
