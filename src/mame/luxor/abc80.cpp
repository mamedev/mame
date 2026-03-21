// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

Luxor ABC 80

PCB Layout
----------

55 10470-02

          CN1               CN2                                                CN5
  SW1   |-----|  |------------------------|                                  |-----|
|-------|     |--|                        |----------------------------------|     |---|
|                                                             CN3       CN4            |
|                                                    7912                              |
|            MC1488                                                                    |
|   MC1489                                           7812                              |
|            LS245                     LS138                                           |
|                                                   |-------|                          |
|   |-----CN6-----|   LS241   LS241    LS138  LS32  |SN76477| LS04    LM339            |
|                                                   |-------|                          |
|   |--------------|  |------------|   PROM0  LS132   LS273   7406    XTAL     LS08    |
|   |   Z80A PIO   |  |    Z80A    |                                                   |
|   |--------------|  |------------|   LS04   LS74A   LS86    LS161   LS166    74393   |
|                                                                                      |
|     ROM0   LS107    4116    4116     LS10   LS257   LS74A   LS08    LS107    PROM2   |
|                                                                                      |
|     ROM2   LS257    4116    4116     LS139  74393   LS107   LS32    LS175    74393   |
|                                                                                      |
|     ROM1   LS257    4116    4116     LS08   LS283   LS10    LS32    PROM1    74393   |
|                                                                                      |
|     ROM3   LS257    4116    4116     LS257  74393   LS375   74S263  LS145    PROM4   |
|                                                                                      |
|     SB1    SB2      4045    4045     LS257  LS245   LS375   LS273   LS166    PROM3   |
|--------------------------------------------------------------------------------------|

Notes:
    All IC's shown.

    ROM0-3  - Texas Instruments TMS4732 4Kx8 General Purpose Mask Programmable ROM
    PROM0-2 - Philips Semiconductors N82S129 256x4 TTL Bipolar PROM
    PROM3-4 - Philips Semiconductors N82S131 512x4 TTL Bipolar PROM
    4116    - Texas Instruments TMS4116-25 16Kx1 Dynamic RAM
    4045    - Texas Instruments TMS4045-15 1Kx4 General Purpose Static RAM with Multiplexed I/O
    Z80A    - Sharp LH0080A Z80A CPU
    Z80APIO - SGS-Thomson Z8420AB1 Z80A PIO
    SN76477 - Texas Instruments SN76477N Complex Sound Generator
    74S263  - Texas Instruments SN74S263N Row Output Character Generator
    MC1488  - Texas Instruments MC1488 Quadruple Line Driver
    MC1489  - Texas Instruments MC1489 Quadruple Line Receiver
    XTAL    - 11.9808 MHz Crystal
    CN1     - RS-232 connector
    CN2     - ABC bus connector (DIN 41612)
    CN3     - video connector
    CN4     - cassette motor connector
    CN5     - cassette connector
    CN6     - keyboard connector
    SW1     - reset switch
    SB1     - solder bridge for A11/CS1 to ROM1/ROM3
    SB2     - solder bridge for A11/CS1 to ROM0/ROM2


Calculate ROM checksum:

10 FOR I%=0% TO 16383%
20 A%=A%+PEEK(I%)
30 NEXT I%
40 ;A%
RUN

*/

/*

Luxor ABC 80 with ABC80L

PCB Layout
----------

CATAB-80:1718

          CN1               CN2                                                CN5
  SW1   |-----|  |------------------------|                                  |-----|
|-------|     |--|                        |----------------------------------|     |---|
|                                                             CN3       CN4            |
|                                                    7912                              |
|            MC1488                                                                    |
|   MC1489                                           7812                              |
|            LS245                     LS138                                           |
|                 |-------------------------------------------------|                  |
|   |-----CN6-----|                                                 | LM339            |
|                 |   |------------|                                |                  |
|   |-------------|   |    Z80A    |    LS266 LS04 LS00 LS10 LS03   | XTAL     LS08    |
|   |   Z80A PIO  |   |------------|                                |                  |
|   |-------------|                                                 | LS166    74393   |
|                 |                                                 |                  |
|     ROM0   LS107|   LS367       RAM RAM RAM RAM       LS125 LS74  | LS107    PROM2   |
|                 |        LS157    RAM RAM RAM RAM     LS139 LS74  |                  |
|     ROM2   LS257|   ROM                                           | LS175    74393   |
|                 |        LS157  *   *   *   *                     |                  |
|     ROM1   LS257|                 *   *   *   *                   | PROM1    74393   |
|                 |-------------------------------------------------|                  |
|     ROM3   LS257    4116    4116     LS257  74393   LS375   74S263  LS145    PROM4   |
|                                                                                      |
|     SB1    SB2      4045    4045     LS257  LS245   LS375   LS273   LS166    PROM3   |
|--------------------------------------------------------------------------------------|

Notes:
    All IC's shown.

    ROM     - Mostek MK2716J-B 2Kx8 EPROM "L3.0"
    RAM     - Mostek MK4116J-3 16Kx1 Dynamic RAM
    *       - Empty footprints for additional RAM chips


Activation:

;CALL(20480)

New BASIC commands:

IPR, EXIT, CAT, DEL, LIST, RUN, VAR, CROSS, LEN, FREE, BOFA, EOFA, CONT, AUTO, ED, <Ctrl-L>

*/

