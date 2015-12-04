// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Myarc Hard and Floppy Disk Controller ("HFDC")

    The HFDC is based on the HDC9234 controller chip from Standard
    Microsystems Corporation (SMC). It can work with up to three MFM hard disk
    drives and up to four floppy disk drives.

    Data flow is detached from the main CPU. The HDC transfers data to/from
    the drives using direct memory access to attached memory circuits. That
    is, to write a sector on a drive the CPU must set up the contents in the
    memory, then initiate a sector write operation.

    The advantage is a much higher data rate (in particular important when
    working with hard disks) with less load for the main CPU. Also, we do not
    need a READY line control (as seen with the WD17xx-based controllers).
    Any kinds of asynchronous events are propagated via INTA* (configurable
    to INTB*).

    Most of the control logic is hidden in the custom Gate Array chip. We do
    not have details on its contents, but the specifications in the HFDC manual
    and in the schematics are sufficient to create a (functionally) proper
    emulation.

    The HDC9234 can also control tape drives. In early HFDC controller card
    layouts, a socket for connecting a drive is available. However, there
    never was a support from the DSR (firmware), so this feature was eliminated
    in later releases.

    DIP switches
    - Settings for step rate and track count for each floppy drive (DSK1-DSK4)
    - CRU base address. Note that only on all other addresses than 1100, the
      floppy drives are labeled DSK5-DSK8 by the card software.


    Components

    HDC 9234      - Universal Disk Controller
    FDC 9216      - Floppy disk data separator (8 MHz, divider is set by CD0 and CD1)
    HDC 92C26     - MFM hard disk data separator (10 MHz, also used for 9234)
    HDC 9223      - Analog data separator support
    DS 1000-50    - Delay circuit
    MM 58274BN    - Real time clock
    HM 6264-LP15  - SRAM 8K x 8   (may also be 32K x 8)
    27C128        - EPROM 16K x 8

    References:
    [1] Myarc Inc.: Hard and Floppy Disk Controller / Users Manual

    Michael Zapf
    July 2015

*****************************************************************************/

#include "emu.h"
#include "peribox.h"
#include "hfdc.h"
#include "formats/mfm_hd.h"
#include "formats/ti99_dsk.h"       // Format

#define BUFFER "ram"
#define FDC_TAG "hdc9234"
#define CLOCK_TAG "mm58274c"

#define MOTOR_TIMER 1

#define TAPE_ADDR   0x0fc0
#define HDC_R_ADDR  0x0fd0
#define HDC_W_ADDR  0x0fd2
#define CLK_ADDR    0x0fe0
#define RAM_ADDR    0x1000

#define TRACE_EMU 0
#define TRACE_CRU 0
#define TRACE_COMP 0
#define TRACE_RAM 0
#define TRACE_ROM 0
#define TRACE_LINES 0
#define TRACE_MOTOR 0
#define TRACE_DMA 0
#define TRACE_INT 0

// =========================================================================

/*
   Constructor for the HFDC card.
*/
myarc_hfdc_device::myarc_hfdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ti_expansion_card_device(mconfig, TI99_HFDC, "Myarc Hard and Floppy Disk Controller", tag, owner, clock, "ti99_hfdc", __FILE__), m_motor_on_timer(nullptr),
		m_hdc9234(*this, FDC_TAG),
		m_clock(*this, CLOCK_TAG), m_current_floppy(nullptr), m_current_harddisk(nullptr), m_see_switches(false), m_irq(), m_dip(), m_motor_running(false),
	m_inDsrArea(false), m_HDCsel(false), m_RTCsel(false), m_tapesel(false), m_RAMsel(false), m_ROMsel(false), m_address(0), m_dma_in_progress(false),
	m_wait_for_hd1(false), m_dsrrom(nullptr), m_rom_page(0), m_buffer_ram(nullptr), m_status_latch(0), m_dma_address(0), m_output1_latch(0), m_output2_latch(0), m_lastval(0), m_MOTOR_ON(), m_readyflags(0)
{
}

SETADDRESS_DBIN_MEMBER( myarc_hfdc_device::setaddress_dbin )
{
	// Do not allow setaddress for the debugger. It will mess up the
	// setaddress/memory access pairs when the CPU enters wait states.
	if (space.debugger_access()) return;

	// Selection login in the PAL and some circuits on the board

	// Is the card being selected?
	// Area = 4000-5fff
	// 010x xxxx xxxx xxxx
	m_address = offset;

	m_inDsrArea = ((m_address & m_select_mask)==m_select_value);

	if (!m_inDsrArea) return;

	// Is the HDC chip on the card being selected?
	// HDC9234: read: 4fd0,4 (mirror 8,c)
	// HDC9234: write: 4fd2,6 (mirror a,e)
	// read:  ...0 1111 1101 xx00
	// write: ...0 1111 1101 xx10

	m_HDCsel = m_inDsrArea &&
				((state==ASSERT_LINE && ((m_address & 0x1ff3)==HDC_R_ADDR))    // read
				|| (state==CLEAR_LINE && ((m_address & 0x1ff3)==HDC_W_ADDR)));  // write

	// Is the tape selected?
	// ...0 1111 1100 xxxx
	m_tapesel = m_inDsrArea && ((m_address & 0x1ff0)==TAPE_ADDR);

	// Is the RTC selected on the card? (even addr)
	// ...0 1111 111x xxx0
	m_RTCsel = m_inDsrArea && ((m_address & 0x1fe1)==CLK_ADDR);

	// Is RAM selected?
	// ...1 xxxx xxxx xxxx
	m_RAMsel = m_inDsrArea && ((m_address & 0x1000)==RAM_ADDR);

	// Is ROM selected?
	// not 0100 1111 11xx xxxx
	m_ROMsel = m_inDsrArea && !m_RAMsel && !((m_address & 0x0fc0)==0x0fc0);
}

