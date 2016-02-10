// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    TMP68301 basic emulation + Interrupt Handling

    The Toshiba TMP68301 is a 68HC000 + serial I/O, parallel I/O,
    3 timers, address decoder, wait generator, interrupt controller,
    all integrated in a single chip.

    TODO:
    - Interrupt generation: handle pending / in-service mechanisms
    - Parallel port: handle timing latency
    - Serial port: not done at all
    - (and many other things)

***************************************************************************/

#include "emu.h"
#include "machine/tmp68301.h"

const device_type TMP68301 = &device_creator<tmp68301_device>;

static ADDRESS_MAP_START( tmp68301_regs, AS_0, 16, tmp68301_device )
//  AM_RANGE(0x000,0x3ff) AM_RAM
	AM_RANGE(0x094,0x095) AM_READWRITE(imr_r,imr_w)
	AM_RANGE(0x098,0x099) AM_READWRITE(iisr_r,iisr_w)

	/* Parallel Port */
	AM_RANGE(0x100,0x101) AM_READWRITE(pdir_r,pdir_w)
	AM_RANGE(0x10a,0x10b) AM_READWRITE(pdr_r,pdr_w)

	/* Serial Port */
	AM_RANGE(0x18e,0x18f) AM_READWRITE(scr_r,scr_w)
ADDRESS_MAP_END

// IRQ Mask register, 0x94
READ16_MEMBER(tmp68301_device::imr_r)
{
	return m_imr;
}

WRITE16_MEMBER(tmp68301_device::imr_w)
{
	COMBINE_DATA(&m_imr);
}

// IRQ In-Service Register
READ16_MEMBER(tmp68301_device::iisr_r)
{
	return m_iisr;
}

WRITE16_MEMBER(tmp68301_device::iisr_w)
{
	COMBINE_DATA(&m_iisr);
}

// Serial Control Register (TODO: 8-bit wide)
READ16_MEMBER(tmp68301_device::scr_r)
{
	return m_scr;
}

WRITE16_MEMBER(tmp68301_device::scr_w)
{
	/*
	    *--- ---- CKSE
	    --*- ---- RES
	    ---- ---* INTM
	*/

	COMBINE_DATA(&m_scr);
	m_scr &= 0xa1;
}

/* Parallel direction: 1 = output, 0 = input */
READ16_MEMBER(tmp68301_device::pdir_r)
{
	return m_pdir;
}

WRITE16_MEMBER(tmp68301_device::pdir_w)
{
	COMBINE_DATA(&m_pdir);
}

READ16_MEMBER(tmp68301_device::pdr_r)
{
	return (m_in_parallel_cb(0) & ~m_pdir) | (m_pdr & m_pdir);
}

WRITE16_MEMBER(tmp68301_device::pdr_w)
{
	UINT16 old = m_pdr;
	COMBINE_DATA(&m_pdr);
	m_pdr = (old & ~m_pdir) | (m_pdr & m_pdir);
	m_out_parallel_cb(0, m_pdr, mem_mask);
}


