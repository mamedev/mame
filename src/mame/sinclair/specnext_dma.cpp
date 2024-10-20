// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next DMA

    Spectrum Next DMA operates in two mode: z80dma compatible and
    N-mode with Next specific fixes.

    z80dma mode is implemented there based on intensive testing of
    the device. Potentially any mismatches not related to N-mode
    covered here must be moved to the z80dma parent.

**********************************************************************/

#include "emu.h"
#include "specnext_dma.h"

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_DMA, specnext_dma_device, "specnext_dma", "Spectrum Next DMA")


// TODO: this stuff is copy/pasted from machine/z80dma.cpp - ideally it wouldn't need to be
#define WR0                     REG(0, 0)
#define WR1                     REG(1, 0)
#define WR2                     REG(2, 0)

#define PORTA_INC               (WR1 & 0x10)
#define PORTB_INC               (WR2 & 0x10)
#define PORTA_FIXED             (((WR1 >> 4) & 0x02) == 0x02)
#define PORTB_FIXED             (((WR2 >> 4) & 0x02) == 0x02)

#define TRANSFER_MODE           (WR0 & 0x03)


specnext_dma_device::specnext_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80dma_device(mconfig, SPECNEXT_DMA, tag, owner, clock)
{
}

void specnext_dma_device::write(u8 data)
{
	z80dma_device::write(data);

	if (num_follow() == 0)
	{
		if ((data & 0x83) == 0x83) // WR6
		{
			switch (data)
			{
			case COMMAND_ENABLE_DMA:
				m_byte_counter = 0;
				break;
			default:
				break;
			}
		}
	}
}

void specnext_dma_device::do_write()
{
	if (m_dma_mode)
	{
		z80dma_device::do_write();
		return;
	}
	// else (zxnDMA)

	if (m_byte_counter)
		m_addressB += PORTB_FIXED ? 0 : PORTB_INC ? 1 : -1;

	switch (TRANSFER_MODE)
	{
	case TM_TRANSFER:
		do_transfer_write();
		break;

	case TM_SEARCH:
		do_search();
		break;

	case TM_SEARCH_TRANSFER:
		do_transfer_write();
		do_search();
		break;

	default:
		logerror("z80dma_do_operation: invalid mode %d!\n", TRANSFER_MODE);
		break;
	}

	m_addressA += PORTA_FIXED ? 0 : PORTA_INC ? 1 : -1;

	m_byte_counter++;
	if ((m_byte_counter + 1) == m_count)
		m_byte_counter++;
}


void specnext_dma_device::device_start()
{
	z80dma_device::device_start();

	save_item(NAME(m_dma_mode));
}

void specnext_dma_device::device_reset()
{
	z80dma_device::device_reset();

	m_dma_mode = 0;
}
