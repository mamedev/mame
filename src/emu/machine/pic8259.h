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

#ifndef __PIC8259_H__
#define __PIC8259_H__

#include "devlegcy.h"
#include "devcb.h"

class pic8259_device : public device_t
{
public:
	pic8259_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	UINT32 acknowledge();

	DECLARE_WRITE_LINE_MEMBER( ir0_w ) { set_irq_line(0, state); }
	DECLARE_WRITE_LINE_MEMBER( ir1_w ) { set_irq_line(1, state); }
	DECLARE_WRITE_LINE_MEMBER( ir2_w ) { set_irq_line(2, state); }
	DECLARE_WRITE_LINE_MEMBER( ir3_w ) { set_irq_line(3, state); }
	DECLARE_WRITE_LINE_MEMBER( ir4_w ) { set_irq_line(4, state); }
	DECLARE_WRITE_LINE_MEMBER( ir5_w ) { set_irq_line(5, state); }
	DECLARE_WRITE_LINE_MEMBER( ir6_w ) { set_irq_line(6, state); }
	DECLARE_WRITE_LINE_MEMBER( ir7_w ) { set_irq_line(7, state); }

	UINT8 inta_r();

	TIMER_CALLBACK_MEMBER( timerproc );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	inline void set_timer() { m_timer->adjust(attotime::zero); }
	void set_irq_line(int irq, int state);


	enum pic8259_state_t
	{
		STATE_ICW1,
		STATE_ICW2,
		STATE_ICW3,
		STATE_ICW4,
		STATE_READY
	};

	devcb_resolved_write_line m_out_int_func;
	devcb_resolved_read_line m_sp_en_func;
	devcb_resolved_read8 m_read_slave_ack_func;

	emu_timer *m_timer;

	pic8259_state_t m_state;

	UINT8 m_isr;
	UINT8 m_irr;
	UINT8 m_prio;
	UINT8 m_imr;
	UINT8 m_irq_lines;

	UINT8 m_input;
	UINT8 m_ocw3;

	UINT8 m_master;
	/* ICW1 state */
	UINT32 m_level_trig_mode : 1;
	UINT32 m_vector_size : 1;
	UINT32 m_cascade : 1;
	UINT32 m_icw4_needed : 1;
	UINT32 m_vector_addr_low;
	/* ICW2 state */
	UINT8 m_base;
	UINT8 m_vector_addr_high;

	/* ICW3 state */
	UINT8 m_slave;

	/* ICW4 state */
	UINT32 m_nested : 1;
	UINT32 m_mode : 2;
	UINT32 m_auto_eoi : 1;
	UINT32 m_is_x86 : 1;
};

extern const device_type PIC8259;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct pic8259_interface
{
	/* Called when int line changes */
	devcb_write_line out_int_func;
	/* 1 - when master, 0 - when slave */
	devcb_read_line sp_en_func;
	/* Called when on master slave irq is trigered*/
	devcb_read8 read_slave_ack_func;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_PIC8259_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, PIC8259, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#endif /* __PIC8259_H__ */
