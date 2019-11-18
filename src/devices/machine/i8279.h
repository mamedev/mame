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
	i8279_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_sl_callback() { return m_out_sl_cb.bind(); }
	auto out_disp_callback() { return m_out_disp_cb.bind(); }
	auto out_bd_callback() { return m_out_bd_cb.bind(); }
	auto in_rl_callback() { return m_in_rl_cb.bind(); }
	auto in_shift_callback() { return m_in_shift_cb.bind(); }
	auto in_ctrl_callback() { return m_in_ctrl_cb.bind(); }

	// read & write handlers
	DECLARE_READ8_MEMBER(read);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE8_MEMBER(cmd_w);
	DECLARE_WRITE8_MEMBER(data_w);
	void timer_mainloop();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

	TIMER_CALLBACK_MEMBER( timerproc_callback );

private:

	void timer_adjust();
	void clear_display();
	void new_key(uint8_t data, bool skey, bool ckey);
	void new_fifo(uint8_t data);
	void set_irq(bool state);

	devcb_write_line    m_out_irq_cb;       // IRQ
	devcb_write8        m_out_sl_cb;        // Scanlines SL0-3
	devcb_write8        m_out_disp_cb;      // Display outputs B0-3, A0-3
	devcb_write_line    m_out_bd_cb;        // BD
	devcb_read8         m_in_rl_cb;         // kbd readlines RL0-7
	devcb_read_line     m_in_shift_cb;      // Shift key
	devcb_read_line     m_in_ctrl_cb;       // Ctrl-Strobe line

	emu_timer *m_timer;

	uint8_t m_d_ram[16];      // display ram
	uint8_t m_d_ram_ptr;
	uint8_t m_s_ram[8]; // might be same as fifo ram
	uint8_t m_s_ram_ptr;
	uint8_t m_fifo[8];    // queued keystrokes
	uint8_t m_cmd[8];   // Device settings
	uint8_t m_status;     // Returned via status_r
	uint32_t m_clock;     // Internal scan clock
	uint8_t m_scanner;    // next output on SL lines

	bool m_autoinc;     // auto-increment flag
	bool m_read_flag;   // read from where
	bool m_ctrl_key;    // previous state of strobe input
	uint16_t m_key_down;
};


// device type definition
DECLARE_DEVICE_TYPE(I8279, i8279_device)

#endif // MAME_MACHINE_I8279_H