/*
    Access for debugger. This is a stripped-down version of the
    main methods below. We only allow ROM and RAM access.
*/
void myarc_hfdc_device::debug_read(offs_t offset, UINT8* value)
{
	if (((offset & m_select_mask)==m_select_value) && m_selected)
	{
		if ((offset & 0x1000)==RAM_ADDR)
		{
			int bank = (offset & 0x0c00) >> 10;
			*value = m_buffer_ram[(m_ram_page[bank]<<10) | (offset & 0x03ff)];
		}
		else
		{
			if ((offset & 0x0fc0)!=0x0fc0)
			{
				*value = m_dsrrom[(m_rom_page << 12) | (offset & 0x0fff)];
			}
		}
	}
}

void myarc_hfdc_device::debug_write(offs_t offset, UINT8 data)
{
	if (((offset & m_select_mask)==m_select_value) && m_selected)
	{
		if ((offset & 0x1000)==RAM_ADDR)
		{
			int bank = (offset & 0x0c00) >> 10;
			m_buffer_ram[(m_ram_page[bank]<<10) | (m_address & 0x03ff)] = data;
		}
	}
}

/*
    Read a byte from the memory address space of the HFDC

    0x4000 - 0x4fbf one of four possible ROM pages
    0x4fc0 - 0x4fcf Tape control (only available in prototype HFDC models)
    0x4fd0 - 0x4fdf HDC 9234 ports
    0x4fe0 - 0x4fff RTC chip ports

    0x5000 - 0x53ff static RAM page 0x08
    0x5400 - 0x57ff static RAM page any of 32 pages
    0x5800 - 0x5bff static RAM page any of 32 pages
    0x5c00 - 0x5fff static RAM page any of 32 pages

    HFDC manual, p. 44
*/
READ8Z_MEMBER(myarc_hfdc_device::readz)
{
	if (space.debugger_access())
	{
		debug_read(offset, value);
		return;
	}

	if (m_inDsrArea && m_selected)
	{
		if (m_tapesel)
		{
			logerror("%s: Tape support not available on this HFDC version (access to address %04x)\n", tag(), m_address & 0xffff);
			return;
		}

		if (m_HDCsel)
		{
			*value = m_hdc9234->read(space, (m_address>>2)&1, 0xff);
			if (TRACE_COMP) logerror("%s: %04x[HDC] -> %02x\n", tag(), m_address & 0xffff, *value);
			return;
		}

		if (m_RTCsel)
		{
			*value = m_clock->read(space, (m_address & 0x001e) >> 1);
			if (TRACE_COMP) logerror("%s: %04x[CLK] -> %02x\n", tag(), m_address & 0xffff, *value);
			return;
		}

		if (m_RAMsel)
		{
			// 0101 00xx xxxx xxxx  static 0x08
			// 0101 01xx xxxx xxxx  bank 1
			// 0101 10xx xxxx xxxx  bank 2
			// 0101 11xx xxxx xxxx  bank 3
			int bank = (m_address & 0x0c00) >> 10;

			// If a DMA is in progress, do not respond
			if (m_dip == CLEAR_LINE) *value = m_buffer_ram[(m_ram_page[bank]<<10) | (m_address & 0x03ff)];
			if (TRACE_RAM)
			{
				if ((m_address & 1)==0)  // only show even addresses with words
				{
					int valword = (((*value) << 8) | m_buffer_ram[(m_ram_page[bank]<<10) | ((m_address+1) & 0x03ff)])&0xffff;
					logerror("%s: %04x[%02x] -> %04x\n", tag(), m_address & 0xffff, m_ram_page[bank], valword);
				}
			}
			return;
		}

		if (m_ROMsel)
		{
			*value = m_dsrrom[(m_rom_page << 12) | (m_address & 0x0fff)];
			if (TRACE_ROM)
			{
				if ((m_address & 1)==0)  // only show even addresses with words
				{
					int valword = (((*value) << 8) | m_dsrrom[(m_rom_page << 12) | ((m_address + 1) & 0x0fff)])&0xffff;
					logerror("%s: %04x[%02x] -> %04x\n", tag(), m_address & 0xffff, m_rom_page, valword);
				}
			}
			return;
		}
	}
}

