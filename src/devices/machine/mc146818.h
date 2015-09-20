// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    mc146818.h

    Implementation of the MC146818 chip

    Real time clock chip with CMOS battery backed ram
    Used in IBM PC/AT, several PC clones, Amstrad NC200, Apollo workstations

*********************************************************************/

#ifndef __MC146818_H__
#define __MC146818_H__

#include "emu.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MC146818_ADD(_tag, _xtal) \
	MCFG_DEVICE_ADD(_tag, MC146818, _xtal)

#define MCFG_MC146818_IRQ_HANDLER(_irq) \
	downcast<mc146818_device *>(device)->set_irq_callback(DEVCB_##_irq);

// The MC146818 doesn't have century support, but when syncing the date & time at startup we can optionally store the century.
#define MCFG_MC146818_CENTURY_INDEX(_century_index) \
	downcast<mc146818_device *>(device)->set_century_index(_century_index);

// The MC146818 doesn't have UTC support, but when syncing the data & time at startup we can use UTC instead of local time.
#define MCFG_MC146818_UTC(_utc) \
	downcast<mc146818_device *>(device)->set_use_utc(_utc);

#define MCFG_MC146818_BINARY(_bin) \
	downcast<mc146818_device *>(device)->set_binary(_bin);

#define MCFG_MC146818_EPOCH(_epoch) \
	downcast<mc146818_device *>(device)->set_epoch(_epoch);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mc146818_device

class mc146818_device : public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	mc146818_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	mc146818_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// callbacks
	template<class _irq> void set_irq_callback(_irq irq) { m_write_irq.set_callback(irq); }
	void set_century_index(int century_index) { m_century_index = century_index; }
	void set_use_utc(bool use_utc) { m_use_utc = use_utc; }
	void set_binary(bool binary) { m_binary = binary; }
	void set_epoch(int epoch) { m_epoch = epoch; }

	// read/write access
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

	static const unsigned char ALARM_DONTCARE = 0xc0;
	static const unsigned char HOURS_PM = 0x80;

	virtual int data_size() { return 64; }

private:
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
	int to_ram(int a);
	int from_ram(int a);
	void set_base_datetime();
	void update_irq();
	void update_timer();

	int get_seconds();
	void set_seconds(int seconds);
	int get_minutes();
	void set_minutes(int minutes);
	int get_hours();
	void set_hours(int hours);
	int get_dayofweek();
	void set_dayofweek(int dayofweek);
	int get_dayofmonth();
	void set_dayofmonth(int dayofmonth);
	int get_month();
	void set_month(int month);
	int get_year();
	void set_year(int year);

	// internal state

	UINT8           m_index;
	dynamic_buffer  m_data;

	attotime        m_last_refresh;

	static const device_timer_id TIMER_CLOCK = 0;
	static const device_timer_id TIMER_PERIODIC = 1;

	emu_timer *m_clock_timer;
	emu_timer *m_periodic_timer;

	devcb_write_line m_write_irq;
	int m_century_index, m_epoch;
	bool m_use_utc, m_binary;
};


// device type definition
extern const device_type MC146818;


#endif /* __MC146818_H__ */
