// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sony PocketStation

    05/2009 Skeleton driver written.
    11/2009 Initial bring-up commenced by Ryan Holtz.
    11/2009 Playable state achieved by Ryan Holtz.

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

To start a game:
- Wait for the set-date screen to appear
- Press down arrow
- set date with arrows (optional)
- Press Ctrl, wait a sec, press ctrl, press right arrow, game starts

It doesn't save the date so you have to go through this procedure every time.

If you do nothing for about 20 secs, it turns itself off (screen goes white).

****************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "sound/dac.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define MAX_PS_TIMERS   3

struct ps_ftlb_regs_t
{
	UINT32 control;
	UINT32 stat;
	UINT32 valid;
	UINT32 wait1;
	UINT32 wait2;
	UINT32 entry[16];
	UINT32 serial;
};

struct ps_intc_regs_t
{
	UINT32 hold;
	UINT32 status;
	UINT32 enable;
	UINT32 mask;
};

struct ps_timer_t
{
	UINT32 period;
	UINT32 count;
	UINT32 control;
	emu_timer *timer;
};

struct ps_timer_regs_t
{
	ps_timer_t timer[MAX_PS_TIMERS];
};

struct ps_clock_regs_t
{
	UINT32 mode;
	UINT32 control;
};

#define PS_CLOCK_STEADY     0x10

struct ps_rtc_regs_t
{
	UINT32 mode;
	UINT32 control;
	UINT32 time;
	UINT32 date;
	emu_timer *timer;
};



class pockstat_state : public driver_device
{
public:
	pockstat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_lcd_buffer(*this, "lcd_buffer"),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot")
	{ }

	required_shared_ptr<UINT32> m_lcd_buffer;
	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dac;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_rom;

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
	DECLARE_READ32_MEMBER(ps_flash_r);
	DECLARE_WRITE32_MEMBER(ps_flash_w);
	DECLARE_READ32_MEMBER(ps_audio_r);
	DECLARE_WRITE32_MEMBER(ps_audio_w);
	DECLARE_WRITE32_MEMBER(ps_dac_w);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_pockstat(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_INPUT_CHANGED_MEMBER(input_update);
	TIMER_CALLBACK_MEMBER(timer_tick);
	TIMER_CALLBACK_MEMBER(rtc_tick);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( pockstat_flash );
	inline void ATTR_PRINTF(3,4) verboselog( int n_level, const char *s_fmt, ... );
	UINT32 ps_intc_get_interrupt_line(UINT32 line);
	void ps_intc_set_interrupt_line(UINT32 line, int state);
	void ps_timer_start(int index);
};


#define DEFAULT_CLOCK   2000000

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

#define VERBOSE_LEVEL       (0)

#define ENABLE_VERBOSE_LOG  (0)

inline void ATTR_PRINTF(3,4) pockstat_state::verboselog( int n_level, const char *s_fmt, ... )
{
#if ENABLE_VERBOSE_LOG
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine().describe_context(), buf );
	}
#endif
}

#define PS_INT_BTN_ACTION       0x00000001 // "Action button"
#define PS_INT_BTN_RIGHT        0x00000002 // "Right button"
#define PS_INT_BTN_LEFT         0x00000004 // "Left button"
#define PS_INT_BTN_DOWN         0x00000008 // "Down button"
#define PS_INT_BTN_UP           0x00000010 // "Up button"
#define PS_INT_UNKNOWN          0x00000020 // "Unknown"
#define PS_INT_COM              0x00000040 // "COM" ???
#define PS_INT_TIMER0           0x00000080 // "Timer 0"
#define PS_INT_TIMER1           0x00000100 // "Timer 1"
#define PS_INT_RTC              0x00000200 // "RTC"
#define PS_INT_BATTERY          0x00000400 // "Battery Monitor"
#define PS_INT_IOP              0x00000800 // "IOP"
#define PS_INT_IRDA             0x00001000 // "IrDA"
#define PS_INT_TIMER2           0x00002000 // "Timer 2"
#define PS_INT_IRQ_MASK         0x00001fbf
#define PS_INT_FIQ_MASK         0x00002040
#define PS_INT_STATUS_MASK      0x0000021f

