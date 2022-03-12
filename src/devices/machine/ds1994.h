// license:BSD-3-Clause
// copyright-holders:Aaron Giles, smf, Grull Osgo
/*
 * DS1994
 *
 * Dallas Semiconductor
 * 1-Wire Protocol
 * RTC + BACKUP RAM
 *
 */

#ifndef MAME_MACHINE_DS1994_H
#define MAME_MACHINE_DS1994_H

#pragma once

class ds1994_device : public device_t, public device_nvram_interface
{
 public:
	// construction/destruction
	ds1994_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: ds1994_device(mconfig, tag, owner, uint32_t(0))
	{
	}

	ds1994_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	void ref_year(uint32_t year) { m_ref_year = year; }
	void ref_month(uint8_t month) { m_ref_month = month; }
	void ref_day(uint8_t day) { m_ref_day = day; }

	DECLARE_WRITE_LINE_MEMBER(write);
	DECLARE_READ_LINE_MEMBER(read);

 protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

 private:
	enum {
		ROM_SIZE = 8,
		DATA_SIZE = 0x202,
		SPD_SIZE = 0x20,
		RTC_SIZE = 0x05,
		REGS_SIZE = 0x19,
		// Rom Commands
		ROMCMD_READROM = 0x33,
		ROMCMD_MATCHROM = 0x55,
		ROMCMD_SEARCHROM = 0xf0,
		ROMCMD_SEARCHINT = 0xec,
		ROMCMD_SKIPROM = 0xcc,
		// Memory Commands
		COMMAND_READ_MEMORY = 0xf0,
		COMMAND_WRITE_SCRATCHPAD = 0x0f,
		COMMAND_READ_SCRATCHPAD = 0xaa,
		COMMAND_COPY_SCRATCHPAD = 0x55
	};

	enum {
		TIMER_MAIN,
		TIMER_RESET,
		TIMER_CLOCK
	};

	enum {
		STATE_IDLE,
		STATE_RESET,
		STATE_RESET1,
		STATE_RESET2,
		STATE_ROMCMD,
		STATE_READROM,
		STATE_SKIPROM,
		STATE_MATCHROM,
		STATE_COMMAND,
		STATE_ADDRESS1,
		STATE_ADDRESS2,
		STATE_OFFSET,
		STATE_TXADDRESS1,
		STATE_TXADDRESS2,
		STATE_TXOFFSET,
		STATE_INIT_COMMAND,
		STATE_READ_MEMORY,
		STATE_WRITE_SCRATCHPAD,
		STATE_READ_SCRATCHPAD,
		STATE_COPY_SCRATCHPAD
	};

	void ds1994_rom_cmd(void);
	void ds1994_cmd(void);
	bool one_wire_tx_bit(uint8_t value);
	bool one_wire_rx_bit(void);
	uint8_t ds1994_readmem();
	void ds1994_writemem(uint8_t value);

	emu_timer *m_timer_main, *m_timer_reset, *m_timer_clock;

	uint32_t m_ref_year;
	uint8_t  m_ref_month;
	uint8_t  m_ref_day;

	uint16_t m_address;
	uint16_t m_offset;
	uint8_t  m_a1;
	uint8_t  m_a2;
	int      m_bit, m_shift;
	uint8_t  m_byte;
	bool     m_rx, m_tx;
	uint8_t  m_rom[ROM_SIZE];
	uint8_t  m_sram[DATA_SIZE];
	uint8_t  m_ram[SPD_SIZE];
	uint8_t  m_rtc[RTC_SIZE];
	uint8_t  m_regs[REGS_SIZE];
	int      m_state[6];
	int      m_state_ptr;
	bool     m_auth;
	bool     m_offs_ro;

	attotime t_samp, t_rdv, t_rstl, t_pdh, t_pdl;

	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3, 4);
};

// device type definition
DECLARE_DEVICE_TYPE(DS1994, ds1994_device)

#endif  // MAME_MACHINE_DS1994_H
