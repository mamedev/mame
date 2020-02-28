// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

#include "emu.h"
#include "segaic16.h"

#include <algorithm>

//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_MULTIPLY    (0)
#define LOG_DIVIDE      (0)
#define LOG_COMPARE     (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SEGA_315_5248_MULTIPLIER,    sega_315_5248_multiplier_device,    "sega_315_5248", "Sega 315-5248 Multiplier")
DEFINE_DEVICE_TYPE(SEGA_315_5249_DIVIDER,       sega_315_5249_divider_device,       "sega_315_5249", "Sega 315-5249 Divider")
DEFINE_DEVICE_TYPE(SEGA_315_5250_COMPARE_TIMER, sega_315_5250_compare_timer_device, "sega_315_5250", "Sega 315-5250 Compare/Timer")



//**************************************************************************
//  315-5248 MULTIPLIER
//**************************************************************************

//-------------------------------------------------
//  sega_315_5248_multiplier_device - constructor
//-------------------------------------------------

sega_315_5248_multiplier_device::sega_315_5248_multiplier_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGA_315_5248_MULTIPLIER, tag, owner, clock)
{
}


//-------------------------------------------------
//  read - read the registers
//-------------------------------------------------

u16 sega_315_5248_multiplier_device::read(offs_t offset)
{
	switch (offset & 3)
	{
		// if bit 1 is 0, just return register values
		case 0: return m_regs[0];
		case 1: return m_regs[1];

		// if bit 1 is 1, return ther results
		case 2: return (s16(m_regs[0]) * s16(m_regs[1])) >> 16;
		case 3: return (s16(m_regs[0]) * s16(m_regs[1])) & 0xffff;
	}

	// should never get here
	return 0xffff;
}


//-------------------------------------------------
//  write - write to the registers
//-------------------------------------------------

void sega_315_5248_multiplier_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	// only low bit matters
	COMBINE_DATA(&m_regs[offset & 1]);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5248_multiplier_device::device_start()
{
	save_item(NAME(m_regs));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_315_5248_multiplier_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
}


//**************************************************************************
//  315-5249 DIVIDER
//**************************************************************************

//-------------------------------------------------
//  sega_315_5249_divider_device - constructor
//-------------------------------------------------

sega_315_5249_divider_device::sega_315_5249_divider_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGA_315_5249_DIVIDER, tag, owner, clock)
{
}


//-------------------------------------------------
//  read - read the registers
//-------------------------------------------------

u16 sega_315_5249_divider_device::read(offs_t offset)
{
	// 8 effective read registers
	switch (offset & 7)
	{
		case 0: return m_regs[0];   // dividend high
		case 1: return m_regs[1];   // dividend low
		case 2: return m_regs[2];   // divisor
		case 4: return m_regs[4];   // quotient (mode 0) or quotient high (mode 1)
		case 5: return m_regs[5];   // remainder (mode 0) or quotient low (mode 1)
		case 6: return m_regs[6];   // flags
	}
	return 0xffff;
}


//-------------------------------------------------
//  write - write to the registers
//-------------------------------------------------

void sega_315_5249_divider_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	if (LOG_DIVIDE) logerror("divide_w(%X) = %04X\n", offset, data);

	// only 4 effective write registers
	switch (offset & 3)
	{
		case 0: COMBINE_DATA(&m_regs[0]); break;    // dividend high
		case 1: COMBINE_DATA(&m_regs[1]); break;    // dividend low
		case 2: COMBINE_DATA(&m_regs[2]); break;    // divisor/trigger
		case 3: break;
	}

	// if A4 line is high, divide, using A3 as the mode
	if (offset & 8)
		execute(offset & 4);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5249_divider_device::device_start()
{
	save_item(NAME(m_regs));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_315_5249_divider_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
}


//-------------------------------------------------
//  execute - execute the divide
//-------------------------------------------------

