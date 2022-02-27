// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

IBM 6580 Displaywriter.

A green-screen dedicated word-processing workstation. It uses 8" floppy
disks. It could have up to 224k of ram.  Consists of:
    Electronics Module 6580
    Display 3300
    Keyboard 5330 [a "beamspring"-type]
    Diskette Unit 6360
Optional:
    Printers: 5215, 5218, 5228
    Printer Sharing feature
    Mag Card Unit
    Asynchronous and Bisynchronous communications features
    66-line display and adapter (800x1056 px, 8x16 character cell)


    All chips have IBM part numbers on them.  F.e. on system board:

    8493077 - 8086
    4178619 - 8251A
    4178617 - 8257-5
    4178623 - 8259A
    4178628 - 8255A-5
    4178625 - 8253-5


    Useful parts of PSM Feb83 (document / PDF page numbers):

    - 6-3/87 -- bus buffers ... separate ... into four sections during BAT
    - 6-5/89 -- irq levels
    - 6-6/90 -- timer tick is 50ms

    IRQ levels:

    0   incoming data for printer sharing/3277 DE
    1   transfer data to commo data link
    2   printer and mag card data xfer
    3   keyboard incoming data
    4   diskette
    5   (not in use)
    6   software timer [50 ms period]
    7   error on commo data link
    NMI "when a dump switch operation is initiated" ["memory record" button]


To do:

- verify all frequency sources, document ROM revisions
- memory size options
- bus errors, interrupts

- 92-key keyboard variant, keyboard click/beep, keyboard layouts

- 25-line video board (instant scroll, sub/superscripts, graphics mode)
- 66-line video board (apparently requires 'new' ROM)

- floppy adapter board, single and double density floppies
- "memory record" (system dump) generation to floppies
- printer
- speaker

- pass BAT with no errors (Basic Assurance Test)
- pass RNA with no errors (Resident Non-Automatic Test)
- pass PDD with no errors (Problem Determination Disk)
- pass CED with no errors (Customer Engineering Diagnostics)

- boot Textpack successfully (currently crashes with *90x* message)


Useful documents:

bitsavers://pdf/ibm/6580_Displaywriter/S241-6248-3_Displaywriter_Product_Support_Manual_Feb83.pdf
bitsavers://pdf/ibm/6580_Displaywriter/S241-6248-2_Displaywriter_6360_6580_Product_Support_Manual_May82.pdf
bitsavers://pdf/ibm/6580_Displaywriter/S241-6250-5_Displaywriter_6250_6580_Maintenance_Analysis_Procedures_May82.pdf
http://www.nostalgia8.nl/cpm/ibm/cpm6dwrm.pdf
http://www.kbdbabel.org/schematic/kbdbabel_doc_ibm_displaywriter.pdf
https://docs.google.com/spreadsheets/d/1SYY_HrBqKjSOX9W4fe5xUsjbfiCt0Umjpo4ZIwgG3Nk/edit?usp=sharing


Wanted:

Displaywriter System Manual S544-2023-0 (?) -- mentioned in US patents 4648071 and 5675827
"IBM Displaywriter System Printer Guide," Order No. S544-0861-2, Copyright 1980.
"Displaywriter System Product Support Manual," Order No. S241-6248-1, Copyright 1980


