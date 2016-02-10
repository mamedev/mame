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
|   |--------------|  |------------|   PROM0  LS132   LS273   7406             LS08    |
|   |   Z80A PIO   |  |    Z80A    |                                                   |
|   |--------------|  |------------|   LS04   LS74A   LS86    LS161   LS166    74393   |
|                                                                                      |
|     ROM3   LS107    4116    4116     LS10   LS257   LS74A   LS08    LS107    PROM2   |
|                                                                                      |
|     ROM2   LS257    4116    4116     LS139  74393   LS107   LS32    LS175    74393   |
|                                                                                      |
|     ROM1   LS257    4116    4116     LS08   LS283   LS10    LS32    PROM1    74393   |
|                                                                                      |
|     ROM0   LS257    4116    4116     LS257  74393   LS375   74S263  LS145    PROM4   |
|                                                                                      |
|     DIPSW1 DIPSW2   4045    4045     LS257  LS245   LS375   LS273   LS166    PROM3   |
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
    CN1     - RS-232 connector
    CN2     - ABC bus connector (DIN 41612)
    CN3     - video connector
    CN4     - cassette motor connector
    CN5     - cassette connector
    CN6     - keyboard connector
    SW1     - reset switch
    DIPSW1  -
    DIPSW2  -

*/

/*

    TODO:

    - proper keyboard controller emulation
    - MyAB TKN80 80-column card
    - GeJo 80-column card
    - Mikrodatorn 64K expansion
    - floppy
    - Metric ABC CAD 1000

*/

#include "includes/abc80.h"
#include "softlist.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  MEMORY MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( abc80_state::read )
{
	UINT8 data = 0xff;
	UINT8 mmu = m_mmu_rom->base()[0x40 | (offset >> 10)];

	if (!(mmu & MMU_XM))
	{
		data = m_bus->xmemfl_r(space, offset);
	}
	else if (!(mmu & MMU_ROM))
	{
		data = m_rom->base()[offset & 0x3fff];
	}
	else if (mmu & MMU_VRAMS)
	{
		data = m_video_ram[offset & 0x3ff];
	}
	else if (!(mmu & MMU_RAM))
	{
		data = m_ram->pointer()[offset & 0x3fff];
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( abc80_state::write )
{
	UINT8 mmu = m_mmu_rom->base()[0x40 | (offset >> 10)];

	if (!(mmu & MMU_XM))
	{
		m_bus->xmemw_w(space, offset, data);
	}
	else if (mmu & MMU_VRAMS)
	{
		m_video_ram[offset & 0x3ff] = data;
	}
	else if (!(mmu & MMU_RAM))
	{
		m_ram->pointer()[offset & 0x3fff] = data;
	}
}



//**************************************************************************
//  SOUND
//**************************************************************************

//-------------------------------------------------
//  vco_voltage_w - CSG VCO voltage select
//-------------------------------------------------

WRITE_LINE_MEMBER( abc80_state::vco_voltage_w )
{
	m_psg->vco_voltage_w(state ? 2.5 : 0);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( abc80_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc80_mem, AS_PROGRAM, 8, abc80_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc80_io, AS_IO, 8, abc80_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x17)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE(ABCBUS_TAG, abcbus_slot_t, inp_r, out_w)
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE(ABCBUS_TAG, abcbus_slot_t, stat_r, cs_w)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c1_w)
	AM_RANGE(0x03, 0x03) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c2_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c3_w)
	AM_RANGE(0x05, 0x05) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c4_w)
	AM_RANGE(0x06, 0x06) AM_WRITE_PORT("SN76477")
	AM_RANGE(0x07, 0x07) AM_DEVREAD(ABCBUS_TAG, abcbus_slot_t, rst_r)
	AM_RANGE(0x10, 0x13) AM_MIRROR(0x04) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read_alt, write_alt)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( abc80 )
//-------------------------------------------------

