// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, MetalliC
#ifndef MAME_BUS_MEGADRIVE_STM95_H
#define MAME_BUS_MEGADRIVE_STM95_H

#pragma once

#include "md_slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* ST M95320 32Kbit serial EEPROM implementation */
// TO DO: STM95 should be made a separate EEPROM device and this should be merged with md_eeprom.c!

class stm95_eeprom_device
{
public:
	static constexpr unsigned M95320_SIZE = 0x1000;

	stm95_eeprom_device(running_machine &machine, uint8_t *eeprom);
	running_machine &machine() const { return m_machine; }

	uint8_t   *eeprom_data;
	void    set_cs_line(int);
	void    set_halt_line(int state) {}; // not implemented
	void    set_si_line(int);
	void    set_sck_line(int state);
	int     get_so_line(void);

protected:
	enum STMSTATE
	{
		IDLE = 0,
		CMD_WRSR,
		CMD_RDSR,
		M95320_CMD_READ,
		CMD_WRITE,
		READING,
		WRITING
	};

	int     latch;
	int     reset_line;
	int     sck_line;
	int     WEL;

	STMSTATE    stm_state;
	int     stream_pos;
	int     stream_data;
	int     eeprom_addr;

	running_machine& m_machine;
};


// ======================> md_eeprom_stm95_device

class md_eeprom_stm95_device : public device_t, public device_md_cart_interface
{
public:
	// construction/destruction
	md_eeprom_stm95_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual uint16_t read_a13(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	md_eeprom_stm95_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_bank[3];
	int m_rdcnt;

	std::unique_ptr<stm95_eeprom_device> m_stm95;
};


// device type definition
DECLARE_DEVICE_TYPE(MD_EEPROM_STM95, md_eeprom_stm95_device)

#endif // MAME_BUS_MEGADRIVE_STM95_H
