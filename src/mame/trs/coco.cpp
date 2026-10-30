// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco.cpp

    TRS-80 Radio Shack Color Computer Family

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

  References:
        There are two main references for the info for this driver
        - Tandy Color Computer Unravelled Series
                    (https://colorcomputerarchive.com/search?q=unravelled)
        - Assembly Language Programming For the CoCo 3 by Laurence A. Tepolt
        - Kevin K. Darlings GIME reference
                    (http://www.cris.com/~Alxevans/gime.txt)
        - Sock Masters's GIME register reference
        			(https://www.6809.org.uk/twilight/sock/gime.html)
                    (http://www.axess.com/twilight/sock/gime.html)
        - Robert Gault's FAQ
                    (https://web.archive.org/web/http://home.att.net/~robert.gault/Coco/FAQ/FAQ_main.htm)
        - Discussions with L. Curtis Boyle (LCB) and John Kowalski (JK)

  TODO:
        - Implement unimplemented SAM registers
        - Choose and implement more appropriate ratios for the speed up poke

  In the CoCo, all timings should be exactly relative to each other.  This
  table shows how all clocks are relative to each other (info: JK):
        - Main CPU Clock                0.89 MHz
        - Horizontal Sync Interrupt     15.7 kHz/63.5us (57 clock cycles)
        - Vertical Sync Interrupt       60 Hz           (14934 clock cycles)
        - Composite Video Color Carrier 3.58 MHz/279ns  (1/4 clock cycles)

  CPU Interrupts:

  The CoCo/Dragon have two PIAs. These PIAs can trigger interrupts.  PIA0
  is set up to trigger IRQ on the CPU, and PIA1 can trigger FIRQ.  Each PIA
  has two output lines, and an interrupt will be triggered if either of these
  lines are asserted.

  -----  IRQ
  6809 |-<----------- PIA0
       |
       |
       |
       |
       | FIRQ
       |-<----------- PIA1
  -----

  Sound / Keyboard / Joystick:

  The sound MUX (mc14529) has 4 possible settings, depend on SELA and SELB inputs:

  Channel Z:
    00    - Horizontal potentimeter of right joystick
    01    - Vertical potentimeter of right joystick
    10    - Horizontal potentimeter of left joystick
    11    - Vertical potentimeter of left joystick

  Channel W:
    00    - DAC (digital - analog converter)
    01    - CSN (cassette)
    10    - SND input from cartridge
    11    - Grounded (0)

  Source - Tandy Color Computer Service Manual

  Note on the Dragon Alpha state 11, selects the AY-3-8912, this is currently
  un-implemented - phs.

***************************************************************************/

#include "emu.h"
#include "coco.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"



//**************************************************************************
//  coco_state
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

coco_state::coco_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, MAINCPU_TAG),
	m_pia_0(*this, "pia0"),
	m_pia_1(*this, "pia1"),
	m_mux(*this, "mux"),
	m_dac(*this, "dac"),
	m_sbs(*this, "sbs"),
	m_screen(*this, "screen"),
	m_cococart(*this, "ext"),
	m_ram(*this, RAM_TAG),
	m_cassette(*this, "cassette"),
	m_floating(*this, "floating"),
	m_rs232(*this, RS232_TAG),
	m_vhd_0(*this, "vhd0"),
	m_vhd_1(*this, "vhd1"),
	m_beckerport(*this, "dwsock"),
	m_beckerportconfig(*this, BECKERPORT_TAG),
	m_irqs(*this, "irqs"),
	m_firqs(*this, "firqs"),
	m_joystick_ports(*this, {JOYSTICK_RX_TAG, JOYSTICK_RY_TAG, JOYSTICK_LX_TAG, JOYSTICK_LY_TAG}),
	m_keyboard(*this, "row%u", 0),
	m_joystick_type_right(*this, CTRL_SEL_RIGHT),
	m_joystick_type_left(*this, CTRL_SEL_LEFT),
	m_in_floating_bus_read(false)
{
}


//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void coco_state::machine_start()
{
	// timer used by joystick system
	m_joy_timer = timer_alloc(FUNC(coco_state::joy_timer_callback), this);

	// cart slot
	m_cococart->set_cart_base_update(cococart_base_update_delegate(&coco_state::update_cart_base, this));
	// 12 allowed one more instruction to finished after the line is pulled
	m_cococart->set_line_delay(cococart_slot_device::line::NMI, 12);
	// 6 allowed one more instruction to finished after the line is pulled
	m_cococart->set_line_delay(cococart_slot_device::line::HALT, 6);

	// miscellaneous
	m_in_floating_bus_read = false;

	// save state support
	save_item(NAME(m_dac_output));
	save_item(NAME(m_pia0_pa_buffer));
	save_item(NAME(m_pia0_pb_buffer));
	save_item(NAME(m_vhd_select));
	save_item(NAME(m_in_floating_bus_read));
}



//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void coco_state::machine_reset()
{
	/* reset state */
	m_dac_output = 0;
	m_pia0_pa_buffer = 0;
	m_vhd_select = 0;
}



//-------------------------------------------------
//  floating_bus_read
//-------------------------------------------------
//
// From Darren A on the CoCo list:
//
// Whenever you read from an un-mapped hardware address or from an
// address where some of the bits are undefined (like the GIME's palette
// and MMU registers), the value obtained for those undefined bits is
// predictable on the CoCo.
//
// There are two possibilites which depend on the addressing mode you use
// to read from such an address.  If you use the "no-offset" indexed mode
// (as in LDA ,X) the undefined bits will come from the first byte of the
// next instruction.  For any other addressing mode, the undefined bits
// will come from the byte at $FFFF (LSB of the Reset vector).
//
// When you use BASIC's PEEK command to read from an un-mapped address
// (such as $FF70), you get a value of 126.  This is because PEEK reads
// the address with a LDA ,X instruction, so the value returned is the
// opcode of the next instruction (JMP Extended = $7E = 126).  If you
// patch the PEEK command to use a 5-bit offset (LDA 0,X), the value
// returned for an un-mapped address will instead come from $FFFF (27 on
// a CoCo 3 or 39 on a CoCo 1/2).
//
// The reason for this behavior is that the 6809 normally does a VMA
// cycle just before reading the instruction's effective address. The
// exception is the "no-offset" indexed mode in which case the next
// instruction byte is read just prior to the effective address.  During
// a VMA cycle the address bus goes to Hi Impedance and the R/W line is
// HI.  This has the effect of loading the value from $FFFF onto the data
// bus.  This stale data from the previous cycle supplies the value for
// the undefined bits.
//
// Here is a small routine which will demonstrate this behavior:
//
//   ldx   #$FF70
//   lda   ,x
//   ldb   $FF70
//   std   $400
//   rts
//
// On a CoCo 3, you should end up with the value $F61B at $400-401.  On a
// CoCo 1/2 you should get $F627 instead.
//-------------------------------------------------

uint8_t coco_state::floating_bus_read()
{
	uint8_t result;

	// this method calls program.read_byte() - therefore we run the risk of a stack overflow if we don't check for
	// a reentrant invocation
	if (m_in_floating_bus_read)
	{
		// not sure what should really happen in this extremely degenerate scenario (the PC is probably
		// in $FFxx never-never land), but I guess 0xFF is as good as anything.
		result = 0xFF;
	}
	else
	{
		// prevent stack overflows
		m_in_floating_bus_read = true;

		// get the previous and current PC
		uint16_t prev_pc = m_maincpu->pcbase();
		uint16_t pc = m_maincpu->pc();

		// get the byte; and skip over header bytes
		uint8_t byte = m_maincpu->space().read_byte(prev_pc);
		if ((byte == 0x10) || (byte == 0x11))
			byte = m_maincpu->space().read_byte(++prev_pc);

		// check to see if the opcode specifies the indexed addressing mode, and the secondary byte
		// specifies no-offset
		bool is_nooffset_indexed = (((byte & 0xF0) == 0x60) || ((byte & 0xF0) == 0xA0) || ((byte & 0xF0) == 0xE0))
			&& ((m_maincpu->space().read_byte(prev_pc + 1) & 0xBF) == 0x84);

		// finally read the byte
		result = m_maincpu->space().read_byte(is_nooffset_indexed ? pc : 0xFFFF);

		// we're done reading
		m_in_floating_bus_read = false;
	}
	return result;
}



//-------------------------------------------------
//  floating_space_read
//-------------------------------------------------

uint8_t coco_state::floating_space_read(offs_t offset)
{
	// The "floating space" is intended to be a catch all for address space
	// not handled by the normal CoCo infrastructure, but may be read directly
	// by cartridge hardware and other miscellany
	//
	// Most of the time, the read below will result in floating_bus_read() being
	// invoked
	return m_floating->read8(offset);
}



//-------------------------------------------------
//  floating_space_write
//-------------------------------------------------

void coco_state::floating_space_write(offs_t offset, uint8_t data)
{
	m_floating->write8(offset, data);
}



/***************************************************************************
  PIA0 ($FF00-$FF1F) (Chip U8)

  PIA0 PA7  	- Joystick comparator read
  PIA0 PA0-PA6  - Keyboard read
  PIA0 PB0-PB7  - Keyboard write
  PIA0 CA1      - MC6847 HS (Horizontal Sync)
  PIA0 CA2      - SEL1 (Used by sound mux and joystick)
  PIA0 CB1      - MC6847 FS (Field Sync)
  PIA0 CB2      - SEL2 (Used by sound mux and joystick)
***************************************************************************/

//-------------------------------------------------
//  pia0_pa_w
//-------------------------------------------------

void coco_state::pia0_pa_w(uint8_t value)
{
	LOG("%s pia0_pa_w value: %02x (%11.6f)\n", machine().describe_context(), value, machine().time().as_double());

	uint8_t mux_addr = m_mux->current_address();
	int joy_port = BIT(mux_addr, 1);
	if (auto cur_joy = dynamic_cast<coco_cm3_hires_joy*>(m_joy_handlers[joy_port].get()))
	{
		cur_joy->hires_trigger(value & 0x0f, machine().time(), BIT(mux_addr, 0), current_joystick_value(mux_addr));
	}

	m_pia0_pa_buffer = value;
	refresh_keyboard_matrix();
}



//-------------------------------------------------
//  current_joystick_value
//-------------------------------------------------

int coco_state::current_joystick_value(uint8_t mux_addr)
{
	return m_joystick_ports[mux_addr & 3]->read();
}



//-------------------------------------------------
//  pia0_pb_w
//-------------------------------------------------

void coco_state::pia0_pb_w(uint8_t value)
{
	m_pia0_pb_buffer = value;
	refresh_keyboard_matrix();
}


//-------------------------------------------------
//  joystick_button_changed
//-------------------------------------------------

void coco_state::joystick_button_changed(ioport_field &field, u32 param, ioport_value oldval, ioport_value newval)
{
	refresh_keyboard_matrix();
}


//-------------------------------------------------
//  refresh_keyboard_matrix
//-------------------------------------------------

void coco_state::refresh_keyboard_matrix()
{
	uint8_t gated_buttons = m_joy_handlers[0]->button_status() | m_joy_handlers[1]->button_status();

	// Effective PA output driven low by CPU or joystick fire buttons
	uint8_t effective_pa = m_pia0_pa_buffer & ~gated_buttons;

	uint8_t calculated_pb_in = 0xFF; // PB input state returned to CPU
	uint8_t calculated_pa_in = 0x7F; // PA input state returned to CPU
	uint8_t any_pressed = 0;

	// Single pass across all 7 keyboard matrix rows
	for (unsigned i = 0; i < m_keyboard.size(); i++)
	{
		uint8_t key_column = m_keyboard[i]->read();
		any_pressed |= ~key_column;

		// Forward: PA driving -> PB reading
		if (!(effective_pa & (0x01 << i)))
		{
			calculated_pb_in &= key_column;
		}

		// Reverse: PB driving -> PA reading
		if ((key_column | m_pia0_pb_buffer) != 0xFF)
		{
			calculated_pa_in &= ~(0x01 << i);
		}
	}

	// Apply joystick interference to PA input lines
	calculated_pa_in &= ~gated_buttons;
	any_pressed |= gated_buttons;

	// Update Port A buffer (preserving top bit) and feed back to PIA inputs
	m_pia0_pa_buffer = (m_pia0_pa_buffer & ~0x7F) | (calculated_pa_in & 0x7F);
	m_pia_0->set_a_input(m_pia0_pa_buffer);

	// Feed calculated input pin levels to PIA Port B
	m_pia_0->portb_w(calculated_pb_in);

	// Notify CoCo 3 GIME hook (virtual call overridden only in coco3_state)
	on_keyboard_state_changed(any_pressed != 0);
}



//-------------------------------------------------
//  pia0_pa7_w - comparator output
//-------------------------------------------------

void coco_state::pia0_pa7_w(uint8_t value)
{
	uint8_t mux_addr = m_mux->current_address();
	int joy_port = BIT(mux_addr, 1);

	if (auto cur_joy = dynamic_cast<coco_cm3_hires_joy*>(m_joy_handlers[joy_port].get()))
	{
		LOG("%s pia0_pa7_w value: %02x (cm3) (%11.6f)\n", machine().describe_context(), value, machine().time().as_double());
		cur_joy->hires_trigger(m_pia_0->a_output() & 0x0f, machine().time(), BIT(mux_addr, 0), current_joystick_value(mux_addr));
	}
	else if (auto cur_joy = dynamic_cast<coco_tandy_hires_joy*>(m_joy_handlers[joy_port].get()))
	{
		LOG("%s pia0_pa7_w value: %02x (tandy) (%11.6f)\n", machine().describe_context(), value, machine().time().as_double());
		cur_joy->hires_trigger(m_dac_output, machine().time(), BIT(mux_addr, 0), current_joystick_value(mux_addr));
	}

	bool result = m_joy_handlers[joy_port]->evaluate_comparator(m_dac_output, value);

	m_pia0_pa_buffer = (m_pia0_pa_buffer & ~0x80) | (result ? 0x80 : 0);
	m_pia_0->set_a_input(m_pia0_pa_buffer);
	LOG("%s pia0_pa7_w, setting comparator: %d (%11.6f)\n", machine().describe_context(), result, machine().time().as_double());
}



/***************************************************************************
  PIA1 ($FF20-$FF3F) (Chip U4)

  PIA1 PA0      - CASSDIN
  PIA1 PA1      - RS232 OUT (CoCo), Printer Strobe (Dragon)
  PIA1 PA2-PA7  - DAC
  PIA1 PB0      - RS232 IN
  PIA1 PB1      - Single bit sound
  PIA1 PB2      - RAMSZ (32/64K, 16K, and 4K three position switch)
  PIA1 PB3      - M6847 CSS
  PIA1 PB4      - M6847 INT/EXT and M6847 GM0
  PIA1 PB5      - M6847 GM1
  PIA1 PB6      - M6847 GM2
  PIA1 PB7      - M6847 A/G
  PIA1 CA1      - CD (Carrier Detect; NYI)
  PIA1 CA2      - CASSMOT (Cassette Motor)
  PIA1 CB1      - CART (Cartridge Detect)
  PIA1 CB2      - SNDEN (Sound Enable)
***************************************************************************/

//-------------------------------------------------
//  pia1_pa_r
//-------------------------------------------------

uint8_t coco_state::pia1_pa_r()
{
	// Port A: we need to specify the values of all the lines, regardless of whether
	// they are in input or output mode in the DDR
	return (m_cassette->input() >= 0 ? 0x01 : 0x00) | 0xfe;
}



//-------------------------------------------------
//  pia1_pb_r - this handles the reading of the
//  memory sense switch (PB2) for the CoCo 1 and
//  serial-in (PB0)
//-------------------------------------------------

uint8_t coco_state::pia1_pb_r()
{
	// Port B: lines in output mode are handled automatically by the PIA object.
	// We only need to specify the input lines here
	uint32_t ram_size = m_ram->size();

	//  For the CoCo 1, the logic has been changed to only select 64K rams
	//  if there is more than 16K of memory, as the Color Basic 1.0 rom
	//  can only configure 4K or 16K ram banks (as documented in "Color
	//  Basic Unreveled"), doing this allows this  allows the coco driver
	//  to access 32K of ram, and also allows the cocoe driver to access
	//  the full 64K, as this uses Color Basic 1.2, which can configure 64K rams
	bool memory_sense = (ram_size >= 0x4000 && ram_size <= 0x7fff)
		|| (ram_size >= 0x8000 && (m_pia_0->b_output() & 0x40));

	// serial in (PB0)
	bool serial_in = (m_rs232 != nullptr) && (m_rs232->rxd_r() ? true : false);

	// composite the results
	return (memory_sense ? 0x04 : 0x00)
		| (serial_in ? 0x01 : 0x00);
}



//-------------------------------------------------
//  ff20_write
//-------------------------------------------------

void coco_state::ff20_write(offs_t offset, uint8_t data)
{
	/* write to the PIA */
	m_pia_1->write(offset, data);

	/* we have to do this to do something that approximates the cartridge Q line behavior */
	m_cococart->twiddle_q_lines();
}



//-------------------------------------------------
//  pia1_pa_w
//-------------------------------------------------

void coco_state::pia1_pa_w(uint8_t data)
{
	m_dac_output = data >> 2;
	m_dac->write(m_dac_output);

	LOG("%s pia1_pa_w: dac: %d (%11.6f)\n", machine().describe_context(), m_dac_output, machine().time().as_double());

	m_cassette->output((m_dac_output - 0x20) / 32.0);

	// Update joystick comparator & trigger handlers
	pia0_pa7_w(m_mux->zx_value());

	// special case joystick handling
	uint8_t mux_addr = m_mux->current_address();
	int joy_port = BIT(mux_addr, 1);
	if (auto cur_joy = dynamic_cast<coco_cm3_hires_joy*>(m_joy_handlers[joy_port].get()))
	{
		LOG("%s pia1_pa_w (cm3) (%11.6f)\n", machine().describe_context(), machine().time().as_double());
		cur_joy->hires_trigger(m_pia_0->a_output() & 0x0f, machine().time(), BIT(mux_addr, 0), current_joystick_value(mux_addr));
	}
	else if (auto cur_joy = dynamic_cast<coco_tandy_hires_joy*>(m_joy_handlers[joy_port].get()))
	{
		LOG("%s pia1_pa_w (tandy) (%11.6f)\n", machine().describe_context(), machine().time().as_double());
		cur_joy->hires_trigger(m_dac_output, machine().time(), BIT(mux_addr, 0), current_joystick_value(mux_addr));
	}

	int serial_bit = BIT(data, 1);

	if (auto* lightgun = dynamic_cast<coco_diecom_light_gun*>(m_joy_handlers[0].get()))
	{
		lightgun->lightgun_clock(serial_bit);
	}
	else if (m_rs232 != nullptr)
	{
		m_rs232->write_txd(serial_bit);
	}
}



//-------------------------------------------------
//  pia1_pb_w
//-------------------------------------------------

void coco_state::pia1_pb_w(uint8_t data)
{
	/* PB1 will drive the 1 bit sound output.  This is a rarely
	* used single bit sound mode. It is always connected thus
	* cannot be disabled.
	*
	* Source:  Page 31 of the Tandy Color Computer Serice Manual
	*/

	m_sbs->write(BIT(data, 1));
}



/***************************************************************************
  OTHER I/O SPACE
 ***************************************************************************/

//-------------------------------------------------
//  ff40_write
//-------------------------------------------------

void coco_state::ff40_write(offs_t offset, uint8_t data)
{
	if (offset >= 1 && offset <= 2 && m_beckerportconfig.read_safe(0) == 1)
	{
		return m_beckerport->write(offset-1, data);
	}

	m_cococart->scs_write(offset, data);
}


//-------------------------------------------------
//  ff40_read
//-------------------------------------------------

uint8_t coco_state::ff40_read(offs_t offset)
{
	if (offset >= 1 && offset <= 2 && m_beckerportconfig.read_safe(0) == 1)
	{
		return m_beckerport->read(offset-1);
	}

	return m_cococart->scs_read(offset);
}



//-------------------------------------------------
//  ff60_read
//-------------------------------------------------

uint8_t coco_state::ff60_read(offs_t offset)
{
	uint8_t result;

	if ((current_vhd() != nullptr) && (offset >= 32) && (offset <= 37))
	{
		result = current_vhd()->read(offset - 32);
	}
	else
	{
		result = floating_space_read(0xff60 + offset);
	}

	return result;
}



//-------------------------------------------------
//  ff60_write
//-------------------------------------------------

void coco_state::ff60_write(offs_t offset, uint8_t data)
{
	if ((current_vhd() != nullptr) && (offset >= 32) && (offset <= 37))
	{
		current_vhd()->write(offset - 32, data);
	}
	else if (offset == 38)
	{
		/* writes to $FF86 will switch the VHD */
		m_vhd_select = data;
	}
	else
	{
		floating_space_write(0xff60 + offset, data);
	}
}



/***************************************************************************
  VHD
 ***************************************************************************/

//-------------------------------------------------
//  current_vhd
//-------------------------------------------------

coco_vhd_image_device *coco_state::current_vhd()
{
	switch(m_vhd_select)
	{
		case 0:     return m_vhd_0;
		case 1:     return m_vhd_1;
		default:
			osd_printf_warning("Unknown Color Computer virtual hard disk index.\n");
			return nullptr;
	}
}



/***************************************************************************
  CARTRIDGE HANDLING
 ***************************************************************************/

//-------------------------------------------------
//  cart_w
//-------------------------------------------------

void coco_state::cart_w(bool state)
{
	m_pia_1->cb1_w(state);
}



//-------------------------------------------------
//  cartridge_space
//-------------------------------------------------

address_space &coco_state::cartridge_space()
{
	return m_floating->space(0);
}



//-------------------------------------------------
//  add_sound_route
//-------------------------------------------------

void coco_state::add_sound_route(device_sound_interface &sound_device, int output_index, double gain)
{
	sound_device.add_route(output_index, *m_mux, gain, mc14529_device::y_sound_input(2));
}



//-------------------------------------------------
//  add_sound_route
//-------------------------------------------------

void coco_state::set_sound_gain(device_sound_interface &sound_device, int output_index, double gain)
{
	sound_device.set_route_gain(output_index, m_mux, gain, mc14529_device::y_sound_input(2));
}


/***************************************************************************
  DISASSEMBLY OVERRIDE (OS9 syscalls)
 ***************************************************************************/

static const char *const os9syscalls[] =
{
	"F$Link",          // Link to Module
	"F$Load",          // Load Module from File
	"F$UnLink",        // Unlink Module
	"F$Fork",          // Start New Process
	"F$Wait",          // Wait for Child Process to Die
	"F$Chain",         // Chain Process to New Module
	"F$Exit",          // Terminate Process
	"F$Mem",           // Set Memory Size
	"F$Send",          // Send Signal to Process
	"F$Icpt",          // Set Signal Intercept
	"F$Sleep",         // Suspend Process
	"F$SSpd",          // Suspend Process
	"F$ID",            // Return Process ID
	"F$SPrior",        // Set Process Priority
	"F$SSWI",          // Set Software Interrupt
	"F$PErr",          // Print Error
	"F$PrsNam",        // Parse Pathlist Name
	"F$CmpNam",        // Compare Two Names
	"F$SchBit",        // Search Bit Map
	"F$AllBit",        // Allocate in Bit Map
	"F$DelBit",        // Deallocate in Bit Map
	"F$Time",          // Get Current Time
	"F$STime",         // Set Current Time
	"F$CRC",           // Generate CRC
	"F$GPrDsc",        // get Process Descriptor copy
	"F$GBlkMp",        // get System Block Map copy
	"F$GModDr",        // get Module Directory copy
	"F$CpyMem",        // Copy External Memory
	"F$SUser",         // Set User ID number
	"F$UnLoad",        // Unlink Module by name
	"F$Alarm",         // Color Computer Alarm Call (system wide)
	nullptr,
	nullptr,
	"F$NMLink",        // Color Computer NonMapping Link
	"F$NMLoad",        // Color Computer NonMapping Load
	nullptr,
	nullptr,
	"F$TPS",           // Return System's Ticks Per Second
	"F$TimAlm",        // COCO individual process alarm call
	"F$VIRQ",          // Install/Delete Virtual IRQ
	"F$SRqMem",        // System Memory Request
	"F$SRtMem",        // System Memory Return
	"F$IRQ",           // Enter IRQ Polling Table
	"F$IOQu",          // Enter I/O Queue
	"F$AProc",         // Enter Active Process Queue
	"F$NProc",         // Start Next Process
	"F$VModul",        // Validate Module
	"F$Find64",        // Find Process/Path Descriptor
	"F$All64",         // Allocate Process/Path Descriptor
	"F$Ret64",         // Return Process/Path Descriptor
	"F$SSvc",          // Service Request Table Initialization
	"F$IODel",         // Delete I/O Module
	"F$SLink",         // System Link
	"F$Boot",          // Bootstrap System
	"F$BtMem",         // Bootstrap Memory Request
	"F$GProcP",        // Get Process ptr
	"F$Move",          // Move Data (low bound first)
	"F$AllRAM",        // Allocate RAM blocks
	"F$AllImg",        // Allocate Image RAM blocks
	"F$DelImg",        // Deallocate Image RAM blocks
	"F$SetImg",        // Set Process DAT Image
	"F$FreeLB",        // Get Free Low Block
	"F$FreeHB",        // Get Free High Block
	"F$AllTsk",        // Allocate Process Task number
	"F$DelTsk",        // Deallocate Process Task number
	"F$SetTsk",        // Set Process Task DAT registers
	"F$ResTsk",        // Reserve Task number
	"F$RelTsk",        // Release Task number
	"F$DATLog",        // Convert DAT Block/Offset to Logical
	"F$DATTmp",        // Make temporary DAT image (Obsolete)
	"F$LDAXY",         // Load A [X,[Y]]
	"F$LDAXYP",        // Load A [X+,[Y]]
	"F$LDDDXY",        // Load D [D+X,[Y]]
	"F$LDABX",         // Load A from 0,X in task B
	"F$STABX",         // Store A at 0,X in task B
	"F$AllPrc",        // Allocate Process Descriptor
	"F$DelPrc",        // Deallocate Process Descriptor
	"F$ELink",         // Link using Module Directory Entry
	"F$FModul",        // Find Module Directory Entry
	"F$MapBlk",        // Map Specific Block
	"F$ClrBlk",        // Clear Specific Block
	"F$DelRAM",        // Deallocate RAM blocks
	"F$GCMDir",        // Pack module directory
	"F$AlHRam",        // Allocate HIGH RAM Blocks
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"F$RegDmp",        // Ron Lammardo's debugging register dump call
	"F$NVRAM",         // Non Volatile RAM (RTC battery backed static) read/write
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"I$Attach",        // Attach I/O Device
	"I$Detach",        // Detach I/O Device
	"I$Dup",           // Duplicate Path
	"I$Create",        // Create New File
	"I$Open",          // Open Existing File
	"I$MakDir",        // Make Directory File
	"I$ChgDir",        // Change Default Directory
	"I$Delete",        // Delete File
	"I$Seek",          // Change Current Position
	"I$Read",          // Read Data
	"I$Write",         // Write Data
	"I$ReadLn",        // Read Line of ASCII Data
	"I$WritLn",        // Write Line of ASCII Data
	"I$GetStt",        // Get Path Status
	"I$SetStt",        // Set Path Status
	"I$Close",         // Close Path
	"I$DeletX"         // Delete from current exec dir
};



//-------------------------------------------------
//  dasm_override
//-------------------------------------------------

offs_t coco_state::dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	return os9_dasm_override(stream, pc, opcodes, params);
}



//-------------------------------------------------
//  os9_dasm_override
//-------------------------------------------------

offs_t coco_state::os9_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	unsigned call;
	offs_t result = 0;

	// Microware OS-9 (on the CoCo) and a number of other 6x09 based systems used the SWI2
	// instruction for syscalls.  This checks for a SWI2 and looks up the syscall as appropriate
	if ((opcodes.r8(pc) == 0x10) && (opcodes.r8(pc+1) == 0x3f))
	{
		call = opcodes.r8(pc+2);
		if ((call < std::size(os9syscalls)) && (os9syscalls[call] != nullptr))
		{
			util::stream_format(stream, "OS9   %s", os9syscalls[call]);
			result = 3;
		}
	}
	return result;
}



/***************************************************************************
  Color Computer Joystick Abstraction
 ***************************************************************************/

//-------------------------------------------------
//  joystick_mode_changed - change notification
//  from the port system
//-------------------------------------------------

void coco_state::joystick_mode_changed(ioport_field &field, u32 param, ioport_value oldval, ioport_value newval)
{
	// param == 0 is Right (Port 0), param == 1 is Left (Port 1)
	int port = BIT(param, 0);
	uint8_t selection = newval & 0x0f;
	update_input_port(port, selection);

	// A change on one port can resolve an exclusivity conflict with the
	// other port
	int const other_port = port ^ 1;
	char const *const other_tag = (other_port == 0) ? CTRL_SEL_RIGHT : CTRL_SEL_LEFT;
	ioport_port *const other_ioport = ioport(other_tag);
	if (other_ioport)
		update_input_port(other_port, other_ioport->read() & 0x0f);
}


//-------------------------------------------------
//  update_input_port
//-------------------------------------------------

void coco_state::update_input_port(int port, uint8_t selection)
{
	if (m_joy_handlers[port] == nullptr ||
				get_type_info_for_selection(selection) != typeid(*m_joy_handlers[port]))
	{
		m_joy_handlers[port] = make_joy_handler(selection, port ? 2 : 0);
	}
}

const std::type_info& coco_state::get_type_info_for_selection(uint8_t selection)
{
	switch (selection)
	{
		case JOY_DEVICE_STANDARD:    return typeid(coco_joy_standard);
		case JOY_DEVICE_TANDY_HIRES: return typeid(coco_tandy_hires_joy);
		case JOY_DEVICE_CM3_HIRES:   return typeid(coco_cm3_hires_joy);

		case JOY_DEVICE_DIECOM_LG:   return typeid(coco_diecom_light_gun);
		default:                     return typeid(coco_joy_disconnected);
	}
}

//-------------------------------------------------
//  make_joy_handler
//-------------------------------------------------

std::unique_ptr<coco_joy_handler> coco_state::make_joy_handler(uint8_t selection, int port)
{
	bool const is_exclusive = (selection == JOY_DEVICE_TANDY_HIRES) || (selection == JOY_DEVICE_DIECOM_LG);

	if (is_exclusive)
	{
		int const other_port = BIT(port, 1) ^ 1;
		char const *const other_tag = (other_port == 0) ? CTRL_SEL_RIGHT : CTRL_SEL_LEFT;
		ioport_port *const other_ioport = ioport(other_tag);
		uint8_t const other_selection = other_ioport ? (other_ioport->read() & 0x0f) : JOY_DEVICE_UNCONNECTED;

		if (other_selection == selection)
		{
			char const *device_name = "device";
			switch (selection)
			{
				case JOY_DEVICE_TANDY_HIRES: device_name = "the Tandy Hi-res Joystick";        break;
				case JOY_DEVICE_DIECOM_LG:   device_name = "the Diecom Light Gun Interface";   break;
			}

			char const *port_name = BIT(port, 1) ? "left" : "right";

			popmessage("ERROR: %s cannot be connected to both controller ports at once!\n"
				"The %s port is unconnected.", device_name, port_name);
			return std::make_unique<coco_joy_disconnected>(*this, port, ioport(JOYSTICK_BUTTONS_TAG));
		}
	}

	switch (selection)
	{
		case JOY_DEVICE_STANDARD:    return std::make_unique<coco_joy_standard>(*this, port, ioport(JOYSTICK_BUTTONS_TAG));
		case JOY_DEVICE_TANDY_HIRES: return std::make_unique<coco_tandy_hires_joy>(*this, port, ioport(JOYSTICK_BUTTONS_TAG));
		case JOY_DEVICE_CM3_HIRES:   return std::make_unique<coco_cm3_hires_joy>(*this, port, ioport(JOYSTICK_BUTTONS_TAG));

		case JOY_DEVICE_DIECOM_LG:   return std::make_unique<coco_diecom_light_gun>(*this, port,
			ioport(DIECOM_LIGHTGUN_BUTTONS_TAG), ioport(DIECOM_LIGHTGUN_RX_TAG), ioport(DIECOM_LIGHTGUN_RY_TAG));
		case JOY_DEVICE_UNCONNECTED: return std::make_unique<coco_joy_disconnected>(*this, port, ioport(JOYSTICK_BUTTONS_TAG));
		default:
			fatalerror("Unknown Color Computer joystick.\n");
			return nullptr;
	}
}



//-------------------------------------------------
//  joystick_changed
//-------------------------------------------------

void coco_state::joystick_changed(ioport_field &field, u32 param, ioport_value oldval, ioport_value newval)
{
	int axis = BIT(param, 0);
	int port = BIT(param, 1);
	m_joy_handlers[port]->joy_changed(axis, newval);
}



//-------------------------------------------------
//  write_joystick_mux - helper to send data to the mux
//-------------------------------------------------

void coco_state::write_joystick_mux(int slot, uint8_t val)
{
	m_mux->x_analog_w(slot, val);
}



//-------------------------------------------------
//  adjust_host_joy_timer
//-------------------------------------------------

void coco_state::adjust_host_joy_timer(int mux_axis, attotime duration)
{
	m_joy_timer->adjust(duration, mux_axis);
}



//-------------------------------------------------
//  joy_timer_callback - the joystick system uses
//  a single timer for all it's shenangians
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(coco_state::joy_timer_callback)
{
	s32 mux_address = param;
	int handler_index = BIT(param, 1);

	coco_joy_handler* joy_handler = m_joy_handlers[handler_index].get();
	joy_handler->saturated(mux_address);
}



//**************************************************************************
//  coco_joy_handler - Classes for things that plug into the joystick port
//**************************************************************************
//-------------------------------------------------
//  coco_joy_handler ctor
//-------------------------------------------------

coco_joy_handler::coco_joy_handler(coco_state &host, int base_slot, ioport_port *buttons)
	: m_host(host)
	, m_base_slot(base_slot)
	, m_buttons(buttons)
{
}



//-------------------------------------------------
//  coco_joy_handler::button_status
//-------------------------------------------------

uint8_t coco_joy_handler::button_status()
{
	return m_buttons->read() & ~(m_base_slot ? 0x5 : 0xa);
}


//-------------------------------------------------
//  coco_joy_handler::evaluate_comparator
//-------------------------------------------------

bool coco_joy_handler::evaluate_comparator(int dac, int joy_val)
{
	return joy_val >= dac;
}



//-------------------------------------------------
//  coco_joy_standard::joy_changed
//-------------------------------------------------

void coco_joy_standard::joy_changed(int axis, int joy_val)
{
	int target_slot = m_base_slot + axis;
	m_host.write_joystick_mux(target_slot, joy_val>>4);
}


//-------------------------------------------------
//  coco_tandy_hires_joy ctor
//-------------------------------------------------

coco_tandy_hires_joy::coco_tandy_hires_joy(coco_state &host, int base_slot, ioport_port *buttons)
	: coco_joy_handler(host, base_slot, buttons)
	// chosen to perfect stock Multi-Vue
	, m_multiplier(4856.0)
	, m_offset(560.0)
	, m_was_low(false)
	, m_charge_start_time{attotime::zero}
{
}


//-------------------------------------------------
//  coco_tandy_hires_joy
//-------------------------------------------------

void coco_tandy_hires_joy::hires_trigger(uint8_t state, attotime current_time, int axis, int joy_val)
{
	bool is_low = (state == 0); // PA0-PA3 cleared

	if (!m_was_low && is_low)
	{
		m_charge_start_time = current_time; // Cache the start timestamp

		double usec_val = (joy_val / 1023.0) * m_multiplier + m_offset;
		attotime total_duration = attotime::from_usec(s64(usec_val));

		m_host.adjust_host_joy_timer(m_base_slot + axis, total_duration);
	}
	else if (m_was_low && is_low)
	{
		double usec_val = (joy_val / 1023.0) * m_multiplier + m_offset;
		attotime total_duration = attotime::from_usec(s64(usec_val));

		attotime target_time = m_charge_start_time + total_duration;

		// CRITICAL CHECK: Has it been so long since m_charge_start_time that this
		// L->L call is actually a MUX switch reconnecting to a dormant axis?
		if (target_time <= current_time)
		{
			// The previous start time is completely stale (> duration ago).
			// Re-arm as a fresh charge cycle starting RIGHT NOW!
			m_charge_start_time = current_time;
			m_host.adjust_host_joy_timer(m_base_slot + axis, total_duration);
		}
		else
		{
			// Reschedule timer for remaining time of a active charge
			m_host.adjust_host_joy_timer(m_base_slot + axis, target_time - current_time);
		}
	}
	else if (!is_low)
	{
		m_host.adjust_host_joy_timer(m_base_slot + axis, attotime::never);
		m_host.write_joystick_mux(m_base_slot + axis, 0x0);
	}

	m_was_low = is_low;
}



//-------------------------------------------------
//  coco_tandy_hires_joy::evaluate_comparator
//-------------------------------------------------

bool coco_tandy_hires_joy::evaluate_comparator(int dac, int joy_val)
{
	return joy_val > dac;
}



//-------------------------------------------------
//  coco_tandy_hires_joy::saturated
//-------------------------------------------------

void coco_tandy_hires_joy::saturated(s32 mux_address)
{
	m_host.write_joystick_mux(mux_address, 0x3f);
}



//-------------------------------------------------
//  coco_cm3_hires_joy ctor
//-------------------------------------------------

coco_cm3_hires_joy::coco_cm3_hires_joy(coco_state &host, int base_slot, ioport_port *buttons)
	: coco_tandy_hires_joy(host, base_slot, buttons)
{
	// chosen by fair dice roll
	m_multiplier = 2624.0;
	m_offset = 500;
}



//-------------------------------------------------
//  coco_diecom_light_gun ctor
//-------------------------------------------------

coco_diecom_light_gun::coco_diecom_light_gun(coco_state &host, int base_slot, ioport_port *buttons, ioport_port *h_port, ioport_port *v_port)
	: coco_joy_handler(host, base_slot, buttons)
	, m_h_port(h_port)
	, m_v_port(v_port)
	, m_output_v(0)
	, m_output_h(0)
	, m_previous_bit(0)
	, m_adaptor_state(0)
	, m_horizontal_clock_count(0)
{
}



//-------------------------------------------------
//  dclg_table
//-------------------------------------------------

const int coco_diecom_light_gun::dclg_table[] = {0, 14, 30, 49};



//-------------------------------------------------
//  coco_diecom_light_gun::lightgun_clock
//-------------------------------------------------

void coco_diecom_light_gun::lightgun_clock(int clock)
{
	if (m_previous_bit != clock)
	{
		m_previous_bit = clock;
		m_adaptor_state++;
		m_adaptor_state &= 0x1f;
		int half_state = m_adaptor_state >> 1;

		/* clear hit bit for every transistion */
		m_output_v &= ~0x02;
		m_output_h = 0;

		if (half_state > 7)
		{
			/* bit shift timer data on half states 8 thru 15 */
			if (m_horizontal_clock_count & (1 << (half_state - 7)))
			{
				m_output_h |= 0x01;
			}

			/* bit 9 of timer is only available if half state == 8 */
			if (half_state == 8 && (m_horizontal_clock_count & (1 << 8)))
				m_output_h |= 0x02;
		}

		/* during half state 15, this bit is high. */
		/* it is used to sync the state of the converter box with the computer */
		if (half_state == 15)
			m_output_v |= 0x01;
		else
			m_output_v &= ~0x01;

		/* while in full state 15, prepare to check next video frame for a hit */
		if (m_adaptor_state == 15)
		{
			int dclg_vpos = m_v_port->read() + 12;
			// this clock starts at zero at the left edge of the screen,
			// and rolls over every scanline
			m_horizontal_clock_count = m_h_port->read();

			int horizontal_pixel = ((m_horizontal_clock_count - 105.0) / (420.0 - 110.0)) * (639.0 - 0.0) + 0.0;
			attotime dclg_time = m_host.get_screen()->time_until_pos(dclg_vpos, horizontal_pixel);
			m_host.adjust_host_joy_timer(m_base_slot+1, dclg_time);

		}
		else
		{
			m_host.adjust_host_joy_timer(m_base_slot, attotime::never);
		}

		m_host.write_joystick_mux(m_base_slot, dclg_table[m_output_h]);
		m_host.write_joystick_mux(m_base_slot+1, dclg_table[m_output_v]);
	}
}



//-------------------------------------------------
//  coco_diecom_light_gun::saturated
//-------------------------------------------------

void coco_diecom_light_gun::saturated(s32 target_slot)
{
	m_output_v |= 0x02;
	m_host.write_joystick_mux(target_slot, dclg_table[m_output_v]);
}

