// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sony PocketStation

    PocketStation games were downloaded from PS1 games into flash RAM after
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

    Currently, a handful of games run, but some die due to odd hardware
    issues.

    To start a game:
    - Wait for the set-date screen to appear
    - Press Down
    - Set date with directional controls (optional)
    - Press Button 1, wait, press Button 1, press Right, game starts

    If you do nothing for about 20 secs, it turns itself off (screen goes white).

****************************************************************************/

#include "emu.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/arm7/arm7.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_UNKNOWN (1U << 1)
#define LOG_FTLB    (1U << 2)
#define LOG_IRQS    (1U << 3)
#define LOG_INTC    (1U << 4)
#define LOG_TIMER   (1U << 5)
#define LOG_CLOCK   (1U << 6)
#define LOG_RTC     (1U << 7)
#define LOG_LCD     (1U << 8)
#define LOG_AUDIO   (1U << 9)
#define LOG_ALL     (LOG_UNKNOWN | LOG_FTLB | LOG_IRQS | LOG_INTC | LOG_TIMER | LOG_CLOCK | LOG_RTC | LOG_LCD | LOG_AUDIO)
#define LOG_DEFAULT LOG_ALL

#define VERBOSE     (0)
#include "logmacro.h"


namespace {

class pockstat_state : public driver_device
{
public:
	pockstat_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_lcd_buffer(*this, "lcd_buffer"),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot")
	{ }

