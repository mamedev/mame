// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 6526/8520 Complex Interface Adapter emulation

**********************************************************************/

/*

    TODO:

    - pass Lorenz test suite 2.15
        - ICR01
        - IMR
        - CIA1TA/TB
        - CIA2TA/TB
    - pass VICE cia tests
    - 8520 read/write
    - 5710 read/write
    - optimize
    - off by one errors in vAmigaTS/showcia1 TODLO (reproducible particularly with -nothrottle)
    - flag_w & amigafdc both auto-inverts index pulses, it also fails ICR vAmigaTS/showcia1 test
      (expected: 0x00, actual: 0x10)

*/

#include "emu.h"
#include "mos6526.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

// registers
enum
{
	PRA = 0,
	PRB,
	DDRA,
	DDRB,
	TA_LO,
	TA_HI,
	TB_LO,
	TB_HI,
	TOD_10THS,
	TOD_SEC,
	TOD_MIN,
	TOD_HR,
	SDR,
	ICR, IMR = ICR,
	CRA,
	CRB
};


// interrupt control register
#define ICR_TA      0x01
#define ICR_TB      0x02
#define ICR_ALARM   0x04
#define ICR_SP      0x08
#define ICR_FLAG    0x10


// interrupt mask register
#define IMR_TA      BIT(m_imr, 0)
#define IMR_TB      BIT(m_imr, 1)
#define IMR_ALARM   BIT(m_imr, 2)
#define IMR_SP      BIT(m_imr, 3)
#define IMR_FLAG    BIT(m_imr, 4)
#define IMR_SET     BIT(data, 7)


// control register A
enum
{
	CRA_INMODE_PHI2 = 0,
	CRA_INMODE_CNT
};

#define CRA_START       0x01
#define CRA_STARTED     BIT(m_cra, 0)
#define CRA_PBON        BIT(m_cra, 1)
#define CRA_OUTMODE     BIT(m_cra, 2)
#define CRA_RUNMODE     BIT(m_cra, 3)
#define CRA_LOAD        BIT(m_cra, 4)
#define CRA_INMODE      BIT(m_cra, 5)
#define CRA_SPMODE      BIT(m_cra, 6)
#define CRA_TODIN       BIT(m_cra, 7)


// control register B
enum
{
	CRB_INMODE_PHI2 = 0,
	CRB_INMODE_CNT,
	CRB_INMODE_TA,
	CRB_INMODE_CNT_TA
};

#define CRB_START       0x01
#define CRB_STARTED     BIT(m_crb, 0)
#define CRB_PBON        BIT(m_crb, 1)
#define CRB_OUTMODE     BIT(m_crb, 2)
#define CRB_RUNMODE     BIT(m_crb, 3)
#define CRB_LOAD        BIT(m_crb, 4)
#define CRB_INMODE      ((m_crb & 0x60) >> 5)
#define CRB_ALARM       BIT(m_crb, 7)



//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MOS6526,  mos6526_device,  "mos6526",  "MOS 6526 CIA")
DEFINE_DEVICE_TYPE(MOS6526A, mos6526a_device, "mos6526a", "MOS 6526A CIA")
DEFINE_DEVICE_TYPE(MOS8520,  mos8520_device,  "mos8520",  "MOS 8520 CIA")
DEFINE_DEVICE_TYPE(MOS5710,  mos5710_device,  "mos5710",  "MOS 5710 CIA")



//**************************************************************************
//  DEVICE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  update_pa - update port A
//-------------------------------------------------

void mos6526_device::update_pa()
{
	uint8_t pa = m_pra | (m_pa_in & ~m_ddra);

	if (m_pa != pa)
	{
		m_pa = pa;
		m_write_pa((offs_t)0, pa);
	}
}


//-------------------------------------------------
//  update_pb - update port B
//-------------------------------------------------

void mos6526_device::update_pb()
{
	uint8_t pb = m_prb | (m_pb_in & ~m_ddrb);

	if (CRA_PBON)
	{
		int pb6 = CRA_OUTMODE ? m_ta_pb6 : m_ta_out;

		pb &= ~0x40;
		pb |= pb6 << 6;
	}

	if (CRB_PBON)
	{
		int pb7 = CRB_OUTMODE ? m_tb_pb7 : m_tb_out;

		pb &= ~0x80;
		pb |= pb7 << 7;
	}

	if (m_pb != pb)
	{
		m_write_pb((offs_t)0, pb);
		m_pb = pb;
	}
}