Notes on floppy drive:

    Diskette Unit 6360 models -010, -011 have Type 1D (SS/SD) drives;
    models -020, -021 -- Type 2D (DS/DD).  Each drive has "file control card".


    Useful parts of PSM Feb83 (pdf page numbers):

    - 6-7/91 -- brief description
    - 7-14..20/116..122 -- more detailed description
    - 8-13/161 -- S1 (internal diskette signal cable), system board side
    - 8-16/164 -- 5 (internal diskette signal cable), panel side
    - 8-32/180 -- B1 (diskette signal cable), diskette adapter side
    - 9-10..12/202..204 -- description of RNA tests


    S1 connector has more signals than B1.  Only these are present in B1:

    Address Bit 1-4 -- from host
    Data Bus Bit 0-7 -- bidirectional
    Interrupt 4 -- to host
    I/O Read -- from host
    I/O Write -- from host
    DMA Request -- to host
    DMA Acknowledge -- from host
    Terminal Count -- from host
    Interface Ready -- to host
    Diskette Select -- from host


    Signals between adapter board and file control cards:

    from/to 765:

    write data, inner tracks, write gate, erase gate ->
    write/erase enable, file data <-

    from/to MCU:

    drive present -> ground
    diskette sense <- always ground on 1D; 2D disk index hole sends +5V
    head engage ->
    index (1.5 to 3.0 ms pulse) <-
    select head, switch filter (2D drive) ->


    RNA test L performs in the following sequence:

    1. PORs the Diskette Adapter Cards
    2. Samples the Diskette Index Pulse
    3. Checks the Drive Set Ready Signal
    4. Engages the Read/Write Head
    5. Checks the Write/Erase Enable Line (This ensures the system will not write on the customer's diskette.)
    6. Reads the Track ID
    7. Disengages the Head.


****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ibm6580_kbd.h"
//nclude "machine/ibm6580_fdc.h"
#include "machine/ram.h"
#include "machine/upd765.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

#include "ibm6580.lh"


#define I8086_TAG       "i8086"
#define I8259A_TAG      "i8259"
#define I8255A_TAG      "i8255a"
#define I8253_TAG       "i8253"
#define UPD765_TAG      "upd765"


//#define LOG_GENERAL (1U <<  0) //defined in logmacro.h already
#define LOG_KEYBOARD  (1U <<  1)
#define LOG_DEBUG     (1U <<  2)

//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGKBD(...) LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


const uint8_t gfx_expand[16] = {
	0x00,   0x03,   0x0c,   0x0f,
	0x30,   0x33,   0x3c,   0x3f,
	0xc0,   0xc3,   0xcc,   0xcf,
	0xf0,   0xf3,   0xfc,   0xff
};


class ibm6580_state : public driver_device
{
public:
	ibm6580_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_ram(*this, RAM_TAG)
		, m_maincpu(*this, "maincpu")
		, m_pic8259(*this, "pic8259")
		, m_pit8253(*this, "pit8253")
		, m_ppi8255(*this, "ppi8255")
		, m_dma8257(*this, "dma8257")
		, m_screen(*this, "screen")
		, m_kbd(*this, "kbd")
		, m_p_chargen(*this, "chargen")
		, m_leds(*this, "led%u", 5U)
		, m_mcu(*this, "mcu")
		, m_mcuppi(*this, "mcuppi")
		, m_fdc(*this, UPD765_TAG)
		, m_drive(*this, UPD765_TAG ":%u", 0U)
	{ }

	void ibm6580(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void pic_latch_w(uint16_t data);
	void unk_latch_w(uint16_t data);

	void p40_w(offs_t offset, uint8_t data);
	uint8_t p40_r(offs_t offset);

	void gate_open_w(offs_t offset, uint8_t data);
	void gate_close_w(offs_t offset, uint8_t data);

	void video_w(offs_t offset, uint8_t data);
	uint8_t video_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(vblank_w);

	uint8_t kb_data_r();
	void led_w(uint8_t data);
	void ppi_c_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(kb_data_w);
	DECLARE_WRITE_LINE_MEMBER(kb_clock_w);
	DECLARE_WRITE_LINE_MEMBER(kb_clock_w_internal);
	DECLARE_WRITE_LINE_MEMBER(kb_strobe_w);

	void floppy_w(offs_t offset, uint8_t data);
	uint8_t floppy_r(offs_t offset);
	static void floppy_formats(format_registration &fr);
	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ibm6580_io(address_map &map);
	void ibm6580_mem(address_map &map);

	void mcu_io(address_map &map);
	void mcu_mem(address_map &map);

	uint16_t m_gate;
	uint8_t m_dma0pg;
	uint8_t m_p40, m_p4a, m_p50;
	uint8_t m_kb_data, m_ppi_c, m_led_state;
	bool m_e000;
	bool m_kb_data_bit, m_kb_strobe, m_kb_clock;

	uint8_t m_mcu_p1, m_mcu_p2;
	uint8_t m_mcuppi_a, m_mcuppi_b, m_mcuppi_c;

	void mcu_io_w(offs_t offset, uint8_t data);
	uint8_t mcu_io_r(offs_t offset);

	void mcuppi_c_w(uint8_t data);

	bool m_floppy_intrq, m_floppy_idle;
	uint8_t m_floppy_mcu_wrap;
	struct { floppy_image_device *image; bool hdl; } m_floppy[2];

	required_shared_ptr<uint16_t> m_p_videoram;
	required_device<ram_device> m_ram;
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259;
	required_device<pit8253_device> m_pit8253;
	required_device<i8255_device> m_ppi8255;
	required_device<i8257_device> m_dma8257;
	required_device<screen_device> m_screen;
	required_device<dw_keyboard_device> m_kbd;
	required_region_ptr<u8> m_p_chargen;
	output_finder<4> m_leds;
	required_device<i8048_device> m_mcu;
	required_device<i8255_device> m_mcuppi;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_drive;
};


void ibm6580_state::p40_w(offs_t offset, uint8_t data)
{
	LOG("___ %02x(%d) <- %02x\n", 0x40 + (offset << 1), offset, data);

	switch (offset)
	{
	case 0:
		m_p40 = data | 0x80;
		break;

	case 2:
		if (data)
			m_p40 |= 4;
		break;

	case 3:
		m_dma8257->dreq0_w(BIT(data, 0));
		m_dma8257->dreq1_w(BIT(data, 1));
		m_dma8257->dreq2_w(BIT(data, 2));
		break;

	case 4:
		m_dma8257->dreq3_w(BIT(data, 0));
		break;

	case 5:
		// write_gate0 doesn't work -- counter is read back as 0
		if (BIT(data, 2))
			// hack. video test checks timer counter value and this lets it pass.
			m_pit8253->set_clockin(0, (double)26880000);
		else
			m_pit8253->set_clockin(0, 0.0);
		m_p4a = data;
		m_p50 = 0;
		break;

	case 6:
		m_dma0pg = data;
		break;

	case 7:
		break;

	case 8:
		m_p50 = data;
		break;

	case 12:
		if (data)
			m_p40 &= ~0x14;
		break;
	}
}

uint8_t ibm6580_state::p40_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_p40;
		m_p40 &= ~4;
		break;

	case 8:
		data = m_p50;
		m_p50 = 1;
		break;
	}

	LOGDBG("___ %02x == %02x\n", 0x40 + (offset << 1), data);

	return data;
}