static INPUT_PORTS_START( abc80 )
	PORT_START("SW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("SW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("SN76477")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(SN76477_TAG, sn76477_device, enable_w)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, abc80_state, vco_voltage_w)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(SN76477_TAG, sn76477_device, vco_w)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(SN76477_TAG, sn76477_device, mixer_b_w)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(SN76477_TAG, sn76477_device, mixer_a_w)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(SN76477_TAG, sn76477_device, mixer_c_w)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(SN76477_TAG, sn76477_device, envelope_2_w)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(SN76477_TAG, sn76477_device, envelope_1_w)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  Z80PIO
//-------------------------------------------------

READ8_MEMBER( abc80_state::pio_pa_r )
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

	UINT8 data = 0;

	//data |= m_kb->data_r();
	data |= m_key_data;
	data |= (m_key_strobe << 7);

	return data;
}

READ8_MEMBER( abc80_state::pio_pb_r )
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

	UINT8 data = 0;

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

WRITE8_MEMBER( abc80_state::pio_pb_w )
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
//  ABC80_KEYBOARD_INTERFACE( kb_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( abc80_state::keydown_w )
{
	m_key_strobe = state;

	m_pio->port_a_write(m_key_strobe << 7);
}

WRITE8_MEMBER( abc80_state::kbd_w )
{
	m_key_data = data;
	m_key_strobe = 1;

	UINT8 pio_data = 0x80 | data;
	m_pio->port_a_write(pio_data);

	timer_set(attotime::from_msec(50), TIMER_ID_FAKE_KEYBOARD_CLEAR);
}

/*
DEVICE_INPUT_DEFAULTS_START( abc830_slow )
    DEVICE_INPUT_DEFAULTS("SW1", 0x0f, 0x03)
    DEVICE_INPUT_DEFAULTS("S1", 0x01, 0x01)
DEVICE_INPUT_DEFAULTS_END
*/


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void abc80_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_PIO:
		m_pio_astb = !m_pio_astb;

		m_pio->strobe_a(m_pio_astb);
		break;

	case TIMER_ID_CASSETTE:
		{
			if (!m_motor) return;

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
		break;

	case TIMER_ID_BLINK:
		m_blink = !m_blink;
		break;

	case TIMER_ID_VSYNC_ON:
		if (LOG) logerror("%s vsync 1\n", machine().time().as_string());
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		break;

	case TIMER_ID_VSYNC_OFF:
		if (LOG) logerror("%s vsync 0\n", machine().time().as_string());
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		break;

	case TIMER_ID_FAKE_KEYBOARD_CLEAR:
		m_pio->port_a_write(m_key_data);
		m_key_strobe = 0;
		m_key_data = 0;
		break;
	}
}


//-------------------------------------------------
//  MACHINE_START( abc80 )
//-------------------------------------------------

void abc80_state::machine_start()
{
	// start timers
	m_cassette_timer = timer_alloc(TIMER_ID_CASSETTE);
	m_cassette_timer->adjust(attotime::from_hz(44100), 0, attotime::from_hz(44100));

	// register for state saving
	save_item(NAME(m_key_data));
	save_item(NAME(m_key_strobe));
	save_item(NAME(m_pio_astb));
	save_item(NAME(m_latch));
	save_item(NAME(m_blink));
	save_item(NAME(m_motor));
	save_item(NAME(m_tape_in));
	save_item(NAME(m_tape_in_latch));
}

QUICKLOAD_LOAD_MEMBER( abc80_state, bac )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	offs_t address = space.read_byte(BOFA + 1) << 8 | space.read_byte(BOFA);
	if (LOG) logerror("BOFA %04x\n",address);

	dynamic_buffer data;
	data.resize(quickload_size);
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

	return IMAGE_INIT_PASS;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( abc80 )
//-------------------------------------------------

