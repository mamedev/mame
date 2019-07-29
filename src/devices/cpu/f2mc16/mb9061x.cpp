// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Fujitsu Micro MB9061x Microcontroller Family
    Emulation by R. Belmont
*/

#include "emu.h"
#include "mb9061x.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MB90610A, mb90610_device, "mb90610a", "Fujitsu MB90610A")
DEFINE_DEVICE_TYPE(MB90611A, mb90611_device, "mb90611a", "Fujitsu MB90611A")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mb9061x_device - constructor
//-------------------------------------------------
mb9061x_device::mb9061x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map) :
	f2mc16_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 24, 0, internal_map)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb9061x_device::device_start()
{
	f2mc16_device::device_start();

	m_tbtc_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mb9061x_device::tbtc_tick), this));
	m_tbtc_timer->adjust(attotime::never);
}


device_memory_interface::space_config_vector mb9061x_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb9061x_device::device_reset()
{
	f2mc16_device::device_reset();
}

void mb9061x_device::execute_set_input(int inputnum, int state)
{
}

/* MB90610 - "Evaluation device" with extra RAM */
void mb90610_device::mb90610_map(address_map &map)
{
	map(0x00a9, 0x00a9).rw(FUNC(mb9061x_device::tbtc_r), FUNC(mb9061x_device::tbtc_w));
	map(0x00b0, 0x00bf).rw(FUNC(mb9061x_device::intc_r), FUNC(mb9061x_device::intc_w));
	map(0x0100, 0x10ff).ram();  // 4K of internal RAM from 0x100 to 0x1100
}

mb90610_device::mb90610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mb90610_device(mconfig, MB90610A, tag, owner, clock)
{
}

mb90610_device::mb90610_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mb9061x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(mb90610_device::mb90610_map), this))
{
}

/* TBTC: timebase counter */
enum
{
	TBTC_TEST = 0x80,   // test, must always write 1
	TBTC_TBIE = 0x10,   // interrupt enable
	TBTC_TBOF = 0x08,   // expire flag
	TBTC_TBR  = 0x04,   // write 0 to restart
	TBTC_TBC1 = 0x02,   // rate select bit 1
	TBTC_TBC0 = 0x01    // rate select bit 0
};

READ8_MEMBER(mb9061x_device::tbtc_r)
{
	return m_tbtc;
}

WRITE8_MEMBER(mb9061x_device::tbtc_w)
{
	static const float periods[4] = { 1.024, 4.096, 16.384, 131.072 };

	//printf("%02x to TBTC\n", data);
	if ((!(data & TBTC_TBR)) || ((data & (TBTC_TBC1|TBTC_TBC0)) != (m_tbtc & (TBTC_TBC1|TBTC_TBC0))))
	{
		m_tbtc_timer->adjust(attotime(0, ATTOSECONDS_IN_MSEC(periods[data & (TBTC_TBC1|TBTC_TBC0)])));

		if (!(data & TBTC_TBR))
		{
			intc_clear_irq(11, 0x22);
		}
	}

	if (!(data & TBTC_TBOF) && (m_tbtc & TBTC_TBOF))
	{
		intc_clear_irq(11, 0x22);
	}

	m_tbtc = data;
}

/* INTC: Interrupt controller */
TIMER_CALLBACK_MEMBER(mb9061x_device::tbtc_tick)
{
	//printf("TBTC tick\n");
	m_tbtc_timer->adjust(attotime::never);
	m_tbtc |= TBTC_TBOF;
	if (m_tbtc & TBTC_TBIE)
	{
		intc_trigger_irq(11, 0x22);
	}
}

READ8_MEMBER(mb9061x_device::intc_r)
{
	return m_intc[offset];
}

WRITE8_MEMBER(mb9061x_device::intc_w)
{
	//printf("INTC ICR %d to %02x\n", offset, data);
	m_intc[offset] = data;
}

void mb9061x_device::intc_trigger_irq(int icr, int vector)
{
	int level = m_intc[icr] & 7;

	//printf("trigger_irq: ICR %d, level %d\n", icr, level);

	// Extended Intelligent I/O Service?
	if (m_intc[icr] & 8)
	{
		fatalerror("MB9061X: ICR %d (vector %x) calls for EI2OS, not implemented\n", icr, vector);
	}
	else
	{
		if (level != 7)
		{
			set_irq(vector, level);
		}
	}
}

void mb9061x_device::intc_clear_irq(int icr, int vector)
{
	int level = m_intc[icr] & 7;

	// Make sure this isn't the Extended Intelligent I/O Service
	if (!(m_intc[icr] & 8))
	{
		if (level != 7)
		{
			clear_irq(vector);
		}
	}
}

/* MB90611 - Production version of this series */
void mb90611_device::mb90611_map(address_map &map)
{
	map(0x00a9, 0x00a9).rw(FUNC(mb9061x_device::tbtc_r), FUNC(mb9061x_device::tbtc_w));
	map(0x00b0, 0x00bf).rw(FUNC(mb9061x_device::intc_r), FUNC(mb9061x_device::intc_w));
	map(0x0100, 0x04ff).ram();  // 1K of internal RAM from 0x100 to 0x500
}

mb90611_device::mb90611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mb90611_device(mconfig, MB90611A, tag, owner, clock)
{
}

mb90611_device::mb90611_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mb9061x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(mb90611_device::mb90611_map), this))
{
}