void ibm6580_state::gate_open_w(offs_t offset, uint8_t data)
{
	LOG("___ %02x(%d) <- %02x\n", 0x60 + (offset << 1), offset, data);

	m_gate |= (1 << offset);

	switch (offset)
	{
	case 10:
		m_kbd->reset_w(1);
		break;
	}
}

void ibm6580_state::gate_close_w(offs_t offset, uint8_t data)
{
	LOG("___ %04x(%d) <- %02x\n", 0x8060 + (offset << 1), offset, data);

	m_gate &= ~(1 << offset);

	switch (offset)
	{
	case 10:
		m_kbd->reset_w(0);
		break;
	}
}

void ibm6580_state::video_w(offs_t offset, uint8_t data)
{
	LOG("Video %02x <- %02x\n", 0xe000 + (offset << 1), data);

	switch (offset)
	{
	// some kind of gate
	case 2:
		m_e000 = true;
		break;

	case 4:
		m_e000 = false;
		break;
	}
}

uint8_t ibm6580_state::video_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 8:
		// 25-line video board ID.  66-line is 0x40.
		data = 1;
		// pure guesswork.  0x20 cannot be zero when 0x10 is zero.  0x2 is unknown, may be vsync.
		data |= (m_screen->hblank() ? 4 : 0);
		data |= (m_screen->vblank() ? 8 : 0);
		data |= ((m_screen->frame_number() & 1) ? 6 : 0);
		data |= ((m_screen->vpos() > 196) ? 0x80 : 0);
		if (m_e000) {
			data |= (m_screen->vblank() ? 0x20 : 0);
			data |= (m_screen->vblank() ? 0 : 0x10);
		}
		break;
	}

	if (offset != 8)
		LOG("Video %02x == %02x\n", 0xe000 + (offset << 1), data);

	return data;
}

WRITE_LINE_MEMBER(ibm6580_state::vblank_w)
{
//  if (state)
//      m_pic8259->ir6_w(state);

	m_p40 |= m_kbd->memory_record_r();
}

