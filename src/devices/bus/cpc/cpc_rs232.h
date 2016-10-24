// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpc_rs232.h
 *
 *  Created on: 22/04/2014
 */

#ifndef CPC_RS232_H_
#define CPC_RS232_H_

#include "emu.h"
#include "machine/z80dart.h"
#include "machine/pit8253.h"
#include "bus/rs232/rs232.h"
#include "cpcexp.h"

class cpc_rs232_device : public device_t,
							public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	cpc_rs232_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

			// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	void pit_out0_w(int state);
	void pit_out1_w(int state);
	void pit_out2_w(int state);

	uint8_t dart_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dart_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pit_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pit_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	required_device<pit8253_device> m_pit;
	required_device<z80dart_device> m_dart;
	required_device<rs232_port_device> m_rs232;
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	cpc_expansion_slot_device *m_slot;
};

class cpc_ams_rs232_device : public cpc_rs232_device
{
public:
	// construction/destruction
	cpc_ams_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
};

// device type definition
extern const device_type CPC_RS232;
extern const device_type CPC_RS232_AMS;

#endif /* CPC_RS232_H_ */