/*
    Write a byte to the memory address space of the HFDC

    0x4fc0 - 0x4fcf Tape control (only available in prototype HFDC models)
    0x4fd0 - 0x4fdf HDC 9234 ports
    0x4fe0 - 0x4fff RTC chip ports

    0x5000 - 0x53ff static RAM page 0x08
    0x5400 - 0x57ff static RAM page any of 32 pages
    0x5800 - 0x5bff static RAM page any of 32 pages
    0x5c00 - 0x5fff static RAM page any of 32 pages
*/
WRITE8_MEMBER( myarc_hfdc_device::write )
{
	if (space.debugger_access())
	{
		debug_write(offset, data);
		return;
	}

	if (m_inDsrArea && m_selected)
	{
		if (m_tapesel)
		{
			logerror("%s: Tape support not available on this HFDC version (write access to address %04x: %02x)\n", tag(), m_address & 0xffff, data);
			return;
		}

		if (m_HDCsel)
		{
			if (TRACE_COMP) logerror("%s: %04x[HDC] <- %02x\n", tag(), m_address & 0xffff, data);
			m_hdc9234->write(space, (m_address>>2)&1, data, 0xff);
			return;
		}

		if (m_RTCsel)
		{
			if (TRACE_COMP) logerror("%s: %04x[CLK] <- %02x\n", tag(), m_address & 0xffff, data);
			m_clock->write(space, (m_address & 0x001e) >> 1, data);
			return;
		}

		if (m_RAMsel)
		{
			// 0101 00xx xxxx xxxx  static 0x08
			// 0101 01xx xxxx xxxx  bank 1
			// 0101 10xx xxxx xxxx  bank 2
			// 0101 11xx xxxx xxxx  bank 3
			int bank = (m_address & 0x0c00) >> 10;
			if (TRACE_RAM) logerror("%s: %04x[%02x] <- %02x\n", tag(), m_address & 0xffff, m_ram_page[bank], data);

			// When a DMA is in progress, do not change anything
			if (m_dip == CLEAR_LINE) m_buffer_ram[(m_ram_page[bank]<<10) | (m_address & 0x03ff)] = data;
			return;
		}
		// The rest is ROM
		if (m_ROMsel)
		{
			if (TRACE_ROM) logerror("%s: Ignoring write ROM %04x[%02x]: %02x\n", tag(), m_address & 0xffff, m_rom_page, data);
		}
	}
}

/*
    Read a set of 8 bits in the CRU space of the HFDC
    There are two banks, according to the state of m_see_switches

    m_see_switches == true:

       7     6     5     4     3     2     1     0      CRU bit
    +-----+-----+-----+-----+-----+-----+-----+-----+
    |DIP5*|DIP6*|DIP7*|DIP8*|DIP1*|DIP2*|DIP3*|DIP4*|
    +-----+-----+-----+-----+-----+-----+-----+-----+
    |   DSK3    |   DSK4    |   DSK1    |   DSK2    |
    +-----+-----+-----+-----+-----+-----+-----+-----+

    Settings for DSKn: (n=1..4)

    DIP(2n-1) DIP(2n)   Tracks     Step(ms)    Sectors (256 byte)
    off       off        40        16          18/16/9
    on        off        40        8           18/16/9
    off       on         80/40     2           18/16/9
    on        on         80        2           36

    Inverted logic: switch=on means a 0 bit, off is a 1 bit when read by the CRU

    Caution: The last setting is declared as "future expansion" and is
    locked to a 1.44 MiB capacity. No lower formats can be used.

    ---

    m_see_switches == false:

       7     6     5     4     3     2     1     0
    +-----+-----+-----+-----+-----+-----+-----+-----+
    |  0  |  0  |  0  |  0  | WAIT| MON*| DIP | IRQ |
    +-----+-----+-----+-----+-----+-----+-----+-----+

    WAIT = Wait for WDS1 to become ready
    MON* = Motor on
    DIP = DMA in progress
    IRQ = Interrupt request
    ---
    0 on all other locations
*/
READ8Z_MEMBER(myarc_hfdc_device::crureadz)
{
	UINT8 reply;
	if ((offset & 0xff00)==m_cru_base)
	{
		if ((offset & 0x00ff)==0)  // CRU bits 0-7
		{
			if (m_see_switches)
			{
				reply = ~(ioport("HFDCDIP")->read());
			}
			else
			{
				reply = 0;
				if (m_irq == ASSERT_LINE)  reply |= 0x01;
				if (m_dip == ASSERT_LINE)  reply |= 0x02;
				if (!m_motor_running) reply |= 0x04;
				if (m_wait_for_hd1) reply |= 0x08;
			}
			*value = reply;
		}
		else   // CRU bits 8+
		{
			*value = 0;
		}

		if (TRACE_CRU) logerror("%s: CRU %04x -> %02x\n", tag(), offset & 0xffff, *value);
	}
}

