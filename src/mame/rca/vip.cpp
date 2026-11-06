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


Usage:
    - If you turn it on as is, it quickly jumps into the weeds as it is expecting
      valid code to exist at 0000. To enter the monitor, press R, hold C, press R,
      (you will see the memory editor) then choose a command (0 for example).
    - If you load a chip-8 cart, press R twice. If it doesn't do anything you may
      need to do a hard reset, then hit R twice. R toggles between the CPU running
      or stopped. Most chip-8 (.c8) programs work, but make sure 4k RAM is enabled.
    - For VP-700 Tiny BASIC select the "basic" expansion option and the "ascii"
      byte I/O option, which connects a VP-601 or VP-611 keyboard.
    - For more RAM select the "exp" expansion option and put VP-570 boards in
      its slots, each jumpered to its own 4K block.
    - A VP-111 with the VP-114 Expansion Kit installed is equivalent to the
      VP-711, so use the vip driver for it.
    - Not known if (.bin) files work - don't have any for this machine.
    - (.c8x) files do not work.

*/

#include "emu.h"
#include "vip.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


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
//  board_r - on-board ROM and RAM read
//-------------------------------------------------

uint8_t vp111_state::board_r(offs_t offset, uint8_t data, int minh)
{
	if (cs(offset))
	{
		data = m_rom->base()[offset & 0x1ff];
	}
	else if (!minh && (offset & 0xfff) < m_ram->size())
	{
		data = m_ram->pointer()[offset & 0xfff];
	}

	return data;
}


//-------------------------------------------------
//  board_w - on-board RAM write
//-------------------------------------------------

void vp111_state::board_w(offs_t offset, uint8_t data, int minh)
{
	if (!cs(offset) && !minh && (offset & 0xfff) < m_ram->size())
	{
		m_ram->pointer()[offset & 0xfff] = data;
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t vp111_state::read(offs_t offset)
{
	return board_r(offset, 0xff, 0);
}

uint8_t vip_state::read(offs_t offset)
{
	int minh = 0;
	uint8_t const data = m_exp->program_r(offset, cs(offset), cdef(offset), &minh);

	return board_r(offset, data, minh);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void vp111_state::write(offs_t offset, uint8_t data)
{
	board_w(offset, data, 0);
}

void vip_state::write(offs_t offset, uint8_t data)
{
	int minh = 0;
	m_exp->program_w(offset, data, cdef(offset), &minh);

	board_w(offset, data, minh);
}


//-------------------------------------------------
//  io_r -
//-------------------------------------------------

uint8_t vp111_state::io_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (offset == 1)
		{
			m_vdc->disp_on_w(1);
			m_vdc->disp_on_w(0);
		}

		if (BIT(offset, 2))
		{
			m_8000 = 0;
		}
	}

	return 0xff;
}

uint8_t vip_state::io_r(offs_t offset)
{
	uint8_t data = m_exp->io_r(offset);

	if (offset == 3)
	{
		data = m_byteio_inst ? m_byteio->in_r() : m_byteio_data;
	}

	vp111_state::io_r(offset);

	return data;
}


//-------------------------------------------------
//  io_w -
//-------------------------------------------------

void vp111_state::io_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 1:
		m_vdc->disp_off_w(1);
		m_vdc->disp_off_w(0);
		break;

	case 2:
		m_keylatch = data & 0x0f;
		break;
	}

	if (BIT(offset, 2))
	{
		m_8000 = 0;
	}
}

void vip_state::io_w(offs_t offset, uint8_t data)
{
	m_exp->io_w(offset, data);

	if (offset == 3)
	{
		m_byteio->out_w(data);
	}

	vp111_state::io_w(offset, data);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( vip_mem )
//-------------------------------------------------

void vp111_state::vip_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(vp111_state::read), FUNC(vp111_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( vip_io )
//-------------------------------------------------

void vp111_state::vip_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x07).rw(FUNC(vp111_state::io_r), FUNC(vp111_state::io_w));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( vip )
//-------------------------------------------------

INPUT_CHANGED_MEMBER(vp111_state::reset_w)
{
	run_w(newval);

	if (oldval && !newval)
	{
		machine_reset();
	}
}

INPUT_CHANGED_MEMBER(vp111_state::beeper_w)
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Run/Reset") PORT_CODE(KEYCODE_R) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vp111_state::reset_w), 0)

	PORT_START("BEEPER")
	PORT_CONFNAME( 0x01, 0x01, "Internal Speaker" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vp111_state::beeper_w), 0)
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  COSMAC_INTERFACE( cosmac_intf )
//-------------------------------------------------

int vp111_state::clear_r()
{
	return BIT(m_run->read(), 0);
}

int vp111_state::int_r()
{
	return m_vdc->int_r();
}

int vip_state::int_r()
{
	return vp111_state::int_r() || m_exp_int;
}

