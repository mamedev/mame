// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    video/cirrus.c

    Cirrus SVGA card emulation (preliminary)

    Cirrus has the following additional registers that are not present in
    conventional VGA:

    SEQ 06h:        Unlock Cirrus registers; write 12h to unlock registers,
                    and read 12h back to confirm Cirrus presence.
    SEQ 07h
        bit 3-1:    Pixel depth
                        0x00    8 bpp
                        0x02    16 bpp (double vert clock)
                        0x04    24 bpp
                        0x06    16 bpp
                        0x08    32 bpp
        bit 0:      VGA/SVGA (0=VGA, 1=SVGA)
    SEQ 0Fh
        bit 7:      Bankswitch enable
        bits 4-3:   Memory size
                        0x00    256K
                        0x08    512K
                        0x10    1M
                        0x18    2M
    SEQ 12h:        Hardware Cursor




    GC 09h:         Set 64k bank (bits 3-0 only)
    GC 20h:         Blit Width (bits 7-0)
    GC 21h:         Blit Width (bits 12-8)
    GC 22h:         Blit Height (bits 7-0)
    GC 23h:         Blit Height (bits 12-8)
    GC 24h:         Blit Destination Pitch (bits 7-0)
    GC 25h:         Blit Destination Pitch (bits 12-8)
    GC 26h:         Blit Source Pitch (bits 7-0)
    GC 27h:         Blit Source Pitch (bits 12-8)
    GC 28h:         Blit Destination Address (bits 7-0)
    GC 29h:         Blit Destination Address (bits 15-8)
    GC 2Ah:         Blit Destination Address (bits 21-16)
    GC 2Ch:         Blit Source Address (bits 7-0)
    GC 2Dh:         Blit Source Address (bits 15-8)
    GC 2Eh:         Blit Source Address (bits 21-16)
    GC 2Fh:         Blit Write Mask
    GC 30h:         Blit Mode
    GC 31h:         Blit Status
                        bit 7 - Autostart
                        bit 4 - FIFO Used
                        bit 2 - Blit Reset
                        bit 1 - Blit Started
                        bit 0 - Blit Busy
    GC 32h:         Raster Operation
    GC 33h:         Blit Mode Extension
    GC 34h:         Blit Transparent Color (bits 7-0)
    GC 35h:         Blit Transparent Color (bits 15-8)
    GC 38h:         Blit Transparent Color Mask (bits 7-0)
    GC 39h:         Blit Transparent Color Mask (bits 15-8)

***************************************************************************/

#include "emu.h"
#include "cirrus.h"
#include "video/pc_vga.h"

#define LOG_PCIACCESS   0

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CIRRUS = &device_creator<cirrus_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cirrus_device - constructor
//-------------------------------------------------

cirrus_device::cirrus_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, CIRRUS, "CIRRUS", tag, owner, clock, "cirrus", __FILE__),
		pci_device_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cirrus_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cirrus_device::device_reset()
{
}

//-------------------------------------------------
//  pci_read - implementation of PCI read
//-------------------------------------------------

UINT32 cirrus_device::pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask)
{
	UINT32 result = 0;

	if (function == 0)
	{
		switch(offset)
		{
			case 0x00:  /* vendor/device ID */
				result = 0x00A01013;
				break;

			case 0x08:
				result = 0x03000000;
				break;

			case 0x10:
				result = 0xD0000000;
				break;

			default:
				result = 0;
				break;
		}
	}

	if (LOG_PCIACCESS)
		logerror("cirrus5430_pci_read(): function=%d offset=0x%02X result=0x%04X\n", function, offset, result);
	return result;
}


//-------------------------------------------------
//  pci_write - implementation of PCI write
//-------------------------------------------------

void cirrus_device::pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask)
{
	if (LOG_PCIACCESS)
		logerror("cirrus5430_pci_write(): function=%d offset=0x%02X data=0x%04X\n", function, offset, data);
}

/*************************************
 *
 *  Ports
 *
 *************************************/

WRITE8_MEMBER( cirrus_device::cirrus_42E8_w )
{
	if (data & 0x80)
		machine().device("vga")->reset();
}
