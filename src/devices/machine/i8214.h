// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8214/3214 Priority Interrupt Control Unit

**********************************************************************
                            _____   _____
                   _B0   1 |*    \_/     | 24  Vcc
                   _B1   2 |             | 23  _ECS
                   _B2   3 |             | 22  _R7
                  _SGS   4 |             | 21  _R6
                  _INT   5 |             | 20  _R5
                  _CLK   6 |    8214     | 19  _R4
                  INTE   7 |    3214     | 18  _R3
                   _A0   8 |             | 17  _R2
                   _A1   9 |             | 16  _R1
                   _A2  10 |             | 15  _R0
                  _ELR  11 |             | 14  ENLG
                   GND  12 |_____________| 13  ETLG

**********************************************************************/

#ifndef MAME_MACHINE_I8214_H
#define MAME_MACHINE_I8214_H

#pragma once

class i8214_device : public device_t
{
public:
	// construction/destruction
	i8214_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_int_dis_hack(bool hack) { m_int_dis_hack = hack; }

	auto int_wr_callback() { return m_write_int.bind(); }
	auto enlg_wr_callback() { return m_write_enlg.bind(); }

	void sgs_w(int state);
	void etlg_w(int state);
	void inte_w(int state);

	uint8_t a_r();
	uint8_t vector_r();
	void b_w(uint8_t data);
	void b_sgs_w(uint8_t data);
	void r_w(int line, int state);
	void r_all_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void trigger_interrupt(int level);
	void check_interrupt();
	void update_interrupt_line();

	devcb_write_line   m_write_int;
	devcb_write_line   m_write_enlg;

	bool m_int_dis_hack;

	int m_inte;                 // interrupt enable
	int m_int_dis;              // interrupt (latch) disable flip-flop
	int m_a;                    // request level
	int m_current_status;
	uint8_t m_r;                  // interrupt request latch
	int m_sgs;                  // status group select
	int m_etlg;                 // enable this level group
};


// device type definition
DECLARE_DEVICE_TYPE(I8214, i8214_device)

#endif // MAME_MACHINE_I8214_H
