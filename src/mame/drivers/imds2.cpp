// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// ***************************************
// Driver for Intel Intellec MDS series-II
// ***************************************
//
// Documentation used for this driver:
// [1]  Intel, manual 9800554-04 rev. D - Intellec series II MDS - Schematic drawings
// [2]  Intel, manual 9800556-02 rev. B - Intellec series II MDS - Hardware reference manual
// [3]  Intel, manual 9800555-02 rev. B - Intellec series II MDS - Hardware interface manual
// [4]  Intel, manual 9800605-02 rev. B - Intellec series II MDS - Boot/monitor listing
//
// All these manuals are available on http://www.bitsavers.org
//
// An Intellec MDS series-II is composed of the following boards:
//
// **********
// Integrated Processor Card (IPC) or Integrated Processor Board (IPB)
//
// This is the board where the OS (ISIS-II) and the application software run.
// This driver emulates an IPC.
// IPC is composed as follows:
// A83 8085A-2  CPU @ 4 MHz
//              64k of DRAM
// A82 2732     EPROM with monitor & boot firmware (assembly source of this ROM is in [4])
// A66 8259A    System PIC
// A89 8259A    Local PIC
// A86 8253     PIT
// A91 8251A    Serial channel #0
// A90 8251A    Serial channel #1
//
// **********
// I/O Controller (IOC)
//
// This board acts as a controller for all I/O of the system.
// It is structured as if it were 2 boards in one:
// One part (around 8080) controls Keyboard, CRT & floppy, the other part (around PIO 8041A) controls all parallel I/Os
// (Line printer, Paper tape puncher, Paper tape reader, I/O to PROM programmer).
// Both parts are interfaced to IPC through a bidirectional 8-bit bus.
// IOC is composed of these parts:
// A69 8080A-2  CPU @ 2.448 MHz
//              8k of DRAM
// A50 2716
// A51 2716
// A52 2716
// A53 2716     EPROMs with IOC firmware
// A58 8257     DMA controller
// A35 8253     PIT
// A1  8271     Floppy controller
// A20 8275     CRT controller
// A19 2708     Character generator ROM
// LS1          3.3 kHz beeper
// A72 8041A    CPU @ 6 MHz (PIO: parallel I/O)
//
// **********
// Keyboard controller
//
// Keyboard is controlled by a 8741 CPU that scans the key matrix and sends key codes to IOC through a 8-bit bus.
//
// A3 8741      CPU @ 3.58 MHz
//
// NOTE:
// Firmware running on PIO is NOT original because a dump is not available at the moment.
// Emulator runs a version of PIO firmware that was specifically developped by me to implement
// line printer output.
//
// TODO:
// - Find a dump of the original PIO firmware
// - Adjust speed of processors. Wait states are not accounted for yet.
//
// Huge thanks to Dave Mabry for dumping IOC firmware, KB firmware and character generator. This driver would not
// exist without his dumps.
// (https://web.archive.org/web/20080509062332/http://www.s100-manuals.com/intel/IOC_iMDX_511_Upgrade.zip)
//
// Basic usage / test info:
// To use the system set DIP switches to:
// Floppy present
// IOC mode Diagnostic
// Keyboard present
// and reset the system. The built-in diagnostic mode should start.
//
// Another test is loading ISIS-II (the Intel OS for this system). Floppy image
// 9500007-07_ISIS-II_OPERATING_SYSTEM_DISKETTE_Ver_4.3.IMD
// To load it, the "IOC mode" should be set to "On line", the floppy image
// should be mounted and the system reset. After a few seconds the ISIS-II
// prompt should appear. A command that could be tried is "DIR" that lists
// the content of floppy disk.

#include "includes/imds2.h"

// CPU oscillator of IPC board: 8 MHz
#define IPC_XTAL_Y2     XTAL_8MHz

// Y1 oscillator of IPC board: 19.6608 MHz
#define IPC_XTAL_Y1     XTAL_19_6608MHz

// Main oscillator of IOC board: 22.032 MHz
#define IOC_XTAL_Y2     22032000

// FDC oscillator of IOC board: 8 MHz
#define IOC_XTAL_Y1     XTAL_8MHz

// PIO oscillator: 6 MHz
#define IOC_XTAL_Y3     XTAL_6MHz

// Frequency of beeper
#define IOC_BEEP_FREQ   3300