READ32_MEMBER(pockstat_state::ps_ftlb_r)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(0, "ps_ftlb_r: FlashROM TLB Control = %08x & %08x\n", m_ftlb_regs.control, mem_mask );
			return m_ftlb_regs.control | 1; // ???
		case 0x0004/4:
			verboselog(0, "ps_ftlb_r: Unknown (F_STAT) = %08x & %08x\n", m_ftlb_regs.stat, mem_mask );
			return m_ftlb_regs.stat;
		case 0x0008/4:
			verboselog(0, "ps_ftlb_r: FlashROM TLB Valid Tag = %08x & %08x\n", m_ftlb_regs.valid, mem_mask );
			return m_ftlb_regs.valid;
		case 0x000c/4:
			verboselog(0, "ps_ftlb_r: Unknown (F_WAIT1) = %08x & %08x\n", m_ftlb_regs.wait1, mem_mask );
			return m_ftlb_regs.wait1;
		case 0x0010/4:
			verboselog(0, "ps_ftlb_r: Unknown (F_WAIT2) = %08x & %08x\n", m_ftlb_regs.wait2 | 0x04, mem_mask );
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
			verboselog(0, "ps_ftlb_r: FlashROM TLB Entry %d = %08x & %08x\n", offset - 0x100/4, m_ftlb_regs.entry[offset - 0x100/4], mem_mask );
			return m_ftlb_regs.entry[offset - 0x100/4];
		case 0x0300/4:
			verboselog(0, "ps_ftlb_r: Unknown (F_SN) = %08x & %08x\n", m_ftlb_regs.serial, mem_mask );
			return m_ftlb_regs.serial;
		default:
			verboselog(0, "ps_ftlb_r: Unknown Register %08x & %08x\n", 0x06000000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_ftlb_w)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(0, "ps_ftlb_w: FlashROM TLB Control = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.control);
			break;
		case 0x0004/4:
			verboselog(0, "ps_ftlb_w: Unknown (F_STAT) = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.stat);
			break;
		case 0x0008/4:
			verboselog(0, "ps_ftlb_w: FlashROM TLB Valid Tag = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.valid);
			break;
		case 0x000c/4:
			verboselog(0, "ps_ftlb_w: Unknown (F_WAIT1) = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.wait1);
			break;
		case 0x0010/4:
			verboselog(0, "ps_ftlb_w: Unknown (F_WAIT2) = %08x & %08x\n", data, mem_mask );
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
			verboselog(0, "ps_ftlb_w: FlashROM TLB Entry %d = %08x & %08x\n", offset - 0x100/4, data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.entry[offset - 0x100/4]);
			break;
		case 0x0300/4:
			verboselog(0, "ps_ftlb_w: Unknown (F_SN) = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_ftlb_regs.serial);
			break;
		default:
			verboselog(0, "ps_ftlb_w: Unknown Register %08x = %08x & %08x\n", 0x06000000 + (offset << 2), data, mem_mask );
			break;
	}
}

UINT32 pockstat_state::ps_intc_get_interrupt_line(UINT32 line)
{
	return m_intc_regs.status & line;
}

void pockstat_state::ps_intc_set_interrupt_line(UINT32 line, int state)
{
	//printf( "%08x %d %08x %08x %08x\n", line, state, drvm_intc_regs.hold, drvm_intc_regs.status, drvm_intc_regs.enable );
	if(line)
	{
		if(state)
		{
			m_intc_regs.status |= line & PS_INT_STATUS_MASK;
			m_intc_regs.hold |= line &~ PS_INT_STATUS_MASK;
			//printf( " Setting %08x, status = %08x, hold = %08x\n", line, m_intc_regs.status, m_intc_regs.hold );
		}
		else
		{
			m_intc_regs.status &= ~line;
			m_intc_regs.hold &= ~line;
			//printf( "Clearing %08x, status = %08x, hold = %08x\n", line, m_intc_regs.status, m_intc_regs.hold );
		}
	}
	if(m_intc_regs.hold & m_intc_regs.enable & PS_INT_IRQ_MASK)
	{
		m_maincpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(ARM7_IRQ_LINE, CLEAR_LINE);
	}
	if(m_intc_regs.hold & m_intc_regs.enable & PS_INT_FIQ_MASK)
	{
		m_maincpu->set_input_line(ARM7_FIRQ_LINE, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(ARM7_FIRQ_LINE, CLEAR_LINE);
	}
}

READ32_MEMBER(pockstat_state::ps_intc_r)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(0, "ps_intc_r: Held Interrupt = %08x & %08x\n", m_intc_regs.hold, mem_mask );
			return m_intc_regs.hold;
		case 0x0004/4:
			verboselog(0, "ps_intc_r: Interrupt Status = %08x & %08x\n", m_intc_regs.status, mem_mask );
			return m_intc_regs.status;
		case 0x0008/4:
			verboselog(0, "ps_intc_r: Interrupt Enable = %08x & %08x\n", m_intc_regs.enable, mem_mask );
			return m_intc_regs.enable;
		case 0x000c/4:
			verboselog(0, "ps_intc_r: Interrupt Mask (Invalid Read) = %08x & %08x\n", 0, mem_mask );
			return 0;
		case 0x0010/4:
			verboselog(0, "ps_intc_r: Interrupt Acknowledge (Invalid Read) = %08x & %08x\n", 0, mem_mask );
			return 0;
		default:
			verboselog(0, "ps_intc_r: Unknown Register %08x & %08x\n", 0x0a000000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_intc_w)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(0, "ps_intc_w: Held Interrupt (Invalid Write) = %08x & %08x\n", data, mem_mask );
			break;
		case 0x0004/4:
			verboselog(0, "ps_intc_w: Interrupt Status (Invalid Write) = %08x & %08x\n", data, mem_mask );
			break;
		case 0x0008/4:
			verboselog(0, "ps_intc_w: Interrupt Enable = %08x & %08x\n", data, mem_mask );
			m_intc_regs.enable |= data;
			//COMBINE_DATA(&m_intc_regs.enable);
			//m_intc_regs.status &= m_intc_regs.enable;
			//m_intc_regs.hold &= m_intc_regs.enable;
			ps_intc_set_interrupt_line(0, 0);
			break;
		case 0x000c/4:
			verboselog(0, "ps_intc_w: Interrupt Mask = %08x & %08x\n", data, mem_mask );
			m_intc_regs.enable &= ~data;
			COMBINE_DATA(&m_intc_regs.mask);
			//m_intc_regs.status &= m_intc_regs.enable;
			//m_intc_regs.hold &= m_intc_regs.enable;
			ps_intc_set_interrupt_line(0, 0);
			break;
		case 0x0010/4:
			verboselog(0, "ps_intc_w: Interrupt Acknowledge = %08x & %08x\n", data, mem_mask );
			m_intc_regs.hold &= ~data;
			m_intc_regs.status &= ~data;
			ps_intc_set_interrupt_line(0, 0);
			//COMBINE_DATA(&m_intc_regs.acknowledge);
			break;
		default:
			verboselog(0, "ps_intc_w: Unknown Register %08x = %08x & %08x\n", 0x0a000000 + (offset << 2), data, mem_mask );
			break;
	}
}

TIMER_CALLBACK_MEMBER(pockstat_state::timer_tick)
{
	ps_intc_set_interrupt_line(param == 2 ? PS_INT_TIMER2 : (param == 1 ? PS_INT_TIMER1 : PS_INT_TIMER0), 1);
	//printf( "Timer %d is calling back\n", param );
	m_timer_regs.timer[param].count = m_timer_regs.timer[param].period;
	ps_timer_start(param);
}

void pockstat_state::ps_timer_start(int index)
{
	int divisor = 1;
	attotime period;
	switch(m_timer_regs.timer[index].control & 3)
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
	period = attotime::from_hz(CPU_FREQ[m_clock_regs.mode & 0x0f] / 2) * divisor;
	period = period * m_timer_regs.timer[index].count;
	m_timer_regs.timer[index].timer->adjust(period, index);
}

READ32_MEMBER(pockstat_state::ps_timer_r)
{
	switch(offset)
	{
		case 0x0000/4:
		case 0x0010/4:
		case 0x0020/4:
			verboselog(0, "ps_timer_r: Timer %d Period = %08x & %08x\n", offset / (0x10/4), m_timer_regs.timer[offset / (0x10/4)].period, mem_mask );
			return m_timer_regs.timer[offset / (0x10/4)].period;
		case 0x0004/4:
		case 0x0014/4:
		case 0x0024/4:
			verboselog(0, "ps_timer_r: Timer %d Count = %08x & %08x\n", offset / (0x10/4), m_timer_regs.timer[offset / (0x10/4)].count, mem_mask );
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
			verboselog(0, "ps_timer_r: Timer %d Control = %08x & %08x\n", offset / (0x10/4), m_timer_regs.timer[offset / (0x10/4)].control, mem_mask );
			return m_timer_regs.timer[offset / (0x10/4)].control;
		default:
			verboselog(0, "ps_timer_r: Unknown Register %08x & %08x\n", 0x0a800000 + (offset << 2), mem_mask );
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
			verboselog(0, "ps_timer_w: Timer %d Period = %08x & %08x\n", offset / (0x10/4), data, mem_mask );
			COMBINE_DATA(&m_timer_regs.timer[offset / (0x10/4)].period);
			break;
		case 0x0004/4:
		case 0x0014/4:
		case 0x0024/4:
			verboselog(0, "ps_timer_w: Timer %d Count = %08x & %08x\n", offset / (0x10/4), data, mem_mask );
			COMBINE_DATA(&m_timer_regs.timer[offset / (0x10/4)].count);
			break;
		case 0x0008/4:
		case 0x0018/4:
		case 0x0028/4:
			verboselog(0, "ps_timer_w: Timer %d Control = %08x & %08x\n", offset / (0x10/4), data, mem_mask );
			COMBINE_DATA(&m_timer_regs.timer[offset / (0x10/4)].control);
			if(m_timer_regs.timer[offset / (0x10/4)].control & 4)
			{
				ps_timer_start(offset / (0x10/4));
			}
			else
			{
				m_timer_regs.timer[offset / (0x10/4)].timer->adjust(attotime::never, offset / (0x10/4));
			}
			break;
		default:
			verboselog(0, "ps_timer_w: Unknown Register %08x = %08x & %08x\n", 0x0a800000 + (offset << 2), data, mem_mask );
			break;
	}
}

READ32_MEMBER(pockstat_state::ps_clock_r)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(0, "ps_clock_r: Clock Mode = %08x & %08x\n", m_clock_regs.mode | 0x10, mem_mask );
			return m_clock_regs.mode | PS_CLOCK_STEADY;
		case 0x0004/4:
			verboselog(0, "ps_clock_r: Clock Control = %08x & %08x\n", m_clock_regs.control, mem_mask );
			return m_clock_regs.control;
		default:
			verboselog(0, "ps_clock_r: Unknown Register %08x & %08x\n", 0x0b000000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_clock_w)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(0, "ps_clock_w: Clock Mode = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_clock_regs.mode);
			m_maincpu->set_unscaled_clock(CPU_FREQ[m_clock_regs.mode & 0x0f]);
			break;
		case 0x0004/4:
			verboselog(0, "ps_clock_w: Clock Control = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_clock_regs.control);
			break;
		default:
			verboselog(0, "ps_clock_w: Unknown Register %08x = %08x & %08x\n", 0x0b000000 + (offset << 2), data, mem_mask );
			break;
	}
}

TIMER_CALLBACK_MEMBER(pockstat_state::rtc_tick)
{
	//printf( "RTC is calling back\n" );
	ps_intc_set_interrupt_line(PS_INT_RTC, ps_intc_get_interrupt_line(PS_INT_RTC) ? 0 : 1);
	if(!(m_rtc_regs.mode & 1))
	{
		m_rtc_regs.time++;
		if((m_rtc_regs.time & 0x0000000f) == 0x0000000a)
		{
			m_rtc_regs.time &= 0xfffffff0;
			m_rtc_regs.time += 0x00000010;
			if((m_rtc_regs.time & 0x000000ff) == 0x00000060)
			{
				m_rtc_regs.time &= 0xffffff00;
				m_rtc_regs.time += 0x00000100;
				if((m_rtc_regs.time & 0x00000f00) == 0x00000a00)
				{
					m_rtc_regs.time &= 0xfffff0ff;
					m_rtc_regs.time += 0x00001000;
					if((m_rtc_regs.time & 0x0000ff00) == 0x00006000)
					{
						m_rtc_regs.time &= 0xffff00ff;
						m_rtc_regs.time += 0x00010000;
						if((m_rtc_regs.time & 0x00ff0000) == 0x00240000)
						{
							m_rtc_regs.time &= 0xff00ffff;
							m_rtc_regs.time += 0x01000000;
							if((m_rtc_regs.time & 0x0f000000) == 0x08000000)
							{
								m_rtc_regs.time &= 0xf0ffffff;
								m_rtc_regs.time |= 0x01000000;
							}
						}
						else if((m_rtc_regs.time & 0x000f0000) == 0x000a0000)
						{
							m_rtc_regs.time &= 0xfff0ffff;
							m_rtc_regs.time += 0x00100000;
						}
					}
				}
			}
		}
	}
	m_rtc_regs.timer->adjust(attotime::from_hz(1));
}

READ32_MEMBER(pockstat_state::ps_rtc_r)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(0, "ps_rtc_r: RTC Mode = %08x & %08x\n", m_rtc_regs.mode, mem_mask );
			return m_rtc_regs.mode;
		case 0x0004/4:
			verboselog(0, "ps_rtc_r: RTC Control = %08x & %08x\n", m_rtc_regs.control, mem_mask );
			return m_rtc_regs.control;
		case 0x0008/4:
			verboselog(0, "ps_rtc_r: RTC Time = %08x & %08x\n", m_rtc_regs.time, mem_mask );
			return m_rtc_regs.time;
		case 0x000c/4:
			verboselog(0, "ps_rtc_r: RTC Date = %08x & %08x\n", m_rtc_regs.date, mem_mask );
			return m_rtc_regs.date;
		default:
			verboselog(0, "ps_rtc_r: Unknown Register %08x & %08x\n", 0x0b800000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_rtc_w)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(0, "ps_rtc_w: RTC Mode = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_rtc_regs.mode);
			break;
		case 0x0004/4:
			verboselog(0, "ps_rtc_w: RTC Control = %08x & %08x\n", data, mem_mask );
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
			verboselog(0, "ps_rtc_w: Unknown Register %08x = %08x & %08x\n", 0x0b800000 + (offset << 2), data, mem_mask );
			break;
	}
}


READ32_MEMBER(pockstat_state::ps_lcd_r)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(0, "ps_lcd_r: LCD Control = %08x & %08x\n", m_lcd_control | 0x100, mem_mask );
			return m_lcd_control;
		default:
			verboselog(0, "ps_lcd_r: Unknown Register %08x & %08x\n", 0x0d000000 + (offset << 2), mem_mask );
			break;
	}
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_lcd_w)
{
	switch(offset)
	{
		case 0x0000/4:
			verboselog(0, "ps_lcd_w: LCD Control = %08x & %08x\n", data, mem_mask );
			COMBINE_DATA(&m_lcd_control);
			break;
		default:
			verboselog(0, "ps_lcd_w: Unknown Register %08x = %08x & %08x\n", 0x0d000000 + (offset << 2), data, mem_mask );
			break;
	}
}

