// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpc_rs232.h
 *
 *  Created on: 22/04/2014
 */

#ifndef MAME_BUS_CPC_CPC_RS232_H
#define MAME_BUS_CPC_CPC_RS232_H

#pragma once

#include "cpcexp.h"
#include "machine/z80sio.h"
#include "machine/pit8253.h"
#include "bus/rs232/rs232.h"

class cpc_rs232_device : public device_t, public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t dart_r(offs_t offset);
	void dart_w(offs_t offset, uint8_t data);
	uint8_t pit_r(offs_t offset);
	void pit_w(offs_t offset, uint8_t data);

protected:
	cpc_rs232_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<pit8253_device> m_pit;
	required_device<z80dart_device> m_dart;
	required_device<rs232_port_device> m_rs232;

private:
	void pit_out0_w(int state);
	void pit_out1_w(int state);
	void pit_out2_w(int state);

	cpc_expansion_slot_device *m_slot;
};

class cpc_ams_rs232_device : public cpc_rs232_device
{
public:
	// construction/destruction
	cpc_ams_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_RS232,     cpc_rs232_device)
DECLARE_DEVICE_TYPE(CPC_RS232_AMS, cpc_ams_rs232_device)

#endif // MAME_BUS_CPC_CPC_RS232_H
