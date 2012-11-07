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
*/

/*
                       Detailed Coleco ADAM Computer I/O Address Map

Port #    Device                        Input                    Output
__________________________________________________________________________________________

00        Powermate SASI Hard Drive     Input Data               Output Data
01        Powermate SASI Hard Drive     Status Register          Command Register
01        MIB2 RESET line               * Not Used on MIB2 *     Bit 3 = 1 for MIB2 RESET
01        Powermate IDE Hard Drive      Error Register           * Not Used on IDE HD *
02        Powermate IDE Hard Drive      Sector Count Register    Sector Count Register
03        Powermate IDE Hard Drive      Sector Number Register   Sector Number Register
04        Powermate IDE Hard Drive      Cylinder Low Register    Cylinder Low Register
05        Powermate IDE Hard Drive      Cylinder High Register   Cylinder High Register
06        Powermate IDE Hard Drive      SDH Register             SDH Register
07        Powermate IDE Hard Drive      Status Register          Command Register
08        Bonafide Sys MIDI Interface
09        Bonafide Sys MIDI Interface
0A        Bonafide Sys MIDI Interface
0B        Bonafide Sys MIDI Interface
0C        Bonafide Sys MIDI Interface
0D        Bonafide Sys MIDI Interface
0E        Bonafide Sys MIDI Interface
0F        Bonafide Sys MIDI Interface
10        Powermate Serial ports        Mode Register A          Mode Register A
11        Powermate Serial ports        Status Register A        Clock Select Reg A
12        Powermate Serial ports        * DO NOT USE *           Command Register A
13        Powermate Serial ports        RX Holding Register A    TX Holding Reg A
14        Powermate Serial ports        Input Port Change Reg    Aux Control Register
15        Powermate Serial ports        Interrupt Status Reg     Interrupt Mask Reg
16        Powermate Serial ports        Read Counter Upper       Set C/T Upper Register
17        Powermate Serial ports        Read Counter Lower       Set C/T Lower Register
18        Powermate Serial ports        Mode Register B          Mode Register B
19        Powermate Serial ports        Status Register B        Clock Select Reg B
1A        Powermate Serial ports        * DO NOT USE *           Command Register B
1B        Powermate Serial ports        RX Holding Register B    TX Holding Register B
1C        Powermate Serial ports        * Reserved (note 5) *    MIB3 Serial Port RESET
1D        Powermate Serial ports        Read Input Port Bits     Output Port Config Reg
1E        Coleco AutoDialer             ??                       ??
1E        Powermate Serial ports        Start Counter Cmd Port   Set Output Port Bits
1F        Powermate Serial ports        Stop Counter Cmd Port    Reset Output Port Bits
20-3F     AdamNet Reset                 Input MAY be available   Output is NOT available
40        Parallel Printer interface    Printer status           Output Data
41        May be unused (see note 1)    Input may NOT be avail   Output MAY be available
42        Expansion Memory              * Not Used *             Bank Number
43        May be unused (see note 1)    Input may NOT be avail   Output MAY be available
44-47     Eve/Orphanware Serial Port
48-4B     Eve Speech Synth/Clock Card
4C-4F     Orphanware Serial Port 2      (Standard Eve 80 column terminal ports)
4F        Coleco Steering controller    (Listed in Hackers guide as Expansion conn #2)
50-53     Super Game Module
54-57     Orphanware Serial Port 3      (Standard Orphanware 80 column terminal ports)
58        Powermate IDE Hard Disk       Input Data Lower 8 bits  Output Data Lower 8 bits
59        Powermate IDE Hard Disk       Input Data Upper 8 bits  Output Data Upper 8 bits
5A        Powermate IDE Hard Disk       Alternate Status Reg     Fixed Disk Control Reg
5B        Powermate IDE Hard Disk       Digital Input Register   ** Not Used by IDE HD **
5C-5F     Orphanware Serial Port 4
5E        Adamlink Modem                Input Data               Output Data
5F        Adamlink Modem                Status                   Control
60-7F     Memory Bank Switch Port       Input MAY be available   Output is NOT available
80-8F     *** Unused ***                (see note 2)             STA (?)
90-9F     Orphanware Hard Drive                                  STA (?)
A0-BF     Video Display Processor
C0        Strobe Reset                                           STB (?)
C1-DF     *** Unused ***                (see note 2)             STB (?)
EO-FF     Sound Chip (Out only)
FC        Joystick #1 (In only)
FE        Joystick #2 (In only)


Notes:

1)   Port 41 or port 43 is used by the Eve 80 column unit as a keyboard input port.
2)   Not useable from expansion card slots (can't read or write data to or from ports) -
     may be available on side port.
3)   Powermate IDE hard disk drive will not interfere with Powermate serial ports.
4)   Powermate serial ports will probably interfere with autodialer.
5)   Reserved ports in Powermate serial port map:  Input ports 12 and 1A - screw up serial
     ports if used; Input port 1C doesn't bother anything but the 2681 drives the bus;
6)   Orphanware serial port number 4 probably interferes with the ADAMlink modem.
     7)   According to my analysis of circuit U6 in the ADAM computer, all of upper I/O address
     space is decoded (by an LS138).  However, not all outputs appear to be used.  The
     circuit description follows.  Please correct any misassumptions I've made.  Note that
     if my analysis is correct, then the Orphanware hard disk should be interfering with
     the signal STA\ (which is associated with the joysticks in some way).



                      U6
                    74LS138             A6   A5   WR\
               |--------------|
   WR\    -----|A           Y0|o----    0    0    0    80-9F Write    (STA\)
               |              |
    A5    -----|B           Y1|o----    0    0    1    80-9F Read     (Not Used)
               |              |
    A6    -----|C           Y2|o----    0    1    0    A0-BF Write    (VDP CSW\)
               |              |
    A7    -----|G1          Y3|o----    0    1    1    A0-BF Read     (VDP CSR\)
               |              |
IORQ\    ----o|G2A         Y4|o----    1    0    0    C0-DF Write    (STB\)
               |              |
WAIT\    ----o|G2B         Y5|o----    1    0    1    C0-DF Read     (Not Used)
               |              |
               |            Y6|o----    1    1    0    E0-FF Write    (Sound CE\)
               |              |
               |            Y7|o----    1    1    1    E0-FF Read     (Joystick Enables)
               |--------------|

Conventions:

1)   The "o" symbol next to an input or an output implies that the pin requires an active
     low signal.
2)   The "\" symbol following a signal mnemonic indicates that the signal is active low.


Rev. 3
8/30/92
Mark Gordon
*/