INPUT_CHANGED_MEMBER(pockstat_state::input_update)
{
	UINT32 buttons = ioport("BUTTONS")->read();

	ps_intc_set_interrupt_line(PS_INT_BTN_ACTION, (buttons &  1) ? 1 : 0);
	ps_intc_set_interrupt_line(PS_INT_BTN_RIGHT, (buttons &  2) ? 1 : 0);
	ps_intc_set_interrupt_line(PS_INT_BTN_LEFT,  (buttons &  4) ? 1 : 0);
	ps_intc_set_interrupt_line(PS_INT_BTN_DOWN,  (buttons &  8) ? 1 : 0);
	ps_intc_set_interrupt_line(PS_INT_BTN_UP,    (buttons & 16) ? 1 : 0);
}

READ32_MEMBER(pockstat_state::ps_rombank_r)
{
	INT32 bank = (offset >> 11) & 0x0f;
	for (int index = 0; index < 32; index++)
	{
		if (m_ftlb_regs.valid & (1 << index))
		{
			if (m_ftlb_regs.entry[index] == bank)
			{
				//printf( "Address %08x is assigned to %08x in entry %d\n", 0x02000000 + (offset << 2), index * 0x2000 + ((offset << 2) & 0x1fff), index );
				return m_cart->read32_rom(space, index * (0x2000/4) + (offset & (0x1fff/4)), mem_mask);
			}
		}
	}
	return m_cart->read32_rom(space, offset & 0x7fff, mem_mask);
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
		COMBINE_DATA(&((UINT32*)(m_cart_rom->base()))[offset]);
	}
}