void sega_315_5249_divider_device::execute(int mode)
{
	// clear the flags by default
	m_regs[6] = 0;

	// mode 0: signed divide, return 16-bit quotient/remainder
	if (mode == 0)
	{
		// perform signed divide
		const s32 dividend = s32((m_regs[0] << 16) | m_regs[1]);
		const s32 divisor = s16(m_regs[2]);
		s32 quotient;

		// check for divide by 0, signal if we did
		if (divisor == 0)
		{
			quotient = dividend;//((s32)(dividend ^ divisor) < 0) ? 0x8000 : 0x7fff;
			m_regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;

		// clamp to 16-bit signed, signal overflow if we did
		if (quotient < -32768)
		{
			quotient = -32768;
			m_regs[6] |= 0x8000;
		}
		else if (quotient > 32767)
		{
			quotient = 32767;
			m_regs[6] |= 0x8000;
		}

		// store quotient and remainder
		m_regs[4] = s16(quotient);
		m_regs[5] = s16(dividend - quotient * divisor);
	}

	// mode 1: unsigned divide, 32-bit quotient only
	else
	{
		// perform unsigned divide
		const u32 dividend = u32((m_regs[0] << 16) | m_regs[1]);
		const u32 divisor = u16(m_regs[2]);
		u32 quotient;

		// check for divide by 0, signal if we did
		if (divisor == 0)
		{
			quotient = dividend;//0x7fffffff;
			m_regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;

		// store 32-bit quotient
		m_regs[4] = quotient >> 16;
		m_regs[5] = quotient & 0xffff;
	}
}


//**************************************************************************
//  315-5250 COMPARE/TIMER
//**************************************************************************

//-------------------------------------------------
//  sega_315_5250_compare_timer_device -
//  constructor
//-------------------------------------------------

sega_315_5250_compare_timer_device::sega_315_5250_compare_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGA_315_5250_COMPARE_TIMER, tag, owner, clock)
	, m_68kint_callback(*this)
	, m_zint_callback(*this)
	, m_exck(false)
{
}


//-------------------------------------------------
//  exck_w - clock the timer
//-------------------------------------------------

WRITE_LINE_MEMBER(sega_315_5250_compare_timer_device::exck_w)
{
	if (m_exck == bool(state))
		return;

	// update on rising edge
	m_exck = bool(state);
	if (!m_exck)
		return;

	// if we're enabled, clock the upcounter
	const int old_counter = m_counter;
	if (m_regs[10] & 1)
		m_counter++;

	// regardless of the enable, a value of 0xfff will generate the IRQ
	if (old_counter == 0xfff)
	{
		if (!m_68kint_callback.isnull())
			m_68kint_callback(ASSERT_LINE);
		m_counter = m_regs[8] & 0xfff;
	}
}


//-------------------------------------------------
//  interrupt_ack - acknowledge timer interrupt
//-------------------------------------------------

void sega_315_5250_compare_timer_device::interrupt_ack()
{
	if (!m_68kint_callback.isnull())
		m_68kint_callback(CLEAR_LINE);
}


//-------------------------------------------------
//  read - read the registers
//-------------------------------------------------

u16 sega_315_5250_compare_timer_device::read(offs_t offset)
{
	if (LOG_COMPARE) logerror("compare_r(%X) = %04X\n", offset, m_regs[offset]);
	switch (offset & 15)
	{
		case 0x0:   return m_regs[0];
		case 0x1:   return m_regs[1];
		case 0x2:   return m_regs[2];
		case 0x3:   return m_regs[3];
		case 0x4:   return m_regs[4];
		case 0x5:   return m_regs[1];
		case 0x6:   return m_regs[2];
		case 0x7:   return m_regs[7];
		case 0x9:
		case 0xd:   if (!machine().side_effects_disabled()) interrupt_ack(); break;
	}
	return 0xffff;
}


//-------------------------------------------------
//  write - write to the registers
//-------------------------------------------------

void sega_315_5250_compare_timer_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	if (LOG_COMPARE) logerror("compare_w(%X) = %04X\n", offset, data);
	switch (offset & 15)
	{
		case 0x0:   COMBINE_DATA(&m_regs[0]); execute(); break;
		case 0x1:   COMBINE_DATA(&m_regs[1]); execute(); break;
		case 0x2:   COMBINE_DATA(&m_regs[2]); execute(true); break;
		case 0x4:   m_regs[4] = 0; m_bit = 0; break;
		case 0x6:   COMBINE_DATA(&m_regs[2]); execute(); break;
		case 0x8:
		case 0xc:   COMBINE_DATA(&m_regs[8]); break;
		case 0x9:
		case 0xd:   interrupt_ack(); break;
		case 0xa:
		case 0xe:   COMBINE_DATA(&m_regs[10]); break;
		case 0xb:
		case 0xf:
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(sega_315_5250_compare_timer_device::write_to_sound), this), data & 0xff);
			break;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5250_compare_timer_device::device_start()
{
	// bind our handlers
	m_68kint_callback.resolve();
	m_zint_callback.resolve();

	// save states
	save_item(NAME(m_regs));
	save_item(NAME(m_counter));
	save_item(NAME(m_bit));
	save_item(NAME(m_exck));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_315_5250_compare_timer_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_counter = 0;
	m_bit = 0;

	interrupt_ack();
	if (!m_zint_callback.isnull())
		m_zint_callback(CLEAR_LINE);
}


//-------------------------------------------------
//  write_to_sound - write data for the sound CPU
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(sega_315_5250_compare_timer_device::write_to_sound)
{
	m_regs[11] = param;
	if (!m_zint_callback.isnull())
		m_zint_callback(ASSERT_LINE);
}


//-------------------------------------------------
//  zread - read data from sound CPU bus
//-------------------------------------------------

u8 sega_315_5250_compare_timer_device::zread()
{
	if (!m_zint_callback.isnull() && !machine().side_effects_disabled())
		m_zint_callback(CLEAR_LINE);

	return m_regs[11];
}


//-------------------------------------------------
//  execute - execute the compare
//-------------------------------------------------

void sega_315_5250_compare_timer_device::execute(bool update_history)
{
	const s16 bound1 = s16(m_regs[0]);
	const s16 bound2 = s16(m_regs[1]);
	const s16 value = s16(m_regs[2]);

	const s16 min = (bound1 < bound2) ? bound1 : bound2;
	const s16 max = (bound1 > bound2) ? bound1 : bound2;

	if (value < min)
	{
		m_regs[7] = min;
		m_regs[3] = 0x8000;
	}
	else if (value > max)
	{
		m_regs[7] = max;
		m_regs[3] = 0x4000;
	}
	else
	{
		m_regs[7] = value;
		m_regs[3] = 0x0000;
	}

	if (update_history)
		m_regs[4] |= (m_regs[3] == 0) << m_bit++;
}