/*

    TODO:

    - floppy
    - slot interface
    - printer
    - SPI
    - sound (PSG RDY -> Z80 WAIT)

    http://drushel.cwru.edu/atm/atm.html
    http://rich.dirocco.org/Coleco/adam/ADAM.htm
    http://users.stargate.net/~drushel/pub/coleco/twwmca/index.html

*/

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
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	switch (m_mioc & 0x03)
	{
	case LO_SMARTWRITER:
		if (BIT(m_an, 1))
		{
			program.unmap_readwrite(0x0000, 0x5fff);
			program.install_rom(0x6000, 0x7fff, memregion("wp")->base() + 0x8000);
		}
		else
		{
			program.install_rom(0x0000, 0x7fff, memregion("wp")->base());
		}
		break;

	case LO_INTERNAL_RAM:
		program.install_ram(0x0000, 0x7fff, ram);
		break;

	case LO_RAM_EXPANSION:
		if (m_ram->size() > 64 * 1024)
			program.install_ram(0x0000, 0x7fff, ram + 0x10000);
		else
			program.unmap_readwrite(0x0000, 0x7fff);
		break;

	case LO_OS7_ROM_INTERNAL_RAM:
		program.install_rom(0x0000, 0x1fff, memregion("os7")->base());
		program.install_ram(0x2000, 0x7fff, ram + 0x2000);
		break;
	}

	switch ((m_mioc >> 2) & 0x03)
	{
	case HI_INTERNAL_RAM:
		program.install_ram(0x8000, 0xffff, ram + 0x8000);
		break;

	case HI_ROM_EXPANSION:
		program.install_rom(0x8000, 0xffff, memregion("xrom")->base());
		break;

	case HI_RAM_EXPANSION:
		if (m_game)
		{
			program.install_rom(0x8000, 0xffff, memregion("cart")->base());
		}
		else
		{
			if (m_ram->size() > 64 * 1024)
				program.install_ram(0x8000, 0xffff, ram + 0x18000);
			else
				program.unmap_readwrite(0x8000, 0xffff);
		}
		break;

	case HI_CARTRIDGE_ROM:
		program.install_rom(0x8000, 0xffff, memregion("cart")->base());
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
//  adamnet_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::adamnet_r )
{
	return m_an & 0x0f;
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

	if (BIT(m_an, 0) && !BIT(data, 0))
	{
		// network reset
		m_adamnet->reset_w(ASSERT_LINE);
		m_adamnet->reset_w(CLEAR_LINE);
	}

	m_an = data;

	bankswitch();
}


//-------------------------------------------------
//  m6801_p1_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::m6801_p1_w )
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
//  m6801_p2_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::m6801_p2_r )
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
	data |= m_adamnet->rxd_r() << 3;

	return data;
}


