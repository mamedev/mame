/**********************************************************************

    MOS 6526/8520 Complex Interface Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

*/

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
#define ICR_TA		0x01
#define ICR_TB		0x02
#define ICR_ALARM	0x04
#define ICR_SP		0x08
#define ICR_FLAG	0x10


// interrupt mask register
#define IMR_TA		BIT(m_imr, 0)
#define IMR_TB		BIT(m_imr, 1)
#define IMR_ALARM	BIT(m_imr, 2)
#define IMR_SP		BIT(m_imr, 3)
#define IMR_FLAG	BIT(m_imr, 4)
#define IMR_SET		BIT(data, 7)


// control register A
enum
{
	CRA_INMODE_PHI2 = 0,
	CRA_INMODE_CNT
};

#define CRA_START		0x01
#define CRA_STARTED		BIT(m_cra, 0)
#define CRA_PBON		BIT(m_cra, 1)
#define CRA_OUTMODE 	BIT(m_cra, 2)
#define CRA_RUNMODE 	BIT(m_cra, 3)
#define CRA_LOAD		BIT(m_cra, 4)
#define CRA_INMODE		BIT(m_cra, 5)
#define CRA_SPMODE		BIT(m_cra, 6)
#define CRA_TODIN		BIT(m_cra, 7)


// control register B
enum
{
	CRB_INMODE_PHI2 = 0,
	CRB_INMODE_CNT,
	CRB_INMODE_TA,
	CRB_INMODE_CNT_TA
};

#define CRB_START		0x01
#define CRB_STARTED		BIT(m_crb, 0)
#define CRB_PBON		BIT(m_crb, 1)
#define CRB_OUTMODE 	BIT(m_crb, 2)
#define CRB_RUNMODE 	BIT(m_crb, 3)
#define CRB_LOAD		BIT(m_crb, 4)
#define CRB_INMODE		((m_crb & 0x60) >> 5)
#define CRB_ALARM		BIT(m_crb, 7)



//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

const device_type MOS6526 = &device_creator<mos6526_device>;
const device_type MOS6526A = &device_creator<mos6526a_device>;
const device_type MOS8520 = &device_creator<mos8520_device>;
const device_type MOS5710 = &device_creator<mos5710_device>;


//-------------------------------------------------
//  static_set_tod_clock -
//-------------------------------------------------

