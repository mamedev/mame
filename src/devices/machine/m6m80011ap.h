// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __M6M80011APDEV_H__
#define __M6M80011APDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

/* TODO: frequency */
#define MCFG_M6M80011AP_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, M6M80011AP, XTAL_32_768kHz)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

enum eeprom_cmd_t
{
	EEPROM_GET_CMD = 0,
	EEPROM_READ,
	EEPROM_WRITE,
	EEPROM_WRITE_ENABLE,
	EEPROM_WRITE_DISABLE,
	EEPROM_STATUS_OUTPUT
};


// ======================> m6m80011ap_device

class m6m80011ap_device :   public device_t,
							public device_nvram_interface
{
public:
	// construction/destruction
	m6m80011ap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	int read_bit();
	int ready_line();
	void set_cs_line(int state);
	void set_clock_line(int state);
	void write_bit(int state);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	uint8_t m_latch;
	uint8_t m_reset_line;
	uint8_t m_cmd_stream_pos;
	uint32_t m_current_cmd;
	uint8_t m_read_latch;
	uint8_t m_current_addr;
	uint8_t m_eeprom_we;

	eeprom_cmd_t m_eeprom_state;
	uint16_t m_eeprom_data[0x80];

};


// device type definition
extern const device_type M6M80011AP;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