//-------------------------------------------------
//  m6801_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::m6801_p2_w )
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
	m_adamnet->txd_w(BIT(data, 4));
}


//-------------------------------------------------
//  m6801_p3_r -
//-------------------------------------------------

READ8_MEMBER( adam_state::m6801_p3_r )
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
//  m6801_p3_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::m6801_p3_w )
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
//  m6801_p4_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_state::m6801_p4_w )
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



//**************************************************************************
//  PADDLES
//**************************************************************************

//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK_MEMBER( paddle_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER( adam_state::paddle_tick )
{
	// TODO: improve irq behaviour (see drivers/coleco.c)
	if (coleco_scan_paddles(machine(), &m_joy_status0, &m_joy_status1))
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
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
	AM_RANGE(0x20, 0x20) AM_MIRROR(0x1f) AM_READWRITE(adamnet_r, adamnet_w)
	AM_RANGE(0x60, 0x60) AM_MIRROR(0x1f) AM_READWRITE(mioc_r, mioc_w)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x1f) AM_WRITE(paddle_w)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x1e) AM_DEVREADWRITE(TMS9928A_TAG, tms9928a_device, vram_read, vram_write)
	AM_RANGE(0xa1, 0xa1) AM_MIRROR(0x1e) AM_DEVREADWRITE(TMS9928A_TAG, tms9928a_device, register_read, register_write)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x1f) AM_WRITE(joystick_w)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x1f) AM_DEVWRITE(SN76489A_TAG, sn76489a_device, write)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x1d) AM_READ(input1_r)
	AM_RANGE(0xe2, 0xe2) AM_MIRROR(0x1d) AM_READ(input2_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m6801_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( m6801_mem, AS_PROGRAM, 8, adam_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE_LEGACY(m6801_io_r, m6801_io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION(M6801_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m6801_io )
//-------------------------------------------------

static ADDRESS_MAP_START( m6801_io, AS_IO, 8, adam_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_WRITE(m6801_p1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(m6801_p2_r, m6801_p2_w)
	AM_RANGE(M6801_PORT3, M6801_PORT3) AM_READWRITE(m6801_p3_r, m6801_p3_w)
	AM_RANGE(M6801_PORT4, M6801_PORT4) AM_WRITE(m6801_p4_w)
ADDRESS_MAP_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  TMS9928A_INTERFACE( vdc_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( adam_state::vdc_int_w )
{
	if (state && !m_vdp_nmi)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}

	m_vdp_nmi = state;
}

static TMS9928A_INTERFACE( vdc_intf )
{
	SCREEN_TAG,
	0x4000,
	DEVCB_DRIVER_LINE_MEMBER(adam_state, vdc_int_w)
};


//-------------------------------------------------
//  sn76496_config psg_intf
//-------------------------------------------------

static const sn76496_config psg_intf =
{
	DEVCB_NULL // TODO Z80 RDY
};


//-------------------------------------------------
//  M6801_INTERFACE( m6801_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( adam_state::os3_w )
{
	if (state && !m_dma)
	{
		if (!m_bwr)
		{
			//logerror("Master 6801 write to %04x data %02x\n", m_ba, m_data_in);

			m_ram->pointer()[m_ba] = m_data_in;
		}
		else
		{
			m_data_out = m_ram->pointer()[m_ba];

			//logerror("Master 6801 read from %04x data %02x\n", m_ba, m_data_out);

			m_netcpu->set_input_line(M6801_SC1_LINE, ASSERT_LINE);
			m_netcpu->set_input_line(M6801_SC1_LINE, CLEAR_LINE);
		}
	}
}

static M6801_INTERFACE( m6801_intf )
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
	// state saving
	save_item(NAME(m_mioc));
	save_item(NAME(m_game));
	save_item(NAME(m_an));
	save_item(NAME(m_ba));
	save_item(NAME(m_dma));
	save_item(NAME(m_bwr));
	save_item(NAME(m_data_in));
	save_item(NAME(m_data_out));
	save_item(NAME(m_joy_mode));
	save_item(NAME(m_joy_status0));
	save_item(NAME(m_joy_status1));
	save_item(NAME(m_vdp_nmi));
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

	m_an = 0;

	bankswitch();

	m_maincpu->reset();
	m_netcpu->reset();
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

	MCFG_CPU_ADD(M6801_TAG, M6801, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(m6801_mem)
	MCFG_CPU_IO_MAP(m6801_io)
	MCFG_CPU_CONFIG(m6801_intf)
	MCFG_QUANTUM_PERFECT_CPU(M6801_TAG)

	// video hardware
	MCFG_TMS9928A_ADD(TMS9928A_TAG, TMS9928A, vdc_intf)
	MCFG_TMS9928A_SCREEN_ADD_NTSC(SCREEN_TAG)
	MCFG_SCREEN_UPDATE_DEVICE(TMS9928A_TAG, tms9928a_device, screen_update)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489A_TAG, SN76489A, XTAL_7_15909MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_CONFIG(psg_intf)

	// devices
	MCFG_ADAMNET_BUS_ADD()
	MCFG_ADAMNET_SLOT_ADD("an1", adamnet_devices, "kb", NULL)
	MCFG_ADAMNET_SLOT_ADD("an2", adamnet_devices, "prn", NULL)
	MCFG_ADAMNET_SLOT_ADD("an3", adamnet_devices, "ddp", NULL)
	MCFG_ADAMNET_SLOT_ADD("an4", adamnet_devices, "fdc", NULL)
	MCFG_ADAMNET_SLOT_ADD("an5", adamnet_devices, NULL, NULL)
	MCFG_ADAMNET_SLOT_ADD("an6", adamnet_devices, NULL, NULL)
	MCFG_ADAMNET_SLOT_ADD("an7", adamnet_devices, NULL, NULL)
	MCFG_ADAMNET_SLOT_ADD("an8", adamnet_devices, NULL, NULL)
	MCFG_ADAMNET_SLOT_ADD("an9", adamnet_devices, NULL, NULL)
	MCFG_ADAMNET_SLOT_ADD("an10", adamnet_devices, NULL, NULL)
	MCFG_ADAMNET_SLOT_ADD("an11", adamnet_devices, NULL, NULL)
	MCFG_ADAMNET_SLOT_ADD("an12", adamnet_devices, NULL, NULL)
	MCFG_ADAMNET_SLOT_ADD("an13", adamnet_devices, NULL, NULL)
	MCFG_ADAMNET_SLOT_ADD("an14", adamnet_devices, NULL, NULL)
	MCFG_ADAMNET_SLOT_ADD("an15", adamnet_devices, NULL, NULL)
	
	MCFG_TIMER_DRIVER_ADD_PERIODIC("paddles", adam_state, paddle_tick, attotime::from_msec(20))

	// cartridge
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,col,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("coleco_cart")

	// ROM expansion
	MCFG_CARTSLOT_ADD("xrom")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("adam_xrom")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("128K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("colec_cart_list", "coleco")
	MCFG_SOFTWARE_LIST_ADD("adam_cart_list", "adam_cart")
	//MCFG_SOFTWARE_LIST_ADD("xrom_list", "adam_xrom")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "adam_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "adam_flop")
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
	ROM_LOAD( "wp_r80.rom",             0x0000, 0x8000, BAD_DUMP CRC(58d86a2a) SHA1(d4aec4efe1431e56fe52d83baf9118542c525255) ) // should be separate 8/16K ROMs

	ROM_REGION( 0x800, M6801_TAG, 0 )
	ROM_LOAD( "master rev a 174b.u6", 0x000, 0x800, CRC(035a7a3d) SHA1(0426e6eaf18c2be9fe08066570c214ab5951ee14) )

	ROM_REGION( 0x8000, "xrom", ROMREGION_ERASE00 )
	ROM_CART_LOAD( "xrom", 0x0000, 0x8000, ROM_NOMIRROR | ROM_OPTIONAL )

	ROM_REGION( 0x8000, "cart", 0 )
	ROM_CART_LOAD( "cart", 0x0000, 0x8000, ROM_NOMIRROR | ROM_OPTIONAL )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT                    COMPANY         FULLNAME            FLAGS
COMP( 1982, adam,       0,          coleco, adam,       coleco,     driver_device,  0,      "Coleco",       "Adam",             GAME_SUPPORTS_SAVE )
