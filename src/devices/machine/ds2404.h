// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************

    DALLAS DS2404

    RTC + BACKUP RAM

**********************************************************************/

#ifndef MAME_MACHINE_DS2404_H
#define MAME_MACHINE_DS2404_H

#pragma once

class ds2404_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	ds2404_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	void ref_year(uint32_t year) { m_ref_year = year; }
	void ref_month(uint8_t month) { m_ref_month = month; }
	void ref_day(uint8_t day) { m_ref_day = day; }

	/* 1-wire interface reset  */
	void _1w_reset_w(uint8_t data);

	/* 3-wire interface reset  */
	void _3w_reset_w(uint8_t data);

	uint8_t data_r();
	void data_w(uint8_t data);
	void clk_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override { }
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	TIMER_CALLBACK_MEMBER(timer_tick);

private:
	void rom_cmd(uint8_t cmd);
	void cmd(uint8_t cmd);

	uint8_t readmem();
	void writemem(uint8_t value);

	enum STATE
	{
		STATE_IDLE = 1,              /* waiting for ROM command, in 1-wire mode */
		STATE_COMMAND,               /* waiting for memory command */
		STATE_ADDRESS1,              /* waiting for address bits 0-7 */
		STATE_ADDRESS2,              /* waiting for address bits 8-15 */
		STATE_OFFSET,                /* waiting for ending offset */
		STATE_INIT_COMMAND,
		STATE_READ_MEMORY,           /* Read Memory command active */
		STATE_WRITE_SCRATCHPAD,      /* Write Scratchpad command active */
		STATE_READ_SCRATCHPAD,       /* Read Scratchpad command active */
		STATE_COPY_SCRATCHPAD        /* Copy Scratchpad command active */
	};

	emu_timer *m_tick_timer;

	// configuration state
	uint32_t  m_ref_year;
	uint8_t   m_ref_month;
	uint8_t   m_ref_day;

	uint16_t m_address;
	uint16_t m_offset;
	uint16_t m_end_offset;
	uint8_t m_a1;
	uint8_t m_a2;
	uint8_t m_sram[512];  /* 4096 bits */
	uint8_t m_ram[32];    /* scratchpad ram, 256 bits */
	uint8_t m_rtc[5];     /* 40-bit RTC counter */
	STATE m_state[8];
	int m_state_ptr;
};

DECLARE_DEVICE_TYPE(DS2404, ds2404_device)

#endif // MAME_MACHINE_DS2404_H
