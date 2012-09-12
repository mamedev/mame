/***************************************************************************

    Coleco Adam

    The Coleco ADAM is a Z80-based micro with all peripheral devices
    attached to an internal serial serial bus (ADAMnet) managed by 6801
    microcontrollers (processor, internal RAM, internal ROM, serial port).Each
    device had its own 6801, and the ADAMnet was managed by a "master" 6801 on
    the ADAM motherboard.  Each device was allotted a block of 21 bytes in Z80
    address space; device control was accomplished by poking function codes into
    the first byte of each device control block (hereafter DCB) after setup of
    other DCB locations with such things as:  buffer address in Z80 space, # of
    bytes to transfer, block # to access if it was a block device like a tape
    or disk drive, etc.  The master 6801 would interpret this data, and pass
    along internal ADAMnet requests to the desired peripheral, which would then
    send/receive its data and return the status of the operation.  The status
    codes were left in the same byte of the DCB as the function request, and
    certain bits of the status byte would reflect done/working on it/error/
    not present, and error codes were left in another DCB byte for things like
    CRC error, write protected disk, missing block, etc.

    ADAM's ROM operating system, EOS (Elementary OS), was constructed
    similar to CP/M in that it provided both a filesystem (like BDOS) and raw
    device interface (BIOS).  At the file level, sequential files could be
    created, opened, read, written, appended, closed, and deleted.  Forward-
    only random access was implemented (you could not move the R/W pointer
    backward, except clear to the beginning!), and all files had to be
    contiguous on disk/tape.  Directories could be initialized or searched
    for a matching filename (no wildcards allowed).  At the device level,
    individual devices could be read/written by block (for disks/tapes) or
    character-by-character (for printer, keyboard, and a prototype serial
    board which was never released).  Devices could be checked for their
    ADAMnet status, and reset if necessary.  There was no function provided
    to do low-level formatting of disks/tapes.

     At system startup, the EOS was loaded from ROM into the highest
    8K of RAM, a function call made to initialize the ADAMnet, and then
    any disks or tapes were checked for a boot medium; if found, block 0 of
    the medium was loaded in, and a jump made to the start of the boot code.
    The boot block would take over loading in the rest of the program.  If no
    boot media were found, a jump would be made to a ROM word processor (called
    SmartWriter).

     Coleco designed the ADAMnet to have up to 15 devices attached.
    Before they went bankrupt, Coleco had released a 64K memory expander and
    a 300-baud internal modem, but surprisingly neither of these was an
    ADAMnet device.  Disassembly of the RAMdisk drivers in ADAM CP/M 2.2, and
    of the ADAMlink terminal program revealed that these were simple port I/O
    devices, banks of XRAM being accessed by a special memory switch port not
    documented as part of the EOS.  The modem did not even use the interrupt
    capabilities of the Z80--it was simply polled.  A combination serial/
    parallel interface, each port of which *was* an ADAMnet device, reached the
    prototype stage, as did a 5MB hard disk, but neither was ever released to
    the public.  (One prototype serial/parallel board is still in existence,
    but the microcontroller ROMs have not yet been succcessfully read.)  So
    when Coleco finally bailed out of the computer business, a maximum ADAM
    system consisted of a daisy wheel printer, a keyboard, 2 tape drives, and
    2 disk drives (all ADAMnet devices), a 64K expander and a 300-baud modem
    (which were not ADAMnet devices).

     Third-party vendors reverse-engineered the modem (which had a
    2651 UART at its heart) and made a popular serial interface board.  It was
    not an ADAMnet device, however, because nobody knew how to make a new ADAMnet
    device (no design specs were ever released), and the 6801 microcontrollers
    had unreadable mask ROMs.  Disk drives, however, were easily upgraded from
    160K to as high as 1MB because, for some unknown reason, the disk controller
    boards used a separate microprocessor and *socketed* EPROM (which was
    promptly disassembled and reworked).  Hard drives were cobbled together from
    a Kaypro-designed board and accessed as standard I/O port devices.  A parallel
    interface card was similarly set up at its own I/O port.

      Devices (15 max):
        Device 0 = Master 6801 ADAMnet controller (uses the adam_pcb as DCB)
        Device 1 = Keyboard
        Device 2 = ADAM printer
        Device 3 = Copywriter (projected)
        Device 4 = Disk drive 1
        Device 5 = Disk drive 2
        Device 6 = Disk drive 3 (third party)
        Device 7 = Disk drive 4 (third party)
        Device 8 = Tape drive 1
        Device 9 = Tape drive 3 (projected)
        Device 10 = Unused
        Device 11 = Non-ADAMlink modem
        Device 12 = Hi-resolution monitor
        Device 13 = ADAM parallel interface (never released)
        Device 14 = ADAM serial interface (never released)
        Device 15 = Gateway
        Device 24 = Tape drive 2 (share DCB with Tape1)
        Device 25 = Tape drive 4 (projected, may have share DCB with Tape3)
        Device 26 = Expansion RAM disk drive (third party ID, not used by Coleco)

      Terminology:
        EOS = Elementary Operating System
        DCB = Device Control Block Table (21bytes each DCB, DCB+16=dev#, DCB+0=Status Byte) (0xFD7C)

               0     Status byte
             1-2     Buffer start address (lobyte, hibyte)
             3-4     Buffer length (lobyte, hibyte)
             5-8     Block number accessed (loword, hiword in lobyte, hibyte format)
               9     High nibble of device number
            10-15    Always zero (unknown purpose)
              16     Low nibble of device number
            17-18    Maximum block length (lobyte, hibyte)
              19     Device type (0 for block device, 1 for character device)
              20     Node type

            - Writing to byte0 requests the following operations:
                1     Return current status
                2     Soft reset
                3     Write
                4     Read


        FCB = File Control Block Table (32bytes, 2 max each application) (0xFCB0)
        OCB = Overlay Control Block Table
        adam_pcb = Processor Control Block Table, 4bytes (adam_pcb+3 = Number of valid DCBs) (0xFEC0 relocatable), current adam_pcb=[0xFD70]
                adam_pcb+0 = Status, 0=Request Status of Z80 -> must return 0x81..0x82 to sync Master 6801 clk with Z80 clk
                adam_pcb+1,adam_pcb+2 = address of adam_pcb start
                adam_pcb+3 = device #

                - Writing to byte0:
                    1   Synchronize the Z80 clock (should return 0x81)
                    2   Synchronize the Master 6801 clock (should return 0x82)
                    3   Relocate adam_pcb

                - Status values:
                    0x80 -> Success
                    0x81 -> Z80 clock in sync
                    0x82 -> Master 6801 clock in sync
                    0x83 -> adam_pcb relocated
                    0x9B -> Time Out

        DEV_ID = Device id


        The ColecoAdam I/O map is contolled by the MIOC (Memory Input Output Controller):

                20-3F (W) = Adamnet Writes
                20-3F (R) = Adamnet Reads

                42-42 (W) = Expansion RAM page selection, only useful if expansion greater than 64k

                40-40 (W) = Printer Data Out
                40-40 (R) = Printer (Returns 0x41)

                5E-5E (RW)= Modem Data I/O
                5F-5F (RW)= Modem Data Control Status

                60-7F (W) = Set Memory configuration
                60-7F (R) = Read Memory configuration

                80-9F (W) = Set both controllers to keypad mode
                80-9F (R) = Not Connected

                A0-BF (W) = Video Chip (TMS9928A), A0=0 -> Write Register 0 , A0=1 -> Write Register 1
                A0-BF (R) = Video Chip (TMS9928A), A0=0 -> Read Register 0 , A0=1 -> Read Register 1

                C0-DF (W) = Set both controllers to joystick mode
                C0-DF (R) = Not Connected

                E0-FF (W) = Sound Chip (SN76489A)
                E0-FF (R) = Read Controller data, A1=0 -> read controller 1, A1=1 -> read controller 2

    http://drushel.cwru.edu/atm/atm.html
    http://rich.dirocco.org/Coleco/adam/ADAM.htm
    http://users.stargate.net/~drushel/pub/coleco/twwmca/index.html

    - fc75 GET_STATUS
    - fbe7 MMR_MAC
    - febe MMR_TR_REC
    - ff0f MMR_TR_TCU

****************************************************************************/

