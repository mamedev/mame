// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Intel 8259A

    Programmable Interrupt Controller

                            _____   _____
                   _CS   1 |*    \_/     | 28  VCC
                   _WR   2 |             | 27  A0
                   _RD   3 |             | 26  _INTA
                    D7   4 |             | 25  IR7
                    D6   5 |             | 24  IR6
                    D5   6 |             | 23  IR5
                    D4   7 |    8259A    | 22  IR4
                    D3   8 |             | 21  IR3
                    D2   9 |             | 20  IR2
                    D1  10 |             | 19  IR1
                    D0  11 |             | 18  IR0
                  CAS0  12 |             | 17  INT
                  CAS1  13 |             | 16  _SP/_EN
                   GND  14 |_____________| 15  CAS2

***************************************************************************/

#ifndef MAME_MACHINE_PIC8259_H
#define MAME_MACHINE_PIC8259_H


class pic8259_device : public device_t
{
public:
	pic8259_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto out_int_callback() { return m_out_int_func.bind(); } // Interrupt request output to CPU or master 8259 (active high)
	auto in_sp_callback() { return m_in_sp_func.bind(); } // Slave program select (VCC = master; GND = slave; pin becomes EN output in buffered mode)
	auto read_slave_ack_callback() { return m_read_slave_ack_func.bind(); } // Cascaded interrupt acknowledge request for slave 8259 to place vector on data bus

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t acknowledge();

	void ir0_w(int state) { set_irq_line(0, state); }
	void ir1_w(int state) { set_irq_line(1, state); }
	void ir2_w(int state) { set_irq_line(2, state); }
	void ir3_w(int state) { set_irq_line(3, state); }
	void ir4_w(int state) { set_irq_line(4, state); }
	void ir5_w(int state) { set_irq_line(5, state); }
	void ir6_w(int state) { set_irq_line(6, state); }
	void ir7_w(int state) { set_irq_line(7, state); }

	IRQ_CALLBACK_MEMBER(inta_cb);

protected:
	pic8259_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual bool is_x86() const { return m_is_x86; }

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	void set_irq_line(int irq, int state);

	enum class state_t : u8
	{
		ICW1,
		ICW2,
		ICW3,
		ICW4,
		READY
	};

	devcb_write_line m_out_int_func;
	devcb_read_line m_in_sp_func;
	devcb_read8 m_read_slave_ack_func;

	emu_timer *m_irq_timer;
	state_t m_state;

	uint8_t m_isr;
	uint8_t m_irr;
	uint8_t m_prio;
	uint8_t m_imr;
	uint8_t m_irq_lines;

	uint8_t m_input;
	uint8_t m_ocw3;

	uint8_t m_master;
	/* ICW1 state */
	uint8_t m_level_trig_mode;
	uint8_t m_vector_size;
	uint8_t m_cascade;
	uint8_t m_icw4_needed;
	uint32_t m_vector_addr_low;
	/* ICW2 state */
	uint8_t m_base;
	uint8_t m_vector_addr_high;

	/* ICW3 state */
	uint8_t m_slave;

	/* ICW4 state */
	uint8_t m_nested;
	uint8_t m_mode;
	uint8_t m_auto_eoi;
	uint8_t m_is_x86;

	int8_t m_current_level;
	uint8_t m_inta_sequence;
};

class v5x_icu_device : public pic8259_device
{
public:
	v5x_icu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual bool is_x86() const override { return true; }
};

class mk98pic_device : public pic8259_device
{
public:
	mk98pic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual bool is_x86() const override { return true; }
};

DECLARE_DEVICE_TYPE(PIC8259, pic8259_device)
DECLARE_DEVICE_TYPE(V5X_ICU, v5x_icu_device)
DECLARE_DEVICE_TYPE(MK98PIC, mk98pic_device)

#endif // MAME_MACHINE_PIC8259_H