void ibm6580_state::pic_latch_w(uint16_t data)
{
	LOG("PIC latch <- %02x\n", data);

	if (data)
		m_p40 |= 8;

	m_pic8259->ir0_w(data == 2 ? ASSERT_LINE : CLEAR_LINE);
	m_pic8259->ir1_w(data == 2 ? ASSERT_LINE : CLEAR_LINE);
	m_pic8259->ir2_w(data == 2 ? ASSERT_LINE : CLEAR_LINE);
	m_pic8259->ir3_w(data == 2 ? ASSERT_LINE : CLEAR_LINE);
	m_pic8259->ir4_w(data == 2 ? ASSERT_LINE : CLEAR_LINE);
	m_pic8259->ir5_w(data == 2 ? ASSERT_LINE : CLEAR_LINE);
	m_pic8259->ir6_w(data == 2 ? ASSERT_LINE : CLEAR_LINE);
	m_pic8259->ir7_w(data == 2 ? ASSERT_LINE : CLEAR_LINE);
}

void ibm6580_state::unk_latch_w(uint16_t data)
{
	LOG("UNK latch <- %02x\n", data);

	m_p40 |= 0x10;
}

void ibm6580_state::led_w(uint8_t data)
{
	for (int i = 0; i < 4; i++)
		m_leds[i] = BIT(data, 7 - i);

	if (!BIT(m_p4a, 0))
	{
		kb_clock_w_internal(BIT(data, 1));
	}

	if (data & 0xf)
		return;

	if (data == m_led_state)
		return;

	m_led_state = data;

#ifdef VERBOSE
	switch (data >> 4)
	{
	case 0x1:
		printf ("LED 0 0001: Parity Generator/Checker\n");
		break;

	case 0xe:
		printf ("LED 0 1110: Base RAM\n");
		break;

	case 0x3:
		printf ("LED 0 0011: Processor Extension Test\n");
		break;

	case 0x4:
		printf ("LED 0 0100: Display RAM\n");
		break;

	case 0x5:
		printf ("LED 0 0101: Display Adapter Timing Test, Video Test\n");
		break;

	case 0x6:
		printf ("LED 0 0110: Keyboard Cable Test, Physical Keyboard Test\n");
		break;

	case 0x7:
		printf ("LED 0 0111: DMA Controller Test\n");
		break;

	case 0x8:
		printf ("LED 0 1000: Diskette Module Wrap Test, Adapter Test\n");
		break;

	case 0x9:
		printf ("LED 0 1001: Extra RAM Test\n");
		break;

	case 0xa:
		printf ("LED 0 1010: Bus Time-Out Test\n");
		break;

	case 0xc:
		printf ("LED 0 1100: RAM Addressability Test\n");
		break;

	default:
//      printf ("LED 0x%08x: unknown\n", data);
		break;
	}
#endif
}

void ibm6580_state::ppi_c_w(uint8_t data)
{
	uint8_t diff = m_ppi_c ^ data;

	LOGKBD("PPI Port C %02x <- %02x\n", m_ppi_c, data);

	m_ppi_c = data;

	// bit 3 -- mode 1 INTR.A out
	// bit 4 -- mode 1 INTE
	// bit 5 -- mode 1 IBF.A out
	// bit 6 -- I/O out = reset || to data input of keyboard shift register
	// bit 7 -- I/O out = invert bit 6

	// normal operation
	if (BIT(m_p4a, 0))
	{
		// Port A IBF bit
		m_kbd->ack_w(BIT(data, 5));

		// 0 = reset
		m_kbd->reset_w(BIT(data, 6));

		return;
	}

	// self-tests
	m_kb_data_bit = BIT(data, 6) ^ !BIT(m_ppi_c, 7);
	if (BIT(diff, 6)) m_ppi8255->pc4_w(!m_kb_data_bit);
}

uint8_t ibm6580_state::kb_data_r()
{
	uint8_t data = m_kb_data;

	LOGKBD("PPI Port A == %02x\n", data);

	return data;
}

WRITE_LINE_MEMBER(ibm6580_state::kb_data_w)
{
	if (!BIT(m_p4a, 0)) return;

	m_kb_data_bit = !state;
}

WRITE_LINE_MEMBER(ibm6580_state::kb_clock_w)
{
	if (!BIT(m_p4a, 0)) return;

	kb_clock_w_internal(state);
}