/*

Luxor ABC 80 with TKN 80

PCB Layout
----------

55 10470-02

          CN1               CN2                                                CN5
  SW1   |-----|  |------------------------|                                  |-----|
|-------|     |--|                        |----------------------------------|     |---|
|                                                             CN3       CN4            |
|                                                    7912                              |
|            MC1488                                                                    |
|   MC1489                                           7812                              |
|            LS245                     LS138                                           |
|                                                   |-------|                          |
|   |-----CN6-----|   LS241   LS241    LS138  LS32  |SN76477| LS04    LM339            |
|                                                   |-------|                          |
|   |--------------|  |------------|   PROM0  LS132   LS273   7406    XTAL     LS08    |
|   |   Z80A PIO   |  |    Z80A    |                                                   |
|   |--------------|  |------------|   LS04   LS74A   LS86    LS161   LS166    74393   |
|                                                                                      |
|     ROM0   LS107    4116    4116     LS10   LS257   LS74A   LS08    LS107    PROM2   |
|                                                                                      |
|     ROM2   LS257    4116    4116     LS139  74393   LS107   LS32    LS175    74393   |
|                                                                                      |
|     ROM1   LS257    4116    4116     LS08   LS283   LS10    LS32    PROM1    74393   |
|-----------|                                                                          |
|     ROM3  |       |------------------------------------------------|LS145    PROM4   |
|           |-------|                                                |                 |
|     LS32   LS00 LS257  LS257  LS257  LS00 LS04  LS02  LS51  LS374  |LS166    PROM3   |
|                                                                    |-----------------|
|     ROM4   LS373    LS244   6116    LS74A LS74A LS163 LS163 LS374  |
|                                                             LS374  |
|--------------------------------------------------------------------|

Notes:
    All IC's shown.

    PROM0   - MMI 63S141N 512x4 TTL Bipolar PROM
    ROM4    - 4Kx8 EPROM "TKN80-III"
    6116    - Hitachi HM6116P-4 2Kx8 Static RAM


Switch to 40 column mode:

;INP(3);CHR$(12)

Switch to 80 column mode:

;INP(4);CHR$(12)

*/

/*

    TODO:

    - PWM sound in ABC-klubben/abc80/grafik/flagga.bac
    - proper keyboard controller emulation
    - GeJo 80-column card
    - 64K RAM expansions
        - Mikrodatorn
        - MYAB UNI-80
    - Metric ABC CAD 1000

*/

#include "emu.h"
#include "abc80.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  SOUND
//**************************************************************************

//-------------------------------------------------
//  csg_w -
//-------------------------------------------------