/*
    Set a bit in the CRU space of the HFDC

       7     6     5     4     3     2     1     0
    +-----+-----+-----+-----+-----+-----+-----+-----+
    |  -  |  -  |  -  | ROM1| ROM0| MON | RES*| SEL |
    |     |     |     | CSEL| CD1 | CD0 |     |     |
    +-----+-----+-----+-----+-----+-----+-----+-----+

       17    16    15    14    13    12    11    10    F     E     D     C     B     A     9     8
    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
    |    RAM page select @5C00    |    RAM page select @5800    |     RAM page select @5400   |  -  |
    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+

    SEL  = Select card (and map ROM into address space)
    RES* = Reset controller
    MON  = Motor on / same line goes to CD0 input of floppy separator
    ROM bank select: bank 0..3; bit 3 = MSB, 4 = LSB
    RAM bank select: bank 0..31; bit 9 = LSB (accordingly for other two areas)
    CD0 and CD1 are Clock Divider selections for the Floppy Data Separator (FDC9216)
    CSEL = CRU input select (m_see_switches)

    HFDC manual p. 43
*/
WRITE8_MEMBER(myarc_hfdc_device::cruwrite)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		if (TRACE_CRU) logerror("%s: CRU %04x <- %d\n", tag(), offset & 0xffff, data);

		int bit = (offset >> 1) & 0x1f;

		// Handle the page selects right here
		if (bit >= 0x09 && bit < 0x18)
		{
			if (data)
				// we leave index 0 unchanged; modify indices 1-3
				m_ram_page[(bit-4)/5] |= 1 << ((bit-9)%5);
			else
				m_ram_page[(bit-4)/5] &= ~(1 << ((bit-9)%5));

			if (TRACE_CRU)
			{
				if (bit==0x0d) logerror("%s: RAM page @5400 = %d\n", tag(), m_ram_page[1]);
				if (bit==0x12) logerror("%s: RAM page @5800 = %d\n", tag(), m_ram_page[2]);
				if (bit==0x17) logerror("%s: RAM page @5C00 = %d\n", tag(), m_ram_page[3]);
			}
			return;
		}

		switch (bit)
		{
		case 0:
			{
				bool turnOn = (data!=0);
				// Avoid too many meaningless log outputs
				if (TRACE_CRU) if (m_selected != turnOn) logerror("%s: card %s\n", tag(), turnOn? "selected" : "unselected");
				m_selected = turnOn;
				break;
			}
		case 1:
			if (TRACE_CRU) if (data==0) logerror("%s: trigger HDC reset\n", tag());
			m_hdc9234->reset((data == 0)? ASSERT_LINE : CLEAR_LINE);
			break;

		case 2:
			m_hdc9234->set_clock_divider(0, data);

			// Activate motor
			// When 1, let motor run continuously. When 0, a simple monoflop circuit keeps the line active for another 4 sec
			if (data==1)
			{
				m_motor_on_timer->reset();
				set_floppy_motors_running(true);
			}
			else
			{
				m_motor_on_timer->adjust(attotime::from_msec(4230));
			}
			m_lastval = data;
			break;

		case 3:
			m_hdc9234->set_clock_divider(1, data);
			m_rom_page = (data != 0)? (m_rom_page | 2) : (m_rom_page & 0xfd);
			if (TRACE_CRU) logerror("%s: ROM page = %d\n", tag(), m_rom_page);
			break;

		case 4:
			m_see_switches = (data != 0);
			m_rom_page = (data != 0)? (m_rom_page | 1) : (m_rom_page & 0xfe);
			if (TRACE_CRU) logerror("%s: ROM page = %d, see_switches = %d\n", tag(), m_rom_page, m_see_switches);
			break;

		default:
			logerror("%s: Attempt to set undefined CRU bit %d\n", tag(), bit);
		}
	}
}

/*
    Monoflop has gone back to the OFF state.
*/
void myarc_hfdc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	set_floppy_motors_running(false);
}

/*
    This is called back from the floppy when an index hole is passing by.
*/
void myarc_hfdc_device::floppy_index_callback(floppy_image_device *floppy, int state)
{
	if (TRACE_LINES) if (state==1) logerror("%s: Floppy index pulse\n", tag());
	// m_status_latch = (state==ASSERT_LINE)? (m_status_latch | HDC_DS_INDEX) :  (m_status_latch & ~HDC_DS_INDEX);
	set_bits(m_status_latch, HDC_DS_INDEX, (state==ASSERT_LINE));
	signal_drive_status();
}

/*
    This is called back from the hard disk when an index hole is passing by.
*/
void myarc_hfdc_device::harddisk_index_callback(mfm_harddisk_device *harddisk, int state)
{
	if (TRACE_LINES) if (state==1) logerror("%s: HD index pulse\n", tag());
	set_bits(m_status_latch, HDC_DS_INDEX, (state==ASSERT_LINE));
	signal_drive_status();
}

/*
    This is called back from the hard disk when READY becomes asserted.
*/
void myarc_hfdc_device::harddisk_ready_callback(mfm_harddisk_device *harddisk, int state)
{
	if (TRACE_LINES) logerror("%s: HD READY = %d\n", tag(), state);
	set_bits(m_status_latch, HDC_DS_READY, (state==ASSERT_LINE));
	signal_drive_status();
}

/*
    This is called back from the hard disk when seek_complete becomes asserted.
*/
void myarc_hfdc_device::harddisk_skcom_callback(mfm_harddisk_device *harddisk, int state)
{
	if (TRACE_LINES) logerror("%s: HD seek complete = %d\n", tag(), state);
	set_bits(m_status_latch, HDC_DS_SKCOM, (state==ASSERT_LINE));
	signal_drive_status();
}

void myarc_hfdc_device::set_bits(UINT8& byte, int mask, bool set)
{
	if (set) byte |= mask;
	else byte &= ~mask;
}

/*
   Maps the set bit to an index. The rightmost 1 bit is significant. When no
   bit is set, returns -1.
*/
int myarc_hfdc_device::bit_to_index(int value)
{
	if (value & 0x01) return 0;
	if (value & 0x02) return 1;
	if (value & 0x04) return 2;
	if (value & 0x08) return 3;
	return -1;
}

