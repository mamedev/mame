// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MM74C922/MM74C923 16/20-Key Encoder emulation

**********************************************************************
                            _____   _____
                ROW Y1   1 |*    \_/     | 18  Vcc
                ROW Y2   2 |             | 17  DATA OUT A
                ROW Y3   3 |             | 16  DATA OUT B
                ROW Y4   4 |             | 15  DATA OUT C
            OSCILLATOR   5 |  MM74C922   | 14  DATA OUT D
        KEYBOUNCE MASK   6 |             | 13  _OUTPUT ENABLE
             COLUMN X4   7 |             | 12  DATA AVAILABLE
             COLUMN X3   8 |             | 11  COLUMN X1
                   GND   9 |_____________| 10  COLUMN X2

                            _____   _____
                ROW Y1   1 |*    \_/     | 20  Vcc
                ROW Y2   2 |             | 19  DATA OUT A
                ROW Y3   3 |             | 18  DATA OUT B
                ROW Y4   4 |             | 17  DATA OUT C
                ROW Y5   5 |  MM74C923   | 16  DATA OUT D
            OSCILLATOR   6 |             | 15  DATA OUT E
        KEYBOUNCE MASK   7 |             | 14  _OUTPUT ENABLE
             COLUMN X4   8 |             | 13  DATA AVAILABLE
             COLUMN X3   9 |             | 12  COLUMN X1
                   GND  10 |_____________| 11  COLUMN X2

**********************************************************************/

#ifndef MAME_MACHINE_MM74C922_H
#define MAME_MACHINE_MM74C922_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mm74c922_device

class mm74c922_device :  public device_t
{
public:
	// construction/destruction
	mm74c922_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_cap_osc(double value) { m_cap_osc = value; }
	void set_cap_debounce(double value) { m_cap_debounce = value; }

	auto da_wr_callback() { return m_write_da.bind(); }
	auto x1_rd_callback() { return m_read_x1.bind(); }
	auto x2_rd_callback() { return m_read_x2.bind(); }
	auto x3_rd_callback() { return m_read_x3.bind(); }
	auto x4_rd_callback() { return m_read_x4.bind(); }
	auto x5_rd_callback() { return m_read_x5.bind(); }

	uint8_t read();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void change_output_lines();
	void clock_scan_counters();
	void detect_keypress();

	devcb_write_line   m_write_da;
	devcb_read8        m_read_x1;
	devcb_read8        m_read_x2;
	devcb_read8        m_read_x3;
	devcb_read8        m_read_x4;
	devcb_read8        m_read_x5;

	double              m_cap_osc;
	double              m_cap_debounce;

	int m_max_y;

	int m_inhibit;              // scan counter clock inhibit
	int m_x;                    // currently scanned column
	int m_y;                    // latched row

	uint8_t m_data;               // data latch

	int m_da;                   // data available flag
	int m_next_da;              // next value of data available flag

	// timers
	emu_timer *m_scan_timer;    // keyboard scan timer
};


// device type definition
DECLARE_DEVICE_TYPE(MM74C922, mm74c922_device)
DECLARE_DEVICE_TYPE(MM74C923, mm74c922_device)

#endif // MAME_MACHINE_MM74C922_H