/*

    TODO:

    - sound (PSG RDY -> Z80 WAIT)
    - floppy ROM dump
    - printer
    - SPI

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6800/m6800.h"
#include "formats/basicdsk.h"
#include "formats/adam_cas.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "machine/coleco.h"
#include "machine/ram.h"
#include "machine/wd17xx.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "includes/adam.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	LO_SMARTWRITER = 0,
	LO_INTERNAL_RAM,
	LO_RAM_EXPANSION,
	LO_OS7_ROM_INTERNAL_RAM
};


enum
{
	HI_INTERNAL_RAM = 0,
	HI_ROM_EXPANSION,
	HI_RAM_EXPANSION,
	HI_CARTRIDGE_ROM
};


enum
{
	ADAMNET_MASTER = 0,
	ADAMNET_KEYBOARD,
	ADAMNET_DDP,
	ADAMNET_PRINTER,
	ADAMNET_FDC
};



//**************************************************************************
//  MEMORY BANKING
//**************************************************************************

//-------------------------------------------------
//  bankswitch -
//-------------------------------------------------

void adam_state::bankswitch()
{
	address_space *program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	switch (m_mioc & 0x03)
	{
	case LO_SMARTWRITER:
		if (BIT(m_adamnet, 1))
		{
			program->unmap_readwrite(0x0000, 0x5fff);
			program->install_rom(0x6000, 0x7fff, memregion("wp")->base() + 0x8000);
		}
		else
		{
			program->install_rom(0x0000, 0x7fff, memregion("wp")->base());
		}
		break;

	case LO_INTERNAL_RAM:
		program->install_ram(0x0000, 0x7fff, ram);
		break;

	case LO_RAM_EXPANSION:
		if (m_ram->size() > 64 * 1024)
			program->install_ram(0x0000, 0x7fff, ram + 0x10000);
		else
			program->unmap_readwrite(0x0000, 0x7fff);
		break;

	case LO_OS7_ROM_INTERNAL_RAM:
		program->install_rom(0x0000, 0x1fff, memregion("os7")->base());
		program->install_ram(0x2000, 0x7fff, ram + 0x2000);
		break;
	}

	switch ((m_mioc >> 2) & 0x03)
	{
	case HI_INTERNAL_RAM:
		program->install_ram(0x8000, 0xffff, ram + 0x8000);
		break;

	case HI_ROM_EXPANSION:
		program->install_rom(0x8000, 0xffff, memregion("xrom")->base());
		break;

	case HI_RAM_EXPANSION:
		if (m_game)
		{
			program->install_rom(0x8000, 0xffff, memregion("cart")->base());
		}
		else
		{
			if (m_ram->size() > 64 * 1024)
				program->install_ram(0x8000, 0xffff, ram + 0x18000);
			else
				program->unmap_readwrite(0x8000, 0xffff);
		}
		break;

	case HI_CARTRIDGE_ROM:
		program->install_rom(0x8000, 0xffff, memregion("cart")->base());
		break;
	}
}


//-------------------------------------------------
//  mioc_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::mioc_r )
{
	return m_mioc & 0x0f;
}


//-------------------------------------------------
//  mioc_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::mioc_w )
{
	/*

        bit     description

        0       Lower memory option 0
        1       Lower memory option 1
        2       Upper memory option 0
        3       Upper memory option 1
        4
        5
        6
        7

    */

	m_mioc = data;

	bankswitch();
}