//-------------------------------------------------
//  set_cra - control register A write
//-------------------------------------------------

void mos6526_device::set_cra(uint8_t data)
{
	if (!CRA_STARTED && (data & CRA_START))
	{
		m_ta_pb6 = 1;
	}

	// switching to serial output mode causes sp to go high?
	if (!CRA_SPMODE && BIT(data, 6))
	{
		m_bits = 0;
		m_write_sp(1);
	}

	// lower sp again when switching back to input?
	if (CRA_SPMODE && !BIT(data, 6))
	{
		m_bits = 0;
		m_write_sp(0);
	}

	m_cra = data;
	update_pb();
}


//-------------------------------------------------
//  set_crb - control register B write
//-------------------------------------------------

void mos6526_device::set_crb(uint8_t data)
{
	if (!CRB_STARTED && (data & CRB_START))
	{
		m_tb_pb7 = 1;
	}

	m_crb = data;
	update_pb();
}


//-------------------------------------------------
//  bcd_increment -
//-------------------------------------------------

uint8_t mos6526_device::bcd_increment(uint8_t value)
{
	value++;

	if ((value & 0x0f) >= 0x0a)
		value += 0x10 - 0x0a;

	return value;
}


//-------------------------------------------------
//  clock_tod - time-of-day clock pulse
//-------------------------------------------------

void mos6526_device::clock_tod()
{
	uint8_t subsecond = (uint8_t) (m_tod >>  0);
	uint8_t second    = (uint8_t) (m_tod >>  8);
	uint8_t minute    = (uint8_t) (m_tod >> 16);
	uint8_t hour      = (uint8_t) (m_tod >> 24);

	m_tod_count++;

	if (m_tod_count == (CRA_TODIN ? 5 : 6))
	{
		m_tod_count = 0;

		subsecond = bcd_increment(subsecond);

		if (subsecond >= 0x10)
		{
			subsecond = 0x00;
			second = bcd_increment(second);

			if (second >= 60)
			{
				second = 0x00;
				minute = bcd_increment(minute);

				if (minute >= 0x60)
				{
					minute = 0x00;

					int pm = hour & 0x80;
					hour &= 0x1f;

					if (hour == 11) pm ^= 0x80;
					if (hour == 12) hour = 0;

					hour = bcd_increment(hour);

					hour |= pm;
				}
			}
		}
	}

	m_tod = (((uint32_t) subsecond)   <<  0)
			| (((uint32_t) second)        <<  8)
			| (((uint32_t) minute)        << 16)
			| (((uint32_t) hour)      << 24);
}


//-------------------------------------------------
//  clock_tod - time-of-day clock pulse
//-------------------------------------------------

void mos8520_device::clock_tod()
{
	m_tod++;
	m_tod &= 0x00ffffff;
}


//-------------------------------------------------
//  read_tod - time-of-day read
//-------------------------------------------------

uint8_t mos6526_device::read_tod(int offset)
{
	int shift = 8 * offset;

	if (m_tod_latched)
	{
		return m_tod_latch >> shift;
	}
	else
	{
		return m_tod >> shift;
	}
}


//-------------------------------------------------
//  write_tod - time-of-day write
//-------------------------------------------------

void mos6526_device::write_tod(int offset, uint8_t data)
{
	int shift = 8 * offset;

	if (CRB_ALARM)
	{
		m_alarm = (m_alarm & ~(0xff << shift)) | (data << shift);
	}
	else
	{
		m_tod = (m_tod & ~(0xff << shift)) | (data << shift);
	}
}


//-------------------------------------------------
//  serial_input -
//-------------------------------------------------

void mos6526_device::serial_input()
{
	m_shift <<= 1;
	m_bits++;

	m_shift |= m_sp;

	if (m_bits == 8)
	{
		m_sdr = m_shift;
		m_bits = 0;

		m_icr |= ICR_SP;
	}
}


//-------------------------------------------------
//  clock_ta - clock timer A
//-------------------------------------------------