static MACHINE_CONFIG_START( abc80, abc80_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_11_9808MHz/2/2) // 2.9952 MHz
	MCFG_CPU_PROGRAM_MAP(abc80_mem)
	MCFG_CPU_IO_MAP(abc80_io)
	MCFG_CPU_CONFIG(abc80_daisy_chain)

	// video hardware
	MCFG_FRAGMENT_ADD(abc80_video)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76477_TAG, SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(47), RES_K(330), CAP_P(390)) // noise + filter: R26 47k - R24 330k - C52 390p
	MCFG_SN76477_DECAY_RES(RES_K(47))                   //  decay_res: R23 47k
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(10), RES_K(2.2))   // attack_decay_cap + attack_res: C50 10u/35V - R21 2.2k
	MCFG_SN76477_AMP_RES(RES_K(33))                     // amplitude_res: R19 33k
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))                // feedback_res: R18 10k
	MCFG_SN76477_VCO_PARAMS(0, CAP_N(10), RES_K(100))   // VCO volt + cap + res: 0V or 2.5V - C48 10n - R20 100k
	MCFG_SN76477_PITCH_VOLTAGE(0)                       // pitch_voltage: N/C
	MCFG_SN76477_SLF_PARAMS(CAP_U(1), RES_K(220))       // slf caps + res: C51 1u/35V - R22 220k
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(0.1), RES_K(330)) // oneshot caps + res: C53 0.1u - R25 330k
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, XTAL_11_9808MHz/2/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(abc80_state, pio_pa_r))
	MCFG_Z80PIO_IN_PB_CB(READ8(abc80_state, pio_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(abc80_state, pio_pb_w))

	MCFG_CASSETTE_ADD(CASSETTE_TAG)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("abc80_cass")

	MCFG_DEVICE_ADD(ABC80_KEYBOARD_TAG, ABC80_KEYBOARD, 0)
	MCFG_ABC80_KEYBOARD_KEYDOWN_CALLBACK(WRITELINE(abc80_state, keydown_w))

	MCFG_ABCBUS_SLOT_ADD(ABCBUS_TAG, abc80_cards, "abcexp")

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(abc80_state, kbd_w))

	MCFG_QUICKLOAD_ADD("quickload", abc80_state, bac, "bac", 2)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("cass_list", "abc80_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "abc80_flop")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( abc80 )
//-------------------------------------------------

ROM_START( abc80 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("v2")
	ROM_SYSTEM_BIOS( 0, "v1", "V1" )
	ROMX_LOAD( "3506_3.a5", 0x0000, 0x1000, CRC(7c004fb6) SHA1(9aee1d085122f4537c3e6ecdab9d799bd429ef52), ROM_BIOS(1) )
	ROMX_LOAD( "3507_3.a3", 0x1000, 0x1000, CRC(d1850a84) SHA1(f7719f3af9173601a2aa23ae38ae00de1a387ad8), ROM_BIOS(1) )
	ROMX_LOAD( "3508_3.a4", 0x2000, 0x1000, CRC(b55528e9) SHA1(3e5017e8cacad1f13215242f1bbd89d1d3eee131), ROM_BIOS(1) )
	ROMX_LOAD( "3509_3.a2", 0x3000, 0x1000, CRC(659cab1e) SHA1(181db748cef22cdcccd311a60aa6189c85343db7), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v2", "V2" )
	ROMX_LOAD( "3506_3_v2.a5", 0x0000, 0x1000, CRC(e2afbf48) SHA1(9883396edd334835a844dcaa792d29599a8c67b9), ROM_BIOS(2) )
	ROMX_LOAD( "3507_3_v2.a3", 0x1000, 0x1000, CRC(d224412a) SHA1(30968054bba7c2aecb4d54864b75a446c1b8fdb1), ROM_BIOS(2) )
	ROMX_LOAD( "3508_3_v2.a4", 0x2000, 0x1000, CRC(1502ba5b) SHA1(5df45909c2c4296e5701c6c99dfaa9b10b3a729b), ROM_BIOS(2) )
	ROMX_LOAD( "3509_3_v2.a2", 0x3000, 0x1000, CRC(bc8860b7) SHA1(28b6cf7f5a4f81e017c2af091c3719657f981710), ROM_BIOS(2) )

	ROM_REGION( 0xa00, "chargen", 0 )
	ROM_LOAD( "sn74s263.h2", 0x0000, 0x0a00, BAD_DUMP CRC(9e064e91) SHA1(354783c8f2865f73dc55918c9810c66f3aca751f) ) // created by hand

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



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT  INIT                  COMPANY                FULLNAME    FLAGS
COMP( 1978, abc80,  0,      0,      abc80,  abc80, driver_device,  0,      "Luxor Datorer AB",  "ABC 80",   MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_KEYBOARD )
