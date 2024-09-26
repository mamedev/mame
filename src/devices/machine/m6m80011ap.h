// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_MACHINE_M6M80011AP_H
#define MAME_MACHINE_M6M80011AP_H

#pragma once

class m6m80011ap_device :   public device_t,
							public device_nvram_interface
{
public:
	// construction/destruction
	m6m80011ap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768); /* TODO: frequency */

	// I/O operations
	int read_bit();
	int ready_line();
	void set_cs_line(int state);
	void set_clock_line(int state);
	void write_bit(int state);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	enum eeprom_cmd_t
	{
		EEPROM_GET_CMD = 0,
		EEPROM_READ,
		EEPROM_WRITE,
		EEPROM_WRITE_ENABLE,
		EEPROM_WRITE_DISABLE,
		EEPROM_STATUS_OUTPUT
	};

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

DECLARE_DEVICE_TYPE(M6M80011AP, m6m80011ap_device)

#endif // MAME_MACHINE_M6M80011AP_H