READ32_MEMBER(pockstat_state::ps_flash_r)
{
	return m_cart->read32_rom(space, offset, mem_mask);
}

READ32_MEMBER(pockstat_state::ps_audio_r)
{
	verboselog(0, "ps_audio_r: Unknown Read: %08x = %08x & %08x\n", 0xd800000 + (offset << 2), 0x10, mem_mask);
	return 0;
}

WRITE32_MEMBER(pockstat_state::ps_audio_w)
{
	verboselog(0, "ps_audio_w: Unknown Write: %08x = %08x & %08x\n", 0xd800000 + (offset << 2), data, mem_mask);
}

WRITE32_MEMBER(pockstat_state::ps_dac_w)
{
	m_dac->write_unsigned16((UINT16)((data + 0x8000) & 0x0000ffff));
}

static ADDRESS_MAP_START(pockstat_mem, AS_PROGRAM, 32, pockstat_state )
	AM_RANGE(0x00000000, 0x000007ff) AM_RAM
	AM_RANGE(0x02000000, 0x02ffffff) AM_READ(ps_rombank_r)
	AM_RANGE(0x04000000, 0x04003fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x06000000, 0x06000307) AM_READWRITE(ps_ftlb_r, ps_ftlb_w)
	AM_RANGE(0x08000000, 0x0801ffff) AM_READWRITE(ps_flash_r, ps_flash_w)
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_NAME("Action Button")  PORT_CHANGED_MEMBER(DEVICE_SELF, pockstat_state, input_update, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_NAME("Right")          PORT_CHANGED_MEMBER(DEVICE_SELF, pockstat_state, input_update, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_NAME("Left")           PORT_CHANGED_MEMBER(DEVICE_SELF, pockstat_state, input_update, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_NAME("Down")           PORT_CHANGED_MEMBER(DEVICE_SELF, pockstat_state, input_update, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_NAME("Up")             PORT_CHANGED_MEMBER(DEVICE_SELF, pockstat_state, input_update, 0)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void pockstat_state::machine_start()
{
	int index = 0;
	for (index = 0; index < 3; index++)
	{
		m_timer_regs.timer[index].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pockstat_state::timer_tick),this));
		m_timer_regs.timer[index].timer->adjust(attotime::never, index);
	}

	m_rtc_regs.time = 0x01000000;
	m_rtc_regs.date = 0x19990101;

	m_rtc_regs.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pockstat_state::rtc_tick),this));
	m_rtc_regs.timer->adjust(attotime::from_hz(1), index);

	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	save_item(NAME(m_ftlb_regs.control));
	save_item(NAME(m_ftlb_regs.stat));
	save_item(NAME(m_ftlb_regs.valid));
	save_item(NAME(m_ftlb_regs.wait1));
	save_item(NAME(m_ftlb_regs.wait2));
	save_item(NAME(m_ftlb_regs.entry));

	save_item(NAME(m_intc_regs.hold));
	save_item(NAME(m_intc_regs.status));
	save_item(NAME(m_intc_regs.enable));
	save_item(NAME(m_intc_regs.mask));

	save_item(NAME(m_timer_regs.timer[0].period));
	save_item(NAME(m_timer_regs.timer[0].count));
	save_item(NAME(m_timer_regs.timer[0].control));
	save_item(NAME(m_timer_regs.timer[1].period));
	save_item(NAME(m_timer_regs.timer[1].count));
	save_item(NAME(m_timer_regs.timer[1].control));
	save_item(NAME(m_timer_regs.timer[2].period));
	save_item(NAME(m_timer_regs.timer[2].count));
	save_item(NAME(m_timer_regs.timer[2].control));

	save_item(NAME(m_clock_regs.mode));
	save_item(NAME(m_clock_regs.control));

	save_item(NAME(m_rtc_regs.mode));
	save_item(NAME(m_rtc_regs.control));
	save_item(NAME(m_rtc_regs.time));
	save_item(NAME(m_rtc_regs.date));

	save_item(NAME(m_ps_flash_write_enable_count));
	save_item(NAME(m_ps_flash_write_count));

	save_item(NAME(m_lcd_control));
}