/*
    Notify the controller about the status change
*/
void myarc_hfdc_device::signal_drive_status()
{
	UINT8 reply = 0;
	// Status byte as defined by HDC9234
	// +------+------+------+------+------+------+------+------+
	// | ECC  |Index | SeekC| Tr00 | User | WrPrt| Ready|Fault |
	// +------+------+------+------+------+------+------+------+
	//
	// Set by HFDC
	// 74LS240 is used for driving the lines; it also inverts the inputs
	// If no hard drive or floppy is connected, all lines are 0
	// +------+------+------+------+------+------+------+------+
	// |  0   | Index| SeekC| Tr00 |   0  | WrPrt| Ready|Fault |
	// +------+------+------+------+------+------+------+------+
	//
	//  Ready = /WDS.ready* | DSK
	//  SeekComplete = /WDS.seekComplete* | DSK

	// If DSK is selected, set Ready and SeekComplete to 1
	if ((m_output1_latch & 0x10)!=0)
	{
		reply |= 0x22;

		// Check for TRK00*
		if ((m_current_floppy != nullptr) && (!m_current_floppy->trk00_r()))
			reply |= HDC_DS_TRK00;
	}
	else
	{
		if ((m_output1_latch & 0xe0)!=0)
		{
			if (m_current_harddisk != nullptr)
			{
				if (m_current_harddisk->ready_r()==ASSERT_LINE)
				{
					m_status_latch |= HDC_DS_READY;
					set_bits(m_status_latch, HDC_DS_SKCOM, m_current_harddisk->seek_complete_r()==ASSERT_LINE);
					set_bits(m_status_latch, HDC_DS_TRK00, m_current_harddisk->trk00_r()==ASSERT_LINE);
				}
			}
			// If WDS is selected but not connected, WDS.ready* and WDS.seekComplete* are 1, so Ready=SeekComplete=0
			else set_bits(m_status_latch, HDC_DS_READY | HDC_DS_SKCOM, false);
		}
	}

	reply |= m_status_latch;

	m_hdc9234->auxbus_in(reply);
}

/*
    When the HDC outputs a byte via its AB (auxiliary bus), we need to latch it.
    The target of the transfer is determined by two control lines (S1,S0).
    (0,0) = input drive status
    (0,1) = DMA address
    (1,0) = OUTPUT1
    (1,1) = OUTPUT2
*/
WRITE8_MEMBER( myarc_hfdc_device::auxbus_out )
{
	int index;
	switch (offset)
	{
	case HDC_INPUT_STATUS:
		logerror("%s: Invalid operation: S0=S1=0, but tried to write (expected: read drive status)\n", tag());
		break;

	case HDC_OUTPUT_DMA_ADDR:
		// Value is dma address byte. Shift previous contents to the left.
		// The value is latched inside the Gate Array.
		m_dma_address = ((m_dma_address << 8) + (data&0xff))&0xffffff;
		if (TRACE_DMA) logerror("%s: Setting DMA address; current value = %06x\n", tag(), m_dma_address);
		break;

	case HDC_OUTPUT_1:
		// value is output1
		// The HFDC interprets the value as follows:
		// WDS = Winchester Drive System, old name for hard disk
		// +------+------+------+------+------+------+------+------+
		// | WDS3 | WDS2 | WDS1 | DSKx | x=4  | x=3  | x=2  | x=1  |
		// +------+------+------+------+------+------+------+------+
		// Accordingly, drive 0 is always the floppy; selected by the low nibble

		m_output1_latch = data;

		if ((data & 0x10) != 0) connect_floppy_unit(bit_to_index(data & 0x0f));             // Floppy selected
		else
		{
			index = bit_to_index((data>>4) & 0x0f);

			if (index > 0) connect_harddisk_unit(index-1);  // HD selected; index >= 1
			else
			{
				disconnect_floppy_drives();
				disconnect_hard_drives();

				// Turn off READY and SEEK COMPLETE
				set_bits(m_status_latch, HDC_DS_READY | HDC_DS_SKCOM, false);
			}
		}
		break;

	case HDC_OUTPUT_2:
		// value is output2
		// DS3* = /WDS3
		// WCur = Reduced Write Current
		// Dir = Step direction
		// Step = Step pulse
		// Head = Selected head number (floppy: 0000 or 0001)
		// +------+------+------+------+------+------+------+------+
		// | DS3* | WCur | Dir  | Step |           Head            |
		// +------+------+------+------+------+------+------+------+
		m_output2_latch = data;

		// Output the step pulse to the selected floppy drive
		if (m_current_floppy != nullptr)
		{
			m_current_floppy->ss_w(data & 0x01);
			m_current_floppy->dir_w((data & 0x20)==0);
			m_current_floppy->stp_w((data & 0x10)==0);
		}

		if (m_current_harddisk != nullptr)
		{
			// Dir = 0 -> outward
			m_current_harddisk->direction_in_w((data & 0x20)? ASSERT_LINE : CLEAR_LINE);
			m_current_harddisk->step_w((data & 0x10)? ASSERT_LINE : CLEAR_LINE);
			m_current_harddisk->headsel_w(data & 0x0f);
		}

		// We are pushing the drive status after OUTPUT2
		signal_drive_status();
		break;
	}
}

enum
{
	HFDC_FLOPPY = 1,
	HFDC_HARDDISK = 2
};

void myarc_hfdc_device::connect_floppy_unit(int index)
{
	// Check if we have a new floppy
	if (m_floppy_unit[index] != m_current_floppy)
	{
		// Clear all latched flags from other drives
		m_status_latch = 0;
		disconnect_floppy_drives();
		if (TRACE_LINES) logerror("%s: Select floppy drive DSK%d\n", tag(), index+1);

		// Connect new drive
		m_current_floppy = m_floppy_unit[index];

		// We don't use the READY line of floppy drives.
		// READY is asserted when DSKx = 1
		// The controller fetches the state with the auxbus access
		if (TRACE_LINES) logerror("%s: Connect index callback DSK%d\n", tag(), index+1);
		if (m_current_floppy != nullptr)
			m_current_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(FUNC(myarc_hfdc_device::floppy_index_callback), this));
		else
			logerror("%s: Connection to DSK%d failed because no drive is connected\n", tag(), index+1);
		m_hdc9234->connect_floppy_drive(m_floppy_unit[index]);
	}

	// We can only run a floppy or a harddisk at a time, not both
	disconnect_hard_drives();
}

