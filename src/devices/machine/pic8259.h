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


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

// Interrupt request output to CPU or master 8259 (active high)
#define MCFG_PIC8259_OUT_INT_CB(_devcb) \
	devcb = &downcast<pic8259_device &>(*device).set_out_int_callback(DEVCB_##_devcb);

// Slave program select (VCC = master; GND = slave; pin becomes EN output in buffered mode)
#define MCFG_PIC8259_IN_SP_CB(_devcb) \
	devcb = &downcast<pic8259_device &>(*device).set_in_sp_callback(DEVCB_##_devcb);

// Cascaded interrupt acknowledge request for slave 8259 to place vector on data bus
#define MCFG_PIC8259_CASCADE_ACK_CB(_devcb) \
	devcb = &downcast<pic8259_device &>(*device).set_read_slave_ack_callback(DEVCB_##_devcb);


class pic8259_device : public device_t
{
public:
	pic8259_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_out_int_callback(Object &&cb) { return m_out_int_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_in_sp_callback(Object &&cb) { return m_in_sp_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_slave_ack_callback(Object &&cb) { return m_read_slave_ack_func.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	uint32_t acknowledge();

	DECLARE_WRITE_LINE_MEMBER( ir0_w ) { set_irq_line(0, state); }
	DECLARE_WRITE_LINE_MEMBER( ir1_w ) { set_irq_line(1, state); }
	DECLARE_WRITE_LINE_MEMBER( ir2_w ) { set_irq_line(2, state); }
	DECLARE_WRITE_LINE_MEMBER( ir3_w ) { set_irq_line(3, state); }
	DECLARE_WRITE_LINE_MEMBER( ir4_w ) { set_irq_line(4, state); }
	DECLARE_WRITE_LINE_MEMBER( ir5_w ) { set_irq_line(5, state); }
	DECLARE_WRITE_LINE_MEMBER( ir6_w ) { set_irq_line(6, state); }
	DECLARE_WRITE_LINE_MEMBER( ir7_w ) { set_irq_line(7, state); }

	IRQ_CALLBACK_MEMBER(inta_cb);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	static constexpr device_timer_id TIMER_CHECK_IRQ = 0;

	inline void set_timer() { timer_set(attotime::zero, TIMER_CHECK_IRQ); }
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
};

DECLARE_DEVICE_TYPE(PIC8259, pic8259_device)

#endif // MAME_MACHINE_PIC8259_H
