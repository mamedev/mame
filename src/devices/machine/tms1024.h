// license:BSD-3-Clause
// copyright-holders:hap
/*

  Texas Instruments TMS1024/TMS1025 I/O expander

*/

#ifndef _TMS1024_H_
#define _TMS1024_H_

#include "emu.h"


// ports setup

// 4-bit ports (3210 = DCBA)
// valid ports: 4-7 for TMS1024, 1-7 for TMS1025
#define MCFG_TMS1024_WRITE_PORT_CB(X, _devcb) \
	tms1024_device::set_write_port##X##_callback(*device, DEVCB_##_devcb);

enum
{
	TMS1024_PORT1 = 0,
	TMS1024_PORT2,
	TMS1024_PORT3,
	TMS1024_PORT4,
	TMS1024_PORT5,
	TMS1024_PORT6,
	TMS1024_PORT7
};


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
     MS: Master S.?                      D5 18 |           | 23 A6
    STD: STrobe Data?                    A2 19 |           | 22 D2
      S: Select                          B2 20 |___________| 21 C2
      H: Hold?

*/


class tms1024_device : public device_t
{
public:
	tms1024_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	tms1024_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// static configuration helpers
	template<class _Object> static devcb_base &set_write_port1_callback(device_t &device, _Object object) { return downcast<tms1024_device &>(device).m_write_port1.set_callback(object); }
	template<class _Object> static devcb_base &set_write_port2_callback(device_t &device, _Object object) { return downcast<tms1024_device &>(device).m_write_port2.set_callback(object); }
	template<class _Object> static devcb_base &set_write_port3_callback(device_t &device, _Object object) { return downcast<tms1024_device &>(device).m_write_port3.set_callback(object); }
	template<class _Object> static devcb_base &set_write_port4_callback(device_t &device, _Object object) { return downcast<tms1024_device &>(device).m_write_port4.set_callback(object); }
	template<class _Object> static devcb_base &set_write_port5_callback(device_t &device, _Object object) { return downcast<tms1024_device &>(device).m_write_port5.set_callback(object); }
	template<class _Object> static devcb_base &set_write_port6_callback(device_t &device, _Object object) { return downcast<tms1024_device &>(device).m_write_port6.set_callback(object); }
	template<class _Object> static devcb_base &set_write_port7_callback(device_t &device, _Object object) { return downcast<tms1024_device &>(device).m_write_port7.set_callback(object); }

	DECLARE_WRITE8_MEMBER(write_h);
	DECLARE_WRITE8_MEMBER(write_s);
	DECLARE_WRITE_LINE_MEMBER(write_std);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT8 m_h;      // 4-bit data latch
	UINT8 m_s;      // 3-bit port select
	UINT8 m_std;    // strobe pin

	// callbacks
	devcb_write8 m_write_port1, m_write_port2, m_write_port3, m_write_port4, m_write_port5, m_write_port6, m_write_port7;
	devcb_write8 *m_write_port[7];
};


class tms1025_device : public tms1024_device
{
public:
	tms1025_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};



extern const device_type TMS1024;
extern const device_type TMS1025;


#endif /* _TMS1024_H_ */