void myarc_hfdc_device::connect_harddisk_unit(int index)
{
	if (m_harddisk_unit[index] != m_current_harddisk)
	{
		// Clear all latched flags form other drives
		m_status_latch = 0;
		disconnect_hard_drives();
		if (TRACE_LINES) logerror("%s: Select hard disk WDS%d\n", tag(), index+1);

		// Connect new drive
		m_current_harddisk = m_harddisk_unit[index];

		if (TRACE_LINES) logerror("%s: Connect index callback WDS%d\n", tag(), index+1);
		if (m_current_harddisk != nullptr)
		{
			m_current_harddisk->setup_index_pulse_cb(mfm_harddisk_device::index_pulse_cb(FUNC(myarc_hfdc_device::harddisk_index_callback), this));
			m_current_harddisk->setup_ready_cb(mfm_harddisk_device::ready_cb(FUNC(myarc_hfdc_device::harddisk_ready_callback), this));
			m_current_harddisk->setup_seek_complete_cb(mfm_harddisk_device::seek_complete_cb(FUNC(myarc_hfdc_device::harddisk_skcom_callback), this));
		}
		else
			logerror("%s: Connection to WDS%d failed because no drive is connected\n", tag(), index+1);
		m_hdc9234->connect_hard_drive(m_current_harddisk);
	}

	// We can only run a floppy or a harddisk at a time, not both
	disconnect_floppy_drives();
}

void myarc_hfdc_device::disconnect_floppy_drives()
{
	if (TRACE_LINES) logerror("%s: Unselect floppy drives\n", tag());
	// Disconnect current floppy
	if (m_current_floppy != nullptr)
	{
		m_current_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
		m_current_floppy = nullptr;
	}
}

void myarc_hfdc_device::disconnect_hard_drives()
{
	if (TRACE_LINES) logerror("%s: Unselect hard drives\n", tag());
	if (m_current_harddisk != nullptr)
	{
		m_current_harddisk->setup_index_pulse_cb(mfm_harddisk_device::index_pulse_cb());
		m_current_harddisk->setup_seek_complete_cb(mfm_harddisk_device::seek_complete_cb());
		m_current_harddisk = nullptr;
	}
}

/*
    All floppy motors are operated by the same line.
*/
void myarc_hfdc_device::set_floppy_motors_running(bool run)
{
	if (run)
	{
		if (TRACE_MOTOR)
			if (m_MOTOR_ON==CLEAR_LINE) logerror("%s: Motor START\n", tag());
		m_MOTOR_ON = ASSERT_LINE;
	}
	else
	{
		if (TRACE_MOTOR)
			if (m_MOTOR_ON==ASSERT_LINE) logerror("%s: Motor STOP\n", tag());
		m_MOTOR_ON = CLEAR_LINE;
	}

	// Set all motors
	for (auto & elem : m_floppy_unit)
		if (elem != nullptr) elem->mon_w((run)? 0 : 1);
}

/*
    Called whenever the state of the HDC9234 interrupt pin changes.
*/
WRITE_LINE_MEMBER( myarc_hfdc_device::intrq_w )
{
	m_irq = (line_state)state;
	if (TRACE_INT) logerror("%s: INT pin from controller = %d, propagating to INTA*\n", tag(), state);

	// Set INTA*
	// Signal from SMC is active high, INTA* is active low; board inverts signal
	// Anyway, we stay with ASSERT_LINE and CLEAR_LINE
	m_slot->set_inta(state);
}

/*
    Called whenever the HDC9234 desires bus access to the buffer RAM. The
    controller expects a call to dmarq in 1 byte time.
*/
WRITE_LINE_MEMBER( myarc_hfdc_device::dmarq_w )
{
	if (TRACE_DMA) logerror("%s: DMARQ pin from controller = %d\n", tag(), state);
	if (state == ASSERT_LINE)
	{
		m_hdc9234->dmaack(ASSERT_LINE);
	}
}

/*
    Called whenever the state of the HDC9234 DMA in progress changes.
*/
WRITE_LINE_MEMBER( myarc_hfdc_device::dip_w )
{
	m_dip = (line_state)state;
}

/*
    Read a byte from the onboard SRAM. This is called from the HDC9234.
*/
READ8_MEMBER( myarc_hfdc_device::read_buffer )
{
	if (TRACE_DMA) logerror("%s: Read access to onboard SRAM at %04x\n", tag(), m_dma_address);
	if (m_dma_address > 0x8000) logerror("%s: Read access beyond RAM size: %06x\n", tag(), m_dma_address);
	UINT8 value = m_buffer_ram[m_dma_address & 0x7fff];
	m_dma_address = (m_dma_address+1) & 0x7fff;
	return value;
}

