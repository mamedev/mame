// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SMC CRT9212 Double Row Buffer (DRB) emulation

**********************************************************************
                            _____   _____
                  DIN2   1 |*    \_/     | 28  DIN3
                  DIN1   2 |             | 27  _WCLK
                  DIN0   3 |             | 26  _OE
                 DOUT7   4 |             | 25  WEN2
                 DOUT6   5 |             | 24  WEN1
                 DOUT5   6 |             | 23  GND
                 DOUT4   7 |   CRT9212   | 22  ROF
                   Vcc   8 |             | 21  WOF
                 DOUT3   9 |             | 20  REN
                 DOUT2  10 |             | 19  _CLRCNT
                 DOUT1  11 |             | 18  _TOG
                 DOUT0  12 |             | 17  _RCLK
                  DIN7  13 |             | 16  DIN4
                  DIN6  14 |_____________| 15  DIN5

**********************************************************************/

#ifndef MAME_VIDEO_CRT9212_H
#define MAME_VIDEO_CRT9212_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> crt9212_device

class crt9212_device : public device_t
{
public:
	// construction/destruction
	crt9212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_wen2(int state) { m_wen2 = state; }
	auto dout() { return m_write_dout.bind(); }
	auto rof() { return m_write_rof.bind(); }
	auto wof() { return m_write_wof.bind(); }

	void write(uint8_t data) { m_data = data; }
	void clrcnt_w(int state);
	void tog_w(int state) { m_tog = state; }
	void ren_w(int state) { m_ren = state; }
	void wen1_w(int state) { m_wen1 = state; }
	void wen2_w(int state) { m_wen2 = state; }
	void oe_w(int state) { m_oe = state; }
	void rclk_w(int state) ;
	void wclk_w(int state) ;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	static constexpr int RAM_SIZE  = 135;

	devcb_write8           m_write_dout;
	devcb_write_line       m_write_rof;
	devcb_write_line       m_write_wof;

	// inputs
	uint8_t m_data;
	int m_clrcnt;
	int m_tog;
	int m_ren;
	int m_wen1;
	int m_wen2;
	int m_oe;
	int m_rclk;
	int m_wclk;

	// internal state
	bool m_clrcnt_edge;
	uint8_t m_data_latch;
	int m_ren_int;
	int m_wen_int;
	uint8_t m_ram[RAM_SIZE][2];
	int m_buffer;
	int m_rac;
	int m_wac;
};


// device type definition
DECLARE_DEVICE_TYPE(CRT9212, crt9212_device)

#endif // MAME_VIDEO_CRT9212_H