static ADDRESS_MAP_START(ipc_mem_map , AS_PROGRAM , 8 , imds2_state)
	AM_RANGE(0x0000 , 0xffff) AM_READWRITE(ipc_mem_read, ipc_mem_write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ipc_io_map , AS_IO , 8 , imds2_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0xc0 , 0xc0) AM_READWRITE(imds2_ipc_dbbout_r , imds2_ipc_dbbin_data_w)
	AM_RANGE(0xc1 , 0xc1) AM_READWRITE(imds2_ipc_status_r , imds2_ipc_dbbin_cmd_w)
		AM_RANGE(0xf0 , 0xf3) AM_DEVREADWRITE("ipctimer" , pit8253_device , read , write)
		AM_RANGE(0xf4 , 0xf4) AM_DEVREADWRITE("ipcusart0" , i8251_device , data_r , data_w)
		AM_RANGE(0xf5 , 0xf5) AM_DEVREADWRITE("ipcusart0" , i8251_device , status_r , control_w)
		AM_RANGE(0xf6 , 0xf6) AM_DEVREADWRITE("ipcusart1" , i8251_device , data_r , data_w)
		AM_RANGE(0xf7 , 0xf7) AM_DEVREADWRITE("ipcusart1" , i8251_device , status_r , control_w)
	AM_RANGE(0xf8 , 0xf9) AM_DEVREADWRITE("iocpio" , i8041_device , upi41_master_r , upi41_master_w)
	AM_RANGE(0xfa , 0xfb) AM_READWRITE(imds2_ipclocpic_r , imds2_ipclocpic_w)
	AM_RANGE(0xfc , 0xfd) AM_READWRITE(imds2_ipcsyspic_r , imds2_ipcsyspic_w)
	AM_RANGE(0xff , 0xff) AM_WRITE(imds2_ipc_control_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ioc_mem_map , AS_PROGRAM , 8 , imds2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000 , 0x1fff) AM_ROM
	AM_RANGE(0x4000 , 0x5fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ioc_io_map , AS_IO , 8 , imds2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00 , 0x0f) AM_WRITE(imds2_ioc_dbbout_w)
	AM_RANGE(0x20 , 0x2f) AM_WRITE(imds2_ioc_f0_w)
	AM_RANGE(0x30 , 0x3f) AM_WRITE(imds2_ioc_set_f1_w)
	AM_RANGE(0x40 , 0x4f) AM_WRITE(imds2_ioc_reset_f1_w)
	AM_RANGE(0x50 , 0x5f) AM_WRITE(imds2_start_timer_w)
	AM_RANGE(0x60 , 0x6f) AM_WRITE(imds2_miscout_w)
	AM_RANGE(0x80 , 0x8f) AM_READ(imds2_miscin_r)
	AM_RANGE(0x90 , 0x9f) AM_READ(imds2_kb_read)
	AM_RANGE(0xa0 , 0xaf) AM_READ(imds2_ioc_status_r)
	AM_RANGE(0xb0 , 0xbf) AM_READ(imds2_ioc_dbbin_r)
	AM_RANGE(0xc0 , 0xcf) AM_DEVICE("iocfdc" , i8271_device, map)
	AM_RANGE(0xd0 , 0xdf) AM_DEVREADWRITE("ioccrtc" , i8275_device , read , write)
	AM_RANGE(0xe0 , 0xef) AM_DEVREADWRITE("ioctimer" , pit8253_device , read , write);