/*
    Write a byte to the onboard SRAM. This is called from the HDC9234.
*/
WRITE8_MEMBER( myarc_hfdc_device::write_buffer )
{
	if (TRACE_DMA) logerror("%s: Write access to onboard SRAM at %04x: %02x\n", tag(), m_dma_address, data);
	if (m_dma_address > 0x8000) logerror("%s: Write access beyond RAM size: %06x\n", tag(), m_dma_address);
	m_buffer_ram[m_dma_address & 0x7fff] = data;
	m_dma_address = (m_dma_address+1) & 0x7fff;
}

void myarc_hfdc_device::device_start()
{
	if (TRACE_EMU) logerror("%s: start\n", tag());
	m_dsrrom = memregion(DSRROM)->base();
	m_buffer_ram = memregion(BUFFER)->base();
	m_motor_on_timer = timer_alloc(MOTOR_TIMER);
	// The HFDC does not use READY; it has on-board RAM for DMA
	m_current_floppy = nullptr;
	m_current_harddisk = nullptr;
}

void myarc_hfdc_device::device_reset()
{
	if (TRACE_EMU) logerror("%s: reset\n", tag());

	// The GenMOD mod; our implementation automagically adapts all cards
	if (m_genmod)
	{
		m_select_mask = 0x1fe000;
		m_select_value = 0x174000;
	}
	else
	{
		m_select_mask = 0x7e000;
		m_select_value = 0x74000;
	}

	m_cru_base = ioport("CRUHFDC")->read();
	m_wait_for_hd1 = ioport("WAITHD1")->read();

	// Resetting values
	m_rom_page = 0;

	m_ram_page[0] = 0x08;   // static page 0x08
	for (int i=1; i < 4; i++) m_ram_page[i] = 0;

	m_output1_latch = m_output2_latch = 0;

	m_status_latch = 0x00;

	m_dip = m_irq = CLEAR_LINE;
	m_see_switches = false;
	m_motor_running = false;
	m_selected = false;
	m_lastval = 0;
	m_readyflags = 0;

	for (int i=0; i < 4; i++)
	{
		if (m_floppy_unit[i] != nullptr)
			logerror("%s: FD connector %d with %s\n", tag(), i+1, m_floppy_unit[i]->name());
		else
			logerror("%s: FD connector %d has no floppy attached\n", tag(), i+1);
	}

	for (int i=0; i < 3; i++)
	{
		if (m_harddisk_unit[i] != nullptr)
			logerror("%s: HD connector %d with %s\n", tag(), i+1, m_harddisk_unit[i]->name());
		else
			logerror("%s: HD connector %d has no drive attached\n", tag(), i+1);
	}

	// Disconnect all units
	disconnect_floppy_drives();
	disconnect_hard_drives();
}

void myarc_hfdc_device::device_config_complete()
{
	for (int i=0; i < 3; i++)
	{
		m_floppy_unit[i] = nullptr;
		m_harddisk_unit[i] = nullptr;
	}
	m_floppy_unit[3] = nullptr;

	// Seems to be null when doing a "-listslots"
	if (subdevice("f1")!=nullptr)
	{
		m_floppy_unit[0] = static_cast<floppy_connector*>(subdevice("f1"))->get_device();
		m_floppy_unit[1] = static_cast<floppy_connector*>(subdevice("f2"))->get_device();
		m_floppy_unit[2] = static_cast<floppy_connector*>(subdevice("f3"))->get_device();
		m_floppy_unit[3] = static_cast<floppy_connector*>(subdevice("f4"))->get_device();

		m_harddisk_unit[0] = static_cast<mfm_harddisk_connector*>(subdevice("h1"))->get_device();
		m_harddisk_unit[1] = static_cast<mfm_harddisk_connector*>(subdevice("h2"))->get_device();
		m_harddisk_unit[2] = static_cast<mfm_harddisk_connector*>(subdevice("h3"))->get_device();
	}
}

/*
    The HFDC controller can be configured for different CRU base addresses,
    but DSK1-DSK4 are only available for CRU 1100. For all other addresses,
    the drives 1 to 4 are renamed to DSK5-DSK8 (see [1] p. 7).
*/
INPUT_PORTS_START( ti99_hfdc )
	PORT_START( "WAITHD1" )
	PORT_DIPNAME( 0x01, 0x00, "HFDC Wait for HD1" )
		PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
		PORT_DIPSETTING( 0x01, DEF_STR( On ) )

	PORT_START( "CRUHFDC" )
	PORT_DIPNAME( 0x1f00, 0x1100, "HFDC CRU base" )
		PORT_DIPSETTING( 0x1000, "1000" )
		PORT_DIPSETTING( 0x1100, "1100" )
		PORT_DIPSETTING( 0x1200, "1200" )
		PORT_DIPSETTING( 0x1300, "1300" )
		PORT_DIPSETTING( 0x1400, "1400" )
		PORT_DIPSETTING( 0x1500, "1500" )
		PORT_DIPSETTING( 0x1600, "1600" )
		PORT_DIPSETTING( 0x1700, "1700" )
		PORT_DIPSETTING( 0x1800, "1800" )
		PORT_DIPSETTING( 0x1900, "1900" )
		PORT_DIPSETTING( 0x1a00, "1A00" )
		PORT_DIPSETTING( 0x1b00, "1B00" )
		PORT_DIPSETTING( 0x1c00, "1C00" )
		PORT_DIPSETTING( 0x1d00, "1D00" )
		PORT_DIPSETTING( 0x1e00, "1E00" )
		PORT_DIPSETTING( 0x1f00, "1F00" )

	PORT_START( "HFDCDIP" )
	PORT_DIPNAME( 0x0c, 0x00, "HFDC drive 1 config" )
		PORT_DIPSETTING( 0x00, "40 track, 16 ms")
		PORT_DIPSETTING( 0x08, "40 track, 8 ms")
		PORT_DIPSETTING( 0x04, "80 track, 2 ms")
		PORT_DIPSETTING( 0x0c, "80 track HD, 2 ms")
	PORT_DIPNAME( 0x03, 0x00, "HFDC drive 2 config" )
		PORT_DIPSETTING( 0x00, "40 track, 16 ms")
		PORT_DIPSETTING( 0x02, "40 track, 8 ms")
		PORT_DIPSETTING( 0x01, "80 track, 2 ms")
		PORT_DIPSETTING( 0x03, "80 track HD, 2 ms")
	PORT_DIPNAME( 0xc0, 0x00, "HFDC drive 3 config" )
		PORT_DIPSETTING( 0x00, "40 track, 16 ms")
		PORT_DIPSETTING( 0x80, "40 track, 8 ms")
		PORT_DIPSETTING( 0x40, "80 track, 2 ms")
		PORT_DIPSETTING( 0xc0, "80 track HD, 2 ms")
	PORT_DIPNAME( 0x30, 0x00, "HFDC drive 4 config" )
		PORT_DIPSETTING( 0x00, "40 track, 16 ms")
		PORT_DIPSETTING( 0x20, "40 track, 8 ms")
		PORT_DIPSETTING( 0x10, "80 track, 2 ms")
		PORT_DIPSETTING( 0x30, "80 track HD, 2 ms")
