// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hughes HLCD 0515 family LCD Driver

*/

#ifndef MAME_VIDEO_HLCD0515_H
#define MAME_VIDEO_HLCD0515_H

#pragma once

// pinout reference

/*
             ____   ____                                 ____   ____
    ROW0  1 |*   \_/    | 40 VDD                ROW0  1 |*   \_/    | 40 VDD
    ROW1  2 |           | 39 OSC                ROW1  2 |           | 39 VDRIVE?
    ROW2  3 |           | 38 CLOCK              ROW2  3 |           | 38 OSC
    ROW3  4 |           | 37 DATA IN            ROW3  4 |           | 37 CLOCK
    ROW4  5 |           | 36 _CS                ROW4  5 |           | 36 DATA IN
    ROW5  6 |           | 35 DATA OUT           ROW5  6 |           | 35 _CS
    ROW6  7 |           | 34 COL25              ROW6  7 |           | 34 DATA OUT?
    ROW7  8 |           | 33 COL24              ROW7  8 |           | 33 COL24
    COL1  9 |           | 32 COL23              COL1  9 |           | 32 COL23
    COL2 10 | HLCD 0515 | 31 COL22              COL2 10 | HLCD 0569 | 31 COL22
    COL3 11 |           | 30 COL21              COL3 11 |           | 30 COL21
    COL4 12 |           | 29 COL20              COL4 12 |           | 29 COL20
    COL5 13 |           | 28 COL19              COL5 13 |           | 28 COL19
    COL6 14 |           | 27 COL18              COL6 14 |           | 27 COL18
    COL7 15 |           | 26 COL17              COL7 15 |           | 26 COL17
    COL8 16 |           | 25 COL16              COL8 16 |           | 25 COL16
    COL9 17 |           | 24 COL15              COL9 17 |           | 24 COL15
   COL10 18 |           | 23 COL14             COL10 18 |           | 23 COL14
   COL11 19 |           | 22 COL13             COL11 19 |           | 22 COL13
     GND 20 |___________| 21 COL12               GND 20 |___________| 21 COL12

    OSC is tied to a capacitor, the result frequency is 50000 * cap(in uF), eg. 0.01uF cap = 500Hz.
    Internally, this is divided by 2, and by number of rows to get display refresh frequency.
*/


class hlcd0515_device : public device_t, public device_nvram_interface
{
public:
	hlcd0515_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto write_cols() { return m_write_cols.bind(); } // COL/ROW pins (offset for ROW)
	auto write_data() { return m_write_data.bind(); } // DATA OUT pin

	void clock_w(int state);
	void cs_w(int state);
	void data_w(int state) { m_data = (state) ? 1 : 0; }
	int data_r() { return m_dataout; }

protected:
	hlcd0515_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 colmax);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override { internal_clear(); }
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	void internal_clear();
	TIMER_CALLBACK_MEMBER(scan_lcd);

	virtual void set_control();
	void clock_data(int col = 0);

	const u8 m_colmax;    // number of column pins

	int m_cs;       // input pin state
	int m_clk;      // "
	int m_data;     // "
	int m_dataout;  // DATA OUT pin state
	u8 m_count;
	u8 m_control;
	bool m_blank;   // display blank/visible
	u8 m_rowmax;    // number of rows output by lcd (max 8)
	u8 m_rowout;    // current row for lcd output
	u8 m_rowsel;    // current row for data in/out
	u32 m_buffer;   // data in/out shift register
	u32 m_ram[8];   // internal lcd ram, 8 rows

	emu_timer *m_lcd_timer;

	// callbacks
	devcb_write32 m_write_cols;
	devcb_write_line m_write_data;
};


class hlcd0569_device : public hlcd0515_device
{
public:
	hlcd0569_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void set_control() override;
};

class hlcd0530_device : public hlcd0515_device
{
public:
	hlcd0530_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hlcd0601_device : public hlcd0515_device
{
public:
	hlcd0601_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(HLCD0515, hlcd0515_device)
DECLARE_DEVICE_TYPE(HLCD0569, hlcd0569_device)
DECLARE_DEVICE_TYPE(HLCD0530, hlcd0530_device)
DECLARE_DEVICE_TYPE(HLCD0601, hlcd0601_device)

#endif // MAME_VIDEO_HLCD0515_H