int vp111_state::dma_out_r()
{
	return m_vdc->dma_out_r();
}

int vip_state::dma_out_r()
{
	return vp111_state::dma_out_r() || m_exp_dma_out;
}

int vp111_state::ef1_r()
{
	return m_vdc->efx_r();
}

int vip_state::ef1_r()
{
	return vp111_state::ef1_r() || m_exp->ef1_r();
}

int vp111_state::ef2_r()
{
	m_leds[LED_TAPE] = m_cassette->input() > 0 ? 1 : 0;

	return (m_cassette->input() < 0) ? ASSERT_LINE : CLEAR_LINE;
}

int vp111_state::ef3_r()
{
	return !BIT(m_keypad->read(), m_keylatch);
}

int vip_state::ef3_r()
{
	return vp111_state::ef3_r() || m_byteio->ef3_r() || m_exp->ef3_r();
}

int vp111_state::ef4_r()
{
	return CLEAR_LINE;
}

int vip_state::ef4_r()
{
	return m_byteio->ef4_r() || m_exp->ef4_r();
}

void vp111_state::q_w(int state)
{
	// sound output
	m_beeper->write(NODE_01, state);

	// Q led
	m_leds[LED_Q] = state ? 1 : 0;

	// tape output
	m_cassette->output(state ? 1.0 : -1.0);
}

void vip_state::q_w(int state)
{
	vp111_state::q_w(state);

	m_byteio->q_w(state);
	m_exp->q_w(state);
}

void vp111_state::run_w(int state)
{
}

void vip_state::run_w(int state)
{
	m_exp->run_w(state);
}


uint32_t vp111_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_vdc->screen_update(screen, bitmap, cliprect);

	return 0;
}

uint32_t vip_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	vp111_state::screen_update(screen, bitmap, cliprect);

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

static DISCRETE_SOUND_START( vip_discrete )
	DISCRETE_INPUT_LOGIC(NODE_01)
	DISCRETE_555_ASTABLE_CV(NODE_02, NODE_01, 470, RES_M(1), CAP_P(470), NODE_01, &vip_ca555_a)
	DISCRETE_OUTPUT(NODE_02, 5000)
DISCRETE_SOUND_END


//-------------------------------------------------
//  VIP_BYTEIO_PORT_INTERFACE( byteio_intf )
//-------------------------------------------------

void vip_state::byteio_inst_w(int state)
{
	if (m_byteio_inst && !state)
	{
		m_byteio_data = m_byteio->in_r();
	}

	m_byteio_inst = bool(state);
}


//-------------------------------------------------
//  VIP_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

void vip_state::exp_int_w(int state)
{
	m_exp_int = state;
}

void vip_state::exp_dma_out_w(int state)
{
	m_exp_dma_out = state;
}

void vip_state::exp_dma_in_w(int state)
{
	m_exp_dma_in = state;

	m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, state);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( vip )
//-------------------------------------------------

void vp111_state::machine_start()
{
	uint8_t *ram = m_ram->pointer();

	// randomize RAM contents
	for (uint16_t addr = 0; addr < m_ram->size(); addr++)
	{
		ram[addr] = machine().rand() & 0xff;
	}

	// turn on power LED
	m_leds[LED_POWER] = 1;

	// reset sound
	m_beeper->write(NODE_01, 0);

	// state saving
	save_item(NAME(m_8000));
	save_item(NAME(m_keylatch));
}

void vip_state::machine_start()
{
	vp111_state::machine_start();

	// state saving
	save_item(NAME(m_exp_int));
	save_item(NAME(m_exp_dma_out));
	save_item(NAME(m_exp_dma_in));
	save_item(NAME(m_byteio_data));
	save_item(NAME(m_byteio_inst));
}


void vp111_state::machine_reset()
{
	// reset the VDC
	m_vdc->reset();

	// force map ROM
	m_8000 = 1;

	// internal speaker
	m_beeper->set_output_gain(0, m_io_beeper->read() ? 0.80 : 0);
}

void vip_state::machine_reset()
{
	vp111_state::machine_reset();

	// clear byte I/O latch
	m_byteio_data = 0;
}


//-------------------------------------------------
//  QUICKLOAD_LOAD_MEMBER( vip_state, vip )
//-------------------------------------------------

