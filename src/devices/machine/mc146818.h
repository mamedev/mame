// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    mc146818.h

    Implementation of the MC146818 chip

    Real time clock chip with CMOS battery backed ram
    Used in IBM PC/AT, several PC clones, Amstrad NC200, Apollo workstations

*********************************************************************/

#ifndef MAME_MACHINE_MC146818_H
#define MAME_MACHINE_MC146818_H

#pragma once


class mc146818_device : public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	mc146818_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto irq() { return m_write_irq.bind(); }
	auto sqw() { return m_write_sqw.bind(); }

	// The MC146818 doesn't have century support (some variants do), but when syncing the date & time at startup we can optionally store the century.
	void set_century_index(int century_index) { assert(!century_count_enabled()); m_century_index = century_index; }

	// The MC146818 doesn't have UTC support, but when syncing the data & time at startup we can use UTC instead of local time.
	void set_use_utc(bool use_utc) { m_use_utc = use_utc; }

	void set_binary(bool binary) { m_binary = binary; }
	void set_24hrs(bool hour) { m_hour = hour; }
	void set_epoch(int epoch) { m_epoch = epoch; }
	void set_binary_year(int bin) { m_binyear = bin; }

	// read/write access
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// direct-mapped read/write access
	uint8_t read_direct(offs_t offset);
	void write_direct(offs_t offset, uint8_t data);

protected:
	mc146818_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	static constexpr unsigned char ALARM_DONTCARE = 0xc0;
	static constexpr unsigned char HOURS_PM = 0x80;

	virtual int data_size() const { return 64; }
	virtual int data_logical_size() const { return data_size(); }
	virtual bool century_count_enabled() const { return false; }

	virtual void internal_set_address(uint8_t address);
	virtual uint8_t internal_read(offs_t offset);
	virtual void internal_write(offs_t offset, uint8_t data);

	enum
	{
		REG_SECONDS = 0,
		REG_ALARM_SECONDS = 1,
		REG_MINUTES = 2,
		REG_ALARM_MINUTES = 3,
		REG_HOURS = 4,
		REG_ALARM_HOURS = 5,
		REG_DAYOFWEEK = 6,
		REG_DAYOFMONTH = 7,
		REG_MONTH = 8,
		REG_YEAR = 9,
		REG_A = 0xa,
		REG_B = 0xb,
		REG_C = 0xc,
		REG_D = 0xd
	};

	enum
	{
		REG_A_RS0 = 1,
		REG_A_RS1 = 2,
		REG_A_RS2 = 4,
		REG_A_RS3 = 8,
		REG_A_DV0 = 16,
		REG_A_DV1 = 32,
		REG_A_DV2 = 64,
		REG_A_UIP = 128
	};

	enum
	{
		REG_B_DSE = 1, // TODO: When set the chip will adjust the clock by an hour at start and end of DST
		REG_B_24_12 = 2,
		REG_B_DM = 4,
		REG_B_SQWE = 8, // TODO: When set the chip will output a square wave on SQW pin
		REG_B_UIE = 16,
		REG_B_AIE = 32,
		REG_B_PIE = 64,
		REG_B_SET = 128
	};

	enum
	{
		REG_C_UF = 16,
		REG_C_AF = 32,
		REG_C_PF = 64,
		REG_C_IRQF = 128
	};

	enum
	{
		REG_D_VRT = 128
	};

	// internal helpers
	int to_ram(int a) const;
	int from_ram(int a) const;
	void set_base_datetime();
	void update_irq();
	void update_timer();
	virtual int get_timer_bypass() const;
	int get_seconds() const;
	void set_seconds(int seconds);
	int get_minutes() const;
	void set_minutes(int minutes);
	int get_hours() const;
	void set_hours(int hours);
	int get_dayofweek() const;
	void set_dayofweek(int dayofweek);
	int get_dayofmonth() const;
	void set_dayofmonth(int dayofmonth);
	int get_month() const;
	void set_month(int month);
	int get_year() const;
	void set_year(int year);
	int get_century() const;
	void set_century(int year);

	optional_memory_region m_region;

	// internal state

	uint8_t           m_index;
	std::unique_ptr<uint8_t[]> m_data;

	static const device_timer_id TIMER_CLOCK = 0;
	static const device_timer_id TIMER_UPDATE = 1;
	static const device_timer_id TIMER_PERIODIC = 2;

	emu_timer *m_clock_timer;
	emu_timer *m_update_timer;
	emu_timer *m_periodic_timer;

	devcb_write_line m_write_irq;
	devcb_write_line m_write_sqw;
	int m_century_index, m_epoch;
	bool m_use_utc, m_binary, m_hour, m_binyear;
	bool m_sqw_state;
	unsigned m_tuc; // update cycle time
};

class ds1287_device : public mc146818_device
{
public:
	ds1287_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(MC146818, mc146818_device)
DECLARE_DEVICE_TYPE(DS1287, ds1287_device)

#endif // MAME_MACHINE_MC146818_H