INPUT_PORTS_END

FLOPPY_FORMATS_MEMBER(myarc_hfdc_device::floppy_formats)
	FLOPPY_TI99_SDF_FORMAT,
	FLOPPY_TI99_TDF_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( hfdc_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )        // 40 tracks
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )        // 80 tracks
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )          // 80 tracks
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD )          // 80 tracks 1.4 MiB
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( hfdc_harddisks )
	SLOT_INTERFACE( "generic", MFMHD_GENERIC )    // Generic hard disk (self-adapting to image)
	SLOT_INTERFACE( "st213", MFMHD_ST213 )        // Seagate ST-213 (10 MB)
	SLOT_INTERFACE( "st225", MFMHD_ST225 )        // Seagate ST-225 (20 MB)
	SLOT_INTERFACE( "st251", MFMHD_ST251 )        // Seagate ST-251 (40 MB)
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( ti99_hfdc )
	MCFG_DEVICE_ADD(FDC_TAG, HDC9234, 0)
	MCFG_HDC92X4_INTRQ_CALLBACK(WRITELINE(myarc_hfdc_device, intrq_w))
	MCFG_HDC92X4_DIP_CALLBACK(WRITELINE(myarc_hfdc_device, dip_w))
	MCFG_HDC92X4_AUXBUS_OUT_CALLBACK(WRITE8(myarc_hfdc_device, auxbus_out))
	MCFG_HDC92X4_DMARQ_CALLBACK(WRITELINE(myarc_hfdc_device, dmarq_w))
	MCFG_HDC92X4_DMA_IN_CALLBACK(READ8(myarc_hfdc_device, read_buffer))
	MCFG_HDC92X4_DMA_OUT_CALLBACK(WRITE8(myarc_hfdc_device, write_buffer))

	MCFG_FLOPPY_DRIVE_ADD("f1", hfdc_floppies, "525dd", myarc_hfdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("f2", hfdc_floppies, "525dd", myarc_hfdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("f3", hfdc_floppies, nullptr, myarc_hfdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("f4", hfdc_floppies, nullptr, myarc_hfdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	// NB: Hard disks don't go without image (other than floppy drives)
	MCFG_MFM_HARDDISK_CONN_ADD("h1", hfdc_harddisks, nullptr, MFM_BYTE, 3000, 20, MFMHD_GEN_FORMAT)
	MCFG_MFM_HARDDISK_CONN_ADD("h2", hfdc_harddisks, nullptr, MFM_BYTE, 2000, 20, MFMHD_GEN_FORMAT)
	MCFG_MFM_HARDDISK_CONN_ADD("h3", hfdc_harddisks, nullptr, MFM_BYTE, 2000, 20, MFMHD_GEN_FORMAT)

	MCFG_DEVICE_ADD(CLOCK_TAG, MM58274C, 0)
	MCFG_MM58274C_MODE24(1) // 24 hour
	MCFG_MM58274C_DAY1(0)   // sunday
MACHINE_CONFIG_END

ROM_START( ti99_hfdc )
	ROM_REGION(0x4000, DSRROM, 0)
	ROM_LOAD("hfdc.bin", 0x0000, 0x4000, CRC(66fbe0ed) SHA1(11df2ecef51de6f543e4eaf8b2529d3e65d0bd59)) /* HFDC disk DSR ROM */
	ROM_REGION(0x8000, BUFFER, 0)  /* HFDC RAM buffer 32 KiB */
	ROM_FILL(0x0000, 0x8000, nullptr)
ROM_END


machine_config_constructor myarc_hfdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti99_hfdc );
}

const rom_entry *myarc_hfdc_device::device_rom_region() const
{
	return ROM_NAME( ti99_hfdc );
}

ioport_constructor myarc_hfdc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ti99_hfdc );
}

const device_type TI99_HFDC = &device_creator<myarc_hfdc_device>;