//**************************************************************************
//  ADAMNET
//**************************************************************************

//-------------------------------------------------
//  adamnet_txd_w -
//-------------------------------------------------

void adam_state::adamnet_txd_w(int device, int state)
{
	m_txd[device] = state;

	m_rxd = 1;

	for (int i = 0; i < 4; i++)
	{
		if (!m_txd[i]) m_rxd = 0;
	}
}


//-------------------------------------------------
//  adamnet_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::adamnet_r )
{
	return m_adamnet & 0x0f;
}


//-------------------------------------------------
//  adamnet_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::adamnet_w )
{
	/*

        bit     description

        0       Network reset
        1       EOS enable
        2
        3
        4
        5
        6
        7

    */

	// network reset
	if (BIT(m_adamnet, 0) && !BIT(data, 0))
	{
		m_reset = 1;

		machine().device(M6801_KB_TAG)->reset();
		machine().device(M6801_DDP_TAG)->reset();
		machine().device(M6801_PRN_TAG)->reset();
		machine().device(M6801_FDC_TAG)->reset();
		wd17xx_mr_w(m_fdc, 0);
		wd17xx_mr_w(m_fdc, 1);
		//machine().device(M6801_SPI_TAG)->reset();

		m_reset = 0;
	}

	m_adamnet = data;

	bankswitch();
}


//-------------------------------------------------
//  kb6801_p1_r -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::master6801_p1_w )
{
	/*

        bit     description

        0       BA8
        1       BA9
        2       BA10
        3       BA11
        4       BA12
        5       BA13
        6       BA14
        7       BA15

    */

	m_ba = (data << 8) | (m_ba & 0xff);
}


//-------------------------------------------------
//  master6801_p2_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::master6801_p2_r )
{
	/*

        bit     description

        0       M6801 mode bit 0
        1       M6801 mode bit 1
        2       M6801 mode bit 2
        3       NET RXD
        4

    */

	UINT8 data = M6801_MODE_7;

	// NET RXD
	data |= m_rxd << 3;

	return data;
}


//-------------------------------------------------
//  master6801_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::master6801_p2_w )
{
	/*

        bit     description

        0       _DMA
        1
        2       _BWR
        3
        4       NET TXD

    */

	// DMA
	m_dma = BIT(data, 0);

	// write
	m_bwr = BIT(data, 2);

	// NET TXD
	adamnet_txd_w(ADAMNET_MASTER, BIT(data, 4));
}


//-------------------------------------------------
//  master6801_p3_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::master6801_p3_r )
{
	/*

        bit     description

        0       BD0
        1       BD1
        2       BD2
        3       BD3
        4       BD4
        5       BD5
        6       BD6
        7       BD7

    */

	return m_data_out;
}


//-------------------------------------------------
//  master6801_p3_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::master6801_p3_w )
{
	/*

        bit     description

        0       BD0
        1       BD1
        2       BD2
        3       BD3
        4       BD4
        5       BD5
        6       BD6
        7       BD7

    */

	m_data_in = data;
}


//-------------------------------------------------
//  master6801_p4_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::master6801_p4_w )
{
	/*

        bit     description

        0       BA0
        1       BA1
        2       BA2
        3       BA3
        4       BA4
        5       BA5
        6       BA6
        7       BA7

    */

	m_ba = (m_ba & 0xff00) | data;
}