void mos6526_device::clock_ta()
{
	if (m_count_a3)
	{
		m_ta--;
	}

	m_ta_out = (m_count_a2 && !m_ta);

	if (m_ta_out)
	{
		m_ta_pb6 = !m_ta_pb6;

		if (CRA_RUNMODE || m_oneshot_a0)
		{
			m_cra &= ~CRA_START;
			m_count_a0 = m_count_a1 = m_count_a2 = 0;
		}

		m_load_a1 = 1;
	}

	if (m_load_a1)
	{
		m_count_a2 = 0;
		m_ta = m_ta_latch;
	}
}


//-------------------------------------------------
//  serial_output -
//-------------------------------------------------

void mos6526_device::serial_output()
{
	if (m_ta_out && CRA_SPMODE)
	{
		if (!m_sdr_empty || m_bits)
		{
			if (m_cnt)
			{
				if (m_bits == 0)
				{
					m_sdr_empty = true;
					m_shift = m_sdr;
				}

				m_sp = BIT(m_shift, 7);
				m_write_sp(m_sp);

				m_shift <<= 1;
				m_bits++;

				if (m_bits == 8)
				{
					m_icr |= ICR_SP;
				}
			}
			else
			{
				if (m_bits == 8)
				{
					m_bits = 0;
				}
			}

			m_cnt = !m_cnt;
			m_write_cnt(m_cnt);
		}
	}
}


//-------------------------------------------------
//  clock_tb - clock timer B
//-------------------------------------------------

void mos6526_device::clock_tb()
{
	if (m_count_b3)
	{
		m_tb--;
	}

	m_tb_out = (m_count_b2 && !m_tb);

	if (m_tb_out)
	{
		m_tb_pb7 = !m_tb_pb7;

		if (CRB_RUNMODE || m_oneshot_b0)
		{
			m_crb &= ~CRB_START;
			m_count_b0 = m_count_b1 = m_count_b2 = 0;
		}

		m_load_b1 = 1;
	}

	if (m_load_b1)
	{
		m_count_b2 = 0;
		m_tb = m_tb_latch;
	}
}


//-------------------------------------------------
//  update_interrupt -
//-------------------------------------------------

void mos6526_device::update_interrupt()
{
	if (!m_irq && m_ir1)
	{
		m_write_irq(ASSERT_LINE);
		m_irq = true;
	}

	if (m_ta_out)
	{
		m_icr |= ICR_TA;
	}

	if (m_tb_out && !m_icr_read)
	{
		m_icr |= ICR_TB;
	}

	m_icr_read = false;
}


//-------------------------------------------------
//  clock_pipeline - clock pipeline
//-------------------------------------------------

void mos6526_device::clock_pipeline()
{
	// timer A pipeline
	m_count_a3 = m_count_a2;

	if (CRA_INMODE == CRA_INMODE_PHI2)
		m_count_a2 = 1;

	m_count_a2 &= CRA_STARTED;
	m_count_a1 = m_count_a0;
	m_count_a0 = 0;

	m_load_a2 = m_load_a1;
	m_load_a1 = m_load_a0;
	m_load_a0 = CRA_LOAD;
	m_cra &= ~0x10;

	m_oneshot_a0 = CRA_RUNMODE;

	// timer B pipeline
	m_count_b3 = m_count_b2;

	switch (CRB_INMODE)
	{
	case CRB_INMODE_PHI2:
		m_count_b2 = 1;
		break;

	case CRB_INMODE_TA:
		m_count_b2 = m_ta_out;
		break;

	case CRB_INMODE_CNT_TA:
		m_count_b2 = m_ta_out && m_cnt;
		break;
	}

	m_count_b2 &= CRB_STARTED;
	m_count_b1 = m_count_b0;
	m_count_b0 = 0;

	m_load_b2 = m_load_b1;
	m_load_b1 = m_load_b0;
	m_load_b0 = CRB_LOAD;
	m_crb &= ~0x10;

	m_oneshot_b0 = CRB_RUNMODE;

	// interrupt pipeline
	if (m_ir0) m_ir1 = 1;
	m_ir0 = (m_icr & m_imr) ? 1 : 0;
}


//-------------------------------------------------
//  synchronize -
//-------------------------------------------------