tmp68301_device::tmp68301_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMP68301, "TMP68301", tag, owner, clock, "tmp68301", __FILE__),
		device_memory_interface(mconfig, *this),
		m_in_parallel_cb(*this),
		m_out_parallel_cb(*this),
		m_imr(0),
		m_iisr(0),
		m_scr(0),
		m_pdir(0),
		m_pdr(0),
		m_space_config("regs", ENDIANNESS_LITTLE, 16, 10, 0, nullptr, *ADDRESS_MAP_NAME(tmp68301_regs))
{
	memset(m_regs, 0, sizeof(m_regs));
	memset(m_IE, 0, sizeof(m_IE));
	memset(m_irq_vector, 0, sizeof(m_irq_vector));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmp68301_device::device_start()
{
	int i;
	for (i = 0; i < 3; i++)
		m_tmp68301_timer[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(tmp68301_device::timer_callback), this));

	m_in_parallel_cb.resolve_safe(0);
	m_out_parallel_cb.resolve_safe();

	save_item(NAME(m_regs));
	save_item(NAME(m_IE));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_imr));
	save_item(NAME(m_iisr));
	save_item(NAME(m_scr));
	save_item(NAME(m_pdir));
	save_item(NAME(m_pdr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tmp68301_device::device_reset()
{
	int i;

	for (i = 0; i < 3; i++)
		m_IE[i] = 0;

	m_imr = 0x7f7; // mask all irqs
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *tmp68301_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_byte - read a byte at the given address
//-------------------------------------------------

inline UINT16 tmp68301_device::read_word(offs_t address)
{
	return space(AS_0).read_word(address << 1);
}

//-------------------------------------------------
//  write_byte - write a byte at the given address
//-------------------------------------------------

inline void tmp68301_device::write_word(offs_t address, UINT16 data)
{
	space(AS_0).write_word(address << 1, data);
}

IRQ_CALLBACK_MEMBER(tmp68301_device::irq_callback)
{
	int vector = m_irq_vector[irqline];
//  logerror("%s: irq callback returns %04X for level %x\n",machine.describe_context(),vector,int_level);
	return vector;
}

TIMER_CALLBACK_MEMBER( tmp68301_device::timer_callback )
{
	int i = param;
	UINT16 TCR  =   m_regs[(0x200 + i * 0x20)/2];
	UINT16 ICR  =   m_regs[0x8e/2+i];    // Interrupt Controller Register (ICR7..9)
	UINT16 IVNR =   m_regs[0x9a/2];      // Interrupt Vector Number Register (IVNR)

//  logerror("s: callback timer %04X, j = %d\n",machine.describe_context(),i,tcount);

	if  (   (TCR & 0x0004) &&   // INT
			!(m_imr & (0x100<<i))
		)
	{
		int level = ICR & 0x0007;

		// Interrupt Vector Number Register (IVNR)
		m_irq_vector[level]  =   IVNR & 0x00e0;
		m_irq_vector[level]  +=  4+i;

		machine().firstcpu->set_input_line(level,HOLD_LINE);
	}

	if (TCR & 0x0080)   // N/1
	{
		// Repeat
		update_timer(i);
	}
	else
	{
		// One Shot
	}
}

void tmp68301_device::update_timer( int i )
{
	UINT16 TCR  =   m_regs[(0x200 + i * 0x20)/2];
	UINT16 MAX1 =   m_regs[(0x204 + i * 0x20)/2];
	UINT16 MAX2 =   m_regs[(0x206 + i * 0x20)/2];

	int max = 0;
	attotime duration = attotime::zero;

	m_tmp68301_timer[i]->adjust(attotime::never,i);

	// timers 1&2 only
	switch( (TCR & 0x0030)>>4 )                     // MR2..1
	{
	case 1:
		max = MAX1;
		break;
	case 2:
		max = MAX2;
		break;
	}

	switch ( (TCR & 0xc000)>>14 )                   // CK2..1
	{
	case 0: // System clock (CLK)
		if (max)
		{
			int scale = (TCR & 0x3c00)>>10;         // P4..1
			if (scale > 8) scale = 8;
			duration = attotime::from_hz(machine().firstcpu->unscaled_clock()) * ((1 << scale) * max);
		}
		break;
	}

//  logerror("%s: TMP68301 Timer %d, duration %lf, max %04X\n",machine().describe_context(),i,duration,max);

	if (!(TCR & 0x0002))                // CS
	{
		if (duration != attotime::zero)
			m_tmp68301_timer[i]->adjust(duration,i);
		else
			logerror("%s: TMP68301 error, timer %d duration is 0\n",machine().describe_context(),i);
	}
}

/* Update the IRQ state based on all possible causes */
void tmp68301_device::update_irq_state()
{
	int i;

	/* Take care of external interrupts */

	UINT16 IVNR =   m_regs[0x9a/2];      // Interrupt Vector Number Register (IVNR)

	for (i = 0; i < 3; i++)
	{
		if  (   (m_IE[i]) &&
				!(m_imr & (1<<i))
			)
		{
			UINT16 ICR  =   m_regs[0x80/2+i];    // Interrupt Controller Register (ICR0..2)

			// Interrupt Controller Register (ICR0..2)
			int level = ICR & 0x0007;

			// Interrupt Vector Number Register (IVNR)
			m_irq_vector[level]  =   IVNR & 0x00e0;
			m_irq_vector[level]  +=  i;

			m_IE[i] = 0;     // Interrupts are edge triggerred

			machine().firstcpu->set_input_line(level,HOLD_LINE);
		}
	}
}

READ16_MEMBER( tmp68301_device::regs_r )
{
	return read_word(offset);
}

WRITE16_MEMBER( tmp68301_device::regs_w )
{
	COMBINE_DATA(&m_regs[offset]);

	write_word(offset,m_regs[offset]);

	if (!ACCESSING_BITS_0_7)    return;

//  logerror("CPU #0 PC %06X: TMP68301 Reg %04X<-%04X & %04X\n",space.device().safe_pc(),offset*2,data,mem_mask^0xffff);

	switch( offset * 2 )
	{
		// Timers
		case 0x200:
		case 0x220:
		case 0x240:
		{
			int i = ((offset*2) >> 5) & 3;

			update_timer( i );
		}
		break;
	}
}

void tmp68301_device::external_interrupt_0()    { m_IE[0] = 1;   update_irq_state(); }
void tmp68301_device::external_interrupt_1()    { m_IE[1] = 1;   update_irq_state(); }
void tmp68301_device::external_interrupt_2()    { m_IE[2] = 1;   update_irq_state(); }
