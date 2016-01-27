// license:BSD-3-Clause
// copyright-holders:hap, Couriersud
/**********************************************************************************************

    Texas Instruments TMS6100 Voice Synthesis Memory (VSM)

***********************************************************************************************/

#ifndef __TMS6100_H__
#define __TMS6100_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

// 4-bit mode (mask option)
// note: in 4-bit mode, use data_r, otherwise use data_line_r

#define MCFG_TMS6100_4BIT_MODE() \
	tms6100_device::enable_4bit_mode(*device);


// pinout reference

/*
    TMS6100:
                 +-----------------+
       VDD       |  1           28 |  NC
       NC        |  2           27 |  NC
       DATA/ADD1 |  3           26 |  NC
       DATA/ADD2 |  4           25 |  NC
       DATA/ADD4 |  5           24 |  NC
       DATA/ADD8 |  6           23 |  NC
       CLK       |  7           22 |  NC
       NC        |  8           21 |  NC
       NC        |  9           20 |  NC
       M0        | 10           19 |  NC
       M1        | 11           18 |  NC
       NC        | 12           17 |  NC
       /CS       | 13           16 |  NC
       VSS       | 14           15 |  NC
                 +-----------------+


    TMS6125: two types known

                 +---------+                        +---------+
       DATA/ADD1 | 1    16 |  NC          DATA/ADD1 | 1    16 |  NC
       DATA/ADD2 | 2    15 |  NC          DATA/ADD2 | 2    15 |  NC
       DATA/ADD4 | 3    14 |  NC          DATA/ADD4 | 3    14 |  NC
       RCK       | 4    13 |  NC          DATA/ADD8 | 4    13 |  NC
       CLK       | 5    12 |  VDD         CLK       | 5    12 |  VDD
       DATA/ADD8 | 6    11 |  CS          NC        | 6    11 |  /CS
       NC        | 7    10 |  M1          NC        | 7    10 |  M1
       M0        | 8     9 |  VSS         M0        | 8     9 |  VSS
                 +---------+                        +---------+


    Mitsubishi M58819S EPROM Interface:
    It is a clone of TMS6100, but external EPROM instead

                 +-----------------+
       AD0       |  1           40 |  AD1
       VDDl      |  2           39 |  AD2
       VDD       |  3           38 |  AD3
       A0        |  4           37 |  NC
       NC        |  5           36 |  AD4
       NC        |  6           35 |  AD5
       A1        |  7           34 |  AD6
       A2        |  8           33 |  AD7
       A3/Q      |  9           32 |  AD8
       CLK       | 10           31 |  AD9
       POW       | 11           30 |  AD10
       SL        | 12           29 |  AD11
       C0        | 13           28 |  AD12
       C1        | 14           27 |  AD13
       NC        | 15           26 |  D7
       NC        | 16           25 |  NC
       VSS       | 17           24 |  D6
       D0        | 18           23 |  D5
       D1        | 19           22 |  D4
       D2        | 20           21 |  D3
                 +-----------------+
*/


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tms6100_device : public device_t
{
public:
	tms6100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms6100_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	static void enable_4bit_mode(device_t &device) { downcast<tms6100_device &>(device).m_4bit_mode = true; }

	DECLARE_WRITE_LINE_MEMBER(m0_w);
	DECLARE_WRITE_LINE_MEMBER(m1_w);
	DECLARE_WRITE_LINE_MEMBER(rck_w);
	DECLARE_WRITE_LINE_MEMBER(cs_w);
	DECLARE_WRITE_LINE_MEMBER(clk_w);

	DECLARE_WRITE8_MEMBER(add_w);
	DECLARE_READ8_MEMBER(data_r); // 4bit
	DECLARE_READ_LINE_MEMBER(data_line_r);

protected:
	// device-level overrides
	virtual void device_start() override;

	void handle_command(UINT8 cmd);

	// internal state
	required_region_ptr<UINT8> m_rom;
	bool m_reverse_bits;
	bool m_4bit_mode;

	UINT32 m_rommask;
	UINT32 m_address;   // internal address + chipselect
	UINT8 m_sa;         // romdata shift register
	UINT8 m_count;      // TB/LA counter (-> PLA)
	UINT8 m_prev_cmd;   // previous handled command
	UINT8 m_prev_m;     // previous valid m0/m1 state

	UINT8 m_add;        // ADD/DATA pins input
	UINT8 m_data;       // ADD/DATA pins output
	int m_m0;
	int m_m1;
	int m_cs;           // chipselect pin
	int m_clk;          // CLK pin
	int m_rck;          // RCK pin (mask/gate to CLK?)
};


class m58819_device : public tms6100_device
{
public:
	m58819_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
};


extern const device_type TMS6100;
extern const device_type M58819;


#endif /* __TMS6100_H__ */
