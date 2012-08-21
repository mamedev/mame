/***************************************************************************

    Sony PocketStation

    05/2009 Skeleton driver written.
    11/2009 Initial bring-up commenced by Harmony.
    11/2009 Playable state achieved by Harmony.

    PocketStation games were dowloaded from PS1 games into flash RAM after
    the unit had been inserted in the memory card slot, and so this should
    be emulated alongside the PS1.  However, as many flash dumps exist, it
    is possible to emulate the PocketStation in the meantime.

    CPU: ARM7T (32 bit RISC Processor)
    Memory: 2Kbytes of SRAM, 128Kbytes of FlashROM
    Graphics: 32x32 monochrome LCD
    Sound: 1 12-bit PCM channel
    Input: 5 input buttons, 1 reset button
    Infrared communication: Bi-directional and uni-directional comms
    Other: 1 LED indicator

    Info available at:
      * http://exophase.devzero.co.uk/ps_info.txt
      * http://members.at.infoseek.co.jp/DrHell/pocket/index.html

    Currently, a handful of games run, but some die due to odd hardware
    issues.

****************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "imagedev/cartslot.h"
#include "sound/dac.h"

#define MAX_PS_TIMERS	3

typedef struct
{
	UINT32 control;
	UINT32 stat;
	UINT32 valid;
	UINT32 wait1;
	UINT32 wait2;
	UINT32 entry[16];
	UINT32 serial;
} ps_ftlb_regs_t;

typedef struct
{
	UINT32 hold;
	UINT32 status;
	UINT32 enable;
	UINT32 mask;
} ps_intc_regs_t;

typedef struct
{
	UINT32 period;
	UINT32 count;
	UINT32 control;
	emu_timer *timer;
} ps_timer_t;

typedef struct
{
	ps_timer_t timer[MAX_PS_TIMERS];
} ps_timer_regs_t;

typedef struct
{
	UINT32 mode;
	UINT32 control;
} ps_clock_regs_t;

#define PS_CLOCK_STEADY		0x10

typedef struct
{
	UINT32 mode;
	UINT32 control;
	UINT32 time;
	UINT32 date;
	emu_timer *timer;
} ps_rtc_regs_t;



class pockstat_state : public driver_device
{
public:
	pockstat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_lcd_buffer(*this, "lcd_buffer"){ }

	required_shared_ptr<UINT32> m_lcd_buffer;
	ps_ftlb_regs_t m_ftlb_regs;
	ps_intc_regs_t m_intc_regs;
	ps_timer_regs_t m_timer_regs;
	ps_clock_regs_t m_clock_regs;
	ps_rtc_regs_t m_rtc_regs;
	UINT32 m_lcd_control;
	INT32 m_ps_flash_write_enable_count;
	INT32 m_ps_flash_write_count;
	DECLARE_READ32_MEMBER(ps_ftlb_r);
	DECLARE_WRITE32_MEMBER(ps_ftlb_w);
	DECLARE_READ32_MEMBER(ps_intc_r);
	DECLARE_WRITE32_MEMBER(ps_intc_w);
	DECLARE_READ32_MEMBER(ps_timer_r);
	DECLARE_WRITE32_MEMBER(ps_timer_w);
	DECLARE_READ32_MEMBER(ps_clock_r);
	DECLARE_WRITE32_MEMBER(ps_clock_w);
	DECLARE_READ32_MEMBER(ps_rtc_r);
	DECLARE_WRITE32_MEMBER(ps_rtc_w);
	DECLARE_READ32_MEMBER(ps_lcd_r);
	DECLARE_WRITE32_MEMBER(ps_lcd_w);
	DECLARE_READ32_MEMBER(ps_rombank_r);
	DECLARE_WRITE32_MEMBER(ps_flash_w);
	DECLARE_READ32_MEMBER(ps_audio_r);
	DECLARE_WRITE32_MEMBER(ps_audio_w);
	DECLARE_WRITE32_MEMBER(ps_dac_w);
};


#define DEFAULT_CLOCK	2000000

static const int CPU_FREQ[16] =
{
	0x00f800,
	0x01f000,
	0x03e000,
	0x07c000,
	0x0f8000,
	0x1e8000,
	0x3d0000,
	0x7a0000,
	0x7a0000,
	0x7a0000,
	0x7a0000,
	0x7a0000,
	0x7a0000,
	0x7a0000,
	0x7a0000,
	0x7a0000
};

#define VERBOSE_LEVEL		(0)

#define ENABLE_VERBOSE_LOG	(0)

#if ENABLE_VERBOSE_LOG
INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine.describe_context(), buf );
	}
}
#else
#define verboselog(x,y,z,...)
#endif


// Flash TLB



// Interrupt Controller
static UINT32 ps_intc_get_interrupt_line(running_machine &machine, UINT32 line);
static void ps_intc_set_interrupt_line(running_machine &machine, UINT32 line, int state);



// Timers
static TIMER_CALLBACK( timer_tick );
static void ps_timer_start(running_machine &machine, int index);



// Clock



// RTC
static TIMER_CALLBACK( rtc_tick );



#define PS_INT_BTN_ACTION		0x00000001 // "Action button"
#define PS_INT_BTN_RIGHT		0x00000002 // "Right button"
#define PS_INT_BTN_LEFT			0x00000004 // "Left button"
#define PS_INT_BTN_DOWN			0x00000008 // "Down button"
#define PS_INT_BTN_UP			0x00000010 // "Up button"
#define PS_INT_UNKNOWN			0x00000020 // "Unknown"
#define PS_INT_COM				0x00000040 // "COM" ???
#define PS_INT_TIMER0			0x00000080 // "Timer 0"
#define PS_INT_TIMER1			0x00000100 // "Timer 1"
#define PS_INT_RTC				0x00000200 // "RTC"
#define PS_INT_BATTERY			0x00000400 // "Battery Monitor"
#define PS_INT_IOP				0x00000800 // "IOP"
#define PS_INT_IRDA				0x00001000 // "IrDA"
#define PS_INT_TIMER2			0x00002000 // "Timer 2"
#define PS_INT_IRQ_MASK			0x00001fbf
#define PS_INT_FIQ_MASK			0x00002040
#define PS_INT_STATUS_MASK		0x0000021f

READ32_MEMBER(pockstat_state::ps_ftlb_r)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(machine(), 0, "ps_ftlb_r: FlashROM TLB Control = %08x & %08x\n", m_ftlb_regs.control, mem_mask );
			return m_ftlb_regs.control | 1; // ???
		case 0x0004/4:
			verboselog(machine(), 0, "ps_ftlb_r: Unknown (F_STAT) = %08x & %08x\n", m_ftlb_regs.stat, mem_mask );
			return m_ftlb_regs.stat;
		case 0x0008/4:
			verboselog(machine(), 0, "ps_ftlb_r: FlashROM TLB Valid Tag = %08x & %08x\n", m_ftlb_regs.valid, mem_mask );
			return m_ftlb_regs.valid;
		case 0x000c/4:
			verboselog(machine(), 0, "ps_ftlb_r: Unknown (F_WAIT1) = %08x & %08x\n", m_ftlb_regs.wait1, mem_mask );
			return m_ftlb_regs.wait1;
		case 0x0010/4:
			verboselog(machine(), 0, "ps_ftlb_r: Unknown (F_WAIT2) = %08x & %08x\n", m_ftlb_regs.wait2 | 0x04, mem_mask );
			return m_ftlb_regs.wait2 | 0x04;
		case 0x0100/4:
		case 0x0104/4:
		case 0x0108/4:
		case 0x010c/4:
		case 0x0110/4:
		case 0x0114/4:
		case 0x0118/4:
		case 0x011c/4:
		case 0x0120/4:
		case 0x0124/4:
		case 0x0128/4:
		case 0x012c/4:
		case 0x0130/4:
		case 0x0134/4:
		case 0x0138/4:
		case 0x013c/4:
			verboselog(machine(), 0, "ps_ftlb_r: FlashROM TLB Entry %d = %08x & %08x\n", offset - 0x100/4, m_ftlb_regs.entry[offset - 0x100/4], mem_mask );
			return m_ftlb_regs.entry[offset - 0x100/4];
		case 0x0300/4:
			verboselog(machine(), 0, "ps_ftlb_r: Unknown (F_SN) = %08x & %08x\n", m_ftlb_regs.serial, mem_mask );
			return m_ftlb_regs.serial;
		default:
			verboselog(machine(), 0, "ps_ftlb_r: Unknown Register %08x & %08x\n", 0x06000000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_ftlb_w)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(machine(), 0, "ps_ftlb_w: FlashROM TLB Control = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.control);
			break;
		case 0x0004/4:
			verboselog(machine(), 0, "ps_ftlb_w: Unknown (F_STAT) = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.stat);
			break;
		case 0x0008/4:
			verboselog(machine(), 0, "ps_ftlb_w: FlashROM TLB Valid Tag = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.valid);
			break;
		case 0x000c/4:
			verboselog(machine(), 0, "ps_ftlb_w: Unknown (F_WAIT1) = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.wait1);
			break;
		case 0x0010/4:
			verboselog(machine(), 0, "ps_ftlb_w: Unknown (F_WAIT2) = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.wait2);
			break;
		case 0x0100/4:
		case 0x0104/4:
		case 0x0108/4:
		case 0x010c/4:
		case 0x0110/4:
		case 0x0114/4:
		case 0x0118/4:
		case 0x011c/4:
		case 0x0120/4:
		case 0x0124/4:
		case 0x0128/4:
		case 0x012c/4:
		case 0x0130/4:
		case 0x0134/4:
		case 0x0138/4:
		case 0x013c/4:
			verboselog(machine(), 0, "ps_ftlb_w: FlashROM TLB Entry %d = %08x & %08x\n", offset - 0x100/4, data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.entry[offset - 0x100/4]);
			break;
		case 0x0300/4:
			verboselog(machine(), 0, "ps_ftlb_w: Unknown (F_SN) = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.serial);
			break;
		default:
			verboselog(machine(), 0, "ps_ftlb_w: Unknown Register %08x = %08x & %08x\n", 0x06000000 + (offset << 2), data, mem_mask );
			break;
	}
}

static UINT32 ps_intc_get_interrupt_line(running_machine &machine, UINT32 line)
{
	pockstat_state *state = machine.driver_data<pockstat_state>();
	return state->m_intc_regs.status & line;
}

static void ps_intc_set_interrupt_line(running_machine &machine, UINT32 line, int state)
{
	pockstat_state *drvstate = machine.driver_data<pockstat_state>();
	//printf( "%08x %d %08x %08x %08x\n", line, state, drvstate->m_intc_regs.hold, drvstate->m_intc_regs.status, drvstate->m_intc_regs.enable );
	if(line)
	{
		if(state)
		{
			drvstate->m_intc_regs.status |= line & PS_INT_STATUS_MASK;
			drvstate->m_intc_regs.hold |= line &~ PS_INT_STATUS_MASK;
			//printf( " Setting %08x, status = %08x, hold = %08x\n", line, drvstate->m_intc_regs.status, drvstate->m_intc_regs.hold );
		}
		else
		{
			drvstate->m_intc_regs.status &= ~line;
			drvstate->m_intc_regs.hold &= ~line;
			//printf( "Clearing %08x, status = %08x, hold = %08x\n", line, drvstate->m_intc_regs.status, drvstate->m_intc_regs.hold );
		}
	}
	if(drvstate->m_intc_regs.hold & drvstate->m_intc_regs.enable & PS_INT_IRQ_MASK)
	{
		device_set_input_line(machine.device("maincpu"), ARM7_IRQ_LINE, ASSERT_LINE);
	}
	else
	{
		device_set_input_line(machine.device("maincpu"), ARM7_IRQ_LINE, CLEAR_LINE);
	}
	if(drvstate->m_intc_regs.hold & drvstate->m_intc_regs.enable & PS_INT_FIQ_MASK)
	{
		device_set_input_line(machine.device("maincpu"), ARM7_FIRQ_LINE, ASSERT_LINE);
	}
	else
	{
		device_set_input_line(machine.device("maincpu"), ARM7_FIRQ_LINE, CLEAR_LINE);
	}
}

READ32_MEMBER(pockstat_state::ps_intc_r)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(machine(), 0, "ps_intc_r: Held Interrupt = %08x & %08x\n", m_intc_regs.hold, mem_mask );
			return m_intc_regs.hold;
		case 0x0004/4:
			verboselog(machine(), 0, "ps_intc_r: Interrupt Status = %08x & %08x\n", m_intc_regs.status, mem_mask );
			return m_intc_regs.status;
		case 0x0008/4:
			verboselog(machine(), 0, "ps_intc_r: Interrupt Enable = %08x & %08x\n", m_intc_regs.enable, mem_mask );
			return m_intc_regs.enable;
		case 0x000c/4:
			verboselog(machine(), 0, "ps_intc_r: Interrupt Mask (Invalid Read) = %08x & %08x\n", 0, mem_mask );
			return 0;
		case 0x0010/4:
			verboselog(machine(), 0, "ps_intc_r: Interrupt Acknowledge (Invalid Read) = %08x & %08x\n", 0, mem_mask );
			return 0;
		default:
			verboselog(machine(), 0, "ps_intc_r: Unknown Register %08x & %08x\n", 0x0a000000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_intc_w)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(machine(), 0, "ps_intc_w: Held Interrupt (Invalid Write) = %08x & %08x\n", data, mem_mask );
			break;
		case 0x0004/4:
			verboselog(machine(), 0, "ps_intc_w: Interrupt Status (Invalid Write) = %08x & %08x\n", data, mem_mask );
			break;
		case 0x0008/4:
			verboselog(machine(), 0, "ps_intc_w: Interrupt Enable = %08x & %08x\n", data, mem_mask );
			m_intc_regs.enable |= data;
			//COMBINE_DATA(&m_intc_regs.enable);
			//m_intc_regs.status &= m_intc_regs.enable;
			//m_intc_regs.hold &= m_intc_regs.enable;
			ps_intc_set_interrupt_line(machine(), 0, 0);
			break;
		case 0x000c/4:
			verboselog(machine(), 0, "ps_intc_w: Interrupt Mask = %08x & %08x\n", data, mem_mask );
			m_intc_regs.enable &= ~data;
			COMBINE_DATA(&m_intc_regs.mask);
			//m_intc_regs.status &= m_intc_regs.enable;
			//m_intc_regs.hold &= m_intc_regs.enable;
			ps_intc_set_interrupt_line(machine(), 0, 0);
			break;
		case 0x0010/4:
			verboselog(machine(), 0, "ps_intc_w: Interrupt Acknowledge = %08x & %08x\n", data, mem_mask );
			m_intc_regs.hold &= ~data;
			m_intc_regs.status &= ~data;
			ps_intc_set_interrupt_line(machine(), 0, 0);
			//COMBINE_DATA(&m_intc_regs.acknowledge);
			break;
		default:
			verboselog(machine(), 0, "ps_intc_w: Unknown Register %08x = %08x & %08x\n", 0x0a000000 + (offset << 2), data, mem_mask );
			break;
	}
}

static TIMER_CALLBACK( timer_tick )
{
	pockstat_state *state = machine.driver_data<pockstat_state>();
	ps_intc_set_interrupt_line(machine, param == 2 ? PS_INT_TIMER2 : (param == 1 ? PS_INT_TIMER1 : PS_INT_TIMER0), 1);
	//printf( "Timer %d is calling back\n", param );
	state->m_timer_regs.timer[param].count = state->m_timer_regs.timer[param].period;
	ps_timer_start(machine, param);
}

static void ps_timer_start(running_machine &machine, int index)
{
	pockstat_state *state = machine.driver_data<pockstat_state>();
	int divisor = 1;
	attotime period;
	switch(state->m_timer_regs.timer[index].control & 3)
	{
		case 0:
		case 3:
			divisor = 1;
			break;
		case 1:
			divisor = 16;
			break;
		case 2:
			divisor = 256;
			break;
	}
	period = attotime::from_hz(CPU_FREQ[state->m_clock_regs.mode & 0x0f] / 2) * divisor;
	period = period * state->m_timer_regs.timer[index].count;
	state->m_timer_regs.timer[index].timer->adjust(period, index);
}

READ32_MEMBER(pockstat_state::ps_timer_r)
{
	switch(offset)
	{
		case 0x0000/4:
		case 0x0010/4:
		case 0x0020/4:
			verboselog(machine(), 0, "ps_timer_r: Timer %d Period = %08x & %08x\n", offset / (0x10/4), m_timer_regs.timer[offset / (0x10/4)].period, mem_mask );
			return m_timer_regs.timer[offset / (0x10/4)].period;
		case 0x0004/4:
		case 0x0014/4:
		case 0x0024/4:
			verboselog(machine(), 0, "ps_timer_r: Timer %d Count = %08x & %08x\n", offset / (0x10/4), m_timer_regs.timer[offset / (0x10/4)].count, mem_mask );
			if(m_timer_regs.timer[offset / (0x10/4)].control & 4)
			{
				m_timer_regs.timer[offset / (0x10/4)].count--;
				if(m_timer_regs.timer[offset / (0x10/4)].count > m_timer_regs.timer[offset / (0x10/4)].period)
				{
					m_timer_regs.timer[offset / (0x10/4)].count = m_timer_regs.timer[offset / (0x10/4)].period;
				}
				return --m_timer_regs.timer[offset / (0x10/4)].count;
			}
			return m_timer_regs.timer[offset / (0x10/4)].count;
		case 0x0008/4:
		case 0x0018/4:
		case 0x0028/4:
			verboselog(machine(), 0, "ps_timer_r: Timer %d Control = %08x & %08x\n", offset / (0x10/4), m_timer_regs.timer[offset / (0x10/4)].control, mem_mask );
			return m_timer_regs.timer[offset / (0x10/4)].control;
		default:
			verboselog(machine(), 0, "ps_timer_r: Unknown Register %08x & %08x\n", 0x0a800000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_timer_w)
{
	switch(offset)
	{
		case 0x0000/4:
		case 0x0010/4:
		case 0x0020/4:
			verboselog(machine(), 0, "ps_timer_w: Timer %d Period = %08x & %08x\n", offset / (0x10/4), data, mem_mask );
			COMBINE_DATA(&m_timer_regs.timer[offset / (0x10/4)].period);
			break;
		case 0x0004/4:
		case 0x0014/4:
		case 0x0024/4:
			verboselog(machine(), 0, "ps_timer_w: Timer %d Count = %08x & %08x\n", offset / (0x10/4), data, mem_mask );
			COMBINE_DATA(&m_timer_regs.timer[offset / (0x10/4)].count);
			break;
		case 0x0008/4:
		case 0x0018/4:
		case 0x0028/4:
			verboselog(machine(), 0, "ps_timer_w: Timer %d Control = %08x & %08x\n", offset / (0x10/4), data, mem_mask );
			COMBINE_DATA(&m_timer_regs.timer[offset / (0x10/4)].control);
			if(m_timer_regs.timer[offset / (0x10/4)].control & 4)
			{
				ps_timer_start(machine(), offset / (0x10/4));
			}
			else
			{
				m_timer_regs.timer[offset / (0x10/4)].timer->adjust(attotime::never, offset / (0x10/4));
			}
			break;
		default:
			verboselog(machine(), 0, "ps_timer_w: Unknown Register %08x = %08x & %08x\n", 0x0a800000 + (offset << 2), data, mem_mask );
			break;
	}
}

READ32_MEMBER(pockstat_state::ps_clock_r)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(machine(), 0, "ps_clock_r: Clock Mode = %08x & %08x\n", m_clock_regs.mode | 0x10, mem_mask );
			return m_clock_regs.mode | PS_CLOCK_STEADY;
		case 0x0004/4:
			verboselog(machine(), 0, "ps_clock_r: Clock Control = %08x & %08x\n", m_clock_regs.control, mem_mask );
			return m_clock_regs.control;
		default:
			verboselog(machine(), 0, "ps_clock_r: Unknown Register %08x & %08x\n", 0x0b000000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_clock_w)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(machine(), 0, "ps_clock_w: Clock Mode = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_clock_regs.mode);
			machine().device("maincpu")->set_unscaled_clock(CPU_FREQ[m_clock_regs.mode & 0x0f]);
			break;
		case 0x0004/4:
			verboselog(machine(), 0, "ps_clock_w: Clock Control = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_clock_regs.control);
			break;
		default:
			verboselog(machine(), 0, "ps_clock_w: Unknown Register %08x = %08x & %08x\n", 0x0b000000 + (offset << 2), data, mem_mask );
			break;
	}
}

static TIMER_CALLBACK( rtc_tick )
{
	pockstat_state *state = machine.driver_data<pockstat_state>();
	//printf( "RTC is calling back\n" );
	ps_intc_set_interrupt_line(machine, PS_INT_RTC, ps_intc_get_interrupt_line(machine, PS_INT_RTC) ? 0 : 1);
	if(!(state->m_rtc_regs.mode & 1))
	{
		state->m_rtc_regs.time++;
		if((state->m_rtc_regs.time & 0x0000000f) == 0x0000000a)
		{
			state->m_rtc_regs.time &= 0xfffffff0;
			state->m_rtc_regs.time += 0x00000010;
			if((state->m_rtc_regs.time & 0x000000ff) == 0x00000060)
			{
				state->m_rtc_regs.time &= 0xffffff00;
				state->m_rtc_regs.time += 0x00000100;
				if((state->m_rtc_regs.time & 0x00000f00) == 0x00000a00)
				{
					state->m_rtc_regs.time &= 0xfffff0ff;
					state->m_rtc_regs.time += 0x00001000;
					if((state->m_rtc_regs.time & 0x0000ff00) == 0x00006000)
					{
						state->m_rtc_regs.time &= 0xffff00ff;
						state->m_rtc_regs.time += 0x00010000;
						if((state->m_rtc_regs.time & 0x00ff0000) == 0x00240000)
						{
							state->m_rtc_regs.time &= 0xff00ffff;
							state->m_rtc_regs.time += 0x01000000;
							if((state->m_rtc_regs.time & 0x0f000000) == 0x08000000)
							{
								state->m_rtc_regs.time &= 0xf0ffffff;
								state->m_rtc_regs.time |= 0x01000000;
							}
						}
						else if((state->m_rtc_regs.time & 0x000f0000) == 0x000a0000)
						{
							state->m_rtc_regs.time &= 0xfff0ffff;
							state->m_rtc_regs.time += 0x00100000;
						}
					}
				}
			}
		}
	}
	state->m_rtc_regs.timer->adjust(attotime::from_hz(1));
}

READ32_MEMBER(pockstat_state::ps_rtc_r)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(machine(), 0, "ps_rtc_r: RTC Mode = %08x & %08x\n", m_rtc_regs.mode, mem_mask );
			return m_rtc_regs.mode;
		case 0x0004/4:
			verboselog(machine(), 0, "ps_rtc_r: RTC Control = %08x & %08x\n", m_rtc_regs.control, mem_mask );
			return m_rtc_regs.control;
		case 0x0008/4:
			verboselog(machine(), 0, "ps_rtc_r: RTC Time = %08x & %08x\n", m_rtc_regs.time, mem_mask );
			return m_rtc_regs.time;
		case 0x000c/4:
			verboselog(machine(), 0, "ps_rtc_r: RTC Date = %08x & %08x\n", m_rtc_regs.date, mem_mask );
			return m_rtc_regs.date;
		default:
			verboselog(machine(), 0, "ps_rtc_r: Unknown Register %08x & %08x\n", 0x0b800000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_rtc_w)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(machine(), 0, "ps_rtc_w: RTC Mode = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_rtc_regs.mode);
			break;
		case 0x0004/4:
			verboselog(machine(), 0, "ps_rtc_w: RTC Control = %08x & %08x\n", data, mem_mask );
			if(m_rtc_regs.control == 1 && data == 1)
			{
				switch(m_rtc_regs.mode >> 1)
				{
					case 0: // Seconds
						m_rtc_regs.time += 0x00000001;
						if((m_rtc_regs.time & 0x0000000f) == 0x0000000a)
						{
							m_rtc_regs.time &= 0xfffffff0;
							m_rtc_regs.time += 0x00000010;
							if((m_rtc_regs.time & 0x000000ff) == 0x00000060)
							{
								m_rtc_regs.time &= 0xffffff00;
							}
						}
						break;
					case 1: // Minutes
						m_rtc_regs.time += 0x00000100;
						if((m_rtc_regs.time & 0x00000f00) == 0x00000a00)
						{
							m_rtc_regs.time &= 0xfffff0ff;
							m_rtc_regs.time += 0x00001000;
							if((m_rtc_regs.time & 0x0000ff00) == 0x00006000)
							{
								m_rtc_regs.time &= 0xffff00ff;
							}
						}
						break;
					case 2: // Hours
						m_rtc_regs.time += 0x00010000;
						if((m_rtc_regs.time & 0x00ff0000) == 0x00240000)
						{
							m_rtc_regs.time &= 0xff00ffff;
						}
						else if((m_rtc_regs.time & 0x000f0000) == 0x000a0000)
						{
							m_rtc_regs.time &= 0xfff0ffff;
							m_rtc_regs.time += 0x00100000;
						}
						break;
					case 3: // Day of the week
						m_rtc_regs.time += 0x01000000;
						if((m_rtc_regs.time & 0x0f000000) == 0x08000000)
						{
							m_rtc_regs.time &= 0xf0ffffff;
							m_rtc_regs.time |= 0x01000000;
						}
						break;
					case 4: // Day
						m_rtc_regs.date += 0x00000001;
						if((m_rtc_regs.date & 0x000000ff) == 0x00000032)
						{
							m_rtc_regs.date &= 0xffffff00;
						}
						else if((m_rtc_regs.date & 0x0000000f) == 0x0000000a)
						{
							m_rtc_regs.date &= 0xfffffff0;
							m_rtc_regs.date += 0x00000010;
						}
						break;
					case 5: // Month
						m_rtc_regs.date += 0x00000100;
						if((m_rtc_regs.date & 0x0000ff00) == 0x00001300)
						{
							m_rtc_regs.date &= 0xffffff00;
							m_rtc_regs.date |= 0x00000001;
						}
						else if((m_rtc_regs.date & 0x00000f00) == 0x00000a00)
						{
							m_rtc_regs.date &= 0xfffff0ff;
							m_rtc_regs.date += 0x00001000;
						}
						break;
					case 6: // Year (LSB)
						m_rtc_regs.date += 0x00010000;
						if((m_rtc_regs.date & 0x000f0000) == 0x000a0000)
						{
							m_rtc_regs.date &= 0xfff0ffff;
							m_rtc_regs.date += 0x00100000;
							if((m_rtc_regs.date & 0x00f00000) == 0x00a00000)
							{
								m_rtc_regs.date &= 0xff00ffff;
							}
						}
						break;
					case 7: // Year (MSB)
						break;
				}
				m_rtc_regs.control = 0;
			}
			else if(m_rtc_regs.control == 0)
			{
				COMBINE_DATA(&m_rtc_regs.control);
			}
			break;
		default:
			verboselog(machine(), 0, "ps_rtc_w: Unknown Register %08x = %08x & %08x\n", 0x0b800000 + (offset << 2), data, mem_mask );
			break;
	}
}


READ32_MEMBER(pockstat_state::ps_lcd_r)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(machine(), 0, "ps_lcd_r: LCD Control = %08x & %08x\n", m_lcd_control | 0x100, mem_mask );
			return m_lcd_control;
		default:
			verboselog(machine(), 0, "ps_lcd_r: Unknown Register %08x & %08x\n", 0x0d000000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_lcd_w)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(machine(), 0, "ps_lcd_w: LCD Control = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_lcd_control);
			break;
		default:
			verboselog(machine(), 0, "ps_lcd_w: Unknown Register %08x = %08x & %08x\n", 0x0d000000 + (offset << 2), data, mem_mask );
			break;
	}
}

static INPUT_CHANGED( input_update )
{
	UINT32 buttons = field.machine().root_device().ioport("BUTTONS")->read();

	ps_intc_set_interrupt_line(field.machine(), PS_INT_BTN_ACTION, (buttons &  1) ? 1 : 0);
	ps_intc_set_interrupt_line(field.machine(), PS_INT_BTN_RIGHT,	(buttons &  2) ? 1 : 0);
	ps_intc_set_interrupt_line(field.machine(), PS_INT_BTN_LEFT,	(buttons &  4) ? 1 : 0);
	ps_intc_set_interrupt_line(field.machine(), PS_INT_BTN_DOWN,	(buttons &  8) ? 1 : 0);
	ps_intc_set_interrupt_line(field.machine(), PS_INT_BTN_UP,	(buttons & 16) ? 1 : 0);
}

READ32_MEMBER(pockstat_state::ps_rombank_r)
{
	INT32 bank = (offset >> 11) & 0x0f;
	int index = 0;
	for(index = 0; index < 32; index++)
	{
		if(m_ftlb_regs.valid & (1 << index))
		{
			if(m_ftlb_regs.entry[index] == bank)
			{
				//printf( "Address %08x is assigned to %08x in entry %d\n", 0x02000000 + (offset << 2), index * 0x2000 + ((offset << 2) & 0x1fff), index );
				return memregion("flash")->u32(index * (0x2000/4) + (offset & (0x1fff/4)));
			}
		}
	}
	return memregion("flash")->u32(offset & 0x7fff);
}


// Horrible hack, probably wrong
WRITE32_MEMBER(pockstat_state::ps_flash_w)
{
	if(offset == (0x55a8/4))
	{
		m_ps_flash_write_enable_count++;
		return;
	}
	if(offset == (0x2a54/4))
	{
		m_ps_flash_write_enable_count++;
		return;
	}
	if(m_ps_flash_write_enable_count == 3)
	{
		m_ps_flash_write_enable_count = 0;
		m_ps_flash_write_count = 0x40;
		return;
	}
	if(m_ps_flash_write_count)
	{
		m_ps_flash_write_count--;
		COMBINE_DATA(&((UINT32*)(*machine().root_device().memregion("flash")))[offset]);
	}
}

READ32_MEMBER(pockstat_state::ps_audio_r)
{
	verboselog(machine(), 0, "ps_audio_r: Unknown Read: %08x = %08x & %08x\n", 0xd800000 + (offset << 2), 0x10, mem_mask);
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_audio_w)
{
	verboselog(machine(), 0, "ps_audio_w: Unknown Write: %08x = %08x & %08x\n", 0xd800000 + (offset << 2), data, mem_mask);
}

WRITE32_MEMBER(pockstat_state::ps_dac_w)
{
	machine().device<dac_device>("dac")->write_unsigned16((UINT16)((data + 0x8000) & 0x0000ffff));
}

static ADDRESS_MAP_START(pockstat_mem, AS_PROGRAM, 32, pockstat_state )
	AM_RANGE(0x00000000, 0x000007ff) AM_RAM
	AM_RANGE(0x02000000, 0x02ffffff) AM_READ(ps_rombank_r)
	AM_RANGE(0x04000000, 0x04003fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x06000000, 0x06000307) AM_READWRITE(ps_ftlb_r, ps_ftlb_w)
	AM_RANGE(0x08000000, 0x0801ffff) AM_ROM AM_WRITE(ps_flash_w) AM_REGION("flash", 0)
	AM_RANGE(0x0a000000, 0x0a000013) AM_READWRITE(ps_intc_r, ps_intc_w)
	AM_RANGE(0x0a800000, 0x0a80002b) AM_READWRITE(ps_timer_r, ps_timer_w)
	AM_RANGE(0x0b000000, 0x0b000007) AM_READWRITE(ps_clock_r, ps_clock_w)
	AM_RANGE(0x0b800000, 0x0b80000f) AM_READWRITE(ps_rtc_r, ps_rtc_w)
	AM_RANGE(0x0d000000, 0x0d000003) AM_READWRITE(ps_lcd_r, ps_lcd_w)
	AM_RANGE(0x0d000100, 0x0d00017f) AM_RAM AM_SHARE("lcd_buffer")
	AM_RANGE(0x0d80000c, 0x0d80000f) AM_READWRITE(ps_audio_r, ps_audio_w)
	AM_RANGE(0x0d800014, 0x0d800017) AM_WRITE(ps_dac_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pockstat )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1)		PORT_NAME("Action Button")	PORT_CHANGED(input_update, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_NAME("Right")			PORT_CHANGED(input_update, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)	PORT_NAME("Left")			PORT_CHANGED(input_update, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)	PORT_NAME("Down")			PORT_CHANGED(input_update, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)	PORT_NAME("Up")				PORT_CHANGED(input_update, 0)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static MACHINE_START( pockstat )
{
	pockstat_state *state = machine.driver_data<pockstat_state>();
	int index = 0;
	for(index = 0; index < 3; index++)
	{
		state->m_timer_regs.timer[index].timer = machine.scheduler().timer_alloc(FUNC(timer_tick));
		state->m_timer_regs.timer[index].timer->adjust(attotime::never, index);
	}

	state->m_rtc_regs.time = 0x01000000;
	state->m_rtc_regs.date = 0x19990101;

	state->m_rtc_regs.timer = machine.scheduler().timer_alloc(FUNC(rtc_tick));
	state->m_rtc_regs.timer->adjust(attotime::from_hz(1), index);

	state->save_item(NAME(state->m_ftlb_regs.control));
	state->save_item(NAME(state->m_ftlb_regs.stat));
	state->save_item(NAME(state->m_ftlb_regs.valid));
	state->save_item(NAME(state->m_ftlb_regs.wait1));
	state->save_item(NAME(state->m_ftlb_regs.wait2));
	state->save_item(NAME(state->m_ftlb_regs.entry));

	state->save_item(NAME(state->m_intc_regs.hold));
	state->save_item(NAME(state->m_intc_regs.status));
	state->save_item(NAME(state->m_intc_regs.enable));
	state->save_item(NAME(state->m_intc_regs.mask));

	state->save_item(NAME(state->m_timer_regs.timer[0].period));
	state->save_item(NAME(state->m_timer_regs.timer[0].count));
	state->save_item(NAME(state->m_timer_regs.timer[0].control));
	state->save_item(NAME(state->m_timer_regs.timer[1].period));
	state->save_item(NAME(state->m_timer_regs.timer[1].count));
	state->save_item(NAME(state->m_timer_regs.timer[1].control));
	state->save_item(NAME(state->m_timer_regs.timer[2].period));
	state->save_item(NAME(state->m_timer_regs.timer[2].count));
	state->save_item(NAME(state->m_timer_regs.timer[2].control));

	state->save_item(NAME(state->m_clock_regs.mode));
	state->save_item(NAME(state->m_clock_regs.control));

	state->save_item(NAME(state->m_rtc_regs.mode));
	state->save_item(NAME(state->m_rtc_regs.control));
	state->save_item(NAME(state->m_rtc_regs.time));
	state->save_item(NAME(state->m_rtc_regs.date));

	state->save_item(NAME(state->m_ps_flash_write_enable_count));
	state->save_item(NAME(state->m_ps_flash_write_count));
}

static MACHINE_RESET( pockstat )
{
	pockstat_state *state = machine.driver_data<pockstat_state>();
	cpu_set_reg(machine.device("maincpu"), STATE_GENPC, 0x4000000);

	state->m_ps_flash_write_enable_count = 0;
	state->m_ps_flash_write_count = 0;
}

static SCREEN_UPDATE_RGB32( pockstat )
{
	pockstat_state *state = screen.machine().driver_data<pockstat_state>();
	int x = 0;
	int y = 0;
	for(y = 0; y < 32; y++)
	{
		UINT32 *scanline = &bitmap.pix32(y);
		for(x = 0; x < 32; x++)
		{
			if(state->m_lcd_control != 0) // Hack
			{
				if(state->m_lcd_buffer[y] & (1 << x))
				{
					scanline[x] = 0x00000000;
				}
				else
				{
					scanline[x] = 0x00ffffff;
				}
			}
			else
			{
				scanline[x] = 0x00ffffff;
			}
		}
	}
	return 0;
}

static DEVICE_IMAGE_LOAD( pockstat_flash )
{
	int i, length;
	UINT8 *cart = image.device().machine().root_device().memregion("flash")->base();
	static const char *gme_id = "123-456-STD";

	length = image.fread( cart, 0x20f40);

	if(length != 0x20f40)
	{
		return IMAGE_INIT_FAIL;
	}

	for(i = 0; i < strlen(gme_id); i++)
	{
		if(cart[i] != gme_id[i])
		{
			return IMAGE_INIT_FAIL;
		}
	}

	memcpy(cart, cart + 0xf40, 0x20000);

	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( pockstat, pockstat_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM7, DEFAULT_CLOCK)
	MCFG_CPU_PROGRAM_MAP(pockstat_mem)

	MCFG_MACHINE_RESET(pockstat)
	MCFG_MACHINE_START(pockstat)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(32, 32)
	MCFG_SCREEN_VISIBLE_AREA(0, 32-1, 0, 32-1)
	MCFG_SCREEN_UPDATE_STATIC(pockstat)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

    MCFG_SPEAKER_STANDARD_MONO("mono")
    MCFG_SOUND_ADD("dac", DAC, 0)
    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("gme")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(pockstat_flash)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pockstat )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "kernel.bin", 0x0000, 0x4000, CRC(5fb47dd8) SHA1(6ae880493ddde880827d1e9f08e9cb2c38f9f2ec) )

	ROM_REGION( 0x20f40, "flash", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT     INIT  COMPANY                             FULLNAME       FLAGS */
CONS( 1999, pockstat, 0,      0,      pockstat,  pockstat, driver_device, 0,    "Sony Computer Entertainment Inc", "Sony PocketStation", GAME_SUPPORTS_SAVE )