WRITE_LINE_MEMBER(ibm6580_state::kb_clock_w_internal)
{
	if (m_kb_clock == state) return;
	m_kb_clock = state;

	if (!state)
	{
		m_kb_data = (m_kb_data >> 1) | (m_kb_data_bit << 7);
		LOGKBD("Kbd clock %d data %d -> %02x\n", state, m_kb_data_bit, m_kb_data);
	}
}

WRITE_LINE_MEMBER(ibm6580_state::kb_strobe_w)
{
	if (!BIT(m_p4a, 0)) return;

	if (m_kb_strobe != state)
	LOGKBD("Kbd strobe %d data %02x\n", state, m_kb_data);
	m_kb_strobe = state;
	if (!state)
	{
		LOGKBD("Kbd enqueue %02x (m_ppi_c %02x)\n", m_kb_data, m_ppi_c);
	}
	m_ppi8255->pc4_w(m_kb_strobe);
}

WRITE_LINE_MEMBER(ibm6580_state::hrq_w)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dma8257->hlda_w(state);
}

uint8_t ibm6580_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset | (m_dma0pg << 16));
}

void ibm6580_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset | (m_dma0pg << 16), data);
}

// floppy adapter board

void ibm6580_state::mcu_io_w(offs_t offset, uint8_t data)
{
	LOGDBG("MCU IO %02x(%d) <- %02x\n", m_mcu_p1, m_mcu_p1 & 7, data);

	switch (m_mcu_p1 & 7)
	{
	case 0: case 1: case 2: case 3:
		m_mcuppi->write(m_mcu_p1, data);
		break;
	}
}

uint8_t ibm6580_state::mcu_io_r(offs_t offset)
{
	uint8_t data = 0;

	switch (m_mcu_p1 & 7)
	{
	case 0: case 1: case 2: case 3:
		data = m_mcuppi->read(m_mcu_p1);
		break;
	}

	LOGDBG("MCU IO %02x(%d) == %02x at %s\n", m_mcu_p1, m_mcu_p1 & 7, data, machine().describe_context());

	return data;
}

void ibm6580_state::mcuppi_c_w(uint8_t data)
{
	LOGDBG("MCU PPI PC <- %02x\n", data);
	m_mcuppi_c = data;
	m_mcu->set_input_line(MCS48_INPUT_IRQ, BIT(data, 3)); // invert?
}

// what happens on access to odd address ?
void ibm6580_state::floppy_w(offs_t offset, uint8_t data)
{
	LOG("Floppy %s %02x <- %02x\n", machine().describe_context(), 0x8150 + (offset << 1), data);

	switch (offset)
	{
	case 0: // 8150 -- mcu reset?
		m_mcu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		break;

	case 1: // 8152 -- fdc reset?  status not checked
		// m_fdc->soft_reset();
		break;

	case 2: // 8154
		if (BIT(m_gate, 14))
			m_floppy_mcu_wrap = data;
		break;

	case 5: // 815A
		m_fdc->fifo_w(data);
		break;

	case 6: // 815C
		m_mcuppi_a = data;
		m_mcuppi->pc4_w(1);
		m_mcuppi->pc4_w(0);
		break;
	}
}

uint8_t ibm6580_state::floppy_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0: // 8150
		// bit 4 -- ?? ready
		// bit 5 -- mcu busy
		// bit 6 -- ?? idle
		data = 8 | BIT(m_mcuppi_c, 7) << 6 | (m_mcuppi_c & 0x20);
		break;

	case 3: // 8156
		if (BIT(m_gate, 14))
			data = m_floppy_mcu_wrap << 2;
		break;

	case 4: // 8158
		data = m_fdc->msr_r();
		break;

	case 5: // 815a
		data = m_fdc->fifo_r();
		break;

	case 6: // 815c
		data = m_mcuppi->acka_r();
		break;
	}

	LOG("Floppy %s %02x == %02x\n", machine().describe_context(), 0x8150 + (offset << 1), data);

	return data;
}


void ibm6580_state::ibm6580_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x90000, 0x90001).w(FUNC(ibm6580_state::unk_latch_w));
	map(0xef000, 0xeffff).ram().share("videoram");  // 66-line vram starts at 0xec000
	map(0xfc000, 0xfffff).rom().region("user1", 0);
}