void pockstat_state::machine_reset()
{
	m_maincpu->set_state_int(ARM7_R15, 0x4000000);

	m_ps_flash_write_enable_count = 0;
	m_ps_flash_write_count = 0;
}

UINT32 pockstat_state::screen_update_pockstat(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 32; y++)
	{
		UINT32 *scanline = &bitmap.pix32(y);
		for (int x = 0; x < 32; x++)
		{
			if (m_lcd_control != 0) // Hack
			{
				if (m_lcd_buffer[y] & (1 << x))
					scanline[x] = 0x00000000;
				else
					scanline[x] = 0x00ffffff;
			}
			else
				scanline[x] = 0x00ffffff;
		}
	}
	return 0;
}

DEVICE_IMAGE_LOAD_MEMBER( pockstat_state, pockstat_flash )
{
	static const char *gme_id = "123-456-STD";
	char cart_id[0xf40];
	UINT32 size = image.length();

	if (size != 0x20f40)
		return IMAGE_INIT_FAIL;

	image.fread(cart_id, 0xf40);

	for (int i = 0; i < strlen(gme_id); i++)
	{
		if (cart_id[i] != gme_id[i])
			return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(0x20000, GENERIC_ROM32_WIDTH, ENDIANNESS_LITTLE);
	image.fread(m_cart->get_rom_base(), 0x20000);

	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( pockstat, pockstat_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM7, DEFAULT_CLOCK)
	MCFG_CPU_PROGRAM_MAP(pockstat_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(32, 32)
	MCFG_SCREEN_VISIBLE_AREA(0, 32-1, 0, 32-1)
	MCFG_SCREEN_UPDATE_DRIVER(pockstat_state, screen_update_pockstat)

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "pockstat_cart")
	MCFG_GENERIC_EXTENSIONS("gme")
	MCFG_GENERIC_WIDTH(GENERIC_ROM32_WIDTH)
	MCFG_GENERIC_ENDIAN(ENDIANNESS_LITTLE)
	MCFG_GENERIC_LOAD(pockstat_state, pockstat_flash)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pockstat )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "kernel.bin", 0x0000, 0x4000, CRC(5fb47dd8) SHA1(6ae880493ddde880827d1e9f08e9cb2c38f9f2ec) )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT     INIT  COMPANY                             FULLNAME       FLAGS */
CONS( 1999, pockstat, 0,      0,      pockstat,  pockstat, driver_device, 0,    "Sony Computer Entertainment Inc", "Sony PocketStation", MACHINE_SUPPORTS_SAVE )