QUICKLOAD_LOAD_MEMBER(vp111_state::quickload_cb)
{
	uint8_t *ram = m_ram->pointer();
	uint8_t *chip8_ptr = nullptr;
	int chip8_size = 0;
	int size = image.length();

	if (image.is_filetype("c8"))
	{
		/* CHIP-8 program */
		chip8_ptr = m_chip8->base();
		chip8_size = m_chip8->bytes();
	}
	else if (image.is_filetype("c8x"))
	{
		/* CHIP-8X program */
		chip8_ptr = m_chip8x->base();
		chip8_size = m_chip8x->bytes();
	}

	if ((size + chip8_size) > m_ram->size())
	{
		return std::make_pair(
				image_error::INVALIDIMAGE,
				util::string_format("%u-byte interpreter and %u-byte program are too large for %u-byte RAM", chip8_size, size, m_ram->size()));
	}

	if (chip8_size > 0)
	{
		/* copy CHIP-8 interpreter to RAM */
		memcpy(ram, chip8_ptr, chip8_size);
	}

	/* load image to RAM */
	image.fread(ram + chip8_size, size);

	return std::make_pair(std::error_condition(), std::string());
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( vp111 )
//-------------------------------------------------

void vp111_state::vp111(machine_config &config)
{
	// basic machine hardware
	CDP1802(config, m_maincpu, 3.52128_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &vp111_state::vip_mem);
	m_maincpu->set_addrmap(AS_IO, &vp111_state::vip_io);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(vp111_state::clear_r));
	m_maincpu->int_cb().set(FUNC(vp111_state::int_r));
	m_maincpu->dma_out_cb().set(FUNC(vp111_state::dma_out_r));
	m_maincpu->ef1_cb().set(FUNC(vp111_state::ef1_r));
	m_maincpu->ef2_cb().set(FUNC(vp111_state::ef2_r));
	m_maincpu->ef3_cb().set(FUNC(vp111_state::ef3_r));
	m_maincpu->ef4_cb().set(FUNC(vp111_state::ef4_r));
	m_maincpu->q_cb().set(FUNC(vp111_state::q_w));
	m_maincpu->dma_wr_cb().set(m_vdc, FUNC(cdp1861_device::dma_w));
	m_maincpu->sc_cb().set(m_vdc, FUNC(cdp1861_device::sc_w));
	m_maincpu->tpa_cb().set(m_vdc, FUNC(cdp1861_device::tpa_w));

	// video hardware
	CDP1861(config, m_vdc, 3.52128_MHz_XTAL / 2).set_screen(SCREEN_TAG);

	screen_device &screen(SCREEN(config, SCREEN_TAG));
	screen.set_screen_update(FUNC(vp111_state::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_beeper, vip_discrete);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.80);

	// devices
	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "bin,c8", attotime::from_seconds(2)));
	quickload.set_load_callback(FUNC(vp111_state::quickload_cb));
	quickload.set_interface("chip8quik");
	SOFTWARE_LIST(config, "quik_list").set_original("chip8_quik").set_filter("V");

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("vip_cass");
	SOFTWARE_LIST(config, "cass_list").set_original("vip");

	// internal ram
	RAM(config, m_ram).set_default_size("1K").set_extra_options("2K,3K,4K");
}


//-------------------------------------------------
//  machine_config( vip )
//-------------------------------------------------

void vip_state::vip(machine_config &config)
{
	vp111(config);

	m_maincpu->dma_rd_cb().set(m_exp, FUNC(vip_expansion_slot_device::dma_r));
	m_maincpu->dma_wr_cb().append(m_exp, FUNC(vip_expansion_slot_device::dma_w));
	m_maincpu->sc_cb().append(m_exp, FUNC(vip_expansion_slot_device::sc_w));
	m_maincpu->tpb_cb().set(m_exp, FUNC(vip_expansion_slot_device::tpb_w));

	VIP_BYTEIO_PORT(config, m_byteio, vip_byteio_cards, nullptr);
	m_byteio->inst_callback().set(FUNC(vip_state::byteio_inst_w));

	VIP_EXPANSION_SLOT(config, m_exp, 3.52128_MHz_XTAL / 2, vip_expansion_cards, nullptr);
	m_exp->int_wr_callback().set(FUNC(vip_state::exp_int_w));
	m_exp->dma_out_wr_callback().set(FUNC(vip_state::exp_dma_out_w));
	m_exp->dma_in_wr_callback().set(FUNC(vip_state::exp_dma_in_w));

	// internal ram
	m_ram->set_default_size("2K").set_extra_options("3K,4K");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( vip )
//-------------------------------------------------

ROM_START( vip )
	ROM_REGION( 0x200, CDP1802_TAG, 0 )
	ROM_LOAD( "cdpr566.u10", 0x0000, 0x0200, CRC(334f5527) SHA1(20c9bf689766c09611547db4b3c7a6911c3eb381) )

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

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME                FLAGS
COMP( 1977, vip,   0,      0,      vip,     vip,   vip_state,   empty_init, "RCA",   "Cosmac VIP (VP-711)",  MACHINE_SUPPORTS_SAVE )
COMP( 1977, vp111, vip,    0,      vp111,   vip,   vp111_state, empty_init, "RCA",   "Cosmac VIP (VP-111)",  MACHINE_SUPPORTS_SAVE )