void ibm6580_state::ibm6580_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0007).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0008, 0x000f).w(FUNC(ibm6580_state::pic_latch_w));
	map(0x0010, 0x0017).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0020, 0x003f).rw(m_dma8257, FUNC(i8257_device::read), FUNC(i8257_device::write)).umask16(0x00ff);
	map(0x0040, 0x005f).rw(FUNC(ibm6580_state::p40_r), FUNC(ibm6580_state::p40_w)).umask16(0x00ff);
	map(0x0060, 0x007f).w(FUNC(ibm6580_state::gate_open_w)).umask16(0xff);
	map(0x0120, 0x0127).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x0140, 0x0143).rw("upd8251a", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x0160, 0x0163).rw("upd8251b", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x4000, 0x400f).unmaprw(); // bus error reporting?
	map(0x5000, 0x500f).unmaprw(); // bus error reporting?
	map(0x6000, 0x601f).unmaprw();
	map(0x8060, 0x807f).w(FUNC(ibm6580_state::gate_close_w)).umask16(0xff);
	map(0x8150, 0x815f).rw(FUNC(ibm6580_state::floppy_r), FUNC(ibm6580_state::floppy_w)).umask16(0x00ff);  // HLE of floppy board
	map(0x81a0, 0x81af).unmaprw();
	map(0xc000, 0xc00f).unmaprw();
	map(0xe000, 0xe02f).rw(FUNC(ibm6580_state::video_r), FUNC(ibm6580_state::video_w)).umask16(0x00ff);
}

void ibm6580_state::mcu_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x7ff).rom().region("mcu", 0);
}

void ibm6580_state::mcu_io(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(ibm6580_state::mcu_io_r), FUNC(ibm6580_state::mcu_io_w));
}


uint32_t ibm6580_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=25;

	uint8_t fg = 1, bg = 0;

	for (uint8_t y = 0; y < 25; y++)
	{
		for (uint8_t ra = 0; ra < 16; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			if (m_p_videoram[ma] & 0x100)
			{
				// graphics mode
				for (uint16_t x = ma; x < ma + 80; x++)
				{
					uint8_t const chr = m_p_videoram[x];
					uint8_t const attr = m_p_videoram[x] >> 8;
					uint8_t gfx;

					switch (ra >> 1)
					{
					case 0:
						gfx = gfx_expand[chr & 15];
						break;

					case 2:
						gfx = gfx_expand[chr >> 4];
						break;

					case 4:
						gfx = gfx_expand[attr & 15];
						break;

					case 6:
						gfx = gfx_expand[attr >> 4];
						break;

					default:
						gfx = 0;
						break;
					}

					/* Display a scanline of a character */
					for (int i = 7; i >= 0; i--)
					{
						*p++ = BIT(gfx, i) ? fg : bg;
					}
				}
			}
			else
			{
				// text mode
				for (uint16_t x = ma; x < ma + 80; x++)
				{
					uint8_t const chr = m_p_videoram[x];
					uint8_t const attr = m_p_videoram[x] >> 8;
					uint16_t ca = (chr<<4);
					uint8_t gfx;

					// font 2
					if (attr & 0x02)
						ca += 0x1000;
#if 0
					// superscript
					if (attr & 0x20)
						ca |= (ra < 13) ? ra + 3 : 0;
					// subscript
					if (attr & 0x40)
						ca |= (ra > 2) ? ra - 3 : 0;
#endif

					// underline
					if (attr & 0x08 && ra == 13)
						gfx = 0xff;
					else
						gfx = m_p_chargen[ca | ra];

					// cursor
					if (attr & 0x04 && ra == 14)
						gfx = 0xff;
					else
						gfx = m_p_chargen[ca | ra];

					// reverse video
					if (attr & 0x10)
						gfx ^= 255;

					// intense
					if (attr & 0x04)
						fg = 2;
					else
						fg = 1;

					/* Display a scanline of a character */
					for (int i = 7; i >= 0; i--)
					{
						*p++ = BIT(gfx, i) ? fg : bg;
					}
				}
			}
		}
		ma+=80;
	}
	return 0;
}


void ibm6580_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());

	m_fdc->set_rate(500000); // FIXME: workaround

	m_floppy[0].image = m_fdc->subdevice<floppy_connector>("0")->get_device();
	m_floppy[1].image = m_fdc->subdevice<floppy_connector>("1")->get_device();

	m_leds.resolve();

	memset(m_p_videoram, 0x0, 0x1000);
}

