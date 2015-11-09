// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

RCA COSMAC VIP

PCB Layout
----------

|-------------------------------------------------------------------------------|
|   |---------------CN1---------------|     |---------------CN2---------------| |
|CN6                                                                            |
|   |---------|                 |---------|             4050        4050        |
|   | CDPR566 |                 | CDP1861 |                                 CN3 |
|   |---------|                 |---------|                                     |
|                                                                           CN4 |
| 7805                                      |---------|      |--------|         |
|       2114                    3.521280MHz |  4508   |      |  4508  |     CN5 |
|                                           |---------|      |--------|         |
|       2114                                                 |--------|         |
|                               7474  7400  4049  4051  4028 |  4515  | CA3401  |
|       2114                                                 |--------|         |
|               |-------------|                                                 |
|       2114    |   CDP1802   |                                                 |
|               |-------------|             LED1                                |
|       2114                                                                    |
|                                           LED2                                |
|       2114    4556    4042                                                    |
|                                           LED3                                |
|  555  2114    4011    4013                                                    |
|                                           SW1                                 |
|       2114                                                                    |
|                                                                               |
|-------------------------------------------------------------------------------|

Notes:
    All IC's shown.

    CDPR566 - Programmed CDP1832 512 x 8-Bit Static ROM
    2114    - 2114 4096 Bit (1024x4) NMOS Static RAM
    CDP1802 - RCA CDP1802 CMOS 8-Bit Microprocessor
    CDP1861 - RCA CDP1861 Video Display Controller
    CA3401  - Quad Single-Supply Op-Amp
    CN1     - expansion interface connector
    CN2     - parallel I/O interface connector
    CN3     - video connector
    CN4     - tape in connector
    CN5     - tape out connector
    CN6     - power connector
    LED1    - power led
    LED2    - Q led
    LED3    - tape led
    SW1     - Run/Reset switch

*/

/*

    TODO:

    - ASCII keyboard
    - cassette loading
    - 20K RAM for Floating Point BASIC
    - VP-111 has 1K RAM, no byte I/O, no expansion
    - VP-601/611 ASCII Keyboard (VP-601 58 keys, VP611 58 keys + 16 keys numerical keypad)
    - VP-700 Expanded Tiny Basic Board (4 KB ROM expansion)

*/