void mos6526_device::synchronize()
{
	if (!m_pc)
	{
		m_pc = 1;
		m_write_pc(m_pc);
	}

	clock_ta();

	serial_output();

	clock_tb();

	update_pb();

	update_interrupt();

	clock_pipeline();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6526_device - constructor
//-------------------------------------------------

mos6526_device::mos6526_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant) :
	device_t(mconfig, type, tag, owner, clock),
	device_execute_interface(mconfig, *this),
	m_icount(0),
	m_variant(variant),
	m_tod_clock(0),
	m_write_irq(*this),
	m_write_pc(*this),
	m_write_cnt(*this),
	m_write_sp(*this),
	m_read_pa(*this, 0xff),
	m_write_pa(*this),
	m_read_pb(*this, 0xff),
	m_write_pb(*this)
{
}

mos6526_device::mos6526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6526_device(mconfig, MOS6526, tag, owner, clock, TYPE_6526)
{ }

mos6526a_device::mos6526a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6526_device(mconfig, MOS6526A, tag, owner, clock, TYPE_6526A) { }

mos8520_device::mos8520_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6526_device(mconfig, MOS8520, tag, owner, clock, TYPE_8520) { }

mos5710_device::mos5710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6526_device(mconfig, MOS5710, tag, owner, clock, TYPE_5710) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6526_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	m_flag = 1;
	m_cnt = 1;
	m_cra = 0;

	// allocate timer
	if (m_tod_clock != 0)
	{
		m_tod_timer = timer_alloc(FUNC(mos6526_device::advance_tod_clock), this);
		m_tod_timer->adjust(attotime::from_hz(m_tod_clock), 0, attotime::from_hz(m_tod_clock));
	}

	// state saving
	save_item(NAME(m_ir0));
	save_item(NAME(m_ir1));
	save_item(NAME(m_icr));
	save_item(NAME(m_imr));
	save_item(NAME(m_pc));
	save_item(NAME(m_flag));
	save_item(NAME(m_pra));
	save_item(NAME(m_prb));
	save_item(NAME(m_ddra));
	save_item(NAME(m_ddrb));
	save_item(NAME(m_sp));
	save_item(NAME(m_cnt));
	save_item(NAME(m_sdr));
	save_item(NAME(m_shift));
	save_item(NAME(m_sdr_empty));
	save_item(NAME(m_bits));

	save_item(NAME(m_ta_out));
	save_item(NAME(m_tb_out));
	save_item(NAME(m_ta_pb6));
	save_item(NAME(m_tb_pb7));
	save_item(NAME(m_ta));
	save_item(NAME(m_tb));
	save_item(NAME(m_ta_latch));
	save_item(NAME(m_tb_latch));
	save_item(NAME(m_cra));
	save_item(NAME(m_crb));

	save_item(NAME(m_tod_count));
	save_item(NAME(m_tod));
	save_item(NAME(m_tod_latch));
	save_item(NAME(m_alarm));
	save_item(NAME(m_tod_stopped));
	save_item(NAME(m_tod_latched));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6526_device::device_reset()
{
	m_irq = false;
	m_ir0 = 0;
	m_ir1 = 0;
	m_icr = 0;
	m_imr = 0;
	m_icr_read = false;

	m_pc = 1;
	m_flag = 1;
	m_pra = 0;
	m_prb = 0;
	m_ddra = 0;
	m_ddrb = 0;
	m_pa = 0xff;
	m_pb = 0xff;
	m_pa_in = 0;
	m_pb_in = 0;

	m_sp = 1;
	m_cnt = 1;
	m_sdr = 0;
	m_shift = 0;
	m_sdr_empty = true;
	m_bits = 0;

	m_ta_out = 0;
	m_tb_out = 0;
	m_ta_pb6 = 0;
	m_tb_pb7 = 0;
	m_count_a0 = 0;
	m_count_a1 = 0;
	m_count_a2 = 0;
	m_count_a3 = 0;
	m_load_a0 = 0;
	m_load_a1 = 0;
	m_load_a2 = 0;
	m_oneshot_a0 = 0;
	m_count_b0 = 0;
	m_count_b1 = 0;
	m_count_b2 = 0;
	m_count_b3 = 0;
	m_load_b0 = 0;
	m_load_b1 = 0;
	m_load_b2 = 0;
	m_oneshot_b0 = 0;
	// initial state is confirmed floating high as per vAmigaTS/showcia1
	m_ta = 0xffff;
	m_tb = 0xffff;
	m_ta_latch = 0xffff;
	m_tb_latch = 0xffff;
	m_cra = 0;
	m_crb = 0;

	m_tod_count = 0;
	m_tod = 0x01000000L;
	m_tod_latch = 0;
	m_alarm = 0;
	m_tod_stopped = true;
	m_tod_latched = false;

	m_write_irq(CLEAR_LINE);
	m_write_pc(m_pc);
	m_write_sp(m_sp);
	m_write_cnt(m_cnt);
}