//-------------------------------------------------
//  kb6801_p1_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::kb6801_p1_r )
{
	/*

        bit     description

        0       X0
        1       X1
        2       X2
        3       X3
        4       X4
        5       X5
        6       X6
        7       X7

    */

	UINT8 data = 0xff;

	if (!BIT(m_key_y, 0)) data &= ioport("Y0")->read();
	if (!BIT(m_key_y, 1)) data &= ioport("Y1")->read();
	if (!BIT(m_key_y, 2)) data &= ioport("Y2")->read();
	if (!BIT(m_key_y, 3)) data &= ioport("Y3")->read();
	if (!BIT(m_key_y, 4)) data &= ioport("Y4")->read();
	if (!BIT(m_key_y, 5)) data &= ioport("Y5")->read();
	if (!BIT(m_key_y, 6)) data &= ioport("Y6")->read();
	if (!BIT(m_key_y, 7)) data &= ioport("Y7")->read();
	if (!BIT(m_key_y, 8)) data &= ioport("Y8")->read();
	if (!BIT(m_key_y, 9)) data &= ioport("Y9")->read();
	if (!BIT(m_key_y, 10)) data &= ioport("Y10")->read();
	if (!BIT(m_key_y, 11)) data &= ioport("Y11")->read();
	if (!BIT(m_key_y, 12)) data &= ioport("Y12")->read();

	return data;
}


//-------------------------------------------------
//  kb6801_p2_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::kb6801_p2_r )
{
	/*

        bit     description

        0       mode bit 0
        1       mode bit 1
        2       mode bit 2
        3       NET RXD
        4       NET TXD

    */

	UINT8 data = M6801_MODE_7;

	// NET RXD
	data |= m_rxd << 3;

	return data;
}


//-------------------------------------------------
//  kb6801_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::kb6801_p2_w )
{
	/*

        bit     description

        0       mode bit 0
        1       mode bit 1
        2       mode bit 2
        3       NET RXD
        4       NET TXD

    */

	adamnet_txd_w(ADAMNET_KEYBOARD, BIT(data, 4));
}


//-------------------------------------------------
//  kb6801_p3_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::kb6801_p3_r )
{
	return 0xff;
}


//-------------------------------------------------
//  kb6801_p3_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::kb6801_p3_w )
{
	/*

        bit     description

        0       Y0
        1       Y1
        2       Y2
        3       Y3
        4       Y4
        5       Y5
        6       Y6
        7       Y7

    */

	m_key_y = (m_key_y & 0x1f00) | data;
}


//-------------------------------------------------
//  kb6801_p4_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::kb6801_p4_r )
{
	return 0xff;
}


//-------------------------------------------------
//  kb6801_p4_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::kb6801_p4_w )
{
	/*

        bit     description

        0       Y8
        1       Y9
        2       Y10
        3       Y11
        4       Y12
        5
        6
        7

    */

	m_key_y = ((data & 0x1f) << 8) | (m_key_y & 0xff);
}


//-------------------------------------------------
//  ddp6801_p1_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::ddp6801_p1_w )
{
	/*

        bit     description

        0       SPD SEL (0=20 ips, 1=80ips)
        1       STOP0
        2       STOP1
        3       _GO FWD
        4       _GO REV
        5       BRAKE
        6       _WR0
        7       _WR1

    */

	if(m_ddp0->exists())
	{
		m_ddp0->set_speed(BIT(data, 0) ? (double) 80/1.875 : 20/1.875); // speed select
		if(!(data & 0x08)) m_ddp0->go_forward();
		if(!(data & 0x10)) m_ddp0->go_reverse();
		m_ddp0->change_state(BIT(data, 1) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR); // motor control
	}

	if(m_ddp1->exists())
	{
		m_ddp1->set_speed(BIT(data, 0) ? (double) 80/1.875 : 20/1.875); // speed select
		if(!(data & 0x08)) m_ddp1->go_forward();
		if(!(data & 0x10)) m_ddp1->go_reverse();
		m_ddp1->change_state(BIT(data, 2) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR); // motor control
	}

	// data write 0
	m_wr0 = BIT(data, 6);

	// data write 1
	m_wr1 = BIT(data, 7);
}


//-------------------------------------------------
//  ddp6801_p2_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::ddp6801_p2_r )
{
	/*

        bit     description

        0       mode bit 0
        1       mode bit 1 / CIP1
        2       mode bit 2
        3       NET RXD
        4

    */

	UINT8 data = 0;

	if (m_reset)
		data |= M6801_MODE_6;
	else
		data |= m_ddp1->exists() << 1; // Cassette in place 1

	// NET RXD
	data |= m_rxd << 3;

	return data;
}


//-------------------------------------------------
//  ddp6801_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::ddp6801_p2_w )
{
	/*

        bit     description

        0       WRT DATA
        1
        2       TRACK A/B (0=B, 1=A)
        3
        4       NET TXD

    */

	if(m_ddp0->exists())
	{
		m_ddp0->set_channel(!BIT(data, 2)); // Track select
		if (!m_wr0) m_ddp0->output(BIT(data, 0) ? 1.0 : -1.0); // write data
	}

	if(m_ddp1->exists())
	{
		m_ddp1->set_channel(!BIT(data, 2));
		if (!m_wr1) m_ddp1->output(BIT(data, 0) ? 1.0 : -1.0);
	}

	// NET TXD
	adamnet_txd_w(ADAMNET_DDP, BIT(data, 4));
}