/*

    VP-711 COSMAC MicroComputer $199
    (CDP18S711) Features RCA COSMAC microprocessor. 2K RAM, expandable to
    32K (4K on-board). Built-in cassette interface and video interface.
    16 key hexidecimal keypad. ROM operating system. CHIP-8 language and
    machine language. Tone generator and speaker. 8-bit input port, 8-bit
    output port, and full system expansion connector. Power supply and 3
    manuals (VP-311, VP-320, MPM201 B) included. Completely assembled.

    VP-44 VP-711 RAM On-Board Expansion Kit $36
    Four type 2114 RAM IC's for expanding VP-711 on-board memory
    to 4K bytes.

    VP-111 MicroComputer $99
    RCA COSMAC microprocessor. 1 K RAM expandable to 32K (4K On-
    board). Built-in cassette interface and video interface. 16 key
    Hexidecimal keypad. ROM operating system. CHIP-8 language and Machine
    language. Tone generator. Assembled - user must install Cables
    (supplied) and furnish 5 volt power supply and speaker.

    VP-114 VP-111 Expansion Kit $76
    Includes I/O ports, system expansion connector and additional
    3K of RAM. Expands VP-111 to VP-711 capability.

    VP-155 VP-111 Cover $12
    Attractive protective plastic cover for VP-111.

    VP-3301 Interactive Data Terminal Available Approx. 6 Months
    Microprocessor Based Computer Terminal with keyboard, video
    Interface and color graphics - includes full resident and user
    Definable character font, switch selectable configuration, cursor
    Control, reverse video and many other features.

    VP-590 Color Board $69
    Displays VP-711 output in color! Program control of four
    Background colors and eight foreground colors. CHIP-8X language
    Adds color commands. Includes two sockets for VP-580 Expansion
    Keyboards.

    VP-595 Simple Sound Board $30
    Provides 256 different frequencies in place of VP-711 single-
    tone Output. Use with VP-590 Color Board for simultaneous color and
    Sound. Great for simple music or sound effects! Includes speaker.

    VP-550 Super Sound Board $49
    Turn your VP-711 into a music synthesizer! Two independent
    sound Channels. Frequency, duration and amplitude envelope (voice) of
    Each channel under program control. On-board tempo control. Provision
    for multi-track recording or slaving VP-711's. Output drives audio
    preamp. Does not permit simultaneous video display.

    VP-551 Super Sound 4-Channel Expander Package $74
    VP-551 provides four (4) independent sound channels with
    frequency duration and amplitude envelope for each channel. Package
    includes modified VP-550 super sound board, VP-576 two board
    expander, data cassette with 4-channel PIN-8 program, and instruction
    manual. Requires 4K RAM system and your VP-550 Super Sound Board.

    VP-570 Memory Expansion Board $95
    Plug-in 4K static RAM memory. Jumper locates RAM in any 4K
    block in first 32K of VP-711 memory space.

    VP-580 Auxiliary Keyboard $20
    Adds two-player interactive game capability to VP-711 16-key
    keypad with cable. Connects to sockets on VP-590 Color Board or VP-
    585 Keyboard Interface.

    VP-585 Keyboard Interface Board $15
    Interfaces two VP-580 Expansion Keyboards directly to the VP-
    711. Not required when VP-590 Color Board is used.

    VP-560 EPROM Board $34
    Interfaces two Intel 2716 EPROMs to VP-711. Places EPROMs any-
    where in memory space. Can also re-allocate on-board RAM in memory
    space.

    VP-565 EPROM Programmer Board $99
    Programs Intel 2716 EPROMs with VP-711. Complete with software
    to program, copy, and verify. On-board generation of all programming
    voltages.

    VP-575 Expansion Board $59
    Plug-in board with 4 buffered and one unbuffered socket.
    Permits use of up to 5 Accessory Boards in VP-711 Expansion Socket.

    VP-576 Two-Board Expander $20
    Plug-in board for VP-711 I/O or Expansion Socket permits use
    of two Accessory Boards in either location.

    VP-601* ASCII Keyboard. 7-Bit Parallel Output $69
    Fully encoded, 128-character ASCII alphanumeric keyboard. 58
    light touch keys (2 user defined). Selectable "Upper-Case-Only".

    VP-606* ASCII Keyboard - Serial Output $99
    Same as VP-601. EIA RS232C compatible, 20 mA current loop and
    TTL outputs. Six selectable baud rates. Available mid-1980.

    VP-611* ASCII/Numeric Keyboard. 7-Bit Parallel Output $89
    ASCII Keyboard identical to VP-601 plus 16 key numeric entry
    keyboard for easier entry of numbers.

    VP-616* ASCII/Numeric Keyboard - Serial Output $119
    Same as VP-611. EIA RS232C compatible, 20 mA current loop and
    TTL outputs. Six selectable baud rates. Available mid-1980.

    VP-620 Cable: ASCII Keyboards to VP-711 $20
    Flat ribbon cable, 24" length, for connecting VP-601 or VP-
    611 and VP-711. Includes matching connector on both ends.

    VP-623 Cable: Parallel Output ASCII Keyboards $20
    Flat ribbon cable, 36" length with mating connector for VP-
    601 or VP-611 Keyboards. Other end is unterminated.

    VP-626 Connector: Serial Output ASCII Keyboards & Terminal $7
    25 pin solderable male "D" connector mates to VP-606, VP-616
    or VP-3301.

    TC1210 9" Video Monitor $195
    Ideal, low-cost monochrome monitor for displaying the video
    output from your microcomputer or terminal.

    TC1217 17" Video Monitor $480
    A really BIG monochrome monitor for use with your
    microcomputer or terminal 148 sq. in. pictures.

    VP-700 Tiny BASIC ROM Board $39
    Run Tiny BASIC on your VP-711! All BASIC code stored in ROM.
    Requires separate ASCII keyboard.

    VP-701 Floating Point BASIC for VP-711 $49
    16K bytes on cassette tape, includes floating point and
    integer math, string capability and color graphics. More than 70
    commands and statements. Available mid-1980.

    VP-710 Game Manual $10
    More exciting games for your VP-711! Includes Blackjack,
    Biorythm, Pinball, Bowling and 10 others.

    VP-720 Game Manual II More exciting games. Available mid-1980. $15

    VP-311 VP-711 Instruction Manual (Included with VP-711) $5

    VP-320 VP-711 User Guide Manual (Included with VP-711) $5

    MPM-201B CDP1802 User Manual (Included with VP-711) $5

    * Quantities of 15 or more available less case and speaker (Assembled
    keypad and circut board only). Price on request.

*/

#include "includes/vip.h"
#include "softlist.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

