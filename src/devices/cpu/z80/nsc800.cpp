// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*

    National Semiconductor NSC800

*/

#include "emu.h"
#include "nsc800.h"

#include "z80.inc"

#define LOG_INT   (1U << 1) // z80.lst

//#define VERBOSE (LOG_INT)
#include "logmacro.h"


// device type definition
DEFINE_DEVICE_TYPE(NSC800, nsc800_device, "nsc800", "National Semiconductor NSC800")

nsc800_device::nsc800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, NSC800, tag, owner, clock)
{
}


//-------------------------------------------------
//  initialization
//-------------------------------------------------

void nsc800_device::device_start()
{
	z80_device::device_start();

	save_item(NAME(m_nsc800_irq_state));
}

void nsc800_device::device_reset()
{
	z80_device::device_reset();
	memset(m_nsc800_irq_state, 0, sizeof(m_nsc800_irq_state));
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void nsc800_device::do_op()
{
	#include "cpu/z80/ncs800.hxx"
}

void nsc800_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case NSC800_RSTA:
		m_nsc800_irq_state[0] = state;
		break;

	case NSC800_RSTB:
		m_nsc800_irq_state[1] = state;
		break;

	case NSC800_RSTC:
		m_nsc800_irq_state[2] = state;
		break;

	default:
		z80_device::execute_set_input(inputnum, state);
		break;
	}
}