void ibm6580_state::machine_reset()
{
	m_p40 = m_p4a = m_p50 = m_gate = m_ppi_c = m_led_state = 0;
	m_mcu_p1 = m_mcu_p2 = 0;
	m_mcuppi_a = m_mcuppi_b = m_mcuppi_c = 0;
	m_e000 = false;
	m_kb_data_bit = false;
	m_kb_clock = false;
	m_kb_strobe = true;
	m_kb_data = 0;

	m_pit8253->set_clockin(0, 0.0);

	m_p40 |= m_kbd->memory_record_r();

	m_floppy[0].hdl = m_floppy[1].hdl = false;

	m_fdc->set_floppy(m_drive[0]->get_device());
}

static void dw_floppies(device_slot_interface &device)
{
	device.option_add("8sssd", IBM_6360);
}

void ibm6580_state::ibm6580(machine_config &config)
{
	I8086(config, m_maincpu, 14.7456_MHz_XTAL / 3); // XTAL is confirmed, divisor is not
	m_maincpu->set_addrmap(AS_PROGRAM, &ibm6580_state::ibm6580_mem);
	m_maincpu->set_addrmap(AS_IO, &ibm6580_state::ibm6580_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));

	// DMA tests need this
	config.set_perfect_quantum(m_maincpu);

	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("160K,192K,224K,256K,320K,384K");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_raw(25_MHz_XTAL / 2, 833, 0, 640, 428, 0, 400);
	m_screen->set_screen_update(FUNC(ibm6580_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set(FUNC(ibm6580_state::vblank_w));

	config.set_default_layout(layout_ibm6580);

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);

	PIC8259(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	I8255(config, m_ppi8255);
	m_ppi8255->in_pa_callback().set(FUNC(ibm6580_state::kb_data_r));
	m_ppi8255->out_pb_callback().set(FUNC(ibm6580_state::led_w));
	m_ppi8255->out_pc_callback().set(FUNC(ibm6580_state::ppi_c_w));
	m_ppi8255->tri_pa_callback().set_constant(0);
	m_ppi8255->tri_pc_callback().set_constant(0);

	PIT8253(config, m_pit8253, 0);
	m_pit8253->out_handler<0>().set([this] (int state) { m_p40 = (m_p40 & ~1) | state; });

	DW_KEYBOARD(config, m_kbd, 0);
	m_kbd->out_data_handler().set(FUNC(ibm6580_state::kb_data_w));
	m_kbd->out_clock_handler().set(FUNC(ibm6580_state::kb_clock_w));
	m_kbd->out_strobe_handler().set(FUNC(ibm6580_state::kb_strobe_w));

	I8257(config, m_dma8257, 14.7456_MHz_XTAL / 3);
	m_dma8257->out_hrq_cb().set(FUNC(ibm6580_state::hrq_w));
	m_dma8257->out_tc_cb().set(m_fdc, FUNC(upd765a_device::tc_line_w));
	m_dma8257->in_memr_cb().set(FUNC(ibm6580_state::memory_read_byte));
	m_dma8257->out_memw_cb().set(FUNC(ibm6580_state::memory_write_byte));
	m_dma8257->in_ior_cb<0>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dma8257->out_iow_cb<0>().set(m_fdc, FUNC(upd765a_device::dma_w));

	i8251_device &upd8251a(I8251(config, "upd8251a", 0));
	upd8251a.txd_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	upd8251a.dtr_handler().set("rs232a", FUNC(rs232_port_device::write_dtr));
	upd8251a.rts_handler().set("rs232a", FUNC(rs232_port_device::write_rts));
	upd8251a.rxrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	upd8251a.txrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir2_w));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set("upd8251a", FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set("upd8251a", FUNC(i8251_device::write_dsr));
	rs232a.cts_handler().set("upd8251a", FUNC(i8251_device::write_cts));

	i8251_device &upd8251b(I8251(config, "upd8251b", 0));
	upd8251b.txd_handler().set("rs232b", FUNC(rs232_port_device::write_txd));
	upd8251b.dtr_handler().set("rs232b", FUNC(rs232_port_device::write_dtr));
	upd8251b.rts_handler().set("rs232b", FUNC(rs232_port_device::write_rts));
	upd8251b.rxrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	upd8251b.txrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir2_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("upd8251b", FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set("upd8251b", FUNC(i8251_device::write_dsr));
	rs232b.cts_handler().set("upd8251b", FUNC(i8251_device::write_cts));

	// floppy adapter card (in the drive)

	I8048(config, m_mcu, 24_MHz_XTAL / 6); // XTAL divisor unknown
	m_mcu->set_addrmap(AS_PROGRAM, &ibm6580_state::mcu_mem);
	m_mcu->set_addrmap(AS_IO, &ibm6580_state::mcu_io);
	m_mcu->p1_in_cb().set([this] () {
		return (m_mcu_p1 & ~0x18) | \
			(!m_floppy[0].image->idx_r() << 3) | (!m_floppy[1].image->idx_r() << 4);
	});
	m_mcu->p1_out_cb().set([this] (uint8_t data) { m_mcu_p1 = data; });

	I8255(config, m_mcuppi);
	m_mcuppi->in_pa_callback().set([this] () { LOGDBG("MCU PPI read A: %02X\n", m_mcuppi_a); return m_mcuppi_a; });
	m_mcuppi->out_pb_callback().set([this] (uint8_t data) { m_mcuppi_b = data; });
	m_mcuppi->out_pc_callback().set(FUNC(ibm6580_state::mcuppi_c_w));

	// NEC D765D.   READY and SELECT likely not connected.   SEEK, STEP and DIR likely connected to MCU.
	UPD765A(config, m_fdc, 24_MHz_XTAL / 3, false, false);
	m_fdc->intrq_wr_callback().set([this] (bool state) { m_floppy_intrq = state; });
	m_fdc->drq_wr_callback().set(m_dma8257, FUNC(i8257_device::dreq0_w));
	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", dw_floppies, "8sssd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":1", dw_floppies, "8sssd", floppy_image_device::default_mfm_floppy_formats);

	SOFTWARE_LIST(config, "flop_list").set_original("ibm6580");
}