TIMER_CALLBACK_MEMBER(mos6526_device::advance_tod_clock)
{
	tod_w(1);
	tod_w(0);
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void mos6526_device::execute_run()
{
	do
	{
		synchronize();

		m_icount--;
	} while (m_icount > 0);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t mos6526_device::read(offs_t offset)
{
	uint8_t data = 0;

	switch (offset & 0x0f)
	{
	case PRA:
		if (machine().side_effects_disabled())
			return 0xff;

		if (m_ddra != 0xff)
			data = (m_read_pa(0) & ~m_ddra) | (m_pra & m_ddra);
		else
			data = m_read_pa(0) & m_pra;
		m_pa_in = data;
		break;

	case PRB:
		if (machine().side_effects_disabled())
			return 0xff;

		if (m_ddrb != 0xff)
			data = (m_read_pb(0) & ~m_ddrb) | (m_prb & m_ddrb);
		else
			data = m_read_pb(0) & m_prb;
		m_pb_in = data;

		if (CRA_PBON)
		{
			int pb6 = CRA_OUTMODE ? m_ta_pb6 : m_ta_out;

			data &= ~0x40;
			data |= pb6 << 6;
		}

		if (CRB_PBON)
		{
			int pb7 = CRB_OUTMODE ? m_tb_pb7 : m_tb_out;

			data &= ~0x80;
			data |= pb7 << 7;
		}

		m_pc = 0;
		m_write_pc(m_pc);
		break;

	case DDRA:
		data = m_ddra;
		break;

	case DDRB:
		data = m_ddrb;
		break;

	case TA_LO:
		data = m_ta & 0xff;
		break;

	case TA_HI:
		data = m_ta >> 8;
		break;

	case TB_LO:
		data = m_tb & 0xff;
		break;

	case TB_HI:
		data = m_tb >> 8;
		break;

	case TOD_10THS:
		if (machine().side_effects_disabled())
			return 0xff;

		data = read_tod(0);

		m_tod_latched = false;
		break;

	case TOD_SEC:
		if (machine().side_effects_disabled())
			return 0xff;

		data = read_tod(1);
		break;

	case TOD_MIN:
		if (machine().side_effects_disabled())
			return 0xff;

		data = read_tod(2);
		break;

	case TOD_HR:
		if (machine().side_effects_disabled())
			return 0xff;

		if (!m_tod_latched)
		{
			m_tod_latched = true;
			m_tod_latch = m_tod;
		}

		data = read_tod(3);
		break;

	case SDR:
		data = m_sdr;
		break;

	case ICR:
		data = (m_ir1 << 7) | m_icr;

		// Do not reset irqs unless one is effectively issued.
		// cfr. amigaocs_flop.xml barb2paln4 that polls for Timer B status
		//      until it expires at PC=7821c and other places.
		if (machine().side_effects_disabled() || !m_icr)
			return data;

		m_icr_read = true;

		m_ir0 = 0;
		m_ir1 = 0;
		m_icr = 0;
		m_irq = false;
		m_write_irq(CLEAR_LINE);
		break;

	case CRA:
		data = m_cra;
		break;

	case CRB:
		data = m_crb;
		break;
	}

	return data;
}

uint8_t mos8520_device::read(offs_t offset)
{
	uint8_t data;

	switch (offset & 0x0f)
	{
	case TOD_MIN:
		// tod is not latched when CRB_ALARM is set
		// test case: amigaocs_flop:batman1
		if (!m_tod_latched && !CRB_ALARM)
		{
			m_tod_latched = true;
			m_tod_latch = m_tod;
		}

		data = read_tod(2);
		break;

	// unused register returns floating high as per vAmigaTS/showcia1
	case TOD_HR:
		data = 0xff;
		break;

	default:
		data = mos6526_device::read(offset);
	}

	return data;
}

//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos6526_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x0f)
	{
	case PRA:
		m_pra = data;
		update_pa();
		break;

	case PRB:
		m_prb = data;
		update_pb();

		m_pc = 0;
		m_write_pc(m_pc);
		break;

	case DDRA:
		m_ddra = data;
		update_pa();
		break;

	case DDRB:
		m_ddrb = data;
		update_pb();
		break;

	case TA_LO:
		m_ta_latch = (m_ta_latch & 0xff00) | data;

		if (m_load_a2)
		{
			m_ta = (m_ta & 0xff00) | data;
		}
		break;

	case TA_HI:
		m_ta_latch = (data << 8) | (m_ta_latch & 0xff);

		if (!CRA_STARTED)
		{
			m_load_a0 = 1;
		}

		if (CRA_RUNMODE)
		{
			m_ta = m_ta_latch;
			set_cra(m_cra | CRA_START);
		}

		if (m_load_a2)
		{
			m_ta = (data << 8) | (m_ta & 0xff);
		}
		break;

	case TB_LO:
		m_tb_latch = (m_tb_latch & 0xff00) | data;

		if (m_load_b2)
		{
			m_tb = (m_tb & 0xff00) | data;
		}
		break;

	case TB_HI:
		m_tb_latch = (data << 8) | (m_tb_latch & 0xff);

		if (!CRB_STARTED)
		{
			m_load_b0 = 1;
		}

		if (CRB_RUNMODE)
		{
			m_tb = m_tb_latch;
			set_crb(m_crb | CRB_START);
		}

		if (m_load_b2)
		{
			m_tb = (data << 8) | (m_tb & 0xff);
		}
		break;

	case TOD_10THS:
		write_tod(0, data);

		m_tod_stopped = false;
		break;

	case TOD_SEC:
		write_tod(1, data);
		break;

	case TOD_MIN:
		write_tod(2, data);
		break;

	case TOD_HR:
		m_tod_stopped = true;

		if (((data & 0x1f) == 0x12) && !CRB_ALARM)
		{
			// toggle AM/PM flag
			data ^= 0x80;
		}

		write_tod(3, data);
		break;

	case SDR:
		m_sdr = data;
		m_sdr_empty = false;
		break;

	case IMR:
		if (IMR_SET)
		{
			m_imr |= (data & 0x1f);
		}
		else
		{
			m_imr &= ~(data & 0x1f);
		}

		if (!m_irq && (m_icr & m_imr))
		{
			m_ir0 = 1;
		}
		break;

	case CRA:
		set_cra(data);
		break;

	case CRB:
		set_crb(data);
		break;
	}
}

void mos8520_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x0f)
	{
	default:
		mos6526_device::write(offset, data);
		break;

	case TOD_MIN:
		m_tod_stopped = true;
		write_tod(2, data);
		break;

	case TOD_HR:
		// ignored in mos8520
		break;
	}
}


//-------------------------------------------------
//  sp_w - serial port write
//-------------------------------------------------

void mos6526_device::sp_w(int state)
{
	m_sp = state;
}


//-------------------------------------------------
//  cnt_w - serial counter write
//-------------------------------------------------

void mos6526_device::cnt_w(int state)
{
	if (CRA_SPMODE) return;

	if (!m_cnt && state)
	{
		serial_input();

		if (CRA_INMODE == CRA_INMODE_CNT)
			m_ta--;

		if (CRB_INMODE == CRB_INMODE_CNT)
			m_tb--;
	}

	m_cnt = state;
}


//-------------------------------------------------
//  flag_w - flag write
//-------------------------------------------------

void mos6526_device::flag_w(int state)
{
	if (m_flag && !state)
	{
		m_icr |= ICR_FLAG;
	}

	m_flag = state;
}


//-------------------------------------------------
//  tod_w - time-of-day clock write
//-------------------------------------------------

void mos6526_device::tod_w(int state)
{
	if (state && !m_tod_stopped)
	{
		clock_tod();

		if (m_tod == m_alarm)
		{
			m_icr |= ICR_ALARM;
		}
	}
}