enum
{
	LED_POWER = 0,
	LED_Q,
	LED_TAPE
};



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  update_interrupts -
//-------------------------------------------------

void vip_state::update_interrupts()
{
	int irq = m_vdc_int || m_exp_int;
	int dma_in = m_exp_dma_in;
	int dma_out = m_vdc_dma_out || m_exp_dma_out;

	m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, irq);
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, dma_in);
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAOUT, dma_out);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( vip_state::read )
{
	int cs = BIT(offset, 15) || m_8000;
	int cdef = !((offset >= 0xc00) && (offset < 0x1000));
	int minh = 0;

	UINT8 data = m_exp->program_r(space, offset, cs, cdef, &minh);

	if (cs)
	{
		data = m_rom->base()[offset & 0x1ff];
	}
	else if (!minh)
	{
		data = m_ram->pointer()[offset & m_ram->mask()];
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( vip_state::write )
{
	int cs = BIT(offset, 15) || m_8000;
	int cdef = !((offset >= 0xc00) && (offset < 0x1000));
	int minh = 0;

	m_exp->program_w(space, offset, data, cdef, &minh);

	if (!cs && !minh)
	{
		m_ram->pointer()[offset & m_ram->mask()] = data;
	}
}


//-------------------------------------------------
//  io_r -
//-------------------------------------------------

READ8_MEMBER( vip_state::io_r )
{
	UINT8 data = m_exp->io_r(space, offset);

	switch (offset)
	{
	case 1:
		m_vdc->disp_on_w(1);
		m_vdc->disp_on_w(0);
		break;

	case 3:
		data = m_byteio_data;
		break;
	}

	if (BIT(offset, 2))
	{
		m_8000 = 0;
	}

	return data;
}


//-------------------------------------------------
//  io_w -
//-------------------------------------------------

WRITE8_MEMBER( vip_state::io_w )
{
	m_exp->io_w(space, offset, data);

	switch (offset)
	{
	case 1:
		m_vdc->disp_off_w(1);
		m_vdc->disp_off_w(0);
		break;

	case 2:
		m_keylatch = data & 0x0f;
		break;

	case 3:
		m_byteio->out_w(data);
		break;
	}

	if (BIT(offset, 2))
	{
		m_8000 = 0;
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( vip_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( vip_mem, AS_PROGRAM, 8, vip_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vip_io )
//-------------------------------------------------

static ADDRESS_MAP_START( vip_io, AS_IO, 8, vip_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x07) AM_READWRITE(io_r, io_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( vip )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( vip_state::reset_w )
{
	m_exp->run_w(newval);

	if (oldval && !newval)
	{
		machine_reset();
	}
}

INPUT_CHANGED_MEMBER( vip_state::beeper_w )
{
	m_beeper->set_output_gain(0, newval ? 0.80 : 0);
}

static INPUT_PORTS_START( vip )
	PORT_START("KEYPAD")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0 MW") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("A MR") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("B TR") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F TW") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Run/Reset") PORT_CODE(KEYCODE_R) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, vip_state, reset_w, 0)

	PORT_START("BEEPER")
	PORT_CONFNAME( 0x01, 0x01, "Internal Speaker" ) PORT_CHANGED_MEMBER(DEVICE_SELF, vip_state, beeper_w, 0)
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  COSMAC_INTERFACE( cosmac_intf )
//-------------------------------------------------

READ_LINE_MEMBER( vip_state::clear_r )
{
	return BIT(m_run->read(), 0);
}

READ_LINE_MEMBER( vip_state::ef1_r )
{
	return m_vdc_ef1 || m_exp->ef1_r();
}

READ_LINE_MEMBER( vip_state::ef2_r )
{
	set_led_status(machine(), LED_TAPE, m_cassette->input() > 0);

	return (m_cassette->input() < 0) ? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( vip_state::ef3_r )
{
	return !BIT(m_keypad->read(), m_keylatch) || m_byteio_ef3 || m_exp_ef3;
}

READ_LINE_MEMBER( vip_state::ef4_r )
{
	return m_byteio_ef4 || m_exp_ef4;
}

WRITE_LINE_MEMBER( vip_state::q_w )
{
	// sound output
	m_beeper->write(machine().driver_data()->generic_space(), NODE_01, state);

	// Q led
	set_led_status(machine(), LED_Q, state);

	// tape output
	m_cassette->output(state ? 1.0 : -1.0);

	// expansion
	m_exp->q_w(state);
}

READ8_MEMBER( vip_state::dma_r )
{
	return m_exp->dma_r(space, offset);
}

WRITE8_MEMBER( vip_state::dma_w )
{
	m_vdc->dma_w(space, offset, data);

	m_exp->dma_w(space, offset, data);
}

WRITE8_MEMBER( vip_state::sc_w )
{
	m_exp->sc_w(data);
}


//-------------------------------------------------
//  CDP1861_INTERFACE( vdc_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( vip_state::vdc_int_w )
{
	m_vdc_int = state;

	update_interrupts();
}

WRITE_LINE_MEMBER( vip_state::vdc_dma_out_w )
{
	m_vdc_dma_out = state;

	update_interrupts();
}

WRITE_LINE_MEMBER( vip_state::vdc_ef1_w )
{
	m_vdc_ef1 = state;
}

UINT32 vip_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_vdc->screen_update(screen, bitmap, cliprect);

	m_exp->screen_update(screen, bitmap, cliprect);

	return 0;
}


//-------------------------------------------------
//  DISCRETE_SOUND( vip )
//-------------------------------------------------

static const discrete_555_desc vip_ca555_a =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};

static DISCRETE_SOUND_START( vip )
	DISCRETE_INPUT_LOGIC(NODE_01)
	DISCRETE_555_ASTABLE_CV(NODE_02, NODE_01, 470, (int) RES_M(1), (int) CAP_P(470), NODE_01, &vip_ca555_a)
	DISCRETE_OUTPUT(NODE_02, 5000)
DISCRETE_SOUND_END


//-------------------------------------------------
//  VIP_BYTEIO_PORT_INTERFACE( byteio_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( vip_state::byteio_inst_w )
{
	if (!state)
	{
		m_byteio_data = m_byteio->in_r();
	}
}


//-------------------------------------------------
//  VIP_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( vip_state::exp_int_w )
{
	m_exp_int = state;

	update_interrupts();
}

WRITE_LINE_MEMBER( vip_state::exp_dma_out_w )
{
	m_exp_dma_out = state;

	update_interrupts();
}

WRITE_LINE_MEMBER( vip_state::exp_dma_in_w )
{
	m_exp_dma_in = state;

	update_interrupts();
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( vip )
//-------------------------------------------------

void vip_state::machine_start()
{
	UINT8 *ram = m_ram->pointer();

	// randomize RAM contents
	for (UINT16 addr = 0; addr < m_ram->size(); addr++)
	{
		ram[addr] = machine().rand() & 0xff;
	}

	// turn on power LED
	set_led_status(machine(), LED_POWER, 1);

	// reset sound
	m_beeper->write(machine().driver_data()->generic_space(), NODE_01, 0);

	// state saving
	save_item(NAME(m_8000));
	save_item(NAME(m_vdc_int));
	save_item(NAME(m_vdc_dma_out));
	save_item(NAME(m_vdc_ef1));
	save_item(NAME(m_exp_int));
	save_item(NAME(m_exp_dma_out));
	save_item(NAME(m_exp_dma_in));
	save_item(NAME(m_byteio_ef3));
	save_item(NAME(m_byteio_ef4));
	save_item(NAME(m_exp_ef1));
	save_item(NAME(m_exp_ef3));
	save_item(NAME(m_exp_ef4));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_byteio_data));
}


void vip_state::machine_reset()
{
	// reset the VDC
	m_vdc->reset();

	// force map ROM
	m_8000 = 1;

	// internal speaker
	m_beeper->set_output_gain(0, m_io_beeper->read() ? 0.80 : 0);

	// clear byte I/O latch
	m_byteio_data = 0;
}


//-------------------------------------------------
//  QUICKLOAD_LOAD_MEMBER( vip_state, vip )
//-------------------------------------------------

QUICKLOAD_LOAD_MEMBER( vip_state, vip )
{
	UINT8 *ram = m_ram->pointer();
	UINT8 *chip8_ptr = NULL;
	int chip8_size = 0;
	int size = image.length();

	if (strcmp(image.filetype(), "c8") == 0)
	{
		/* CHIP-8 program */
		chip8_ptr = m_chip8->base();
		chip8_size = m_chip8->bytes();
	}
	else if (strcmp(image.filename(), "c8x") == 0)
	{
		/* CHIP-8X program */
		chip8_ptr = m_chip8x->base();
		chip8_size = m_chip8x->bytes();
	}

	if ((size + chip8_size) > m_ram->size())
	{
		return IMAGE_INIT_FAIL;
	}

	if (chip8_size > 0)
	{
		/* copy CHIP-8 interpreter to RAM */
		memcpy(ram, chip8_ptr, chip8_size);
	}

	/* load image to RAM */
	image.fread(ram + chip8_size, size);

	return IMAGE_INIT_PASS;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( vip )
//-------------------------------------------------

static MACHINE_CONFIG_START( vip, vip_state )
	// basic machine hardware
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_3_52128MHz/2)
	MCFG_CPU_PROGRAM_MAP(vip_mem)
	MCFG_CPU_IO_MAP(vip_io)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(vip_state, clear_r))
	MCFG_COSMAC_EF1_CALLBACK(READLINE(vip_state, ef1_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(vip_state, ef2_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(vip_state, ef3_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(vip_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(vip_state, q_w))
	MCFG_COSMAC_DMAR_CALLBACK(READ8(vip_state, dma_r))
	MCFG_COSMAC_DMAW_CALLBACK(WRITE8(vip_state, dma_w))
	MCFG_COSMAC_SC_CALLBACK(WRITE8(vip_state, sc_w))

	// video hardware
	MCFG_DEVICE_ADD(CDP1861_TAG, CDP1861, XTAL_3_52128MHz/2)
	MCFG_CDP1861_IRQ_CALLBACK(WRITELINE(vip_state, vdc_int_w))
	MCFG_CDP1861_DMA_OUT_CALLBACK(WRITELINE(vip_state, vdc_dma_out_w))
	MCFG_CDP1861_EFX_CALLBACK(WRITELINE(vip_state, vdc_ef1_w))
	MCFG_CDP1861_SCREEN_ADD(CDP1861_TAG, SCREEN_TAG, XTAL_3_52128MHz/2)
	MCFG_SCREEN_UPDATE_DRIVER(vip_state, screen_update)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD(DISCRETE_TAG, DISCRETE, 0)
	MCFG_DISCRETE_INTF(vip)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_VIP_BYTEIO_PORT_ADD(VIP_BYTEIO_PORT_TAG, vip_byteio_cards, NULL, WRITELINE(vip_state, byteio_inst_w))
	MCFG_VIP_EXPANSION_SLOT_ADD(VIP_EXPANSION_SLOT_TAG, XTAL_3_52128MHz/2, vip_expansion_cards, NULL)
	MCFG_VIP_EXPANSION_SLOT_INT_CALLBACK(WRITELINE(vip_state, exp_int_w))
	MCFG_VIP_EXPANSION_SLOT_DMA_OUT_CALLBACK(WRITELINE(vip_state, exp_dma_out_w))
	MCFG_VIP_EXPANSION_SLOT_DMA_IN_CALLBACK(WRITELINE(vip_state, exp_dma_in_w))

	// devices
	MCFG_QUICKLOAD_ADD("quickload", vip_state, vip, "bin,c8,c8x", 0)
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)
	MCFG_CASSETTE_INTERFACE("vip_cass")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cass_list", "vip")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2K")
	MCFG_RAM_EXTRA_OPTIONS("4K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( vp111 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( vp111, vip )
	// internal ram
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1K")
	MCFG_RAM_EXTRA_OPTIONS("2K,4K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( vip )
//-------------------------------------------------

ROM_START( vip )
	ROM_REGION( 0x200, CDP1802_TAG, 0 )
	ROM_LOAD( "cdpr566.u10", 0x0000, 0x0200, CRC(5be0a51f) SHA1(40266e6d13e3340607f8b3dcc4e91d7584287c06) )

	ROM_REGION( 0x200, "chip8", 0 )
	ROM_LOAD( "chip8.bin", 0x0000, 0x0200, CRC(438ec5d5) SHA1(8aa634c239004ff041c9adbf9144bd315ab5fc77) )

	ROM_REGION( 0x300, "chip8x", 0 )
	ROM_LOAD( "chip8x.bin", 0x0000, 0x0300, CRC(79c5f6f8) SHA1(ed438747b577399f6ccbf20fe14156f768842898) )
ROM_END


//-------------------------------------------------
//  ROM( vp111 )
//-------------------------------------------------

#define rom_vp111 rom_vip



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY                             FULLNAME    FLAGS
COMP( 1977, vip,    0,      0,      vip,        vip, driver_device, 0,      "RCA",  "Cosmac VIP (VP-711)",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS )
COMP( 1977, vp111,  vip,    0,      vp111,      vip, driver_device, 0,      "RCA",  "Cosmac VIP (VP-111)",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS )
