// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, MetalliC

#ifndef MAME_MACHINE_M95320_H
#define MAME_MACHINE_M95320_H

#pragma once

#include "machine/eeprom.h"

class m95320_eeprom_device : public device_t, public device_nvram_interface
{
public:
	m95320_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void    set_cs_line(int);
	void    set_halt_line(int state) {}; // TODO: not implemented
	void    set_si_line(int);
	void    set_sck_line(int state);
	int     get_so_line(void);

protected:
	m95320_eeprom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void device_start() override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
private:
	enum stmstate_t
	{
		IDLE = 0,
		CMD_WRSR,
		CMD_RDSR,
		CMD_READ,
		CMD_WRITE,
		READING,
		WRITING
	};
	static constexpr unsigned m_size = 0x1000;

	int     m_latch;
	int     m_reset_line;
	int     m_sck_line;
	int     m_wel;

	stmstate_t   m_internal_state;
	int     m_stream_pos;
	int     m_stream_data;
	int     m_eeprom_addr;
	std::unique_ptr<u8[]> m_eeprom_data;

};

DECLARE_DEVICE_TYPE(M95320_EEPROM, m95320_eeprom_device)


#endif // MAME_MACHINE_M95320_H