//-------------------------------------------------
//  ddp6801_p4_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::ddp6801_p4_r )
{
	/*

        bit     description

        0       A8
        1       A9
        2       A10 (2114 _S)
        3       MSENSE 0
        4       MSENSE 1
        5       CIP0
        6       RD DATA 0 (always 1)
        7       RD DATA 1 (data from drives ORed together)

    */

	UINT8 data = 0;

	// drive 0
	if(m_ddp0->exists())
	{
		data |= ((m_ddp0->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED) << 3; // motion sense
		data |= 1 << 5; // cassette in place
		data |= (m_ddp0->input() < 0) << 7; // read data
	}

	// drive 1
	if(m_ddp1->exists())
	{
		data |= ((m_ddp1->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED) << 4; // motion sense
		data |= (m_ddp1->input() < 0) << 7; // read data
	}

	// read data 0 (always 1)
	data |= 0x40;

	return data;
}


//-------------------------------------------------
//  printer6801_p1_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::printer6801_p1_w )
{
	/*

        bit     description

        0       M2 phase D
        1       M2 phase B
        2       M2 phase C
        3       M2 phase A
        4       M3 phase B
        5       M3 phase D
        6       M3 phase A
        7       M3 phase C

    */
}


//-------------------------------------------------
//  printer6801_p2_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::printer6801_p2_r )
{
	/*

        bit     description

        0       mode bit 0
        1       mode bit 1
        2       mode bit 2
        3       NET RXD
        4       NET TXD

    */

	UINT8 data = M6801_MODE_7;

	// NET RXD
	data |= m_rxd << 3;

	return data;
}


//-------------------------------------------------
//  printer6801_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::printer6801_p2_w )
{
	/*

        bit     description

        0       mode bit 0
        1       mode bit 1
        2       mode bit 2
        3       NET RXD
        4       NET TXD

    */

	adamnet_txd_w(ADAMNET_PRINTER, BIT(data, 4));
}


//-------------------------------------------------
//  printer6801_p3_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::printer6801_p3_r )
{
	return 0xff;
}


//-------------------------------------------------
//  printer6801_p4_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::printer6801_p4_r )
{
	/*

        bit     description

        0
        1
        2
        3
        4       left margin
        5       platen detent
        6       wheel home
        7       self-test

    */

	return 0x80;
}


//-------------------------------------------------
//  printer6801_p4_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::printer6801_p4_w )
{
	/*

        bit     description

        0       print hammer solenoid
        1       ribbon advance solenoid
        2       platen motor advance
        3       platen motor break
        4
        5
        6
        7

    */
}


//-------------------------------------------------
//  fdc6801_p1_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::fdc6801_p1_r )
{
	/*

        bit     description

        0       some kind of optic sensor
        1
        2       FDC DRQ
        3
        4
        5
        6
        7       SW3 (0=DS1, 1=DS2)

    */

	UINT8 data = 0;

	// floppy data request
	data |= wd17xx_drq_r(m_fdc) << 2;

	return data;
}


//-------------------------------------------------
//  fdc6801_p1_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::fdc6801_p1_w )
{
	/*

        bit     description

        0
        1       FDC ENP
        2
        3       FDC _DDEN
        4
        5       DRIVE SELECT
        6       MOTOR ON
        7

    */

	// density select
	wd17xx_dden_w(m_fdc, BIT(data, 3));

	// motor enable
	floppy_mon_w(m_floppy0, !BIT(data, 6));
	floppy_drive_set_ready_state(m_floppy0, 1, 1);
}


//-------------------------------------------------
//  fdc6801_p2_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::fdc6801_p2_r )
{
	/*

        bit     description

        0       mode bit 0
        1       mode bit 1
        2       mode bit 2
        3       NET RXD
        4

    */

	UINT8 data = M6801_MODE_2;

	// NET RXD
	data |= m_rxd << 3;

	return data;
}


//-------------------------------------------------
//  fdc6801_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::fdc6801_p2_w )
{
	/*

        bit     description

        0
        1
        2
        3
        4       NET TXD

    */

	adamnet_txd_w(ADAMNET_FDC, BIT(data, 4));
}


//-------------------------------------------------
//  fdc6801_p4_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::fdc6801_p4_w )
{
	/*

        bit     description

        0       A8
        1       A9
        2       A10
        3       A11
        4       _WE
        5       FDC CA0
        6       FDC CA1
        7       chip select logic?

    */
}



//**************************************************************************
//  PADDLES
//**************************************************************************

//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK( paddle_tick )
//-------------------------------------------------

static TIMER_DEVICE_CALLBACK( paddle_tick )
{
	adam_state *state = timer.machine().driver_data<adam_state>();

	// TODO: improve irq behaviour (see drivers/coleco.c)
	if (coleco_scan_paddles(timer.machine(), &state->m_joy_status0, &state->m_joy_status1))
		device_set_input_line(state->m_maincpu, INPUT_LINE_IRQ0, HOLD_LINE);
}


//-------------------------------------------------
//  paddle_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::paddle_w )
{
	m_joy_mode = 0;
}


//-------------------------------------------------
//  joystick_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::joystick_w )
{
	m_joy_mode = 1;
}


//-------------------------------------------------
//  input1_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::input1_r )
{
	return coleco_paddle_read(machine(), 0, m_joy_mode, m_joy_status0);
}


//-------------------------------------------------
//  input2_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::input2_r )
{
	return coleco_paddle_read(machine(), 1, m_joy_mode, m_joy_status1);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( adam_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( adam_mem, AS_PROGRAM, 8, adam_state )
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( adam_io )
//-------------------------------------------------

static ADDRESS_MAP_START( adam_io, AS_IO, 8, adam_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x1e, 0x1e) Optional Auto Dialer
	AM_RANGE(0x20, 0x20) AM_MIRROR(0x1f) AM_READWRITE(adamnet_r, adamnet_w)
//  AM_RANGE(0x40, 0x40) Printer Data
//  AM_RANGE(0x42, 0x42) Expansion RAM Page Select
//  AM_RANGE(0x52, 0x52) Adam Resident Debugger ?
//  AM_RANGE(0x54, 0x54) Adam Resident Debugger ?
//  AM_RANGE(0x55, 0x55) Adam Resident Debugger ?
//  AM_RANGE(0x5e, 0x5e) Optional Modem Data I/O
//  AM_RANGE(0x5f, 0x5f) Optional Modem Control Status
	AM_RANGE(0x60, 0x60) AM_MIRROR(0x1f) AM_READWRITE(mioc_r, mioc_w)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x1f) AM_WRITE(paddle_w)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x1e) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0xa1, 0xa1) AM_MIRROR(0x1e) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x1f) AM_WRITE(joystick_w)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x1f) AM_DEVWRITE(SN76489A_TAG, sn76489a_new_device, write)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x1d) AM_READ(input1_r)
	AM_RANGE(0xe2, 0xe2) AM_MIRROR(0x1d) AM_READ(input2_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( master6801_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( master6801_mem, AS_PROGRAM, 8, adam_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE_LEGACY(m6801_io_r, m6801_io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION(M6801_MAIN_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( master6801_io )
//-------------------------------------------------

static ADDRESS_MAP_START( master6801_io, AS_IO, 8, adam_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_WRITE(master6801_p1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(master6801_p2_r, master6801_p2_w)
	AM_RANGE(M6801_PORT3, M6801_PORT3) AM_READWRITE(master6801_p3_r, master6801_p3_w)
	AM_RANGE(M6801_PORT4, M6801_PORT4) AM_WRITE(master6801_p4_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( kb6801_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( kb6801_mem, AS_PROGRAM, 8, adam_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE_LEGACY(m6801_io_r, m6801_io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION(M6801_KB_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( kb6801_io )
//-------------------------------------------------

static ADDRESS_MAP_START( kb6801_io, AS_IO, 8, adam_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_READ(kb6801_p1_r)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(kb6801_p2_r, kb6801_p2_w)
	AM_RANGE(M6801_PORT3, M6801_PORT3) AM_READWRITE(kb6801_p3_r, kb6801_p3_w)
	AM_RANGE(M6801_PORT4, M6801_PORT4) AM_READWRITE(kb6801_p4_r, kb6801_p4_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( ddp6801_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( ddp6801_mem, AS_PROGRAM, 8, adam_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE_LEGACY(m6801_io_r, m6801_io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0x0400, 0x07ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION(M6801_DDP_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( ddp6801_io )
//-------------------------------------------------

static ADDRESS_MAP_START( ddp6801_io, AS_IO, 8, adam_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_WRITE(ddp6801_p1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(ddp6801_p2_r, ddp6801_p2_w)
	AM_RANGE(M6801_PORT3, M6801_PORT3) AM_NOP // Multiplexed Address/Data
	AM_RANGE(M6801_PORT4, M6801_PORT4) AM_READ(ddp6801_p4_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( printer6801_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( printer6801_mem, AS_PROGRAM, 8, adam_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE_LEGACY(m6801_io_r, m6801_io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION(M6801_PRN_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( printer6801_io )
//-------------------------------------------------

static ADDRESS_MAP_START( printer6801_io, AS_IO, 8, adam_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_WRITE(printer6801_p1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(printer6801_p2_r, printer6801_p2_w)
	AM_RANGE(M6801_PORT3, M6801_PORT3) AM_READ(printer6801_p3_r)
	AM_RANGE(M6801_PORT4, M6801_PORT4) AM_READWRITE(printer6801_p4_r, printer6801_p4_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( fdc6801_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( fdc6801_mem, AS_PROGRAM, 8, adam_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE_LEGACY(m6801_io_r, m6801_io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION(M6801_FDC_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( fdc6801_io )
//-------------------------------------------------

static ADDRESS_MAP_START( fdc6801_io, AS_IO, 8, adam_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_READWRITE(fdc6801_p1_r, fdc6801_p1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(fdc6801_p2_r, fdc6801_p2_w)
	AM_RANGE(M6801_PORT3, M6801_PORT3)
	AM_RANGE(M6801_PORT4, M6801_PORT4) AM_WRITE(fdc6801_p4_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( adam )
//-------------------------------------------------

static INPUT_PORTS_START( adam )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("WILD CARD") PORT_CODE(KEYCODE_F7)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("II") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("III") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("IV") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("VI") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNDO") PORT_CODE(KEYCODE_F8)

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('+') PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESCAPE/WP") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MOVE/COPY") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STORE/GET") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP))

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INSERT") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PRINT") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DELETE") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN))

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME") PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE( coleco )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  TMS9928a_interface tms9928a_interface
//-------------------------------------------------

static WRITE_LINE_DEVICE_HANDLER(adam_vdp_interrupt)
{
	adam_state *driver_state = device->machine().driver_data<adam_state>();

	if (state && !driver_state->m_vdp_nmi)
	{
		device->machine().device(Z80_TAG)->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}

	driver_state->m_vdp_nmi = state;
}

static TMS9928A_INTERFACE(adam_tms9928a_interface)
{
	"screen",
	0x4000,
	DEVCB_LINE(adam_vdp_interrupt)
};

//-------------------------------------------------
//  cassette_interface adam_cassette_interface
//-------------------------------------------------

static const struct CassetteOptions adam_cassette_options =
{
	2,		/* channels */
	16,		/* bits per sample */
	44100	/* sample frequency */
};

static const cassette_interface adam_cassette_interface =
{
	coleco_adam_cassette_formats,
	&adam_cassette_options,
	(cassette_state)(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};


//-------------------------------------------------
//  wd17xx_interface fdc_intf
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( adam )
	LEGACY_FLOPPY_OPTION( adam, "dsk", "Coleco Adam floppy disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([40])
		SECTORS([8])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface adam_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_SSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(adam),
	NULL,
	NULL
};

static const wd17xx_interface fdc_intf =
{
	DEVCB_NULL,
	DEVCB_CPU_INPUT_LINE(M6801_FDC_TAG, INPUT_LINE_NMI),
	DEVCB_NULL,
	{ FLOPPY_0, NULL, NULL, NULL }
};


//-------------------------------------------------
//  sn76496_config psg_intf
//-------------------------------------------------

static const sn76496_config psg_intf =
{
    DEVCB_NULL
};


//-------------------------------------------------
//  M6801_INTERFACE( master6801_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( adam_state::os3_w )
{
	if (state && !m_dma)
	{
		UINT8 *ram = m_ram->pointer();

		if (!m_bwr)
		{
			//logerror("Master 6801 write to %04x data %02x\n", m_ba, m_data_in);

			ram[m_ba] = m_data_in;
		}
		else
		{
			m_data_out = ram[m_ba];

			//logerror("Master 6801 read from %04x data %02x\n", m_ba, m_data_out);

			device_set_input_line(m_netcpu, M6801_SC1_LINE, ASSERT_LINE);
			device_set_input_line(m_netcpu, M6801_SC1_LINE, CLEAR_LINE);
		}
	}
}

static M6801_INTERFACE( master6801_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(adam_state, os3_w)
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( adam )
//-------------------------------------------------

void adam_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_mioc));
	save_item(NAME(m_game));
	save_item(NAME(m_adamnet));
	save_item(NAME(m_txd));
	save_item(NAME(m_rxd));
	save_item(NAME(m_reset));
	save_item(NAME(m_ba));
	save_item(NAME(m_dma));
	save_item(NAME(m_bwr));
	save_item(NAME(m_data_in));
	save_item(NAME(m_data_out));
	save_item(NAME(m_key_y));
	save_item(NAME(m_joy_mode));
	save_item(NAME(m_joy_status0));
	save_item(NAME(m_joy_status1));
	save_item(NAME(m_vdp_nmi));
	save_item(NAME(m_wr0));
	save_item(NAME(m_wr1));
	save_item(NAME(m_track));
}


//-------------------------------------------------
//  MACHINE_RESET( adam )
//-------------------------------------------------

void adam_state::machine_reset()
{
	device_image_interface *image = dynamic_cast<device_image_interface *>(machine().device("cart"));

    if (image->exists())
	{
		// game reset
		m_game = 1;
		m_mioc = (HI_CARTRIDGE_ROM << 2) | LO_OS7_ROM_INTERNAL_RAM;
	}
	else
	{
		// computer reset
		m_game = 0;
		m_mioc = 0;
	}

	m_adamnet = 0;

	bankswitch();
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( adam )
//-------------------------------------------------

static MACHINE_CONFIG_START( adam, adam_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_7_15909MHz/2)
	MCFG_CPU_PROGRAM_MAP(adam_mem)
	MCFG_CPU_IO_MAP(adam_io)

	MCFG_CPU_ADD(M6801_MAIN_TAG, M6801, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(master6801_mem)
	MCFG_CPU_IO_MAP(master6801_io)
	MCFG_CPU_CONFIG(master6801_intf)

	MCFG_CPU_ADD(M6801_KB_TAG, M6801, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(kb6801_mem)
	MCFG_CPU_IO_MAP(kb6801_io)

	MCFG_CPU_ADD(M6801_DDP_TAG, M6801, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(ddp6801_mem)
	MCFG_CPU_IO_MAP(ddp6801_io)

	MCFG_CPU_ADD(M6801_PRN_TAG, M6801, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(printer6801_mem)
	MCFG_CPU_IO_MAP(printer6801_io)
	MCFG_DEVICE_DISABLE()

	MCFG_CPU_ADD(M6801_FDC_TAG, M6801, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(fdc6801_mem)
	MCFG_CPU_IO_MAP(fdc6801_io)
	MCFG_DEVICE_DISABLE()

	MCFG_CPU_ADD(M6801_SPI_TAG, M6801, XTAL_4MHz)
	MCFG_DEVICE_DISABLE()

	// video hardware
	MCFG_TMS9928A_ADD( "tms9928a", TMS9928A, adam_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489A_TAG, SN76489A_NEW, XTAL_7_15909MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
    MCFG_SOUND_CONFIG(psg_intf)

	// devices
	MCFG_TIMER_ADD_PERIODIC("paddles", paddle_tick, attotime::from_msec(20))
	MCFG_WD2793_ADD(WD2793_TAG, fdc_intf)
	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, adam_floppy_interface)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, adam_cassette_interface)
	MCFG_CASSETTE_ADD(CASSETTE2_TAG, adam_cassette_interface)

	// cartridge
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,col,bin")
	MCFG_CARTSLOT_NOT_MANDATORY

	// ROM expansion
	MCFG_CARTSLOT_ADD("xrom")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_NOT_MANDATORY

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("128K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( adam )
//-------------------------------------------------

ROM_START( adam )
	ROM_REGION( 0x2000, "os7", 0)
	ROM_LOAD( "os7.u2", 0x0000, 0x2000, CRC(3aa93ef3) SHA1(45bedc4cbdeac66c7df59e9e599195c778d86a92) )

	ROM_REGION( 0xa000, "wp", 0)
	ROM_LOAD( "alf #1 rev 57 e3d5.u8",  0x0000, 0x2000, CRC(565b364a) SHA1(ebdafad6e268e7ed1674c1fb89607622748a5b36) )
	ROM_LOAD( "alf #2 rev 57 ae6a.u20", 0x2000, 0x2000, CRC(44a1cff4) SHA1(661cdf36d9699d6c21c5f9e205ebc41c707359dd) )
	ROM_LOAD( "alf #3 rev 57 8534.u21", 0x4000, 0x2000, CRC(77657b90) SHA1(d25d32ab6c8fafbc21b4b925b3e644fa26d111f7) )
	ROM_LOAD( "eos 6 rev 57 08dd.u22",  0x8000, 0x2000, CRC(ef6403c5) SHA1(28c7616cd02e4286f9b4c1c4a8b8850832b49fcb) )
	ROM_CONTINUE(                       0x6000, 0x2000 )
	ROM_LOAD( "wp_r80.rom", 			0x0000, 0x8000, BAD_DUMP CRC(58d86a2a) SHA1(d4aec4efe1431e56fe52d83baf9118542c525255) ) // should be separate 8/16K ROMs

	ROM_REGION( 0x8000, "xrom", ROMREGION_ERASE00 )
	ROM_CART_LOAD( "xrom", 0x0000, 0x8000, ROM_NOMIRROR | ROM_OPTIONAL )

	ROM_REGION( 0x8000, "cart", 0 )
	ROM_CART_LOAD( "cart", 0x0000, 0x8000, ROM_NOMIRROR | ROM_OPTIONAL )

	ROM_REGION( 0x800, M6801_MAIN_TAG, 0 )
	ROM_LOAD( "master rev a 174b.u6", 0x000, 0x800, CRC(035a7a3d) SHA1(0426e6eaf18c2be9fe08066570c214ab5951ee14) )

	ROM_REGION( 0x800, M6801_KB_TAG, 0 )
	ROM_LOAD( "keyboard.u2", 0x000, 0x800, CRC(ef204746) SHA1(83162ffc75847328a05429135b728a63efb05b93) )

	ROM_REGION( 0x800, M6801_DDP_TAG, 0 )
	ROM_LOAD( "tape rev a 8865.u24", 0x000, 0x800, CRC(6b9ea1cf) SHA1(b970f11e8f443fa130fba02ad1f60da51bf89673) )

	ROM_REGION( 0x800, M6801_PRN_TAG, 0 )
	ROM_LOAD( "printer.u2", 0x000, 0x800, CRC(e8db783b) SHA1(32b40679749ad0317c2c9ee9ca619fad6d850ce7) )

	ROM_REGION( 0x800, M6801_FDC_TAG, 0 )
	ROM_LOAD( "floppy disk drive", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, M6801_SPI_TAG, 0 )
	ROM_LOAD( "spi.bin", 0x000, 0x800, CRC(4ba30352) SHA1(99fe5aebd505a208bea6beec5d7322b15426e9c1) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY         FULLNAME            FLAGS */
COMP( 1982, adam,		0,			coleco,	adam,		adam, driver_device,		0,		"Coleco",		"Adam",				GAME_NOT_WORKING )