void mos6526_device::static_set_tod_clock(device_t &device, int tod_clock)
{
	mos6526_device &cia = dynamic_cast<mos6526_device &>(device);

	cia.m_tod_clock = tod_clock;
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mos6526_device::device_config_complete()
{
	// inherit a copy of the static data
	const mos6526_interface *intf = reinterpret_cast<const mos6526_interface *>(static_config());
	if (intf != NULL)
		*static_cast<mos6526_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
    	memset(&m_out_pc_cb, 0, sizeof(m_out_pc_cb));
    	memset(&m_out_cnt_cb, 0, sizeof(m_out_cnt_cb));
    	memset(&m_out_sp_cb, 0, sizeof(m_out_sp_cb));
    	memset(&m_in_pa_cb, 0, sizeof(m_in_pa_cb));
    	memset(&m_out_pa_cb, 0, sizeof(m_out_pa_cb));
    	memset(&m_in_pb_cb, 0, sizeof(m_in_pb_cb));
    	memset(&m_out_pb_cb, 0, sizeof(m_out_pb_cb));
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_pa - update port A
//-------------------------------------------------

inline void mos6526_device::update_pa()
{
	UINT8 pa = m_pra | ~m_ddra;

	if (m_pa != pa)
	{
		m_pa = pa;
		m_out_pa_func(0, pa);
	}
}


//-------------------------------------------------
//  update_pb - update port B
//-------------------------------------------------

inline void mos6526_device::update_pb()
{
	UINT8 pb = m_prb | ~m_ddrb;

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
		m_out_pb_func(0, pb);
		m_pb = pb;
	}
}


//-------------------------------------------------
//  set_cra - control register A write
//-------------------------------------------------

inline void mos6526_device::set_cra(UINT8 data)
{
	if (!CRA_STARTED && (data & CRA_START))
	{
		m_ta_pb6 = 1;
	}

	m_cra = data;
	update_pb();
}


//-------------------------------------------------
//  set_crb - control register B write
//-------------------------------------------------

inline void mos6526_device::set_crb(UINT8 data)
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

inline UINT8 mos6526_device::bcd_increment(UINT8 value)
{
	value++;

	if ((value & 0x0f) >= 0x0a)
		value += 0x10 - 0x0a;

	return value;
}


//-------------------------------------------------
//  clock_tod - time-of-day clock pulse
//-------------------------------------------------

inline void mos6526_device::clock_tod()
{
	UINT8 subsecond	= (UINT8) (m_tod >>  0);
	UINT8 second	= (UINT8) (m_tod >>  8);
	UINT8 minute	= (UINT8) (m_tod >> 16);
	UINT8 hour		= (UINT8) (m_tod >> 24);

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

	m_tod = (((UINT32) subsecond)	<<  0)
		  | (((UINT32) second)		<<  8)
		  | (((UINT32) minute)		<< 16)
		  | (((UINT32) hour)		<< 24);
}


//-------------------------------------------------
//  clock_tod - time-of-day clock pulse
//-------------------------------------------------

inline void mos8520_device::clock_tod()
{
	m_tod++;
	m_tod &= 0xffffff;
}


//-------------------------------------------------
//  read_tod - time-of-day read
//-------------------------------------------------

inline UINT8 mos6526_device::read_tod(int offset)
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

inline void mos6526_device::write_tod(int offset, UINT8 data)
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

inline void mos6526_device::serial_input()
{
	if (m_count_a0 && !CRA_SPMODE)
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
}


//-------------------------------------------------
//  clock_ta - clock timer A
//-------------------------------------------------

inline void mos6526_device::clock_ta()
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

inline void mos6526_device::serial_output()
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
				m_out_sp_func(m_sp);

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
			m_out_cnt_func(m_cnt);
		}
	}
}


//-------------------------------------------------
//  clock_tb - clock timer B
//-------------------------------------------------

inline void mos6526_device::clock_tb()
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