void abc80_state::csg_w(u8 data)
{
	m_csg->enable_w(!BIT(data, 0));
	m_csg->vco_voltage_w(BIT(data, 1) ? 2.5 : 0);
	m_csg->vco_w(BIT(data, 2));
	m_csg->mixer_b_w(BIT(data, 3));
	m_csg->mixer_a_w(BIT(data, 4));
	m_csg->mixer_c_w(BIT(data, 5));
	m_csg->envelope_2_w(BIT(data, 6));
	m_csg->envelope_1_w(BIT(data, 7));
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( abc80_mem )
//-------------------------------------------------

void abc80_state::abc80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().region(Z80_TAG, 0);
	map(0x4000, 0xbfff).lrw8( // fix offset for external memory
		NAME([this](offs_t offset) { return m_bus->xmemfl_r(offset + 0x4000); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->xmemw_w(offset + 0x4000, data); })
	);
	map(0x7c00, 0x7fff).ram().share(m_video_ram);
	map(0xc000, 0xffff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( abc80l_mem )
//-------------------------------------------------

void abc80l_state::abc80l_mem(address_map &map)
{
	abc80_mem(map);
	map(0x5000, 0x57ff).rom().region("abc80l", 0);
	map(0x8000, 0xbfff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( tkn80_mem )
//-------------------------------------------------

void tkn80_state::tkn80_mem(address_map &map)
{
	abc80_mem(map);
	map(0x0000, 0x03ff).view(m_view_rom0);
	m_view_rom0[0](0x0000, 0x03ff).rom().region(Z80_TAG, 0);
	m_view_rom0[1](0x0000, 0x03ff).rom().region("tkn80", 0);
	m_view_rom0[2](0x0000, 0x03ff).rom().region("tkn80", 0x800);
	map(0x2000, 0x23ff).view(m_view_rom2);
	m_view_rom2[0](0x2000, 0x23ff).rom().region(Z80_TAG, 0x2000);
	m_view_rom2[1](0x2000, 0x23ff).rom().region("tkn80", 0x400);
	m_view_rom2[2](0x2000, 0x23ff).rom().region("tkn80", 0xc00);
	map(0x5800, 0x5fff).ram().share(m_char_ram);
	map(0x7c00, 0x7fff).lrw8( // map only the upper 1K of char RAM
		NAME([this](offs_t offset) { return m_char_ram[0x400 | offset]; }),
		NAME([this](offs_t offset, uint8_t data) { m_char_ram[0x400 | offset] = data; })
	);
}


//-------------------------------------------------
//  ADDRESS_MAP( abc80_io )
//-------------------------------------------------

void abc80_state::abc80_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x17);
	map(0x00, 0x00).rw(m_bus, FUNC(abcbus_slot_device::inp_r), FUNC(abcbus_slot_device::out_w));
	map(0x01, 0x01).rw(m_bus, FUNC(abcbus_slot_device::stat_r), FUNC(abcbus_slot_device::cs_w));
	map(0x02, 0x02).w(m_bus, FUNC(abcbus_slot_device::c1_w));
	map(0x03, 0x03).w(m_bus, FUNC(abcbus_slot_device::c2_w));
	map(0x04, 0x04).w(m_bus, FUNC(abcbus_slot_device::c3_w));
	map(0x05, 0x05).w(m_bus, FUNC(abcbus_slot_device::c4_w));
	map(0x06, 0x06).w(FUNC(abc80_state::csg_w));
	map(0x07, 0x07).r(m_bus, FUNC(abcbus_slot_device::rst_r));
	map(0x10, 0x13).mirror(0x04).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}


//-------------------------------------------------
//  ADDRESS_MAP( tkn80_io )
//-------------------------------------------------

void tkn80_state::tkn80_io(address_map &map)
{
	abc80_io(map);
	map(0x03, 0x03).r(FUNC(tkn80_state::in3_r));
	map(0x04, 0x04).r(FUNC(tkn80_state::in4_r));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( tkn80 )
//-------------------------------------------------

static INPUT_PORTS_START( tkn80 )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Columns" )
	PORT_CONFSETTING(    0x00, "40" )
	PORT_CONFSETTING(    0x01, "80" )
	PORT_CONFNAME( 0x02, 0x02, "Cursor" )
	PORT_CONFSETTING(    0x00, "Static" )
	PORT_CONFSETTING(    0x02, "Blinking" )
	PORT_CONFNAME( 0x04, 0x00, "ROM Checksum" )
	PORT_CONFSETTING(    0x00, "9913/10042" )
	PORT_CONFSETTING(    0x04, "11273" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  Z80PIO
//-------------------------------------------------

u8 abc80_state::pio_pa_r()
{
	/*

	    PIO Port A

	    bit     description

	    0       keyboard data
	    1       keyboard data
	    2       keyboard data
	    3       keyboard data
	    4       keyboard data
	    5       keyboard data
	    6       keyboard data
	    7       keyboard strobe

	*/

	u8 data = 0;

	//data |= m_kb->data_r();
	data |= m_key_data;
	data |= (m_key_strobe << 7);

	return data;
}

u8 abc80_state::pio_pb_r()
{
	/*

	    PIO Channel B

	    0  R    RS-232C RxD
	    1  R    RS-232C _CTS
	    2  R    RS-232C _DCD
	    3  W    RS-232C TxD
	    4  W    RS-232C _RTS
	    5  W    Cassette Motor
	    6  W    Cassette Data
	    7  R    Cassette Data

	*/

	u8 data = 0;

	// receive data
	data |= m_rs232->rxd_r();

	// clear to send
	data |= m_rs232->cts_r() << 1;

	// data carrier detect
	data |= m_rs232->dcd_r() << 2;

	// cassette data
	data |= m_tape_in_latch << 7;

	if (LOG) logerror("%s %s read tape latch %u\n", machine().time().as_string(), machine().describe_context(), m_tape_in_latch);

	return data;
}

void abc80_state::pio_pb_w(u8 data)
{
	/*

	    PIO Channel B

	    0  R    RS-232C RxD
	    1  R    RS-232C _CTS
	    2  R    RS-232C _DCD
	    3  W    RS-232C TxD
	    4  W    RS-232C _RTS
	    5  W    Cassette Motor
	    6  W    Cassette Data
	    7  R    Cassette Data

	*/

	// transmit data
	m_rs232->write_txd(BIT(data, 3));

	// request to send
	m_rs232->write_rts(BIT(data, 4));

	// cassette motor
	if (BIT(data, 5))
	{
		if (!m_motor) if (LOG) logerror("%s %s started cassette motor\n", machine().time().as_string(), machine().describe_context());
		m_cassette->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
		m_motor = true;
	}
	else
	{
		if (m_motor) if (LOG) logerror("%s %s stopped cassette motor\n", machine().time().as_string(), machine().describe_context());
		m_cassette->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_motor = false;
	}

	// cassette data
	m_cassette->output(BIT(data, 6) ? -1.0 : +1.0);

	// cassette input latch
	if (BIT(data, 6))
	{
		if (LOG) logerror("%s %s clear tape in latch\n", machine().time().as_string(), machine().describe_context());

		m_tape_in_latch = 1;

		m_pio->pb7_w(m_tape_in_latch);
	}
}


//-------------------------------------------------
//  Z80 Daisy Chain
//-------------------------------------------------

static const z80_daisy_config abc80_daisy_chain[] =
{
	{ Z80PIO_TAG },
	{ nullptr }
};


//-------------------------------------------------
//  ABC80_KEYBOARD_INTERFACE
//-------------------------------------------------

void abc80_state::keydown_w(int state)
{
	m_key_strobe = state;

	m_pio->port_a_write(m_key_strobe << 7);
}

void abc80_state::kbd_w(u8 data)
{
	m_key_data = data;
	m_key_strobe = 1;

	u8 pio_data = 0x80 | data;
	m_pio->port_a_write(pio_data);

	m_keyboard_clear_timer->adjust(attotime::from_msec(50));
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(abc80_state::scanline_tick)
{
	draw_scanline(m_bitmap, m_screen->vpos());

	m_pio_astb = !m_pio_astb;

	m_pio->strobe_a(m_pio_astb);
}

TIMER_CALLBACK_MEMBER(abc80_state::cassette_update)
{
	if (!m_motor)
		return;

	int tape_in = m_cassette->input() > 0;

	if (m_tape_in != tape_in)
		if (LOG) logerror("%s tape flank %u\n", machine().time().as_string(), tape_in);

	if (m_tape_in_latch && (m_tape_in != tape_in))
	{
		if (LOG) logerror("%s set tape in latch\n", machine().time().as_string());
		m_tape_in_latch = 0;

		m_pio->port_b_write(m_tape_in_latch << 7);
	}

	m_tape_in = tape_in;
}

TIMER_CALLBACK_MEMBER(abc80_state::blink_tick)
{
	m_blink = !m_blink;
}

TIMER_CALLBACK_MEMBER(abc80_state::vsync_on)
{
	if (LOG) logerror("%s vsync 1\n", machine().time().as_string());
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(abc80_state::vsync_off)
{
	if (LOG) logerror("%s vsync 0\n", machine().time().as_string());
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(abc80_state::clear_keyboard)
{
	m_pio->port_a_write(m_key_data);
	m_key_strobe = 0;
	m_key_data = 0;
}


//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void abc80_state::machine_start()
{
	// start timers
	m_cassette_timer = timer_alloc(FUNC(abc80_state::cassette_update), this);
	m_cassette_timer->adjust(attotime::from_hz(44100), 0, attotime::from_hz(44100));
	m_keyboard_clear_timer = timer_alloc(FUNC(abc80_state::clear_keyboard), this);

	// register for state saving
	save_item(NAME(m_key_data));
	save_item(NAME(m_key_strobe));
	save_item(NAME(m_pio_astb));
	save_item(NAME(m_latch));
	save_item(NAME(m_blink));
	save_item(NAME(m_motor));
	save_item(NAME(m_tape_in));
	save_item(NAME(m_tape_in_latch));

	// zero-fill
	m_key_data = 0;
	m_key_strobe = 0;
	m_blink = 0;
}


void tkn80_state::machine_start()
{
	abc80_state::machine_start();

	// register for state saving
	save_item(NAME(m_80));
}


void tkn80_state::machine_reset()
{
}



//-------------------------------------------------
//  QUICKLOAD_LOAD_MEMBER( quickload_cb )
//-------------------------------------------------

QUICKLOAD_LOAD_MEMBER(abc80_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	offs_t address = space.read_byte(BOFA + 1) << 8 | space.read_byte(BOFA);
	if (LOG) logerror("BOFA %04x\n",address);

	int quickload_size = image.length();
	std::vector<u8> data(quickload_size);
	image.fread(&data[0], quickload_size);
	for (int i = 1; i < quickload_size; i++)
		space.write_byte(address++, data[i]);

	offs_t eofa = address;
	space.write_byte(EOFA, eofa & 0xff);
	space.write_byte(EOFA + 1, eofa >> 8);
	if (LOG) logerror("EOFA %04x\n",address);

	offs_t head = address + 1;
	space.write_byte(HEAD, head & 0xff);
	space.write_byte(HEAD + 1, head >> 8);
	if (LOG) logerror("HEAD %04x\n",address);

	return std::make_pair(std::error_condition(), std::string());
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( abc80_common )
//-------------------------------------------------

void abc80_state::abc80_common(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(11'980'800)/2/2); // 2.9952 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &abc80_state::abc80_mem);
	m_maincpu->set_addrmap(AS_IO, &abc80_state::abc80_io);
	m_maincpu->set_daisy_config(abc80_daisy_chain);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76477(config, m_csg);
	m_csg->set_noise_params(RES_K(47), RES_K(330), CAP_P(390));
	m_csg->set_decay_res(RES_K(47));
	m_csg->set_attack_params(CAP_U(10), RES_K(2.2));
	m_csg->set_amp_res(RES_K(33));
	m_csg->set_feedback_res(RES_K(10));
	m_csg->set_vco_params(0, CAP_N(10), RES_K(100));
	m_csg->set_pitch_voltage(0);
	m_csg->set_slf_params(CAP_U(1), RES_K(220));
	m_csg->set_oneshot_params(CAP_U(0.1), RES_K(330));
	m_csg->add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	Z80PIO(config, m_pio, XTAL(11'980'800)/2/2);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->in_pa_callback().set(FUNC(abc80_state::pio_pa_r));
	m_pio->in_pb_callback().set(FUNC(abc80_state::pio_pb_r));
	m_pio->out_pb_callback().set(FUNC(abc80_state::pio_pb_w));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("abc80_cass");

	ABC80_KEYBOARD(config, m_kb, 0);
	m_kb->keydown_wr_callback().set(FUNC(abc80_state::keydown_w));

	ABCBUS_SLOT(config, m_bus, XTAL(11'980'800)/2/2, abc80_cards, "abcexp");

	RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr);
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "generic_kb", 0));
	keyboard.set_keyboard_callback(FUNC(abc80_state::kbd_w));

	QUICKLOAD(config, "quickload", "bac", attotime::from_seconds(2)).set_load_callback(FUNC(abc80_state::quickload_cb));

	// internal ram
	RAM(config, RAM_TAG).set_default_size("16K");

	// software list
	SOFTWARE_LIST(config, "cass_list").set_original("abc80_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("abc80_flop");
	SOFTWARE_LIST(config, "rom_list").set_original("abc80_rom");
}

void abc80_state::abc80(machine_config &config)
{
	abc80_state::abc80_common(config);

	// video hardware
	abc80_video(config);
}


//-------------------------------------------------
//  machine_config( abc80l )
//-------------------------------------------------

void abc80l_state::abc80l(machine_config &config)
{
	abc80_state::abc80_common(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &abc80l_state::abc80l_mem);

	// video hardware
	abc80_video(config);
}


//-------------------------------------------------
//  machine_config( tkn80 )
//-------------------------------------------------

void tkn80_state::tkn80(machine_config &config)
{
	abc80_state::abc80_common(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &tkn80_state::tkn80_mem);
	m_maincpu->set_addrmap(AS_IO, &tkn80_state::tkn80_io);

	// video hardware
	tkn80_video(config);
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( abc80 )
//-------------------------------------------------

ROM_START( abc80 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("9913")
	ROM_SYSTEM_BIOS( 0, "11273", "Checksum 11273" )
	ROMX_LOAD( "za3506_11273.a5", 0x0000, 0x1000, CRC(7c004fb6) SHA1(9aee1d085122f4537c3e6ecdab9d799bd429ef52), ROM_BIOS(0) )
	ROMX_LOAD( "za3507_11273.a3", 0x1000, 0x1000, CRC(d1850a84) SHA1(f7719f3af9173601a2aa23ae38ae00de1a387ad8), ROM_BIOS(0) )
	ROMX_LOAD( "za3508_11273.a4", 0x2000, 0x1000, CRC(b55528e9) SHA1(3e5017e8cacad1f13215242f1bbd89d1d3eee131), ROM_BIOS(0) )
	ROMX_LOAD( "za3509_11273.a2", 0x3000, 0x1000, CRC(659cab1e) SHA1(181db748cef22cdcccd311a60aa6189c85343db7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "10042", "Checksum 10042" )
	ROMX_LOAD( "za3506_9913.a5", 0x0000, 0x1000, CRC(e2afbf48) SHA1(9883396edd334835a844dcaa792d29599a8c67b9), ROM_BIOS(1) )
	ROMX_LOAD( "za3507_9913.a3", 0x1000, 0x1000, CRC(d224412a) SHA1(30968054bba7c2aecb4d54864b75a446c1b8fdb1), ROM_BIOS(1) )
	ROMX_LOAD( "za3508_9913.a4", 0x2000, 0x1000, CRC(1502ba5b) SHA1(5df45909c2c4296e5701c6c99dfaa9b10b3a729b), ROM_BIOS(1) )
	ROMX_LOAD( "za3509_10042.a2", 0x3000, 0x1000, CRC(346f0cdb) SHA1(4262137cff9dfc82c5bd5727994ed5f9b7d22395), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "9913", "Checksum 9913" )
	ROMX_LOAD( "za3506_9913.a5", 0x0000, 0x1000, CRC(e2afbf48) SHA1(9883396edd334835a844dcaa792d29599a8c67b9), ROM_BIOS(2) )
	ROMX_LOAD( "za3507_9913.a3", 0x1000, 0x1000, CRC(d224412a) SHA1(30968054bba7c2aecb4d54864b75a446c1b8fdb1), ROM_BIOS(2) )
	ROMX_LOAD( "za3508_9913.a4", 0x2000, 0x1000, CRC(1502ba5b) SHA1(5df45909c2c4296e5701c6c99dfaa9b10b3a729b), ROM_BIOS(2) )
	ROMX_LOAD( "za3509_9913.a2", 0x3000, 0x1000, CRC(bc8860b7) SHA1(28b6cf7f5a4f81e017c2af091c3719657f981710), ROM_BIOS(2) )

	ROM_REGION( 0x100, "hsync", 0 )
	ROM_LOAD( "abc80_11.k5", 0x0000, 0x0100, CRC(e4f7e018) SHA1(63e718a39537f37286ea183e6469808c271dbfa5) ) // "64 40029-01" 82S129 256x4 horizontal sync

	ROM_REGION( 0x200, "vsync", 0 )
	ROM_LOAD( "abc80_21.k2", 0x0000, 0x0200, CRC(445a45b9) SHA1(bcc1c4fafe68b3500b03de785ca32abd63cea252) ) // "64 40030-01" 82S131 512x4 vertical sync

	ROM_REGION( 0x100, "attr", 0 )
	ROM_LOAD( "abc80_12.j3", 0x0000, 0x0100, CRC(6c46811c) SHA1(2d3bdf2d3a2a88ddb1c0c637967e1b2b9541a928) ) // "64 40056-01" 82S129 256x4 attribute

	ROM_REGION( 0x200, "line", 0 )
	ROM_LOAD( "abc80_22.k1", 0x0000, 0x0200, CRC(74de7a0b) SHA1(96f37b0ca65aa8af4242bad38124f410b7f657fe) ) // "64 40058-01" 82S131 512x4 chargen 74S263 row address

	ROM_REGION( 0x100, "mmu", 0 )
	ROM_LOAD( "abc80_13.e7", 0x0000, 0x0100, CRC(f7738834) SHA1(b02df3e678fb50c9cb75b4a97615222d3b4577a3) ) // "64 40057-01" 82S129 256x4 address decoder
ROM_END


//-------------------------------------------------
//  ROM( abc80l )
//-------------------------------------------------

ROM_START( abc80l )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("9913")
	ROM_SYSTEM_BIOS( 0, "11273", "Checksum 11273" )
	ROMX_LOAD( "za3506_11273.a5", 0x0000, 0x1000, CRC(7c004fb6) SHA1(9aee1d085122f4537c3e6ecdab9d799bd429ef52), ROM_BIOS(0) )
	ROMX_LOAD( "za3507_11273.a3", 0x1000, 0x1000, CRC(d1850a84) SHA1(f7719f3af9173601a2aa23ae38ae00de1a387ad8), ROM_BIOS(0) )
	ROMX_LOAD( "za3508_11273.a4", 0x2000, 0x1000, CRC(b55528e9) SHA1(3e5017e8cacad1f13215242f1bbd89d1d3eee131), ROM_BIOS(0) )
	ROMX_LOAD( "za3509_11273.a2", 0x3000, 0x1000, CRC(659cab1e) SHA1(181db748cef22cdcccd311a60aa6189c85343db7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "10042", "Checksum 10042" )
	ROMX_LOAD( "za3506_9913.a5", 0x0000, 0x1000, CRC(e2afbf48) SHA1(9883396edd334835a844dcaa792d29599a8c67b9), ROM_BIOS(1) )
	ROMX_LOAD( "za3507_9913.a3", 0x1000, 0x1000, CRC(d224412a) SHA1(30968054bba7c2aecb4d54864b75a446c1b8fdb1), ROM_BIOS(1) )
	ROMX_LOAD( "za3508_9913.a4", 0x2000, 0x1000, CRC(1502ba5b) SHA1(5df45909c2c4296e5701c6c99dfaa9b10b3a729b), ROM_BIOS(1) )
	ROMX_LOAD( "za3509_10042.a2", 0x3000, 0x1000, CRC(346f0cdb) SHA1(4262137cff9dfc82c5bd5727994ed5f9b7d22395), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "9913", "Checksum 9913" )
	ROMX_LOAD( "za3506_9913.a5", 0x0000, 0x1000, CRC(e2afbf48) SHA1(9883396edd334835a844dcaa792d29599a8c67b9), ROM_BIOS(2) )
	ROMX_LOAD( "za3507_9913.a3", 0x1000, 0x1000, CRC(d224412a) SHA1(30968054bba7c2aecb4d54864b75a446c1b8fdb1), ROM_BIOS(2) )
	ROMX_LOAD( "za3508_9913.a4", 0x2000, 0x1000, CRC(1502ba5b) SHA1(5df45909c2c4296e5701c6c99dfaa9b10b3a729b), ROM_BIOS(2) )
	ROMX_LOAD( "za3509_9913.a2", 0x3000, 0x1000, CRC(bc8860b7) SHA1(28b6cf7f5a4f81e017c2af091c3719657f981710), ROM_BIOS(2) )

	ROM_REGION( 0x100, "hsync", 0 )
	ROM_LOAD( "abc80_11.k5", 0x0000, 0x0100, CRC(e4f7e018) SHA1(63e718a39537f37286ea183e6469808c271dbfa5) ) // "64 40029-01" 82S129 256x4 horizontal sync

	ROM_REGION( 0x200, "vsync", 0 )
	ROM_LOAD( "abc80_21.k2", 0x0000, 0x0200, CRC(445a45b9) SHA1(bcc1c4fafe68b3500b03de785ca32abd63cea252) ) // "64 40030-01" 82S131 512x4 vertical sync

	ROM_REGION( 0x100, "attr", 0 )
	ROM_LOAD( "abc80_12.j3", 0x0000, 0x0100, CRC(6c46811c) SHA1(2d3bdf2d3a2a88ddb1c0c637967e1b2b9541a928) ) // "64 40056-01" 82S129 256x4 attribute

	ROM_REGION( 0x200, "line", 0 )
	ROM_LOAD( "abc80_22.k1", 0x0000, 0x0200, CRC(74de7a0b) SHA1(96f37b0ca65aa8af4242bad38124f410b7f657fe) ) // "64 40058-01" 82S131 512x4 chargen 74S263 row address

	ROM_REGION( 0x100, "mmu", 0 )
	ROM_LOAD( "abc80_13.e7", 0x0000, 0x0100, CRC(f7738834) SHA1(b02df3e678fb50c9cb75b4a97615222d3b4577a3) ) // "64 40057-01" 82S129 256x4 address decoder

	ROM_REGION( 0x800, "abc80l", 0 )
	ROM_LOAD( "l",    0x000, 0x800, CRC(3389c97c) SHA1(8fc0119286755f250d8f689dc330379f1325e6e1) )
	ROM_LOAD( "l2.0", 0x000, 0x800, CRC(4b943785) SHA1(a10812294e6aa6051d40c87053362d3bbebed139) )
	ROM_LOAD( "l3.0", 0x000, 0x800, CRC(e25f14e4) SHA1(07ffbc1e9374e232f0dd9ff7ca49dd87aef6e915) ) // v2.1
ROM_END


//-------------------------------------------------
//  ROM( tkn80 )
//-------------------------------------------------

ROM_START( tkn80 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("9913")
	ROM_SYSTEM_BIOS( 0, "11273", "Checksum 11273" )
	ROMX_LOAD( "za3506_11273.a5", 0x0000, 0x1000, CRC(7c004fb6) SHA1(9aee1d085122f4537c3e6ecdab9d799bd429ef52), ROM_BIOS(0) )
	ROMX_LOAD( "za3507_11273.a3", 0x1000, 0x1000, CRC(d1850a84) SHA1(f7719f3af9173601a2aa23ae38ae00de1a387ad8), ROM_BIOS(0) )
	ROMX_LOAD( "za3508_11273.a4", 0x2000, 0x1000, CRC(b55528e9) SHA1(3e5017e8cacad1f13215242f1bbd89d1d3eee131), ROM_BIOS(0) )
	ROMX_LOAD( "za3509_11273.a2", 0x3000, 0x1000, CRC(659cab1e) SHA1(181db748cef22cdcccd311a60aa6189c85343db7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "10042", "Checksum 10042" )
	ROMX_LOAD( "za3506_9913.a5", 0x0000, 0x1000, CRC(e2afbf48) SHA1(9883396edd334835a844dcaa792d29599a8c67b9), ROM_BIOS(1) )
	ROMX_LOAD( "za3507_9913.a3", 0x1000, 0x1000, CRC(d224412a) SHA1(30968054bba7c2aecb4d54864b75a446c1b8fdb1), ROM_BIOS(1) )
	ROMX_LOAD( "za3508_9913.a4", 0x2000, 0x1000, CRC(1502ba5b) SHA1(5df45909c2c4296e5701c6c99dfaa9b10b3a729b), ROM_BIOS(1) )
	ROMX_LOAD( "za3509_10042.a2", 0x3000, 0x1000, CRC(346f0cdb) SHA1(4262137cff9dfc82c5bd5727994ed5f9b7d22395), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "9913", "Checksum 9913" )
	ROMX_LOAD( "za3506_9913.a5", 0x0000, 0x1000, CRC(e2afbf48) SHA1(9883396edd334835a844dcaa792d29599a8c67b9), ROM_BIOS(2) )
	ROMX_LOAD( "za3507_9913.a3", 0x1000, 0x1000, CRC(d224412a) SHA1(30968054bba7c2aecb4d54864b75a446c1b8fdb1), ROM_BIOS(2) )
	ROMX_LOAD( "za3508_9913.a4", 0x2000, 0x1000, CRC(1502ba5b) SHA1(5df45909c2c4296e5701c6c99dfaa9b10b3a729b), ROM_BIOS(2) )
	ROMX_LOAD( "za3509_9913.a2", 0x3000, 0x1000, CRC(bc8860b7) SHA1(28b6cf7f5a4f81e017c2af091c3719657f981710), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "tkn80", 0 )
	ROM_LOAD( "tkn80-iii.e", 0x0000, 0x1000, CRC(f0d2e4fa) SHA1(b0263c65db39667a6fe62e61f73fe591ea10f14b) )

	ROM_REGION( 0x100, "hsync", 0 )
	ROM_LOAD( "abc80_11.k5", 0x0000, 0x0100, CRC(e4f7e018) SHA1(63e718a39537f37286ea183e6469808c271dbfa5) ) // "64 40029-01" 82S129 256x4 horizontal sync

	ROM_REGION( 0x200, "vsync", 0 )
	ROM_LOAD( "abc80_21.k2", 0x0000, 0x0200, CRC(445a45b9) SHA1(bcc1c4fafe68b3500b03de785ca32abd63cea252) ) // "64 40030-01" 82S131 512x4 vertical sync

	ROM_REGION( 0x100, "attr", 0 )
	ROM_LOAD( "abc80_12.j3", 0x0000, 0x0100, CRC(6c46811c) SHA1(2d3bdf2d3a2a88ddb1c0c637967e1b2b9541a928) ) // "64 40056-01" 82S129 256x4 attribute

	ROM_REGION( 0x200, "line", 0 )
	ROM_LOAD( "abc80_22.k1", 0x0000, 0x0200, CRC(74de7a0b) SHA1(96f37b0ca65aa8af4242bad38124f410b7f657fe) ) // "64 40058-01" 82S131 512x4 chargen 74S263 row address

	ROM_REGION( 0x100, "mmu", 0 )
	ROM_LOAD( "tkn80-13.e7", 0x0000, 0x0100, CRC(28bfdf62) SHA1(209c4a29628168a27445b75f4c44aba2c7c49dbb) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY             FULLNAME             FLAGS
COMP( 1978, abc80,  0,      0,      abc80,   0,     abc80_state,  empty_init, "Luxor Datorer AB", "ABC 80",             MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1981, abc80l, abc80,  0,      abc80l,  0,     abc80l_state, empty_init, "Cat AB",           "ABC 80 with ABC80L", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 198?, tkn80,  abc80,  0,      tkn80,   tkn80, tkn80_state,  empty_init, "MYAB",             "ABC 80 with TKN80",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
