// license:BSD-3-Clause
// copyright-holders:hap
/*

  Texas Instruments TMS1024/TMS1025 I/O expander

*/

#ifndef MAME_MACHINE_TMS1024_H
#define MAME_MACHINE_TMS1024_H

#pragma once

// pinout reference

/*

            ____   ____                         ____   ____
    Vss  1 |*   \_/    | 28 H2          Vss  1 |*   \_/    | 40 H2
     H3  2 |           | 27 H1           H3  2 |           | 39 H1
     H4  3 |           | 26 Vdd          H4  3 |           | 38 Vdd
     CE  4 |           | 25 S2           CE  4 |           | 37 S2
     MS  5 |           | 24 S1           MS  5 |           | 36 S1
    STD  6 |           | 23 S0          STD  6 |           | 35 S0
     A4  7 |  TMS1024  | 22 D7           A1  7 |           | 34 D3
     B4  8 |           | 21 C7           B1  8 |           | 33 C3
     C4  9 |           | 20 B7           C1  9 |           | 32 B3
     D4 10 |           | 19 A7           D1 10 |  TMS1025  | 31 A3
     A5 11 |           | 18 D6           A4 11 |           | 30 D7
     B5 12 |           | 17 C6           B4 12 |           | 29 C7
     C5 13 |           | 16 B6           C4 13 |           | 28 B7
     D5 14 |___________| 15 A6           D4 14 |           | 27 A7
                                         A5 15 |           | 26 D6
                                         B5 16 |           | 25 C6
     CE: Chip Enable                     C5 17 |           | 24 B6
     MS: Mode Select                     D5 18 |           | 23 A6
    STD: STrobe Data                     A2 19 |           | 22 D2
      S: Select                          B2 20 |___________| 21 C2
      H: Hold?

*/


class tms1024_device : public device_t
{
public:
	// 4-bit ports (3210 = DCBA)
	// valid ports: 4-7 for TMS1024, 1-7 for TMS1025
	enum
	{
		PORT1 = 0,
		PORT2,
		PORT3,
		PORT4,
		PORT5,
		PORT6,
		PORT7
	};

	tms1024_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	auto read_port4_callback() { return m_read_port[3].bind(); }
	auto read_port5_callback() { return m_read_port[4].bind(); }
	auto read_port6_callback() { return m_read_port[5].bind(); }
	auto read_port7_callback() { return m_read_port[6].bind(); }
	auto write_port4_callback() { return m_write_port[3].bind(); }
	auto write_port5_callback() { return m_write_port[4].bind(); }
	auto write_port6_callback() { return m_write_port[5].bind(); }
	auto write_port7_callback() { return m_write_port[6].bind(); }
	tms1024_device &set_ms(int state) { m_ms = state ? 1 : 0; return *this; } // if hardwired, can just set MS pin state here

	void write_h(u8 data);
	u8 read_h();
	void write_s(u8 data);
	void write_std(int state);
	void write_ms(int state);

protected:
	tms1024_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	u8 m_h;      // 4-bit data latch
	u8 m_s;      // 3-bit port select
	u8 m_std;    // strobe pin
	u8 m_ms;     // mode select pin, default to read mode

	// callbacks
	devcb_read8::array<7> m_read_port;
	devcb_write8::array<7> m_write_port;
};


class tms1025_device : public tms1024_device
{
public:
	tms1025_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto read_port1_callback() { return m_read_port[0].bind(); }
	auto read_port2_callback() { return m_read_port[1].bind(); }
	auto read_port3_callback() { return m_read_port[2].bind(); }
	auto write_port1_callback() { return m_write_port[0].bind(); }
	auto write_port2_callback() { return m_write_port[1].bind(); }
	auto write_port3_callback() { return m_write_port[2].bind(); }
};



DECLARE_DEVICE_TYPE(TMS1024, tms1024_device)
DECLARE_DEVICE_TYPE(TMS1025, tms1025_device)

#endif // MAME_MACHINE_TMS1024_H
