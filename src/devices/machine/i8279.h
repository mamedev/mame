// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Intel 8279 Programmable Keyboard/Display Interface emulation

****************************************************************************
                            _____   _____
                   RL2   1 |*    \_/     | 40  Vcc
                   RL3   2 |             | 39  RL1
                   CLK   3 |             | 38  RL0
                   IRQ   4 |             | 37  CNTL/STB
                   RL4   5 |             | 36  SHIFT
                   RL5   6 |             | 35  SL3
                   RL6   7 |             | 34  SL2
                   RL7   8 |             | 33  SL1
                 RESET   9 |             | 32  SL0
                   /RD  10 |     8279    | 31  OUT B0
                   /WR  11 |             | 30  OUT B1
                   DB0  12 |             | 29  OUT B2
                   DB1  13 |             | 28  OUT B3
                   DB2  14 |             | 27  OUT A0
                   DB3  15 |             | 26  OUT A1
                   DB4  16 |             | 25  OUT A2
                   DB5  17 |             | 24  OUT A3
                   DB6  18 |             | 23  /BD
                   DB7  19 |             | 22  /CS
                   Vss  20 |_____________| 21  A0 (CTRL/DATA)


***************************************************************************/

#ifndef MAME_MACHINE_I8279_H
#define MAME_MACHINE_I8279_H

#pragma once

class i8279_device :  public device_t
{
public:
	// construction/destruction
	i8279_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_sl_callback() { return m_out_sl_cb.bind(); }
	auto out_disp_callback() { return m_out_disp_cb.bind(); }
	auto out_bd_callback() { return m_out_bd_cb.bind(); }
	auto in_rl_callback() { return m_in_rl_cb.bind(); }
	auto in_shift_callback() { return m_in_shift_cb.bind(); }
	auto in_ctrl_callback() { return m_in_ctrl_cb.bind(); }

	// read & write handlers
	u8 read(offs_t offset);
	u8 status_r();
	u8 data_r();
	void write(offs_t offset, u8 data);
	void cmd_w(u8 data);
	void data_w(u8 data);
	void timer_mainloop();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER( timerproc_callback );

private:
	void timer_adjust();
	void clear_display();
	void new_fifo(u8 data);

	devcb_write_line    m_out_irq_cb;       // IRQ
	devcb_write8        m_out_sl_cb;        // Scanlines SL0-3
	devcb_write8        m_out_disp_cb;      // Display outputs B0-3, A0-3
	devcb_write_line    m_out_bd_cb;        // BD
	devcb_read8         m_in_rl_cb;         // kbd readlines RL0-7
	devcb_read_line     m_in_shift_cb;      // Shift key
	devcb_read_line     m_in_ctrl_cb;       // Ctrl-Strobe line

	emu_timer *m_timer;

	u8 m_d_ram[16];     // display ram
	u8 m_d_ram_ptr;
	u8 m_s_ram[8];      // might be same as fifo ram
	u8 m_s_ram_ptr;
	u8 m_fifo[8];       // queued keystrokes
	u8 m_cmd[8];        // Device settings
	u8 m_status;        // Returned via status_r
	u32 m_scanclock;    // Internal scan clock
	u8 m_scanner;       // next output on SL lines

	bool m_autoinc;     // auto-increment flag
	bool m_read_flag;   // read from where
	bool m_ctrl_key;    // previous state of strobe input
	bool m_se_mode;     // special error mode flag
	u8 m_key_down;      // current key being debounced
	u8 m_debounce;      // debounce counter
};


// device type definition
DECLARE_DEVICE_TYPE(I8279, i8279_device)

#endif // MAME_MACHINE_I8279_H
