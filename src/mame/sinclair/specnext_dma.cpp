// license:BSD-3-Clause
/**********************************************************************

	Spectrum Next DMA

	Spectrum Next DMA operates in two mode: z80dma compatible and N-mode with Next specific fixes.
z80dma mode is implemented there based on intensive testing of the device. Potentially any mismatches
not related to N-mode covered here must be moved to the z80dma parent.

**********************************************************************/

#include "emu.h"
#include "specnext_dma.h"

constexpr int COMMAND_ENABLE_DMA                    = 0x87;

specnext_dma_device::specnext_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80dma_device(mconfig, SPECNEXT_DMA, tag, owner, clock)
{}

void specnext_dma_device::write(u8 data)
{
	z80dma_device::write(data);

	if (m_num_follow == 0)
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

void specnext_dma_device::device_start()
{
	z80dma_device::device_start();

	//save_item(NAME());
}

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_DMA, specnext_dma_device, "dma", "Spectrum Next DMA")