inline void mos6526_device::update_interrupt()
{
	if (!m_irq && m_ir1)
	{
		m_out_irq_func(ASSERT_LINE);
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

inline void mos6526_device::clock_pipeline()
{
	// timer A pipeline
	m_count_a3 = m_count_a2;

	switch (CRA_INMODE)
	{
	case CRA_INMODE_PHI2:
		m_count_a2 = 1;
		break;

	case CRA_INMODE_CNT:
		m_count_a2 = m_count_a1;
		break;
	}

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

	case CRB_INMODE_CNT:
		m_count_b2 = m_count_b1;
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

inline void mos6526_device::synchronize()
{
	if (!m_pc)
	{
		m_pc = 1;
		m_out_pc_func(m_pc);
	}

	serial_input();

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

mos6526_device::mos6526_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant)
    : device_t(mconfig, type, name, tag, owner, clock),
	  device_execute_interface(mconfig, *this),
	  m_icount(0),
	  m_variant(variant)
{
}

mos6526_device::mos6526_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, MOS6526, "MOS6526", tag, owner, clock),
	  device_execute_interface(mconfig, *this),
	  m_icount(0),
	  m_variant(TYPE_6526)
{ }

mos6526a_device::mos6526a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : mos6526_device(mconfig, MOS6526A, "MOS6526A", tag, owner, clock, TYPE_6526A) { }

mos8520_device::mos8520_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : mos6526_device(mconfig, MOS8520, "MOS8520", tag, owner, clock, TYPE_8520) { }

mos5710_device::mos5710_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : mos6526_device(mconfig, MOS5710, "MOS5710", tag, owner, clock, TYPE_5710) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6526_device::device_start()
{
	// set our instruction counter
	m_icountptr = &m_icount;

	// resolve callbacks
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_pc_func.resolve(m_out_pc_cb, *this);
	m_out_cnt_func.resolve(m_out_cnt_cb, *this);
	m_out_sp_func.resolve(m_out_sp_cb, *this);
	m_in_pa_func.resolve(m_in_pa_cb, *this);
	m_out_pa_func.resolve(m_out_pa_cb, *this);
	m_in_pb_func.resolve(m_in_pb_cb, *this);
	m_out_pb_func.resolve(m_out_pb_cb, *this);

	// allocate timer
	if (m_tod_clock > 0)
	{
		m_tod_timer = timer_alloc();
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
	m_ta = 0;
	m_tb = 0;
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

	m_out_irq_func(CLEAR_LINE);
	m_out_pc_func(m_pc);
	m_out_sp_func(m_sp);
	m_out_cnt_func(m_cnt);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mos6526_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (!m_tod_stopped)
	{
		clock_tod();

		if (m_tod == m_alarm)
		{
			m_icr |= ICR_ALARM;
		}
	}
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

READ8_MEMBER( mos6526_device::read )
{
	UINT8 data = 0;

	switch (offset)
	{
	case PRA:
		data = (m_in_pa_func(0) & ~m_ddra) | (m_pra & m_ddra);
		break;

	case PRB:
		data = (m_in_pb_func(0) & ~m_ddrb) | (m_prb & m_ddrb);

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
		m_out_pc_func(m_pc);
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
		data = read_tod(0);

		m_tod_latched = false;
		break;

	case TOD_SEC:
		data = read_tod(1);
		break;

	case TOD_MIN:
		data = read_tod(2);
		break;

	case TOD_HR:
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

		m_icr_read = true;

		m_ir0 = 0;
		m_ir1 = 0;
		m_icr = 0;
		m_irq = false;
		m_out_irq_func(CLEAR_LINE);
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


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( mos6526_device::write )
{
	switch (offset)
	{
	case PRA:
		m_pra = data;
		update_pa();
		break;

	case PRB:
		m_prb = data;
		update_pb();

		m_pc = 0;
		m_out_pc_func(m_pc);
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


//-------------------------------------------------
//  pa_r - port A read
//-------------------------------------------------

UINT8 mos6526_device::pa_r()
{
	return m_pa;
}


//-------------------------------------------------
//  pb_r - port B read
//-------------------------------------------------

UINT8 mos6526_device::pb_r()
{
	return m_pb;
}


//-------------------------------------------------
//  sp_r - serial port read
//-------------------------------------------------

READ_LINE_MEMBER( mos6526_device::sp_r )
{
	return m_sp;
}


//-------------------------------------------------
//  sp_w - serial port write
//-------------------------------------------------

WRITE_LINE_MEMBER( mos6526_device::sp_w )
{
	m_sp = state;
}


//-------------------------------------------------
//  cnt_r - serial counter read
//-------------------------------------------------

READ_LINE_MEMBER( mos6526_device::cnt_r )
{
	return m_cnt;
}


//-------------------------------------------------
//  cnt_w - serial counter write
//-------------------------------------------------

WRITE_LINE_MEMBER( mos6526_device::cnt_w )
{
	if (CRA_SPMODE) return;

	if (!m_cnt && state)
	{
		m_count_a0 = 1;
		m_count_b0 = 1;
	}

	m_cnt = state;
}


//-------------------------------------------------
//  flag_w - flag write
//-------------------------------------------------

WRITE_LINE_MEMBER( mos6526_device::flag_w )
{
	if (m_flag && !state)
	{
		m_icr |= ICR_FLAG;
	}

	m_flag = state;
}