/* ROM definition */
ROM_START( ibm6580 )
	ROM_REGION16_LE( 0x4000, "user1", 0 )
	ROM_DEFAULT_BIOS("old")

	ROM_SYSTEM_BIOS(0, "old", "old bios - 1981")
	ROMX_LOAD("8493823_8k.bin", 0x0001, 0x2000, CRC(aa5524c0) SHA1(9938f2a82828b17966cb0be7fdbf73803c1f10d3), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("8493822_8k.bin", 0x0000, 0x2000, CRC(90e7e73a) SHA1(d3ee7a4d2cb8f4920b5d95e8c7f4fef06599d24e), ROM_SKIP(1) | ROM_BIOS(0))

	// disable halts in video test
	ROM_FILL(0x501,1,0x90)
	ROM_FILL(0x51b,1,0x90)
	ROM_FILL(0x52f,1,0x90)
	ROM_FILL(0x587,1,0x90)
	ROM_FILL(0x5a0,1,0x90)
	ROM_FILL(0x5a6,1,0x90)
	ROM_FILL(0x5f4,1,0x90)
	ROM_FILL(0x609,1,0x90)

	// disable rom checksum halt
	ROM_FILL(0x2027,1,0x01)

	ROM_SYSTEM_BIOS(1, "new", "new bios - 1983?")
	// was downloaded via DDT86
	ROMX_LOAD( "dwrom16kb.bin", 0x0000, 0x4000, BAD_DUMP CRC(ced87929) SHA1(907a46f288809bc93a1f59f3fbef18bd44be42d9), ROM_BIOS(1))

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "8493383_chr.bin", 0x0000, 0x2000, CRC(779044df) SHA1(95ec46f9edf4d44c5dd3c955c73e00754d58e180))

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "4430030_flp_8041.bin", 0x0000, 0x0400, CRC(2bb96799) SHA1(e30b0f2d790197f290858eab74ad5e151ded78c3))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS          INIT        COMPANY  FULLNAME                  FLAGS */
COMP( 1980, ibm6580, 0,      0,      ibm6580, 0,     ibm6580_state, empty_init, "IBM",   "IBM 6580 Displaywriter", MACHINE_IS_SKELETON)
