// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco.c

    TRS-80 Radio Shack Color Computer Family

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

  References:
        There are two main references for the info for this driver
        - Tandy Color Computer Unravelled Series
                    (http://www.giftmarket.org/unravelled/unravelled.shtml)
        - Assembly Language Programming For the CoCo 3 by Laurence A. Tepolt
        - Kevin K. Darlings GIME reference
                    (http://www.cris.com/~Alxevans/gime.txt)
        - Sock Masters's GIME register reference
                    (http://www.axess.com/twilight/sock/gime.html)
        - Robert Gault's FAQ
                    (http://home.att.net/~robert.gault/Coco/FAQ/FAQ_main.htm)
        - Discussions with L. Curtis Boyle (LCB) and John Kowalski (JK)

  TODO:
        - Implement unimplemented SAM registers
        - Choose and implement more appropriate ratios for the speed up poke
        - Handle resets correctly

  In the CoCo, all timings should be exactly relative to each other.  This
  table shows how all clocks are relative to each other (info: JK):
        - Main CPU Clock                0.89 MHz
        - Horizontal Sync Interrupt     15.7 kHz/63.5us (57 clock cycles)
        - Vertical Sync Interrupt       60 Hz           (14934 clock cycles)
        - Composite Video Color Carrier 3.58 MHz/279ns  (1/4 clock cycles)

  It is also noting that the CoCo 3 had two sets of VSync interrupts.  To quote
  John Kowalski:

    One other thing to mention is that the old vertical interrupt and the new
    vertical interrupt are not the same..  The old one is triggered by the
    video's vertical sync pulse, but the new one is triggered on the next scan
    line *after* the last scan line of the active video display.  That is : new
    vertical interrupt triggers somewheres around scan line 230 of the 262 line
    screen (if a 200 line graphics mode is used, a bit earlier if a 192 line
    mode is used and a bit later if a 225 line mode is used).  The old vsync
    interrupt triggers on scanline zero.

    230 is just an estimate [(262-200)/2+200].  I don't think the active part
    of the screen is exactly centered within the 262 line total.  I can
    research that for you if you want an exact number for scanlines before the
    screen starts and the scanline that the v-interrupt triggers..etc.

Added bi-directional bitbanger support. Also fixed reading PIA 1, port A. The
DAC and bitbanger values written should be reflected in the read.
    tim lindner, October 2010

***************************************************************************/

#include "includes/coco.h"
#include "cpu/m6809/m6809.h"
#include "debug/debugcpu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define LOG_INTERRUPTS      0



//**************************************************************************
//  BODY
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

coco_state::coco_state(const machine_config &mconfig, device_type type, std::string tag)
	: driver_device(mconfig, type, tag),
	m_maincpu(*this, MAINCPU_TAG),
	m_pia_0(*this, PIA0_TAG),
	m_pia_1(*this, PIA1_TAG),
	m_dac(*this, DAC_TAG),
	m_wave(*this, WAVE_TAG),
	m_cococart(*this, CARTRIDGE_TAG),
	m_ram(*this, RAM_TAG),
	m_cassette(*this, "cassette"),
	m_rs232(*this, RS232_TAG),
	m_vhd_0(*this, VHD0_TAG),
	m_vhd_1(*this, VHD1_TAG),
	m_beckerport(*this, DWSOCK_TAG),
	m_beckerportconfig(*this, BECKERPORT_TAG)
{
}


//-------------------------------------------------
//  analog_port_start
//-------------------------------------------------

void coco_state::analog_port_start(analog_input_t *analog, const char *rx_tag, const char *ry_tag, const char *lx_tag, const char *ly_tag, const char *buttons_tag)
{
	analog->m_input[0][0] =  ioport(rx_tag);
	analog->m_input[0][1] =  ioport(ry_tag);
	analog->m_input[1][0] =  ioport(lx_tag);
	analog->m_input[1][1] =  ioport(ly_tag);
	analog->m_buttons =  ioport(buttons_tag);
}



//-------------------------------------------------
//  device_start
//-------------------------------------------------

void coco_state::device_start()
{
	/* call base device_start */
	driver_device::device_start();

	/* look up keyboard ports */
	for (int i = 0; i < ARRAY_LENGTH(m_keyboard); i++)
	{
		char name[32];
		snprintf(name, ARRAY_LENGTH(name), "row%d", i);
		m_keyboard[i] =  ioport(name);
	}

	/* look up analog ports */
	analog_port_start(&m_joystick, JOYSTICK_RX_TAG, JOYSTICK_RY_TAG,
		JOYSTICK_LX_TAG, JOYSTICK_LY_TAG, JOYSTICK_BUTTONS_TAG);
	analog_port_start(&m_rat_mouse, RAT_MOUSE_RX_TAG, RAT_MOUSE_RY_TAG,
		RAT_MOUSE_LX_TAG, RAT_MOUSE_LY_TAG, RAT_MOUSE_BUTTONS_TAG);
	analog_port_start(&m_diecom_lightgun, DIECOM_LIGHTGUN_RX_TAG, DIECOM_LIGHTGUN_RY_TAG,
		DIECOM_LIGHTGUN_LX_TAG, DIECOM_LIGHTGUN_LY_TAG, DIECOM_LIGHTGUN_BUTTONS_TAG);

	/* look up miscellaneous controls */
	m_joystick_type_control =  ioport(CTRL_SEL_TAG);
	m_joystick_hires_control =  ioport(HIRES_INTF_TAG);

	/* timers */
	m_hiresjoy_transition_timer[0] = timer_alloc(TIMER_HIRES_JOYSTICK_X);
	m_hiresjoy_transition_timer[1] = timer_alloc(TIMER_HIRES_JOYSTICK_Y);
	m_diecom_lightgun_timer = timer_alloc(TIMER_DIECOM_LIGHTGUN);

	/* cart base update */
	m_cococart->set_cart_base_update(cococart_base_update_delegate(FUNC(coco_state::update_cart_base), this));

	/* save state support */
	save_item(NAME(m_dac_output));
	save_item(NAME(m_hiresjoy_ca));
	save_item(NAME(m_dclg_previous_bit));
	save_item(NAME(m_dclg_output_h));
	save_item(NAME(m_dclg_output_v));
	save_item(NAME(m_dclg_state));
	save_item(NAME(m_dclg_timer));
	save_item(NAME(m_vhd_select));

	/* set up disassembly override */
	if (m_maincpu->debug())
	{
		m_maincpu->debug()->set_dasm_override(dasm_override);
	}

	// miscellaneous
	m_in_floating_bus_read = false;
}



//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void coco_state::device_reset()
{
	/* call base device_start */
	driver_device::device_reset();

	/* reset state */
	m_dac_output = 0;
	m_analog_audio_level = 0;
	m_hiresjoy_ca = false;
	m_dclg_previous_bit = false;
	m_dclg_output_h = 0;
	m_dclg_output_v = 0;
	m_dclg_state = 0;
	m_dclg_timer = 0;
	m_vhd_select = 0;
}



//-------------------------------------------------
//  device_timer
//-------------------------------------------------

void coco_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_HIRES_JOYSTICK_X:
		case TIMER_HIRES_JOYSTICK_Y:
		case TIMER_DIECOM_LIGHTGUN:
			poll_keyboard();
			break;
	}
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

UINT8 coco_state::floating_bus_read(void)
{
	UINT8 result;

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

		// set up the ability to read address spaces
		address_space &program = m_maincpu->space(AS_PROGRAM);

		// get the previous and current PC
		UINT16 prev_pc = m_maincpu->pcbase();
		UINT16 pc = m_maincpu->pc();

		// get the byte; and skip over header bytes
		UINT8 byte = program.read_byte(prev_pc);
		if ((byte == 0x10) || (byte == 0x11))
			byte = program.read_byte(++prev_pc);

		// check to see if the opcode specifies the indexed addressing mode, and the secondary byte
		// specifies no-offset
		bool is_nooffset_indexed = (((byte & 0xF0) == 0x60) || ((byte & 0xF0) == 0xA0) || ((byte & 0xF0) == 0xE0))
			&& ((program.read_byte(prev_pc + 1) & 0xBF) == 0x84);

		// finally read the byte
		result = program.read_byte(is_nooffset_indexed ? pc : 0xFFFF);

		// we're done reading
		m_in_floating_bus_read = false;
	}
	return result;
}



/***************************************************************************
  PIA0 ($FF00-$FF1F) (Chip U8)

  PIA0 PA0-PA7  - Keyboard/Joystick read
  PIA0 PB0-PB7  - Keyboard write
  PIA0 CA1      - MC6847 HS (Horizontal Sync)
  PIA0 CA2      - SEL1 (Used by sound mux and joystick)
  PIA0 CB1      - MC6847 FS (Field Sync)
  PIA0 CB2      - SEL2 (Used by sound mux and joystick)
***************************************************************************/

//-------------------------------------------------
//  ff00_write
//-------------------------------------------------

READ8_MEMBER( coco_state::ff00_read )
{
	return m_pia_0->read(space, offset, mem_mask);
}



//-------------------------------------------------
//  ff00_write
//-------------------------------------------------

WRITE8_MEMBER( coco_state::ff00_write )
{
	m_pia_0->write(space, offset, data, mem_mask);
}



//-------------------------------------------------
//  pia0_pa_w
//-------------------------------------------------

WRITE8_MEMBER( coco_state::pia0_pa_w )
{
	poll_keyboard();
}



//-------------------------------------------------
//  pia0_pb_w
//-------------------------------------------------

WRITE8_MEMBER( coco_state::pia0_pb_w )
{
	poll_keyboard();
}



//-------------------------------------------------
//  pia0_ca2_w
//-------------------------------------------------

WRITE_LINE_MEMBER( coco_state::pia0_ca2_w )
{
	update_sound();     // analog mux SEL1 is tied to PIA0 CA2
	poll_keyboard();
}



//-------------------------------------------------
//  pia0_cb2_w
//-------------------------------------------------

WRITE_LINE_MEMBER( coco_state::pia0_cb2_w )
{
	update_sound();     // analog mux SEL2 is tied to PIA0 CB2
	poll_keyboard();
}



//-------------------------------------------------
//  pia0_irq_a
//-------------------------------------------------

WRITE_LINE_MEMBER( coco_state::pia0_irq_a )
{
	recalculate_irq();
}



//-------------------------------------------------
//  pia0_irq_b
//-------------------------------------------------

WRITE_LINE_MEMBER( coco_state::pia0_irq_b )
{
	recalculate_irq();
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
//  ff20_read
//-------------------------------------------------

READ8_MEMBER( coco_state::ff20_read )
{
	return m_pia_1->read(space, offset, mem_mask);
}



//-------------------------------------------------
//  ff20_write
//-------------------------------------------------

WRITE8_MEMBER( coco_state::ff20_write )
{
	/* write to the PIA */
	m_pia_1->write(space, offset, data, mem_mask);

	/* we have to do this to do something that approximates the cartridge Q line behavior */
	m_cococart->twiddle_q_lines();
}



//-------------------------------------------------
//  pia1_pa_r
//-------------------------------------------------

READ8_MEMBER( coco_state::pia1_pa_r )
{
	// Port A: we need to specify the values of all the lines, regardless of whether
	// they are in input or output mode in the DDR
	return (m_cassette->input() >= 0 ? 0x01 : 0x00)
		| (dac_output() << 2);
}



//-------------------------------------------------
//  pia1_pb_r - this handles the reading of the
//  memory sense switch (PB2) for the CoCo 1 and
//  serial-in (PB0)
//-------------------------------------------------

READ8_MEMBER( coco_state::pia1_pb_r )
{
	// Port B: lines in output mode are handled automatically by the PIA object.
	// We only need to specify the input lines here
	UINT32 ram_size = m_ram->size();

	//  For the CoCo 1, the logic has been changed to only select 64K rams
	//  if there is more than 16K of memory, as the Color Basic 1.0 rom
	//  can only configure 4K or 16K ram banks (as documented in "Color
	//  Basic Unreveled"), doing this allows this  allows the coco driver
	//  to access 32K of ram, and also allows the cocoe driver to access
	//  the full 64K, as this uses Color Basic 1.2, which can configure 64K rams
	bool memory_sense = (ram_size >= 0x4000 && ram_size <= 0x7FFF)
		|| (ram_size >= 0x8000 && (m_pia_0->b_output() & 0x40));

	// serial in (PB0)
	bool serial_in = (m_rs232 != nullptr) && (m_rs232->rxd_r() ? true : false);

	// composite the results
	return (memory_sense ? 0x04 : 0x00)
		| (serial_in ? 0x01 : 0x00);
}



//-------------------------------------------------
//  pia1_pa_w
//-------------------------------------------------

WRITE8_MEMBER( coco_state::pia1_pa_w )
{
	pia1_pa_changed(data);
}



//-------------------------------------------------
//  pia1_pb_w
//-------------------------------------------------

WRITE8_MEMBER( coco_state::pia1_pb_w )
{
	pia1_pb_changed(data);
}



//-------------------------------------------------
//  pia1_ca2_w
//-------------------------------------------------

WRITE_LINE_MEMBER( coco_state::pia1_ca2_w )
{
	m_cassette->change_state(
		state ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,
		CASSETTE_MASK_MOTOR);
}



//-------------------------------------------------
//  pia1_cb2_w
//-------------------------------------------------

WRITE_LINE_MEMBER( coco_state::pia1_cb2_w )
{
	update_sound();     // SOUND_ENABLE is connected to PIA1 CB2
}



//-------------------------------------------------
//  pia1_firq_a
//-------------------------------------------------

WRITE_LINE_MEMBER( coco_state::pia1_firq_a )
{
	recalculate_firq();
}



//-------------------------------------------------
//  pia1_firq_b
//-------------------------------------------------

WRITE_LINE_MEMBER( coco_state::pia1_firq_b )
{
	recalculate_firq();
}



/***************************************************************************
  CPU INTERRUPTS

  The Dragon/CoCo2 have two PIAs.  These PIAs can trigger interrupts.  PIA0
  is set up to trigger IRQ on the CPU, and PIA1 can trigger FIRQ.  Each PIA
  has two output lines, and an interrupt will be triggered if either of these
  lines are asserted.

  -----  IRQ
  6809 |-<----------- PIA0
       |
       |
       |
       |
       |
       |-<----------- PIA1
  -----

***************************************************************************/

//-------------------------------------------------
//  irq_get_line - gets the value of the FIRQ line
//  passed into the CPU
//-------------------------------------------------

bool coco_state::irq_get_line(void)
{
	return m_pia_0->irq_a_state() || m_pia_0->irq_b_state();
}



//-------------------------------------------------
//  recalculate_irq
//-------------------------------------------------

void coco_state::recalculate_irq(void)
{
	bool line = irq_get_line();
	if (LOG_INTERRUPTS)
		logerror("recalculate_irq():  line=%d\n", line ? 1 : 0);
	m_maincpu->set_input_line(M6809_IRQ_LINE, line ? ASSERT_LINE : CLEAR_LINE);
}



//-------------------------------------------------
//  firq_get_line - gets the value of the FIRQ line
//  passed into the CPU
//-------------------------------------------------

bool coco_state::firq_get_line(void)
{
	return m_pia_1->irq_a_state() || m_pia_1->irq_b_state();
}



//-------------------------------------------------
//  recalculate_firq
//-------------------------------------------------

void coco_state::recalculate_firq(void)
{
	bool line = firq_get_line();
	if (LOG_INTERRUPTS)
		logerror("recalculate_firq():  line=%d\n", line ? 1 : 0);
	m_maincpu->set_input_line(M6809_FIRQ_LINE, line ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************
  SOUND / KEYBOARD / JOYSTICK

  The sound MUX has 4 possible settings, depend on SELA and SELB inputs:

  00    - DAC (digital - analog converter)
  01    - CSN (cassette)
  10    - SND input from cartridge (NYI because we only support the FDC)
  11    - Grounded (0)

  Source - Tandy Color Computer Service Manual

  Note on the Dragon Alpha state 11, selects the AY-3-8912, this is currently
  un-implemented - phs.

***************************************************************************/

//-------------------------------------------------
//  soundmux_status
//-------------------------------------------------

coco_state::soundmux_status_t coco_state::soundmux_status(void)
{
	return (soundmux_status_t) (
		(snden() ? SOUNDMUX_ENABLE : 0) |
		(sel1()  ? SOUNDMUX_SEL1 : 0) |
		(sel2()  ? SOUNDMUX_SEL2 : 0));
}



//-------------------------------------------------
//  update_sound
//-------------------------------------------------

void coco_state::update_sound(void)
{
	/* PB1 will drive the sound output.  This is a rarely
	 * used single bit sound mode. It is always connected thus
	 * cannot be disabled.
	 *
	 * Source:  Page 31 of the Tandy Color Computer Serice Manual
	 */
	UINT8 single_bit_sound = (m_pia_1->b_output() & 0x02) ? 0x80 : 0x00;

	/* determine the sound mux status */
	soundmux_status_t status = soundmux_status();

	/* the SC77526 DAC chip internally biases the AC-coupled sound inputs for Cassette and Cartridge at the midpoint of the 3.9v output range */
	bool bCassSoundEnable = (status == (SOUNDMUX_ENABLE | SOUNDMUX_SEL1));
	bool bCartSoundEnable = (status == (SOUNDMUX_ENABLE | SOUNDMUX_SEL2));
	UINT8 cassette_sound = (bCassSoundEnable ? 0x40 : 0);
	UINT8 cart_sound = (bCartSoundEnable ? 0x40 : 0);

	/* determine the value to send to the DAC (this is used by the Joystick read as well as audio out) */
	m_dac_output = (m_pia_1->a_output() & 0xFC) >> 2;
	UINT8 dac_sound =  (status == SOUNDMUX_ENABLE ? m_dac_output << 1 : 0);

	/* The CoCo uses a single DAC for both audio output and joystick axis position measurement.
	 * To avoid introducing artifacts while reading the axis positions, some software will disable
	 * the audio output while using the DAC to read the joystick.  On a real CoCo, there is a low-pass
	 * filter (C57 on the CoCo 3) which will hold the audio level for very short periods of time,
	 * preventing the introduction of artifacts while the joystick value is being read.  We are not going
	 * to simulate the exponential decay of a capacitor here.  Instead, we will store and hold the last
	 * used analog audio output value while the audio is disabled, to avoid introducing artifacts in
	 * software such as Tandy's Popcorn and Sock Master's Donkey Kong.
	 */
	if ((status & SOUNDMUX_ENABLE) != 0)
	{
		m_analog_audio_level = dac_sound + cassette_sound + cart_sound;
	}

	m_dac->write_unsigned8(single_bit_sound + m_analog_audio_level);

	/* determine the cassette sound status */
	cassette_state cas_sound = bCassSoundEnable ? CASSETTE_SPEAKER_ENABLED : CASSETTE_SPEAKER_MUTED;
	m_cassette->change_state(cas_sound, CASSETTE_MASK_SPEAKER);

	/* determine the cartridge sound status */
	m_cococart->cart_set_line(COCOCART_LINE_SOUND_ENABLE, bCartSoundEnable ? COCOCART_LINE_VALUE_ASSERT : COCOCART_LINE_VALUE_CLEAR);
}



//-------------------------------------------------
//  joystick_type - returns the type of joystick
//  in the specified port
//-------------------------------------------------

coco_state::joystick_type_t coco_state::joystick_type(int index)
{
	assert((index == 0) || (index == 1));
	return (m_joystick_type_control != nullptr)
		? (joystick_type_t) ((m_joystick_type_control->read() >> (index * 4)) & 0x0F)
		: JOYSTICK_NONE;
}



//-------------------------------------------------
//  hires_interface_type
//-------------------------------------------------

coco_state::hires_type_t coco_state::hires_interface_type(void)
{
	return (m_joystick_hires_control != nullptr)
		? (hires_type_t) m_joystick_hires_control->read()
		: HIRES_NONE;
}



//-------------------------------------------------
//  is_joystick_hires
//-------------------------------------------------

bool coco_state::is_joystick_hires(int joystick_index)
{
	bool result;
	assert((joystick_index == 0) || (joystick_index == 1));

	switch(hires_interface_type())
	{
		case HIRES_RIGHT:
		case HIRES_RIGHT_COCOMAX3:
			result = (joystick_index == 0);
			break;

		case HIRES_LEFT:
		case HIRES_LEFT_COCOMAX3:
			result = (joystick_index == 1);
			break;

		default:
			result = false;
			break;
	}
	return result;
}



//-------------------------------------------------
//  poll_joystick
//-------------------------------------------------

void coco_state::poll_joystick(bool *joyin, UINT8 *buttons)
{
	static const analog_input_t s_empty = {};
	static const int joy_rat_table[] = {15, 24, 42, 33 };
	static const int dclg_table[] = {0, 14, 30, 49 };

	/* identify the joystick and axis */
	int joystick_axis = sel1() ? 1 : 0;
	int joystick = sel2() ? 1 : 0;

	/* determine the JOYIN value */
	const analog_input_t *analog;
	bool joyin_value;
	UINT8 joyval;
	int dclg_vpos;
	switch(joystick_type(joystick))
	{
		case JOYSTICK_NORMAL:
			analog = &m_joystick;

			/* is any Hi-Res Interface turned on? prepare masks to check it */
			if (is_joystick_hires(joystick))
			{
				/* hi-res joystick or hi-res CoCo3Max joystick */
				attotime remaining = m_hiresjoy_transition_timer[joystick_axis]->remaining();
				joyin_value = remaining.is_zero() || remaining.is_never();
			}
			else
			{
				/* conventional joystick */
				joyval = analog->input(joystick, joystick_axis);
				joyin_value = (dac_output() <= (joyval >> 2));
			}
			break;

		case JOYSTICK_RAT_MOUSE:
			analog = &m_rat_mouse;
			joyval = analog->input(joystick, joystick_axis);
			joyin_value = dac_output() <= joy_rat_table[joyval];
			break;

		case JOYSTICK_DIECOM_LIGHT_GUN:
			analog = &m_diecom_lightgun;

			/* get the vertical position of the lightgun */
			dclg_vpos = analog->input(joystick, 1);

			if (machine().first_screen()->vpos() == dclg_vpos)
			{
				/* if gun is pointing at the current scan line, set hit bit and cache horizontal timer value */
				m_dclg_output_h |= 0x02;
				m_dclg_timer = analog->input(joystick, 0) << 1;
			}

			joyin_value = (dac_output() <= dclg_table[(joystick_axis ? m_dclg_output_h : m_dclg_output_v) & 0x03]);

			if (m_dclg_state == 7)
			{
				/* while in state 7, prepare to check next video frame for a hit */
				attotime dclg_time = machine().first_screen()->time_until_pos(dclg_vpos, 0);
				m_diecom_lightgun_timer->adjust(dclg_time);
			}
			break;

		default: /* None */
			analog = &s_empty;
			joyin_value = false;
			break;
	}

	*joyin = joyin_value;
	*buttons = analog->buttons();
}



//-------------------------------------------------
//  poll_keyboard
//-------------------------------------------------

void coco_state::poll_keyboard(void)
{
	UINT8 pia0_pb = m_pia_0->b_output();
	UINT8 pia0_pb_z = m_pia_0->port_b_z_mask();

	UINT8 pia0_pa = 0x7F;
	UINT8 pia0_pa_z = 0x7F;

	/* poll the keyboard, and update PA6-PA0 accordingly*/
	for (int i = 0; i < ARRAY_LENGTH(m_keyboard); i++)
	{
		int value = m_keyboard[i]->read();
		if ((value | pia0_pb) != 0xFF)
		{
			pia0_pa &= ~(0x01 << i);
		}
		if ((value | pia0_pb_z) != 0xFF)
		{
			pia0_pa_z &= ~(0x01 << i);
		}
	}

	/* hires joystick */
	poll_hires_joystick();

	/* poll the joystick (*/
	bool joyin;
	UINT8 buttons;
	poll_joystick(&joyin, &buttons);

	/* PA7 comes from JOYIN */
	pia0_pa |= joyin ? 0x80 : 0x00;

	/* mask out the buttons */
	pia0_pa &= ~buttons;
	pia0_pa_z &= ~buttons;

	/* and write the result to PIA0 */
	update_keyboard_input(pia0_pa, pia0_pa_z);
}



//-------------------------------------------------
//  update_keyboard_input - writes to PIA0 PA, but
//  on the CoCo 3 controls a GIME input
//-------------------------------------------------

void coco_state::update_keyboard_input(UINT8 value, UINT8 z)
{
	m_pia_0->set_a_input(value, z);
}



//-------------------------------------------------
//  update_cassout - called when CASSOUT changes
//-------------------------------------------------

void coco_state::update_cassout(int cassout)
{
	m_cassette->output((cassout - 0x20) / 32.0);
}



//-------------------------------------------------
//  diecom_lightgun_clock - called the diecom
//  lightgun undergoes a high to low transition
//-------------------------------------------------

void coco_state::diecom_lightgun_clock(void)
{
	/* clock Diecom Light gun interface on a high to low transistion */
	m_dclg_state++;
	m_dclg_state &= 0x0f;

	/* clear hit bit for every transistion */
	m_dclg_output_h &= ~0x02;

	if (m_dclg_state > 7)
	{
		/* Bit shift timer data on state 8 thru 15 */
		if (((m_dclg_timer >> (m_dclg_state - 8 + 1)) & 0x01) == 1)
			m_dclg_output_v |= 0x01;
		else
			m_dclg_output_v &= ~0x01;

		/* Bit 9 of timer is only avaiable if state == 8*/
		if (m_dclg_state == 8 && (((m_dclg_timer >> 9) & 0x01) == 1))
			m_dclg_output_v |= 0x02;
		else
			m_dclg_output_v &= ~0x02;
	}

	/* During state 15, this bit is high. */
	if (m_dclg_state == 15)
		m_dclg_output_h |= 0x01;
	else
		m_dclg_output_h &= ~0x01;
}



//-------------------------------------------------
//  update_prinout - called when PRINOUT changes
//-------------------------------------------------

void coco_state::update_prinout(bool prinout)
{
	if ((joystick_type(0) == JOYSTICK_DIECOM_LIGHT_GUN) || (joystick_type(1) == JOYSTICK_DIECOM_LIGHT_GUN))
	{
		/* printer port is connected to diecom light gun */
		if (m_dclg_previous_bit && !prinout)
		{
			diecom_lightgun_clock();
		}
		m_dclg_previous_bit = prinout;
	}
	else
	{
		/* output bitbanger if present (only on CoCos) */
		if (m_rs232 != nullptr)
		{
			m_rs232->write_txd(prinout ? 1 : 0);
		}
	}
}



//-------------------------------------------------
//  pia1_pa_changed - called when PIA1 PA changes
//-------------------------------------------------

void coco_state::pia1_pa_changed(UINT8 data)
{
	update_sound();     // DAC is connected to PIA1 PA2-PA7
	poll_keyboard();
	update_cassout(dac_output());
	update_prinout(data & 0x02 ? true : false);
}



//-------------------------------------------------
//  pia1_pb_changed - called when PIA1 PB changes
//-------------------------------------------------

void coco_state::pia1_pb_changed(UINT8 data)
{
	update_sound();     // singe_bit_sound is connected to PIA1 PB1
}



//-------------------------------------------------
//  keyboard_changed
//-------------------------------------------------

INPUT_CHANGED_MEMBER(coco_state::keyboard_changed)
{
	poll_keyboard();
}



//-------------------------------------------------
//  joystick_mode_changed
//-------------------------------------------------

INPUT_CHANGED_MEMBER(coco_state::joystick_mode_changed)
{
	poll_keyboard();
}

//-------------------------------------------------
//  poll_hires_joystick
//-------------------------------------------------

void coco_state::poll_hires_joystick(void)
{
	bool newvalue;
	bool is_cocomax3;
	int joystick_index, axis;

	/* we do different things based on the type of hires interface */
	switch(hires_interface_type())
	{
		case HIRES_RIGHT:
			newvalue = (dac_output() >= 0x20);
			joystick_index = 0;
			is_cocomax3 = false;
			break;

		case HIRES_RIGHT_COCOMAX3:
			newvalue = (m_pia_0->a_output() & 0x04);
			joystick_index = 0;
			is_cocomax3 = true;
			break;

		case HIRES_LEFT:
			newvalue = (dac_output() >= 0x20);
			joystick_index = 1;
			is_cocomax3 = false;
			break;

		case HIRES_LEFT_COCOMAX3:
			newvalue = (m_pia_0->a_output() & 0x08);
			joystick_index = 1;
			is_cocomax3 = true;
			break;

		default:
			newvalue = true;
			joystick_index = -1;
			is_cocomax3 = false;
			break;
	}

	/* if the joystick isn't selected, newvalue is true */
	newvalue = newvalue || (joystick_index < 0) || (joystick_type(joystick_index) != JOYSTICK_NORMAL);

	/* make the transition */
	for (axis = 0; axis <= 1; axis++)
	{
		if (m_hiresjoy_ca && !newvalue)
		{
			/* hi to lo */
			double value = m_joystick.input(joystick_index, axis) / 255.0;
			value *= is_cocomax3 ? 2500.0 : 4160.0;
			value += is_cocomax3 ? 400.0 : 592.0;
			attotime duration = m_maincpu->clocks_to_attotime((UINT64) value) * 8;
			m_hiresjoy_transition_timer[axis]->adjust(duration);
		}
		else if (!m_hiresjoy_ca && newvalue)
		{
			/* lo to hi */
			m_hiresjoy_transition_timer[axis]->reset();
		}
	}
	m_hiresjoy_ca = newvalue;
}



/***************************************************************************
  VHD
 ***************************************************************************/

//-------------------------------------------------
//  current_vhd
//-------------------------------------------------

coco_vhd_image_device *coco_state::current_vhd(void)
{
	switch(m_vhd_select)
	{
		case 0:     return m_vhd_0;
		case 1:     return m_vhd_1;
		default:    return nullptr;
	}
}



//-------------------------------------------------
//  ff60_read
//-------------------------------------------------

READ8_MEMBER( coco_state::ff60_read )
{
	UINT8 result;

	if ((current_vhd() != nullptr) && (offset >= 32) && (offset <= 37))
	{
		result = current_vhd()->read(offset - 32);
	}
	else
	{
		result = floating_bus_read();
	}

	return result;
}



//-------------------------------------------------
//  ff60_write
//-------------------------------------------------

WRITE8_MEMBER( coco_state::ff60_write )
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
}



/***************************************************************************
  CARTRIDGE & CASSETTE
 ***************************************************************************/

//-------------------------------------------------
//  ff40_read
//-------------------------------------------------

READ8_MEMBER( coco_state::ff40_read )
{
	if (offset >= 1 && offset <= 2 && m_beckerportconfig && m_beckerportconfig->read() == 1)
	{
		return m_beckerport->read(space, offset-1, mem_mask);
	}

	return m_cococart->read(space, offset, mem_mask);
}



//-------------------------------------------------
//  ff40_write
//-------------------------------------------------

WRITE8_MEMBER( coco_state::ff40_write )
{
	if (offset >= 1 && offset <= 2 && m_beckerportconfig && m_beckerportconfig->read() == 1)
	{
		return m_beckerport->write(space, offset-1, data, mem_mask);
	}

	m_cococart->write(space, offset, data, mem_mask);
}



//-------------------------------------------------
//  cart_w
//-------------------------------------------------

void coco_state::cart_w(bool state)
{
	m_pia_1->cb1_w(state);
}

/***************************************************************************
  DISASSEMBLY OVERRIDE (OS9 syscalls)
 ***************************************************************************/

static const char *const os9syscalls[] =
{
	"F$Link",          /* Link to Module */
	"F$Load",          /* Load Module from File */
	"F$UnLink",        /* Unlink Module */
	"F$Fork",          /* Start New Process */
	"F$Wait",          /* Wait for Child Process to Die */
	"F$Chain",         /* Chain Process to New Module */
	"F$Exit",          /* Terminate Process */
	"F$Mem",           /* Set Memory Size */
	"F$Send",          /* Send Signal to Process */
	"F$Icpt",          /* Set Signal Intercept */
	"F$Sleep",         /* Suspend Process */
	"F$SSpd",          /* Suspend Process */
	"F$ID",            /* Return Process ID */
	"F$SPrior",        /* Set Process Priority */
	"F$SSWI",          /* Set Software Interrupt */
	"F$PErr",          /* Print Error */
	"F$PrsNam",        /* Parse Pathlist Name */
	"F$CmpNam",        /* Compare Two Names */
	"F$SchBit",        /* Search Bit Map */
	"F$AllBit",        /* Allocate in Bit Map */
	"F$DelBit",        /* Deallocate in Bit Map */
	"F$Time",          /* Get Current Time */
	"F$STime",         /* Set Current Time */
	"F$CRC",           /* Generate CRC */
	"F$GPrDsc",        /* get Process Descriptor copy */
	"F$GBlkMp",        /* get System Block Map copy */
	"F$GModDr",        /* get Module Directory copy */
	"F$CpyMem",        /* Copy External Memory */
	"F$SUser",         /* Set User ID number */
	"F$UnLoad",        /* Unlink Module by name */
	"F$Alarm",         /* Color Computer Alarm Call (system wide) */
	nullptr,
	nullptr,
	"F$NMLink",        /* Color Computer NonMapping Link */
	"F$NMLoad",        /* Color Computer NonMapping Load */
	nullptr,
	nullptr,
	"F$TPS",           /* Return System's Ticks Per Second */
	"F$TimAlm",        /* COCO individual process alarm call */
	"F$VIRQ",          /* Install/Delete Virtual IRQ */
	"F$SRqMem",        /* System Memory Request */
	"F$SRtMem",        /* System Memory Return */
	"F$IRQ",           /* Enter IRQ Polling Table */
	"F$IOQu",          /* Enter I/O Queue */
	"F$AProc",         /* Enter Active Process Queue */
	"F$NProc",         /* Start Next Process */
	"F$VModul",        /* Validate Module */
	"F$Find64",        /* Find Process/Path Descriptor */
	"F$All64",         /* Allocate Process/Path Descriptor */
	"F$Ret64",         /* Return Process/Path Descriptor */
	"F$SSvc",          /* Service Request Table Initialization */
	"F$IODel",         /* Delete I/O Module */
	"F$SLink",         /* System Link */
	"F$Boot",          /* Bootstrap System */
	"F$BtMem",         /* Bootstrap Memory Request */
	"F$GProcP",        /* Get Process ptr */
	"F$Move",          /* Move Data (low bound first) */
	"F$AllRAM",        /* Allocate RAM blocks */
	"F$AllImg",        /* Allocate Image RAM blocks */
	"F$DelImg",        /* Deallocate Image RAM blocks */
	"F$SetImg",        /* Set Process DAT Image */
	"F$FreeLB",        /* Get Free Low Block */
	"F$FreeHB",        /* Get Free High Block */
	"F$AllTsk",        /* Allocate Process Task number */
	"F$DelTsk",        /* Deallocate Process Task number */
	"F$SetTsk",        /* Set Process Task DAT registers */
	"F$ResTsk",        /* Reserve Task number */
	"F$RelTsk",        /* Release Task number */
	"F$DATLog",        /* Convert DAT Block/Offset to Logical */
	"F$DATTmp",        /* Make temporary DAT image (Obsolete) */
	"F$LDAXY",         /* Load A [X,[Y]] */
	"F$LDAXYP",        /* Load A [X+,[Y]] */
	"F$LDDDXY",        /* Load D [D+X,[Y]] */
	"F$LDABX",         /* Load A from 0,X in task B */
	"F$STABX",         /* Store A at 0,X in task B */
	"F$AllPrc",        /* Allocate Process Descriptor */
	"F$DelPrc",        /* Deallocate Process Descriptor */
	"F$ELink",         /* Link using Module Directory Entry */
	"F$FModul",        /* Find Module Directory Entry */
	"F$MapBlk",        /* Map Specific Block */
	"F$ClrBlk",        /* Clear Specific Block */
	"F$DelRAM",        /* Deallocate RAM blocks */
	"F$GCMDir",        /* Pack module directory */
	"F$AlHRam",        /* Allocate HIGH RAM Blocks */
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
	"F$RegDmp",        /* Ron Lammardo's debugging register dump call */
	"F$NVRAM",         /* Non Volatile RAM (RTC battery backed static) read/write */
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
	"I$Attach",        /* Attach I/O Device */
	"I$Detach",        /* Detach I/O Device */
	"I$Dup",           /* Duplicate Path */
	"I$Create",        /* Create New File */
	"I$Open",          /* Open Existing File */
	"I$MakDir",        /* Make Directory File */
	"I$ChgDir",        /* Change Default Directory */
	"I$Delete",        /* Delete File */
	"I$Seek",          /* Change Current Position */
	"I$Read",          /* Read Data */
	"I$Write",         /* Write Data */
	"I$ReadLn",        /* Read Line of ASCII Data */
	"I$WritLn",        /* Write Line of ASCII Data */
	"I$GetStt",        /* Get Path Status */
	"I$SetStt",        /* Set Path Status */
	"I$Close",         /* Close Path */
	"I$DeletX"         /* Delete from current exec dir */
};


offs_t coco_state::dasm_override(device_t &device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options)
{
	unsigned call;
	unsigned result = 0;

	/* check for SWI2 instruction */
	if ((oprom[0] == 0x10) && (oprom[1] == 0x3F))
	{
		call = oprom[2];
		if ((call < ARRAY_LENGTH(os9syscalls)) && os9syscalls[call])
		{
			sprintf(buffer, "OS9   %s", os9syscalls[call]);
			result = 3;
		}
	}
	return result;
}