// DMA controller range doesn't extend to 0xff because register 0xfd needs to be read as 0xff
// This register is used by IOC firmware to detect DMA controller model (either 8237 or 8257)
	AM_RANGE(0xf0 , 0xf8) AM_DEVREADWRITE("iocdma" , i8257_device , read , write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(pio_io_map , AS_IO , 8 , imds2_state)
	AM_RANGE(MCS48_PORT_P1 , MCS48_PORT_P1) AM_READWRITE(imds2_pio_port_p1_r , imds2_pio_port_p1_w)
	AM_RANGE(MCS48_PORT_P2 , MCS48_PORT_P2) AM_READWRITE(imds2_pio_port_p2_r , imds2_pio_port_p2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(kb_io_map , AS_IO , 8 , imds2_state)
	AM_RANGE(MCS48_PORT_P1 , MCS48_PORT_P1) AM_WRITE(imds2_kb_port_p1_w)
	AM_RANGE(MCS48_PORT_P2 , MCS48_PORT_P2) AM_READ(imds2_kb_port_p2_r)
	AM_RANGE(MCS48_PORT_T0 , MCS48_PORT_T0) AM_READ(imds2_kb_port_t0_r)
	AM_RANGE(MCS48_PORT_T1 , MCS48_PORT_T1) AM_READ(imds2_kb_port_t1_r)
ADDRESS_MAP_END

imds2_state::imds2_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig , type , tag),
	m_ipccpu(*this , "ipccpu"),
	m_ipcsyspic(*this , "ipcsyspic"),
	m_ipclocpic(*this , "ipclocpic"),
		m_ipctimer(*this , "ipctimer"),
		m_ipcusart0(*this , "ipcusart0"),
		m_ipcusart1(*this , "ipcusart1"),
		m_serial0(*this , "serial0"),
		m_serial1(*this , "serial1"),
	m_ioccpu(*this , "ioccpu"),
	m_iocdma(*this , "iocdma"),
	m_ioccrtc(*this , "ioccrtc"),
	m_iocbeep(*this , "iocbeep"),
	m_ioctimer(*this , "ioctimer"),
	m_iocfdc(*this , "iocfdc"),
	m_flop0(*this, "iocfdc:0"),
	m_iocpio(*this , "iocpio"),
	m_kbcpu(*this , "kbcpu"),
	m_palette(*this , "palette"),
	m_gfxdecode(*this, "gfxdecode"),
	m_centronics(*this , "centronics"),
	m_io_key0(*this , "KEY0"),
	m_io_key1(*this , "KEY1"),
	m_io_key2(*this , "KEY2"),
	m_io_key3(*this , "KEY3"),
	m_io_key4(*this , "KEY4"),
	m_io_key5(*this , "KEY5"),
	m_io_key6(*this , "KEY6"),
	m_io_key7(*this , "KEY7"),
	m_ioc_options(*this , "IOC_OPTS"),
	m_device_status_byte(0xff)
{
}

READ8_MEMBER(imds2_state::ipc_mem_read)
{
	if (imds2_in_ipc_rom(offset)) {
		return m_ipc_rom[ (offset & 0x07ff) | ((offset & 0x1000) >> 1) ];
	} else {
		return m_ipc_ram[ offset ];
	}
}

WRITE8_MEMBER(imds2_state::ipc_mem_write)
{
	if (!imds2_in_ipc_rom(offset)) {
		m_ipc_ram[ offset ] = data;
	}
}

WRITE8_MEMBER(imds2_state::imds2_ipc_control_w)
{
	// See A84, pg 28 of [1]
	// b3 is ~(bit to be written)
	// b2-b0 is ~(no. of bit to be written)
	UINT8 mask = (1U << (~data & 0x07));

	if (BIT(data , 3)) {
		m_ipc_control &= ~mask;
	} else {
		m_ipc_control |= mask;
	}
}

WRITE_LINE_MEMBER(imds2_state::imds2_ipc_intr)
{
	m_ipccpu->set_input_line(I8085_INTR_LINE , (state != 0) && BIT(m_ipc_control , 2));
}

READ8_MEMBER(imds2_state::imds2_ipcsyspic_r)
{
	return m_ipcsyspic->read(space , offset == 0);
}

READ8_MEMBER(imds2_state::imds2_ipclocpic_r)
{
	return m_ipclocpic->read(space , offset == 0);
}

WRITE8_MEMBER(imds2_state::imds2_ipcsyspic_w)
{
	m_ipcsyspic->write(space , offset == 0 , data);
}

WRITE8_MEMBER(imds2_state::imds2_ipclocpic_w)
{
	m_ipclocpic->write(space , offset == 0 , data);
}

WRITE_LINE_MEMBER(imds2_state::imds2_baud_clk_0_w)
{
		m_ipcusart0->write_txc(state);
		m_ipcusart0->write_rxc(state);
}

WRITE_LINE_MEMBER(imds2_state::imds2_baud_clk_1_w)
{
		m_ipcusart1->write_txc(state);
		m_ipcusart1->write_rxc(state);
}

WRITE8_MEMBER(imds2_state::imds2_miscout_w)
{
	m_miscout = data;
	imds2_update_beeper();
	// Send INTR to IPC
	m_ipclocpic->ir6_w(BIT(m_miscout , 1));
}

READ8_MEMBER(imds2_state::imds2_miscin_r)
{
	UINT8 res = m_ioc_options->read();
	return res | ((m_beeper_timer == 0) << 2);
}

WRITE_LINE_MEMBER(imds2_state::imds2_beep_timer_w)
{
	m_beeper_timer = state;
	imds2_update_beeper();
}

WRITE8_MEMBER(imds2_state::imds2_start_timer_w)
{
	// Trigger timer 2 of ioctimer
	m_ioctimer->write_gate2(0);
	m_ioctimer->write_gate2(1);
}

READ8_MEMBER(imds2_state::imds2_kb_read)
{
	return m_kbcpu->upi41_master_r(space , (offset & 2) >> 1);
}

READ8_MEMBER(imds2_state::imds2_kb_port_p2_r)
{
	if ((m_kb_p1 & 3) == 0) {
		// Row selected
		// Row number is encoded on bits P15..P12, they are "backwards" (P15 is LSB) and keyboard rows are encoded starting with value 2 on these bits (see A4, pg 56 of [1])
		unsigned row = (m_kb_p1 >> 2) & 0x0f;
		ioport_value data;

		switch (row) {
		case 4:
			// Row 0
			data = m_io_key0->read();
			break;

		case 12:
			// Row 1
			data = m_io_key1->read();
			break;

		case 2:
			// Row 2
			data = m_io_key2->read();
			break;

		case 10:
			// Row 3
			data = m_io_key3->read();
			break;

		case 6:
			// Row 4
			data = m_io_key4->read();
			break;

		case 14:
			// Row 5
			data = m_io_key5->read();
			break;

		case 1:
			// Row 6
			data = m_io_key6->read();
			break;

		case 9:
			// Row 7
			data = m_io_key7->read();
			break;

		default:
			data = 0xff;
			break;
		}
		return data & 0xff;
	} else {
		// No row selected
		return 0xff;
	}
}

WRITE8_MEMBER(imds2_state::imds2_kb_port_p1_w)
{
	m_kb_p1 = data;
}

READ8_MEMBER(imds2_state::imds2_kb_port_t0_r)
{
	// T0 tied low
	// It appears to be some kind of strapping option on kb hw
	return 0;
}

READ8_MEMBER(imds2_state::imds2_kb_port_t1_r)
{
	// T1 tied low
	// It appears to be some kind of strapping option on kb hw
	return 0;
}

WRITE8_MEMBER(imds2_state::imds2_ioc_dbbout_w)
{
	m_ioc_obf = ~data;
	// Set/reset OBF flag (b0)
	m_ipc_ioc_status = ((offset & 1) == 0) | (m_ipc_ioc_status & ~0x01);
}

WRITE8_MEMBER(imds2_state::imds2_ioc_f0_w)
{
	// Set/reset F0 flag (b2)
	m_ipc_ioc_status = ((offset & 1) << 2) | (m_ipc_ioc_status & ~0x04);
}

WRITE8_MEMBER(imds2_state::imds2_ioc_set_f1_w)
{
	// Set F1 flag (b3)
	m_ipc_ioc_status |= 0x08;
}

WRITE8_MEMBER(imds2_state::imds2_ioc_reset_f1_w)
{
	// Reset F1 flag (b3)
	m_ipc_ioc_status &= ~0x08;
}

READ8_MEMBER(imds2_state::imds2_ioc_status_r)
{
	return ~m_ipc_ioc_status;
}

READ8_MEMBER(imds2_state::imds2_ioc_dbbin_r)
{
	// Reset IBF flag (b1)
	m_ipc_ioc_status &= ~0x02;
	return ~m_ioc_ibf;
}

READ8_MEMBER(imds2_state::imds2_ipc_dbbout_r)
{
	// Reset OBF flag (b0)
	m_ipc_ioc_status &= ~0x01;
	return m_ioc_obf;
}

READ8_MEMBER(imds2_state::imds2_ipc_status_r)
{
	return m_ipc_ioc_status;
}

WRITE8_MEMBER(imds2_state::imds2_ipc_dbbin_data_w)
{
	// Set IBF flag (b1)
	m_ipc_ioc_status |= 0x02;
	// Reset F1 flag (b3)
	m_ipc_ioc_status &= ~0x08;
	m_ioc_ibf = data;
}

WRITE8_MEMBER(imds2_state::imds2_ipc_dbbin_cmd_w)
{
	// Set IBF flag (b1)
	m_ipc_ioc_status |= 0x02;
	// Set F1 flag (b3)
	m_ipc_ioc_status |= 0x08;
	m_ioc_ibf = data;
}

WRITE_LINE_MEMBER(imds2_state::imds2_hrq_w)
{
	// Should be propagated to HOLD input of IOC CPU
	m_iocdma->hlda_w(state);
}

READ8_MEMBER(imds2_state::imds2_ioc_mem_r)
{
	address_space& prog_space = m_ioccpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(imds2_state::imds2_ioc_mem_w)
{
	address_space& prog_space = m_ioccpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset , data);
}

READ8_MEMBER(imds2_state::imds2_pio_port_p1_r)
{
	// If STATUS ENABLE/ == 0 return inverted device status byte, else return 0xff
	// STATUS ENABLE/ == 0 when P23-P20 == 12 & P24 == 0 & P25 = 1 & P26 = 1
	if ((m_pio_port2 & 0x7f) == 0x6c) {
		return ~m_device_status_byte;
	} else {
	return 0xff;
}
}

WRITE8_MEMBER(imds2_state::imds2_pio_port_p1_w)
{
	m_pio_port1 = data;
	imds2_update_printer();
}

READ8_MEMBER(imds2_state::imds2_pio_port_p2_r)
{
	return m_pio_port2;
}

WRITE8_MEMBER(imds2_state::imds2_pio_port_p2_w)
{
	m_pio_port2 = data;
	imds2_update_printer();
	// Send INTR to IPC
	m_ipclocpic->ir5_w(BIT(data , 7));
}

WRITE_LINE_MEMBER(imds2_state::imds2_pio_lpt_ack_w)
{
	if (state) {
		m_device_status_byte |= 0x20;
	} else {
		m_device_status_byte &= ~0x20;
	}
}

WRITE_LINE_MEMBER(imds2_state::imds2_pio_lpt_busy_w)
{
	// Busy is active high in centronics_device whereas it's active low in MDS
	if (!state) {
		m_device_status_byte |= 0x10;
	} else {
		m_device_status_byte &= ~0x10;
	}
}

WRITE_LINE_MEMBER(imds2_state::imds2_pio_lpt_select_w)
{
	if (state) {
		m_device_status_byte |= 0x40;
	} else {
		m_device_status_byte &= ~0x40;
	}
}

I8275_DRAW_CHARACTER_MEMBER(imds2_state::crtc_display_pixels)
{
	unsigned i;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chargen_byte = m_chargen[ (linecount & 7) | ((unsigned)charcode << 3) ];
	UINT16 pixels;

	if (lten) {
		pixels = ~0;
	} else if (vsp != 0 || (linecount & 8) != 0) {
		pixels = 0;
	} else {
		// See [2], pg 58 for the very peculiar way of generating character images
		// Here each half-pixel is translated into a full pixel
		UINT16 exp_pix_l;
		UINT16 exp_pix_r;

		exp_pix_l = (UINT16)chargen_byte;
		exp_pix_l = ((exp_pix_l & 0x80) << 5) |
			((exp_pix_l & 0x40) << 4) |
			((exp_pix_l & 0x20) << 3) |
			((exp_pix_l & 0x10) << 2) |
			((exp_pix_l & 0x08) << 1) |
			(exp_pix_l & 0x04);
		exp_pix_l |= (exp_pix_l << 1);
		exp_pix_r = exp_pix_l;

		// Layout of exp_pix_l/r:
		// Bit #              : F  E  D  C  B  A  9  8  7  6  5  4  3  2  1  0
		// Bit of chargen_byte: 0  0  b7 b7 b6 b6 b5 b5 b4 b4 b3 b3 b2 b2 0  0
		if ((chargen_byte & 2) == 0) {
			exp_pix_l >>= 1;
		}
		exp_pix_l &= 0x3fc0;

		if ((chargen_byte & 1) == 0) {
			exp_pix_r >>= 1;
		}
		exp_pix_r &= 0x003e;

		pixels = exp_pix_l | exp_pix_r;
	}

	if (rvv) {
		pixels = ~pixels;
	}

	for (i = 0; i < 14; i++) {
		bitmap.pix32(y, x + i) = palette[ (pixels & (1U << (13 - i))) != 0 ];
	}
}

void imds2_state::driver_start()
{
	// Allocate 64k for IPC RAM
	m_ipc_ram.resize(0x10000);

	memory_region *ipcrom = memregion("ipcrom");
	if (ipcrom == NULL) {
		fatalerror("Unable to find IPC ROM region\n");
	} else {
		m_ipc_rom = ipcrom->base();
	}
}

void imds2_state::machine_start()
{
		m_iocfdc->set_ready_line_connected(true);
}

void imds2_state::video_start()
{
	m_chargen = memregion("gfx1")->base();
}

void imds2_state::machine_reset()
{
	m_iocbeep->set_frequency(IOC_BEEP_FREQ);
	m_ipc_control = 0x00;
	m_ipc_ioc_status = 0x0f;

	m_iocfdc->set_rate(500000); // The IMD images show a rate of 500kbps
}

bool imds2_state::imds2_in_ipc_rom(offs_t offset) const
{
	offs_t masked_offset = offset & 0xf800;

	// Region 0000-07ff is in ROM when START_UP/ == 0 && SEL_BOOT/ == 0
	if (masked_offset == 0x0000 && (m_ipc_control & 0x28) == 0) {
		return true;
	}

	// Region e800-efff is in ROM when SEL_BOOT/ == 0
	if (masked_offset == 0xe800 && (m_ipc_control & 0x08) == 0) {
		return true;
	}

	// Region f800-ffff is always in ROM
	if (masked_offset == 0xf800) {
		return true;
	}

	return false;
}

void imds2_state::imds2_update_beeper(void)
{
	m_iocbeep->set_state(m_beeper_timer == 0 && BIT(m_miscout , 0) == 0);
}

void imds2_state::imds2_update_printer(void)
{
	// Data to printer is ~P1 when STATUS ENABLE/==1, else 0xff (assuming pull-ups on printer)
	UINT8 printer_data;
	if ((m_pio_port2 & 0x7f) == 0x6c) {
		printer_data = 0xff;
	} else {
		printer_data = ~m_pio_port1;
	}
	m_centronics->write_data0(BIT(printer_data , 0));
	m_centronics->write_data1(BIT(printer_data , 1));
	m_centronics->write_data2(BIT(printer_data , 2));
	m_centronics->write_data3(BIT(printer_data , 3));
	m_centronics->write_data4(BIT(printer_data , 4));
	m_centronics->write_data5(BIT(printer_data , 5));
	m_centronics->write_data6(BIT(printer_data , 6));
	m_centronics->write_data7(BIT(printer_data , 7));

	// LPT DATA STROBE/ == 0 when P23-P20 == 9 & P24 == 0
	m_centronics->write_strobe((m_pio_port2 & 0x1f) != 0x09);
}

static INPUT_PORTS_START(imds2)
		// See [1], pg 56 for key matrix layout
		// See [1], pg 57 for keyboard layout
		PORT_START("KEY0")
		PORT_BIT(0x01 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)           PORT_CHAR('\t')                     // OK
		PORT_BIT(0x02 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR('@') PORT_CHAR('`')       // OK
		PORT_BIT(0x04 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')       // OK
		PORT_BIT(0x08 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)         PORT_CHAR(13)                       // OK
		PORT_BIT(0x10 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')                      // OK
		PORT_BIT(0x20 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR(':') PORT_CHAR('*')       // OK
		PORT_BIT(0x40 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')       // OK
		PORT_BIT(0x80 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')       // OK

		PORT_START("KEY1")
		PORT_BIT(0x01 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')       // OK
		PORT_BIT(0x02 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')       // OK
		PORT_BIT(0x04 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')       // OK
		PORT_BIT(0x08 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')       // OK
		PORT_BIT(0x10 , IP_ACTIVE_LOW , IPT_UNUSED)
		PORT_BIT(0x20 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')       // OK
		PORT_BIT(0x40 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')       // OK
		PORT_BIT(0x80 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')       // OK

		PORT_START("KEY2")
		PORT_BIT(0x01 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0') PORT_CHAR('~')       // OK
		PORT_BIT(0x02 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('[') PORT_CHAR('{')       // OK
		PORT_BIT(0x04 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')       // OK
		PORT_BIT(0x08 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')       // OK
		PORT_BIT(0x10 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR(')')       // OK
		PORT_BIT(0x20 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('=')       // OK
		PORT_BIT(0x40 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')       // OK
		PORT_BIT(0x80 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR(';') PORT_CHAR('+')       // OK

		PORT_START("KEY3")
		PORT_BIT(0x01 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')       // OK
		PORT_BIT(0x02 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')       // OK
		PORT_BIT(0x04 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')       // OK
		PORT_BIT(0x08 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')       // OK
		PORT_BIT(0x10 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')       // OK
		PORT_BIT(0x20 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')       // OK
		PORT_BIT(0x40 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')       // OK
		PORT_BIT(0x80 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')       // OK

		PORT_START("KEY4")
		PORT_BIT(0x01 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')       // OK
		PORT_BIT(0x02 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')       // OK
		PORT_BIT(0x04 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')       // OK
		PORT_BIT(0x08 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')       // OK
		PORT_BIT(0x10 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')       // OK
		PORT_BIT(0x20 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')       // OK
		PORT_BIT(0x40 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')       // OK
		PORT_BIT(0x80 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')       // OK

		PORT_START("KEY5")
		PORT_BIT(0x01 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('"')       // OK
		PORT_BIT(0x02 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')       // OK
		PORT_BIT(0x04 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('(')       // OK
		PORT_BIT(0x08 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')       // OK
		PORT_BIT(0x10 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')       // OK
		PORT_BIT(0x20 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')       // OK
		PORT_BIT(0x40 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')      // OK
		PORT_BIT(0x80 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('&')       // OK

		PORT_START("KEY6")
		PORT_BIT(0x01 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)     PORT_CHAR(8)                        // BS
		PORT_BIT(0x02 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)          PORT_CHAR(UCHAR_MAMEKEY(HOME))      // OK
		PORT_BIT(0x04 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)         PORT_CHAR('\\') PORT_CHAR('|')      // OK
		PORT_BIT(0x08 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)         PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     // OK
		PORT_BIT(0x10 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)      PORT_CHAR(UCHAR_SHIFT_2)            // OK
		PORT_BIT(0x20 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)          PORT_CHAR(UCHAR_MAMEKEY(LEFT))      // OK
		PORT_BIT(0x40 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR(']') PORT_CHAR('}')       // OK
		PORT_BIT(0x80 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)            PORT_CHAR(UCHAR_MAMEKEY(UP))        // OK

		PORT_START("KEY7")
		PORT_BIT(0x01 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)           PORT_CHAR(UCHAR_MAMEKEY(ESC))       // OK
		PORT_BIT(0x02 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)          PORT_CHAR(UCHAR_MAMEKEY(DOWN))      // OK
		PORT_BIT(0x04 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('_') PORT_CHAR('^')       // OK
		PORT_BIT(0x08 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)        PORT_CHAR(UCHAR_SHIFT_1)            // OK
		PORT_BIT(0x10 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)          PORT_CHAR(UCHAR_MAMEKEY(LALT))      // OK
		PORT_BIT(0x20 , IP_ACTIVE_LOW , IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)      PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
		PORT_BIT(0x40 , IP_ACTIVE_LOW , IPT_UNUSED)
		PORT_BIT(0x80 , IP_ACTIVE_LOW , IPT_UNUSED)

		// Options on IOC: see [1], pg 40
		PORT_START("IOC_OPTS")
		PORT_DIPNAME(0x80 , 0x80 , "Keyboard present")
		PORT_DIPSETTING(0x80 , DEF_STR(Yes))
		PORT_DIPSETTING(0x00 , DEF_STR(No))
		PORT_DIPNAME(0x28 , 0x00 , "IOC mode")
		PORT_DIPSETTING(0x00 , "On line")
		PORT_DIPSETTING(0x08 , "Local")
		PORT_DIPSETTING(0x20 , "Diagnostic")
		PORT_DIPNAME(0x02 , 0x00 , "Floppy present")
		PORT_DIPSETTING(0x02 , DEF_STR(Yes))
		PORT_DIPSETTING(0x00 , DEF_STR(No))
		PORT_DIPNAME(0x01 , 0x01 , "CRT frame frequency")
		PORT_DIPSETTING(0x01 , "50 Hz")
		PORT_DIPSETTING(0x00 , "60 Hz")
INPUT_PORTS_END

static GFXLAYOUT_RAW(imds2_charlayout , 8 , 8 , 8 , 64)

static GFXDECODE_START(imds2)
	GFXDECODE_ENTRY("gfx1" , 0x0000 , imds2_charlayout , 0 , 1)
GFXDECODE_END

static SLOT_INTERFACE_START( imds2_floppies )
	SLOT_INTERFACE( "8sssd", FLOPPY_8_SSSD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START(imds2 , imds2_state)
		MCFG_CPU_ADD("ipccpu" , I8085A , IPC_XTAL_Y2 / 2)  // 4 MHz
		MCFG_CPU_PROGRAM_MAP(ipc_mem_map)
		MCFG_CPU_IO_MAP(ipc_io_map)
		MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("ipcsyspic" , pic8259_device , inta_cb)
		MCFG_QUANTUM_TIME(attotime::from_hz(100))

		MCFG_PIC8259_ADD("ipcsyspic" , WRITELINE(imds2_state , imds2_ipc_intr) , VCC , NULL)
		MCFG_PIC8259_ADD("ipclocpic" , DEVWRITELINE("ipcsyspic" , pic8259_device , ir7_w) , VCC  , NULL)

		MCFG_DEVICE_ADD("ipctimer" , PIT8253 , 0)
		MCFG_PIT8253_CLK0(IPC_XTAL_Y1 / 16)
		MCFG_PIT8253_CLK1(IPC_XTAL_Y1 / 16)
		MCFG_PIT8253_CLK2(IPC_XTAL_Y1 / 16)
		MCFG_PIT8253_OUT0_HANDLER(WRITELINE(imds2_state , imds2_baud_clk_0_w))
		MCFG_PIT8253_OUT1_HANDLER(WRITELINE(imds2_state , imds2_baud_clk_1_w))
		MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("ipclocpic" , pic8259_device , ir4_w))

				MCFG_DEVICE_ADD("ipcusart0" , I8251 , 0)
				MCFG_I8251_RTS_HANDLER(DEVWRITELINE("ipcusart0" , i8251_device , write_cts))
				MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("ipclocpic" , pic8259_device , ir0_w))
				MCFG_I8251_TXRDY_HANDLER(DEVWRITELINE("ipclocpic" , pic8259_device , ir1_w))
				MCFG_I8251_TXD_HANDLER(DEVWRITELINE("serial0" , rs232_port_device , write_txd))

				MCFG_DEVICE_ADD("ipcusart1" , I8251 , 0)
				MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("ipclocpic" , pic8259_device , ir2_w))
				MCFG_I8251_TXRDY_HANDLER(DEVWRITELINE("ipclocpic" , pic8259_device , ir3_w))
				MCFG_I8251_TXD_HANDLER(DEVWRITELINE("serial1" , rs232_port_device , write_txd))
				MCFG_I8251_RTS_HANDLER(DEVWRITELINE("serial1" , rs232_port_device , write_rts))
				MCFG_I8251_DTR_HANDLER(DEVWRITELINE("serial1" , rs232_port_device , write_dtr))

		MCFG_RS232_PORT_ADD("serial0" , default_rs232_devices , NULL)
				MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ipcusart0" , i8251_device , write_rxd))
				MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ipcusart0" , i8251_device , write_dsr))

		MCFG_RS232_PORT_ADD("serial1" , default_rs232_devices , NULL)
				MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ipcusart1" , i8251_device , write_rxd))
				MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ipcusart1" , i8251_device , write_cts))
				MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ipcusart1" , i8251_device , write_dsr))

		MCFG_CPU_ADD("ioccpu" , I8080A , IOC_XTAL_Y2 / 18)     // 2.448 MHz but running at 50% (due to wait states & DMA usage of bus)
		MCFG_CPU_PROGRAM_MAP(ioc_mem_map)
		MCFG_CPU_IO_MAP(ioc_io_map)
		MCFG_QUANTUM_TIME(attotime::from_hz(100))

		// The IOC CRT hw is a bit complex, as the character clock (CCLK) to i8275
		// is varied according to the part of the video frame being scanned and according to
		// the 50/60 Hz option jumper (W8).
		// The basic clock (BCLK) runs at 22.032 MHz.
		// CCLK = BCLK / 14 when in the active region of video
		// CCLK = BCLK / 12 when in horizontal retrace (HRTC=1)
		// CCLK = BCLK / 10 when in horizontal retrace of "short scan lines" (50 Hz only)
		//
		// ***** 50 Hz timings *****
		// 80 chars/row, 26 chars/h. retrace, 11 scan lines/row, 25 active rows, 3 vertical retrace rows
		// Scan line: 80 chars * 14 BCLK + 26 chars * 12 BCLK = 1432 BCLK (64.996 usec)
		// Scan row: 11 * scan lines = 15752 BCLK (714.960 usec)
		// "Short" scan line: 80 chars * 14 BCLK + 26 chars * 10 BCLK = 1380 BCLK (62.636 usec)
		// Frame: 28 scan rows (8 scan lines of 27th row are short): 27 * scan row + 3 * scan line + 8 * short scan line: 440640 BCLK (20 msec)
		//
		// ***** 60 Hz timings *****
		// 80 chars/row, 20 chars/h. retrace, 10 scan lines/row, 25 active rows, 2 vertical retrace rows
		// Scan line: 80 chars * 14 BCLK + 20 chars * 12 BCLK = 1360 BCLK (61.728 usec)
		// Scan row: 10 * scan lines = 13600 BCLK (617.284 usec)
		// Frame: 27 scan rows : 367200 BCLK (16.667 msec)
		//
		// Clock here is semi-bogus: it gives the correct frame frequency at 50 Hz (with the incorrect
		// assumption that CCLK is fixed at BCLK / 14)
		MCFG_DEVICE_ADD("ioccrtc" , I8275 , 22853600 / 14)
		MCFG_I8275_CHARACTER_WIDTH(14)
		MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(imds2_state , crtc_display_pixels)
		MCFG_I8275_DRQ_CALLBACK(DEVWRITELINE("iocdma" , i8257_device , dreq2_w))
		MCFG_I8275_IRQ_CALLBACK(INPUTLINE("ioccpu" , I8085_INTR_LINE))

		MCFG_SCREEN_ADD("screen" , RASTER)
		MCFG_SCREEN_UPDATE_DEVICE("ioccrtc" , i8275_device , screen_update)
		MCFG_SCREEN_REFRESH_RATE(50)
		MCFG_GFXDECODE_ADD("gfxdecode" , "palette" , imds2)
		MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

		MCFG_SPEAKER_STANDARD_MONO("mono")
		MCFG_SOUND_ADD("iocbeep" , BEEP , 0)
		MCFG_SOUND_ROUTE(ALL_OUTPUTS , "mono" , 1.00)

		MCFG_DEVICE_ADD("iocdma" , I8257 , IOC_XTAL_Y2 / 9)
		MCFG_I8257_OUT_HRQ_CB(WRITELINE(imds2_state, imds2_hrq_w))
		MCFG_I8257_IN_MEMR_CB(READ8(imds2_state , imds2_ioc_mem_r))
		MCFG_I8257_OUT_MEMW_CB(WRITE8(imds2_state , imds2_ioc_mem_w))
		MCFG_I8257_IN_IOR_1_CB(DEVREAD8("iocfdc" , i8271_device , data_r))
		MCFG_I8257_OUT_IOW_1_CB(DEVWRITE8("iocfdc" , i8271_device , data_w))
		MCFG_I8257_OUT_IOW_2_CB(DEVWRITE8("ioccrtc" , i8275_device , dack_w))

		MCFG_DEVICE_ADD("ioctimer" , PIT8253 , 0)
		MCFG_PIT8253_CLK0(IOC_XTAL_Y1 / 4)
		MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("ioctimer" , pit8253_device , write_clk2));
		MCFG_PIT8253_OUT2_HANDLER(WRITELINE(imds2_state , imds2_beep_timer_w));

		MCFG_DEVICE_ADD("iocfdc" , I8271 , IOC_XTAL_Y1 / 2)
		MCFG_I8271_DRQ_CALLBACK(DEVWRITELINE("iocdma" , i8257_device , dreq1_w))
		MCFG_FLOPPY_DRIVE_ADD("iocfdc:0", imds2_floppies, "8sssd", floppy_image_device::default_floppy_formats)
		MCFG_SLOT_FIXED(true)

		MCFG_CPU_ADD("iocpio" , I8041 , IOC_XTAL_Y3)
		MCFG_CPU_IO_MAP(pio_io_map)
		MCFG_QUANTUM_TIME(attotime::from_hz(100))

		MCFG_CPU_ADD("kbcpu", I8741, XTAL_3_579545MHz)         /* 3.579545 MHz */
		MCFG_CPU_IO_MAP(kb_io_map)
		MCFG_QUANTUM_TIME(attotime::from_hz(100))

		MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
		MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(imds2_state , imds2_pio_lpt_ack_w))
		MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(imds2_state , imds2_pio_lpt_busy_w))
		MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(imds2_state , imds2_pio_lpt_select_w))
MACHINE_CONFIG_END

ROM_START(imds2)
		// ROM definition of IPC cpu (8085A)
		ROM_REGION(0x1000 , "ipcrom" , 0)
		ROM_LOAD("ipc_a82.bin" , 0x0000 , 0x1000 , CRC(0889394f) SHA1(b7525baf1884a7d67402dea4b5566016a9861ef2))

		// ROM definition of IOC cpu (8080A)
		ROM_REGION(0x2000 , "ioccpu" , 0)
		ROM_LOAD("ioc_a50.bin" , 0x0000 , 0x0800 , CRC(d9f926a1) SHA1(bd9d0f7458acc2806120a6dbaab9c48be315b060))
		ROM_LOAD("ioc_a51.bin" , 0x0800 , 0x0800 , CRC(6aa2f86c) SHA1(d3a5314d86e3366545b4c97b29e323dfab383d5f))
		ROM_LOAD("ioc_a52.bin" , 0x1000 , 0x0800 , CRC(b88a38d5) SHA1(934716a1daec852f4d1f846510f42408df0c9584))
		ROM_LOAD("ioc_a53.bin" , 0x1800 , 0x0800 , CRC(c8df4bb9) SHA1(2dfb921e94ae7033a7182457b2f00657674d1b77))

		// ROM definition of PIO controller (8041A)
		// For the time being a specially developped PIO firmware is used until a dump of the original PIO is
		// available.
		ROM_REGION(0x400 , "iocpio" , 0)
		ROM_LOAD("pio_a72.bin" , 0 , 0x400 , BAD_DUMP CRC(8c8e740b))

		// ROM definition of keyboard controller (8741)
		ROM_REGION(0x400 , "kbcpu" , 0)
		ROM_LOAD("kbd511.bin" , 0 , 0x400 , CRC(ba7c4303) SHA1(19899af732d0ae1247bfc79979b1ee5f339ee5cf))
		// ROM definition of character generator (2708, A19 on IOC)
		ROM_REGION(0x400 , "gfx1" , 0)
		ROM_LOAD ("ioc_a19.bin" , 0x0000 , 0x0400 , CRC(47487d0f) SHA1(0ed98f9f06622949ee3cc2ffc572fb9702db0f81))
ROM_END

/*    YEAR  NAME       PARENT    COMPAT MACHINE INPUT     INIT              COMPANY       FULLNAME */
COMP( 1979, imds2,     0,        0,     imds2,  imds2,    driver_device, 0, "Intel",      "Intellec MDS-II" , 0)