	void pockstat(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_update);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	uint32_t screen_update_pockstat(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void set_interrupt_line(uint32_t line, int state);
	uint32_t get_interrupt_line(uint32_t line);

	void timer_start(int index);

	required_shared_ptr<uint32_t> m_lcd_buffer;
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_rom = nullptr;

	static constexpr uint32_t TIMER_COUNT = 3;

	enum : uint32_t
	{
		CLOCK_STEADY = 0x10
	};

	enum
	{
		INT_BTN_ACTION     = 0x00000001, // "Action button"
		INT_BTN_RIGHT      = 0x00000002, // "Right button"
		INT_BTN_LEFT       = 0x00000004, // "Left button"
		INT_BTN_DOWN       = 0x00000008, // "Down button"
		INT_BTN_UP         = 0x00000010, // "Up button"
		INT_UNKNOWN        = 0x00000020, // "Unknown"
		INT_COM            = 0x00000040, // "COM" ???
		INT_TIMER0         = 0x00000080, // "Timer 0"
		INT_TIMER1         = 0x00000100, // "Timer 1"
		INT_RTC            = 0x00000200, // "RTC"
		INT_BATTERY        = 0x00000400, // "Battery Monitor"
		INT_IOP            = 0x00000800, // "IOP"
		INT_IRDA           = 0x00001000, // "IrDA"
		INT_TIMER2         = 0x00002000, // "Timer 2"
		INT_IRQ_MASK       = 0x00001fbf,
		INT_FIQ_MASK       = 0x00002040,
		INT_STATUS_MASK    = 0x0000021f
	};

	struct ftlb_regs_t
	{
		uint32_t control = 0;
		uint32_t stat = 0;
		uint32_t valid = 0;
		uint32_t wait1 = 0;
		uint32_t wait2 = 0;
		uint32_t entry[16]{};
		uint32_t serial = 0;
	} m_ftlb_regs;

	struct intc_regs_t
	{
		uint32_t hold = 0;
		uint32_t status = 0;
		uint32_t enable = 0;
		uint32_t mask = 0;
	} m_intc_regs;

	struct timer_t
	{
		uint32_t period = 0;
		uint32_t count = 0;
		uint32_t control = 0;
		emu_timer *timer = nullptr;
	} m_timers[TIMER_COUNT];

	struct clock_regs_t
	{
		uint32_t mode = 0;
		uint32_t control = 0;
	} m_clock_regs;

	struct rtc_regs_t
	{
		uint32_t mode = 0;
		uint32_t control = 0;
		uint32_t time = 0;
		uint32_t date = 0;
		emu_timer *timer = nullptr;
	} m_rtc_regs;

	uint32_t m_lcd_control = 0;
	int32_t m_flash_write_enable_count = 0;
	int32_t m_flash_write_count = 0;

	uint32_t ftlb_r(offs_t offset, uint32_t mem_mask = ~0);
	void ftlb_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t intc_r(offs_t offset, uint32_t mem_mask = ~0);
	void intc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t timer_r(offs_t offset, uint32_t mem_mask = ~0);
	void timer_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t clock_r(offs_t offset, uint32_t mem_mask = ~0);
	void clock_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rtc_r(offs_t offset, uint32_t mem_mask = ~0);
	void rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t lcd_r(offs_t offset, uint32_t mem_mask = ~0);
	void lcd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rombank_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t flash_r(offs_t offset, uint32_t mem_mask = ~0);
	void flash_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t audio_r(offs_t offset, uint32_t mem_mask = ~0);
	void audio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_CALLBACK_MEMBER(timer_tick);
	TIMER_CALLBACK_MEMBER(rtc_tick);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(flash_load);

	static const int CPU_FREQ[16];
};

const int pockstat_state::CPU_FREQ[16] =
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

uint32_t pockstat_state::ftlb_r(offs_t offset, uint32_t mem_mask)
{
	switch(offset)
	{
	case 0x0000/4:
		LOGMASKED(LOG_FTLB, "%s: FlashROM TLB Control = %08x & %08x\n", machine().describe_context(), m_ftlb_regs.control, mem_mask);
		return m_ftlb_regs.control | 1; // ???
	case 0x0004/4:
		LOGMASKED(LOG_FTLB, "%s: Unknown (F_STAT) = %08x & %08x\n", machine().describe_context(), m_ftlb_regs.stat, mem_mask);
		return m_ftlb_regs.stat;
	case 0x0008/4:
		LOGMASKED(LOG_FTLB, "%s: FlashROM TLB Valid Tag = %08x & %08x\n", machine().describe_context(), m_ftlb_regs.valid, mem_mask);
		return m_ftlb_regs.valid;
	case 0x000c/4:
		LOGMASKED(LOG_FTLB, "%s: Unknown (F_WAIT1) = %08x & %08x\n", machine().describe_context(), m_ftlb_regs.wait1, mem_mask);
		return m_ftlb_regs.wait1;
	case 0x0010/4:
		LOGMASKED(LOG_FTLB, "%s: Unknown (F_WAIT2) = %08x & %08x\n", machine().describe_context(), m_ftlb_regs.wait2 | 0x04, mem_mask);
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
		LOGMASKED(LOG_FTLB, "%s: FlashROM TLB Entry %d = %08x & %08x\n", machine().describe_context(), offset - 0x100/4, m_ftlb_regs.entry[offset - 0x100/4],
			mem_mask);
		return m_ftlb_regs.entry[offset - 0x100/4];
	case 0x0300/4:
		LOGMASKED(LOG_FTLB, "%s: Unknown (F_SN) = %08x & %08x\n", machine().describe_context(), m_ftlb_regs.serial, mem_mask);
		return m_ftlb_regs.serial;
	default:
		LOGMASKED(LOG_FTLB | LOG_UNKNOWN, "%s: Unknown Register %08x & %08x\n", machine().describe_context(), 0x06000000 + (offset << 2), mem_mask);
		break;
	}
	return 0;
}

void pockstat_state::ftlb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch(offset)
	{
	case 0x0000/4:
		LOGMASKED(LOG_FTLB, "%s: FlashROM TLB Control = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ftlb_regs.control);
		break;
	case 0x0004/4:
		LOGMASKED(LOG_FTLB, "%s: Unknown (F_STAT) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ftlb_regs.stat);
		break;
	case 0x0008/4:
		LOGMASKED(LOG_FTLB, "%s: FlashROM TLB Valid Tag = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ftlb_regs.valid);
		break;
	case 0x000c/4:
		LOGMASKED(LOG_FTLB, "%s: Unknown (F_WAIT1) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ftlb_regs.wait1);
		break;
	case 0x0010/4:
		LOGMASKED(LOG_FTLB, "%s: Unknown (F_WAIT2) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
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
		LOGMASKED(LOG_FTLB, "%s: FlashROM TLB Entry %d = %08x & %08x\n", machine().describe_context(), offset - 0x100/4, data, mem_mask);
		COMBINE_DATA(&m_ftlb_regs.entry[offset - 0x100/4]);
		break;
	case 0x0300/4:
		LOGMASKED(LOG_FTLB, "%s: Unknown (F_SN) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_ftlb_regs.serial);
		break;
	default:
		LOGMASKED(LOG_FTLB | LOG_UNKNOWN, "%s: Unknown Register %08x = %08x & %08x\n", machine().describe_context(), 0x06000000 + (offset << 2), data, mem_mask);
		break;
	}
}

uint32_t pockstat_state::get_interrupt_line(uint32_t line)
{
	return m_intc_regs.status & line;
}

void pockstat_state::set_interrupt_line(uint32_t line, int state)
{
	if (line)
	{
		if (state)
		{
			m_intc_regs.status |= line & INT_STATUS_MASK;
			m_intc_regs.hold |= line &~ INT_STATUS_MASK;
			LOGMASKED(LOG_IRQS, "Setting IRQ line %08x, status = %08x, hold = %08x\n", line, m_intc_regs.status, m_intc_regs.hold);
		}
		else
		{
			m_intc_regs.status &= ~line;
			m_intc_regs.hold &= ~line;
			LOGMASKED(LOG_IRQS, "Clearing IRQ line %08x, status = %08x, hold = %08x\n", line, m_intc_regs.status, m_intc_regs.hold);
		}
	}

	const uint32_t new_irq = m_intc_regs.hold & m_intc_regs.enable & INT_IRQ_MASK;
	if (new_irq)
	{
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, CLEAR_LINE);
	}

	const uint32_t new_fiq = m_intc_regs.hold & m_intc_regs.enable & INT_FIQ_MASK;
	if (new_fiq)
	{
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, CLEAR_LINE);
	}
}

uint32_t pockstat_state::intc_r(offs_t offset, uint32_t mem_mask)
{
	switch(offset)
	{
	case 0x0000/4:
		LOGMASKED(LOG_INTC, "%s: Held Interrupt Read: %08x & %08x\n", machine().describe_context(), m_intc_regs.hold, mem_mask);
		return m_intc_regs.hold;
	case 0x0004/4:
		LOGMASKED(LOG_INTC, "%s: Interrupt Status Read: %08x & %08x\n", machine().describe_context(), m_intc_regs.status, mem_mask);
		return m_intc_regs.status;
	case 0x0008/4:
		LOGMASKED(LOG_INTC, "%s: Interrupt Enable Read: %08x & %08x\n", machine().describe_context(), m_intc_regs.enable, mem_mask);
		return m_intc_regs.enable;
	case 0x000c/4:
		LOGMASKED(LOG_INTC, "%s: Interrupt Mask Read (Invalid): %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x0010/4:
		LOGMASKED(LOG_INTC, "%s: Interrupt Acknowledge Read (Invalid): %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	default:
		LOGMASKED(LOG_INTC | LOG_UNKNOWN, "%s: Unknown Register Read: %08x & %08x\n", machine().describe_context(), 0x0a000000 + (offset << 2), mem_mask);
		break;
	}
	return 0;
}

void pockstat_state::intc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch(offset)
	{
	case 0x0000/4:
		LOGMASKED(LOG_INTC, "%s: Held Interrupt (Invalid Write) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x0004/4:
		LOGMASKED(LOG_INTC, "%s: Interrupt Status (Invalid Write) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x0008/4:
		LOGMASKED(LOG_INTC, "%s: Interrupt Enable = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_intc_regs.enable |= data;
		//COMBINE_DATA(&m_intc_regs.enable);
		//m_intc_regs.status &= m_intc_regs.enable;
		//m_intc_regs.hold &= m_intc_regs.enable;
		set_interrupt_line(0, 0);
		break;
	case 0x000c/4:
		LOGMASKED(LOG_INTC, "%s: Interrupt Mask = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_intc_regs.enable &= ~data;
		COMBINE_DATA(&m_intc_regs.mask);
		//m_intc_regs.status &= m_intc_regs.enable;
		//m_intc_regs.hold &= m_intc_regs.enable;
		set_interrupt_line(0, 0);
		break;
	case 0x0010/4:
		LOGMASKED(LOG_INTC, "%s: Interrupt Acknowledge = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_intc_regs.hold &= ~data;
		m_intc_regs.status &= ~data;
		set_interrupt_line(0, 0);
		//COMBINE_DATA(&m_intc_regs.acknowledge);
		break;
	default:
		LOGMASKED(LOG_INTC | LOG_UNKNOWN, "%s: Unknown Register %08x = %08x & %08x\n", machine().describe_context(), 0x0a000000 + (offset << 2), data, mem_mask);
		break;
	}
}

TIMER_CALLBACK_MEMBER(pockstat_state::timer_tick)
{
	set_interrupt_line(param == 2 ? INT_TIMER2 : (param == 1 ? INT_TIMER1 : INT_TIMER0), 1);
	m_timers[param].count = m_timers[param].period;
	timer_start(param);
}

void pockstat_state::timer_start(int index)
{
	int divisor = 1;
	attotime period;
	switch (m_timers[index].control & 3)
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
	period = attotime::from_hz(CPU_FREQ[m_clock_regs.mode & 0x0f] / 2) * divisor * m_timers[index].count;
	m_timers[index].timer->adjust(period, index);
}

uint32_t pockstat_state::timer_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case 0x0000/4:
	case 0x0010/4:
	case 0x0020/4:
	{
		const uint32_t index = offset / (0x10/4);
		LOGMASKED(LOG_TIMER, "%s: Timer %d Period Read: %08x & %08x\n", machine().describe_context(), index, m_timers[index].period, mem_mask);
		return m_timers[index].period;
	}
	case 0x0004/4:
	case 0x0014/4:
	case 0x0024/4:
	{
		const uint32_t index = offset / (0x10/4);
		LOGMASKED(LOG_TIMER, "%s: Timer %d Count Read: %08x & %08x\n", machine().describe_context(), index, m_timers[index].count, mem_mask);
		if(m_timers[index].control & 4)
		{
			m_timers[index].count--;
			if (m_timers[index].count > m_timers[index].period)
			{
				m_timers[index].count = m_timers[index].period;
			}
			return --m_timers[index].count;
		}
		return m_timers[index].count;
	}
	case 0x0008/4:
	case 0x0018/4:
	case 0x0028/4:
	{
		const uint32_t index = offset / (0x10/4);
		LOGMASKED(LOG_TIMER, "%s: Timer %d Control = %08x & %08x\n", machine().describe_context(), index, m_timers[index].control, mem_mask);
		return m_timers[index].control;
	}
	default:
		LOGMASKED(LOG_TIMER | LOG_UNKNOWN, "%s: Unknown Register %08x & %08x\n", machine().describe_context(), 0x0a800000 + (offset << 2), mem_mask);
		break;
	}
	return 0;
}

void pockstat_state::timer_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case 0x0000/4:
	case 0x0010/4:
	case 0x0020/4:
	{
		const uint32_t index = offset / (0x10/4);
		LOGMASKED(LOG_TIMER, "%s: Timer %d Period = %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		COMBINE_DATA(&m_timers[index].period);
		break;
	}
	case 0x0004/4:
	case 0x0014/4:
	case 0x0024/4:
	{
		const uint32_t index = offset / (0x10/4);
		LOGMASKED(LOG_TIMER, "%s: Timer %d Count = %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		COMBINE_DATA(&m_timers[index].count);
		break;
	}
	case 0x0008/4:
	case 0x0018/4:
	case 0x0028/4:
	{
		const uint32_t index = offset / (0x10/4);
		LOGMASKED(LOG_TIMER, "%s: Timer %d Control = %08x & %08x\n", machine().describe_context(), index, data, mem_mask);
		COMBINE_DATA(&m_timers[index].control);
		if(m_timers[index].control & 4)
		{
			timer_start(index);
		}
		else
		{
			m_timers[index].timer->adjust(attotime::never, index);
		}
		break;
	}
	default:
		LOGMASKED(LOG_TIMER | LOG_UNKNOWN, "%s: Unknown Register %08x = %08x & %08x\n", machine().describe_context(), 0x0a800000 + (offset << 2), data, mem_mask);
		break;
	}
}

uint32_t pockstat_state::clock_r(offs_t offset, uint32_t mem_mask)
{
	switch(offset)
	{
		case 0x0000/4:
			LOGMASKED(LOG_CLOCK, "%s: Clock Mode Read: %08x & %08x\n", machine().describe_context(), m_clock_regs.mode | 0x10, mem_mask);
			return m_clock_regs.mode | CLOCK_STEADY;
		case 0x0004/4:
			LOGMASKED(LOG_CLOCK, "%s: Clock Control Read: %08x & %08x\n", machine().describe_context(), m_clock_regs.control, mem_mask);
			return m_clock_regs.control;
		default:
			LOGMASKED(LOG_CLOCK | LOG_UNKNOWN, "%s: Unknown Register %08x & %08x\n", machine().describe_context(), 0x0b000000 + (offset << 2), mem_mask);
			break;
	}
	return 0;
}

void pockstat_state::clock_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch(offset)
	{
		case 0x0000/4:
			LOGMASKED(LOG_CLOCK, "%s: Clock Mode = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_clock_regs.mode);
			m_maincpu->set_unscaled_clock(CPU_FREQ[m_clock_regs.mode & 0x0f]);
			break;
		case 0x0004/4:
			LOGMASKED(LOG_CLOCK, "%s: Clock Control = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_clock_regs.control);
			break;
		default:
			LOGMASKED(LOG_CLOCK | LOG_UNKNOWN, "%s: Unknown Register %08x = %08x & %08x\n", machine().describe_context(), 0x0b000000 + (offset << 2), data, mem_mask);
			break;
	}
}

TIMER_CALLBACK_MEMBER(pockstat_state::rtc_tick)
{
	set_interrupt_line(INT_RTC, get_interrupt_line(INT_RTC) ? 0 : 1);
	if (!(m_rtc_regs.mode & 1))
	{
		m_rtc_regs.time++;
		if ((m_rtc_regs.time & 0x0000000f) == 0x0000000a)
		{
			m_rtc_regs.time &= 0xfffffff0;
			m_rtc_regs.time += 0x00000010;
			if ((m_rtc_regs.time & 0x000000ff) == 0x00000060)
			{
				m_rtc_regs.time &= 0xffffff00;
				m_rtc_regs.time += 0x00000100;
				if ((m_rtc_regs.time & 0x00000f00) == 0x00000a00)
				{
					m_rtc_regs.time &= 0xfffff0ff;
					m_rtc_regs.time += 0x00001000;
					if ((m_rtc_regs.time & 0x0000ff00) == 0x00006000)
					{
						m_rtc_regs.time &= 0xffff00ff;
						m_rtc_regs.time += 0x00010000;
						if ((m_rtc_regs.time & 0x00ff0000) == 0x00240000)
						{
							m_rtc_regs.time &= 0xff00ffff;
							m_rtc_regs.time += 0x01000000;
							if ((m_rtc_regs.time & 0x0f000000) == 0x08000000)
							{
								m_rtc_regs.time &= 0xf0ffffff;
								m_rtc_regs.time |= 0x01000000;
							}
						}
						else if ((m_rtc_regs.time & 0x000f0000) == 0x000a0000)
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

uint32_t pockstat_state::rtc_r(offs_t offset, uint32_t mem_mask)
{
	switch(offset)
	{
	case 0x0000/4:
		LOGMASKED(LOG_RTC, "%s: RTC Mode Read: %08x & %08x\n", machine().describe_context(), m_rtc_regs.mode, mem_mask);
		return m_rtc_regs.mode;
	case 0x0004/4:
		LOGMASKED(LOG_RTC, "%s: RTC Control Read: %08x & %08x\n", machine().describe_context(), m_rtc_regs.control, mem_mask);
		return m_rtc_regs.control;
	case 0x0008/4:
		LOGMASKED(LOG_RTC, "%s: RTC Time Read: %08x & %08x\n", machine().describe_context(), m_rtc_regs.time, mem_mask);
		return m_rtc_regs.time;
	case 0x000c/4:
		LOGMASKED(LOG_RTC, "%s: RTC Date Read: %08x & %08x\n", machine().describe_context(), m_rtc_regs.date, mem_mask);
		return m_rtc_regs.date;
	default:
		LOGMASKED(LOG_RTC | LOG_UNKNOWN, "%s: Unknown Register %08x & %08x\n", machine().describe_context(), 0x0b800000 + (offset << 2), mem_mask);
		break;
	}
	return 0;
}

void pockstat_state::rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case 0x0000/4:
		LOGMASKED(LOG_RTC, "%s: RTC Mode = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_rtc_regs.mode);
		break;
	case 0x0004/4:
		LOGMASKED(LOG_RTC, "%s: RTC Control = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (m_rtc_regs.control == 1 && data == 1)
		{
			switch (m_rtc_regs.mode >> 1)
			{
			case 0: // Seconds
				m_rtc_regs.time += 0x00000001;
				if ((m_rtc_regs.time & 0x0000000f) == 0x0000000a)
				{
					m_rtc_regs.time &= 0xfffffff0;
					m_rtc_regs.time += 0x00000010;
					if ((m_rtc_regs.time & 0x000000ff) == 0x00000060)
					{
						m_rtc_regs.time &= 0xffffff00;
					}
				}
				break;
			case 1: // Minutes
				m_rtc_regs.time += 0x00000100;
				if ((m_rtc_regs.time & 0x00000f00) == 0x00000a00)
				{
					m_rtc_regs.time &= 0xfffff0ff;
					m_rtc_regs.time += 0x00001000;
					if ((m_rtc_regs.time & 0x0000ff00) == 0x00006000)
					{
						m_rtc_regs.time &= 0xffff00ff;
					}
				}
				break;
			case 2: // Hours
				m_rtc_regs.time += 0x00010000;
				if ((m_rtc_regs.time & 0x00ff0000) == 0x00240000)
				{
					m_rtc_regs.time &= 0xff00ffff;
				}
				else if ((m_rtc_regs.time & 0x000f0000) == 0x000a0000)
				{
					m_rtc_regs.time &= 0xfff0ffff;
					m_rtc_regs.time += 0x00100000;
				}
				break;
			case 3: // Day of the week
				m_rtc_regs.time += 0x01000000;
				if ((m_rtc_regs.time & 0x0f000000) == 0x08000000)
				{
					m_rtc_regs.time &= 0xf0ffffff;
					m_rtc_regs.time |= 0x01000000;
				}
				break;
			case 4: // Day
				m_rtc_regs.date += 0x00000001;
				if ((m_rtc_regs.date & 0x000000ff) == 0x00000032)
				{
					m_rtc_regs.date &= 0xffffff00;
				}
				else if ((m_rtc_regs.date & 0x0000000f) == 0x0000000a)
				{
					m_rtc_regs.date &= 0xfffffff0;
					m_rtc_regs.date += 0x00000010;
				}
				break;
			case 5: // Month
				m_rtc_regs.date += 0x00000100;
				if ((m_rtc_regs.date & 0x0000ff00) == 0x00001300)
				{
					m_rtc_regs.date &= 0xffffff00;
					m_rtc_regs.date |= 0x00000001;
				}
				else if ((m_rtc_regs.date & 0x00000f00) == 0x00000a00)
				{
					m_rtc_regs.date &= 0xfffff0ff;
					m_rtc_regs.date += 0x00001000;
				}
				break;
			case 6: // Year (LSB)
				m_rtc_regs.date += 0x00010000;
				if ((m_rtc_regs.date & 0x000f0000) == 0x000a0000)
				{
					m_rtc_regs.date &= 0xfff0ffff;
					m_rtc_regs.date += 0x00100000;
					if ((m_rtc_regs.date & 0x00f00000) == 0x00a00000)
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
		LOGMASKED(LOG_RTC | LOG_UNKNOWN, "%s: Unknown Register %08x = %08x & %08x\n", machine().describe_context(), 0x0b800000 + (offset << 2), data, mem_mask);
		break;
	}
}


uint32_t pockstat_state::lcd_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case 0x0000/4:
		LOGMASKED(LOG_LCD, "%s: LCD Control Read: %08x & %08x\n", machine().describe_context(), m_lcd_control | 0x100, mem_mask);
		return m_lcd_control;
	default:
		LOGMASKED(LOG_LCD | LOG_UNKNOWN, "%s: Unknown Register %08x & %08x\n", machine().describe_context(), 0x0d000000 + (offset << 2), mem_mask);
		break;
	}
	return 0;
}

void pockstat_state::lcd_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case 0x0000/4:
		LOGMASKED(LOG_LCD, "%s: LCD Control = %08x & %08x\n", machine().describe_context(), data, mem_mask);
		COMBINE_DATA(&m_lcd_control);
		break;
	default:
		LOGMASKED(LOG_LCD | LOG_UNKNOWN, "%s: Unknown Register %08x = %08x & %08x\n", machine().describe_context(), 0x0d000000 + (offset << 2), data, mem_mask);
		break;
	}
}

INPUT_CHANGED_MEMBER(pockstat_state::input_update)
{
	uint32_t buttons = ioport("BUTTONS")->read();

	set_interrupt_line(INT_BTN_ACTION, (buttons &  1) ? 1 : 0);
	set_interrupt_line(INT_BTN_RIGHT,  (buttons &  2) ? 1 : 0);
	set_interrupt_line(INT_BTN_LEFT,   (buttons &  4) ? 1 : 0);
	set_interrupt_line(INT_BTN_DOWN,   (buttons &  8) ? 1 : 0);
	set_interrupt_line(INT_BTN_UP,     (buttons & 16) ? 1 : 0);
}

uint32_t pockstat_state::rombank_r(offs_t offset, uint32_t mem_mask)
{
	int32_t bank = (offset >> 11) & 0x0f;
	for (int index = 0; index < 32; index++)
	{
		if (m_ftlb_regs.valid & (1 << index))
		{
			if (m_ftlb_regs.entry[index] == bank)
			{
				return m_cart->read32_rom(index * (0x2000/4) + (offset & (0x1fff/4)), mem_mask);
			}
		}
	}
	return m_cart->read32_rom(offset & 0x7fff, mem_mask);
}


// Horrible hack, probably wrong
void pockstat_state::flash_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == (0x55a8/4))
	{
		m_flash_write_enable_count++;
		return;
	}
	if (offset == (0x2a54/4))
	{
		m_flash_write_enable_count++;
		return;
	}
	if (m_flash_write_enable_count == 3)
	{
		m_flash_write_enable_count = 0;
		m_flash_write_count = 0x40;
		return;
	}
	if (m_flash_write_count)
	{
		m_flash_write_count--;
		COMBINE_DATA(&((uint32_t*)(m_cart_rom->base()))[offset]);
	}
}

uint32_t pockstat_state::flash_r(offs_t offset, uint32_t mem_mask)
{
	return m_cart->read32_rom(offset, mem_mask);
}

uint32_t pockstat_state::audio_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO | LOG_UNKNOWN, "%s: Unknown Audio Read: %08x = %08x & %08x\n", machine().describe_context(), 0xd800000 + (offset << 2), 0x10, mem_mask);
	return 0;
}

void pockstat_state::audio_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO | LOG_UNKNOWN, "%s: Unknown Audio Write: %08x = %08x & %08x\n", machine().describe_context(), 0xd800000 + (offset << 2), data, mem_mask);
}

void pockstat_state::mem_map(address_map &map)
{
	map(0x00000000, 0x000007ff).ram();
	map(0x02000000, 0x02ffffff).r(FUNC(pockstat_state::rombank_r));
	map(0x04000000, 0x04003fff).rom().region("maincpu", 0);
	map(0x06000000, 0x06000307).rw(FUNC(pockstat_state::ftlb_r), FUNC(pockstat_state::ftlb_w));
	map(0x08000000, 0x0801ffff).rw(FUNC(pockstat_state::flash_r), FUNC(pockstat_state::flash_w));
	map(0x0a000000, 0x0a000013).rw(FUNC(pockstat_state::intc_r), FUNC(pockstat_state::intc_w));
	map(0x0a800000, 0x0a80002b).rw(FUNC(pockstat_state::timer_r), FUNC(pockstat_state::timer_w));
	map(0x0b000000, 0x0b000007).rw(FUNC(pockstat_state::clock_r), FUNC(pockstat_state::clock_w));
	map(0x0b800000, 0x0b80000f).rw(FUNC(pockstat_state::rtc_r), FUNC(pockstat_state::rtc_w));
	map(0x0d000000, 0x0d000003).rw(FUNC(pockstat_state::lcd_r), FUNC(pockstat_state::lcd_w));
	map(0x0d000100, 0x0d00017f).ram().share("lcd_buffer");
	map(0x0d80000c, 0x0d80000f).rw(FUNC(pockstat_state::audio_r), FUNC(pockstat_state::audio_w));
	map(0x0d800014, 0x0d800015).w("dac", FUNC(dac_word_interface::data_w));
}

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
	for (uint32_t index = 0; index < TIMER_COUNT; index++)
	{
		m_timers[index].timer = timer_alloc(FUNC(pockstat_state::timer_tick), this);
		m_timers[index].timer->adjust(attotime::never, index);
		m_timers[index].count = 0;
	}

	m_rtc_regs.time = 0x01000000;
	m_rtc_regs.date = 0x19990101;

	m_rtc_regs.timer = timer_alloc(FUNC(pockstat_state::rtc_tick), this);
	m_rtc_regs.timer->adjust(attotime::from_hz(1), TIMER_COUNT);

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

	save_item(NAME(m_timers[0].period));
	save_item(NAME(m_timers[0].count));
	save_item(NAME(m_timers[0].control));
	save_item(NAME(m_timers[1].period));
	save_item(NAME(m_timers[1].count));
	save_item(NAME(m_timers[1].control));
	save_item(NAME(m_timers[2].period));
	save_item(NAME(m_timers[2].count));
	save_item(NAME(m_timers[2].control));

	save_item(NAME(m_clock_regs.mode));
	save_item(NAME(m_clock_regs.control));

	save_item(NAME(m_rtc_regs.mode));
	save_item(NAME(m_rtc_regs.control));
	save_item(NAME(m_rtc_regs.time));
	save_item(NAME(m_rtc_regs.date));

	save_item(NAME(m_flash_write_enable_count));
	save_item(NAME(m_flash_write_count));

	save_item(NAME(m_lcd_control));
}

void pockstat_state::machine_reset()
{
	m_maincpu->set_state_int(arm7_cpu_device::ARM7_R15, 0x4000000);

	m_flash_write_enable_count = 0;
	m_flash_write_count = 0;
}

uint32_t pockstat_state::screen_update_pockstat(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 32; y++)
	{
		uint32_t *const scanline = &bitmap.pix(y);
		for (int x = 0; x < 32; x++)
		{
			if (m_lcd_control != 0) // Hack
			{
				if (BIT(m_lcd_buffer[y], x))
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

DEVICE_IMAGE_LOAD_MEMBER(pockstat_state::flash_load)
{
	static const char *gme_id = "123-456-STD";
	char cart_id[0xf40];
	uint32_t const size = image.length();

	if (size != 0x20f40)
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	image.fread(cart_id, 0xf40);

	for (int i = 0; i < strlen(gme_id); i++)
	{
		if (cart_id[i] != gme_id[i])
			return std::make_pair(image_error::INVALIDIMAGE, std::string());
	}

	m_cart->rom_alloc(0x20000, GENERIC_ROM32_WIDTH, ENDIANNESS_LITTLE);
	image.fread(m_cart->get_rom_base(), 0x20000);

	return std::make_pair(std::error_condition(), std::string());
}

void pockstat_state::pockstat(machine_config &config)
{
	static constexpr uint32_t DEFAULT_CLOCK = 2000000;

	/* basic machine hardware */
	ARM7(config, m_maincpu, DEFAULT_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &pockstat_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32, 32);
	screen.set_visarea(0, 32-1, 0, 32-1);
	screen.set_screen_update(FUNC(pockstat_state::screen_update_pockstat));

	PALETTE(config, "palette", palette_device::MONOCHROME);

	SPEAKER(config, "speaker").front_center();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "pockstat_cart", "gme");
	m_cart->set_width(GENERIC_ROM32_WIDTH);
	m_cart->set_endian(ENDIANNESS_LITTLE);
	m_cart->set_device_load(FUNC(pockstat_state::flash_load));
}

/* ROM definition */
ROM_START( pockstat )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "kernel.bin", 0x0000, 0x4000, CRC(5fb47dd8) SHA1(6ae880493ddde880827d1e9f08e9cb2c38f9f2ec) )
ROM_END

} // Anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                            FULLNAME              FLAGS
CONS( 1999, pockstat, 0,      0,      pockstat, pockstat, pockstat_state, empty_init, "Sony Computer Entertainment Inc", "Sony PocketStation", MACHINE_SUPPORTS_SAVE )
