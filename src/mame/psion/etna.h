// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Ash Wolf
/***************************************************************************

        Psion 5mx (EPOC R5) series ETNA peripheral

        Skeleton driver by Ryan Holtz, ported from work by Ash Wolf

****************************************************************************/

#ifndef MAME_PSION_ETNA_H
#define MAME_PSION_ETNA_H

#pragma once

#include "emu.h"

class etna_device : public device_t
{
public:
	etna_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	void regs_w(offs_t offset, uint8_t data);
	uint8_t regs_r(offs_t offset);

	void eeprom_cs_in(int state);
	void eeprom_clk_in(int state);
	void eeprom_data_in(int state);

	auto eeprom_data_out() { return m_eeprom_data_out.bind(); }

private:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	enum
	{
		REG_UNK0,
		REG_UNK1,
		REG_UART_INT_STATUS,
		REG_UART_INT_MASK,
		REG_UART_BAUD_LO,
		REG_UART_BAUD_HI,
		REG_PCCD_INT_STATUS,
		REG_PCCD_INT_MASK,
		REG_INT_CLEAR,
		REG_SKT_VAR_A0,
		REG_SKT_VAR_A1,
		REG_SKT_CTRL,
		REG_WAKE1,
		REG_SKT_VAR_B0,
		REG_SKT_VAR_B1,
		REG_WAKE2
	};

	devcb_write_line m_eeprom_data_out;

	uint8_t m_prom_addr_count;
	uint16_t m_prom_addr;
	uint16_t m_prom_value;
	bool m_prom_cs;
	bool m_prom_clk;

	uint8_t m_pending_ints;

	uint8_t m_regs[0x10];
	uint8_t m_prom[0x80];
};

DECLARE_DEVICE_TYPE(ETNA, etna_device)

#endif // MAME_PSION_ETNA_H
