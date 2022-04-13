// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************************

    Rockwell A17XX ROM, RAM and I/O chip

    A ROM of 2048 x 8 bits is addressed whenever the RRSEL line
    (ROM/RAM select) is 0. A RAM of 128 x 4 bit is addressed when
    RRSEL is 1. The 16 I/O ports are addressed when the WI/O line
    is 1, i.e. whenever the CPU executes an IOL instruction.
    There are two basic I/O instructions:
    SES = Select Enable Status and SOS = Select Output Status
    The lower 4 bits of the I/O address select one of 16 I/O lines.
    Actually the lowest 6 bits are used, but bits 4 and 5 must be 0.

    There are at most two A17XX per system, one for the lower
    ROM and RAM portion and one for the higher.

    I/O section instructions

    Menmonic  I/O bus            Accu      Description
    ------------------------------------------------------------------
    SES       0 S S 0 X X X 0    1 X X X   Enable all outputs
                                           Acuu:3 <- I/O(BL)
    ------------------------------------------------------------------
    SES       0 S S 0 X X X 0    0 X X X   Disable all outputs
                                           Acuu:3 <- I/O(BL)
    ------------------------------------------------------------------
    SOS       0 S S 0 X X X 1    1 X X X   I/O(BL) <- 1
                                           Acuu:3 <- I/O(BL)
    ------------------------------------------------------------------
    SOS       0 S S 0 X X X 1    0 X X X   I/O(BL) <- 0
                                           Acuu:3 <- I/O(BL)

    This device emulation takes care of the I/O commands, not the
    ROM and RAM, because these are emulated using the generic MAME
    memory system.
**********************************************************************/

#include "emu.h"
#include "machine/ra17xx.h"

//#define VERBOSE 1
#include "logmacro.h"


/*************************************
 *
 *  Device interface
 *
 *************************************/

DEFINE_DEVICE_TYPE(RA17XX, ra17xx_device, "ra17xx", "Rockwell A17xx")

ra17xx_device::ra17xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RA17XX, tag, owner, clock)
	, m_enable(false)
	, m_iord(*this)
	, m_iowr(*this)
	, m_cpu(*this, finder_base::DUMMY_TAG)
{
}

/**
 * @brief ra17xx_device::device_start device-specific startup
 */
void ra17xx_device::device_start()
{
	m_iord.resolve();
	m_iowr.resolve();

	save_item(NAME(m_line));
}

/**
 * @brief ra17xx_device::device_reset device-specific reset
 */
void ra17xx_device::device_reset()
{
	memset(m_line, 0, sizeof(m_line));
}


/*************************************
 *
 *  Constants
 *
 *************************************/

/*************************************
 *
 *  Command access handlers
 *
 *************************************/

void ra17xx_device::io_w(address_space &space, offs_t offset, uint8_t data)
{
	assert(offset < 16);

	m_bl = m_cpu->address_bus_r(space) & 63;

	if (offset & 1)
	{
		// SOS command
		if (m_bl >= 16)
		{
			logerror("Attempt to write to nonexistent output 0x%02X\n",m_bl);
		}
		else if (data & (1 << 3))
		{
			m_line[m_bl] = 1;   // enable output
//          if (m_enable)
				m_iowr(m_bl, 1, 1);
		}
		else
		{
			m_line[m_bl] = 0;   // disable output
//          if (m_enable)
				m_iowr(m_bl, 0, 1);
		}
	}
	else
	{
		// SES command
		if (data & (1 << 3))
		{
			// enable all outputs
			m_enable = true;
			for (int i = 0; i < 16; i++)
				m_iowr(i, m_line[i], 1);
		}
		else
		{
			// disable all outputs
			m_enable = false;
		}
	}
}


uint8_t ra17xx_device::io_r(offs_t offset)
{
	assert(offset < 16);
	return (m_bl >= 16 || (m_iord(m_bl) & 1)) ? 0x0f : 0x07;
}
