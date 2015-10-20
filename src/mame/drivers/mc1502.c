// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    drivers/mc1502.c

    Driver file for Electronika MC 1502

***************************************************************************/

#include "emu.h"
#include "includes/mc1502.h"
#include "bus/rs232/rs232.h"

#include "cpu/i86/i86.h"
#include "machine/kb_7007_3.h"
#include "sound/speaker.h"
#include "sound/wave.h"

#define VERBOSE_DBG 0

#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s",machine().time().as_double(),(char*)M ); \
			logerror A; \
		} \
	} while (0)

/*
 * onboard devices:
 */

// Timer

/* check if any keys are pressed, raise IRQ1 if so */

TIMER_CALLBACK_MEMBER(mc1502_state::keyb_signal_callback)
{
	UINT8 key = 0;

	key |= ioport("Y1")->read();
	key |= ioport("Y2")->read();
	key |= ioport("Y3")->read();
	key |= ioport("Y4")->read();
	key |= ioport("Y5")->read();
	key |= ioport("Y6")->read();
	key |= ioport("Y7")->read();
	key |= ioport("Y8")->read();
	key |= ioport("Y9")->read();
	key |= ioport("Y10")->read();
	key |= ioport("Y11")->read();
	key |= ioport("Y12")->read();
//  DBG_LOG(1,"mc1502_k_s_c",("= %02X (%d) %s\n", key, m_kbd.pulsing,
//      (key || m_kbd.pulsing) ? " will IRQ" : ""));

	/*
	   If a key is pressed and we're not pulsing yet, start pulsing the IRQ1;
	   keep pulsing while any key is pressed, and pulse one time after all keys
	   are released.
	 */
	if (key) {
		if (m_kbd.pulsing < 2) {
			m_kbd.pulsing += 2;
		}
	}

	if (m_kbd.pulsing) {
		m_pic8259->ir1_w(m_kbd.pulsing & 1);
		m_kbd.pulsing--;
	}
}

WRITE8_MEMBER(mc1502_state::mc1502_ppi_portb_w)
{
//  DBG_LOG(2,"mc1502_ppi_portb_w",("( %02X )\n", data));
	m_ppi_portb = data;
	m_pit8253->write_gate2(BIT(data, 0));
	mc1502_speaker_set_spkrdata(BIT(data, 1));
	m_centronics->write_strobe(BIT(data, 2));
	m_centronics->write_autofd(BIT(data, 3));
	m_centronics->write_init(BIT(data, 4));
}

// bit 0: parallel port data transfer direction (default = 0 = out)
// bits 1-2: CGA_FONT (default = 01)
// bit 3: i8251 SYNDET pin triggers NMI (default = 1 = no)
WRITE8_MEMBER(mc1502_state::mc1502_ppi_portc_w)
{
//  DBG_LOG(2,"mc1502_ppi_portc_w",("( %02X )\n", data));
	m_ppi_portc = data & 15;
}

// 0x80 -- serial RxD
// 0x40 -- CASS IN, also loops back T2OUT (gated by CASWR)
// 0x20 -- T2OUT
// 0x10 -- SNDOUT
READ8_MEMBER(mc1502_state::mc1502_ppi_portc_r)
{
	int data = 0xff;
	double tap_val = m_cassette->input();

	data = ( data & ~0x40 ) | ( tap_val < 0 ? 0x40 : 0x00 ) | ( (BIT(m_ppi_portb, 7) && m_pit_out2) ? 0x40 : 0x00 );
	data = ( data & ~0x20 ) | ( m_pit_out2 ? 0x20 : 0x00 );
	data = ( data & ~0x10 ) | ( (BIT(m_ppi_portb, 1) && m_pit_out2) ? 0x10 : 0x00 );

//  DBG_LOG(2,"mc1502_ppi_portc_r",("= %02X (tap_val %f t2out %d) at %s\n",
//      data, tap_val, m_pit_out2, machine().describe_context()));
	return data;
}

READ8_MEMBER(mc1502_state::mc1502_kppi_porta_r)
{
	UINT8 key = 0;

	if (m_kbd.mask & 0x0001) { key |= ioport("Y1")->read(); }
	if (m_kbd.mask & 0x0002) { key |= ioport("Y2")->read(); }
	if (m_kbd.mask & 0x0004) { key |= ioport("Y3")->read(); }
	if (m_kbd.mask & 0x0008) { key |= ioport("Y4")->read(); }
	if (m_kbd.mask & 0x0010) { key |= ioport("Y5")->read(); }
	if (m_kbd.mask & 0x0020) { key |= ioport("Y6")->read(); }
	if (m_kbd.mask & 0x0040) { key |= ioport("Y7")->read(); }
	if (m_kbd.mask & 0x0080) { key |= ioport("Y8")->read(); }
	if (m_kbd.mask & 0x0100) { key |= ioport("Y9")->read(); }
	if (m_kbd.mask & 0x0200) { key |= ioport("Y10")->read(); }
	if (m_kbd.mask & 0x0400) { key |= ioport("Y11")->read(); }
	if (m_kbd.mask & 0x0800) { key |= ioport("Y12")->read(); }
	key ^= 0xff;
//  DBG_LOG(2,"mc1502_kppi_porta_r",("= %02X\n", key));
	return key;
}

WRITE8_MEMBER(mc1502_state::mc1502_kppi_portb_w)
{
	m_kbd.mask &= ~255;
	m_kbd.mask |= data ^ 255;
	if (!BIT(data, 0))
		m_kbd.mask |= 1 << 11;
	else
		m_kbd.mask &= ~(1 << 11);
//  DBG_LOG(2,"mc1502_kppi_portb_w",("( %02X -> %04X )\n", data, m_kbd.mask));
}

WRITE8_MEMBER(mc1502_state::mc1502_kppi_portc_w)
{
	m_kbd.mask &= ~(7 << 8);
	m_kbd.mask |= ((data ^ 7) & 7) << 8;
//  DBG_LOG(2,"mc1502_kppi_portc_w",("( %02X -> %04X )\n", data, m_kbd.mask));
}

WRITE_LINE_MEMBER(mc1502_state::mc1502_i8251_syndet)
{
	if (!BIT(m_ppi_portc,3))
		m_maincpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(mc1502_state::mc1502_pit8253_out1_changed)
{
	m_upd8251->write_txc(state);
	m_upd8251->write_rxc(state);
}

WRITE_LINE_MEMBER(mc1502_state::mc1502_pit8253_out2_changed)
{
	m_pit_out2 = state;
	m_speaker->level_w(m_spkrdata & m_pit_out2);
	m_cassette->output(state ? 1 : -1);
}

WRITE_LINE_MEMBER(mc1502_state::mc1502_speaker_set_spkrdata)
{
	m_spkrdata = state ? 1 : 0;
	m_speaker->level_w(m_spkrdata & m_pit_out2);
}

DRIVER_INIT_MEMBER( mc1502_state, mc1502 )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	DBG_LOG(0,"init",("driver_init()\n"));

	program.unmap_readwrite(0, 0x7ffff);
	program.install_readwrite_bank(0, m_ram->size()-1, "bank10");
	membank( "bank10" )->set_base( m_ram->pointer() );
}

MACHINE_START_MEMBER( mc1502_state, mc1502 )
{
	DBG_LOG(0,"init",("machine_start()\n"));

	/*
	       Keyboard polling circuit holds IRQ1 high until a key is
	       pressed, then it starts a timer that pulses IRQ1 low each
	       40ms (check) for 20ms (check) until all keys are released.
	       Last pulse causes BIOS to write a 'break' scancode into port 60h.
	 */
	m_pic8259->ir1_w(1);
	memset(&m_kbd, 0, sizeof(m_kbd));
	m_kbd.keyb_signal_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc1502_state::keyb_signal_callback),this));
	m_kbd.keyb_signal_timer->adjust( attotime::from_msec(20), 0, attotime::from_msec(20) );
}

MACHINE_RESET_MEMBER( mc1502_state, mc1502 )
{
	DBG_LOG(0,"init",("machine_reset()\n"));

	m_spkrdata = 0;
	m_pit_out2 = 1;
	m_ppi_portb = 0;
	m_ppi_portc = 0;
	m_speaker->level_w(0);
}

/*
 * macros
 */

static ADDRESS_MAP_START( mc1502_map, AS_PROGRAM, 8, mc1502_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x97fff) AM_RAM   /* 96K on mainboard + 512K on extension card */
	AM_RANGE(0xc0000, 0xfbfff) AM_NOP
	AM_RANGE(0xfc000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pk88_map, AS_PROGRAM, 8, mc1502_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x1ffff) AM_RAMBANK("bank10") /* 96K on mainboard */
	AM_RANGE(0xf0000, 0xf7fff) AM_ROM       /* BASIC */
	AM_RANGE(0xfc000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(mc1502_io, AS_IO, 8, mc1502_state )
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE(0x0028, 0x0028) AM_DEVREADWRITE("upd8251", i8251_device, data_r, data_w)
	AM_RANGE(0x0029, 0x0029) AM_DEVREADWRITE("upd8251", i8251_device, status_r, control_w)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255n1", i8255_device, read, write)
	AM_RANGE(0x0068, 0x006B) AM_DEVREADWRITE("ppi8255n2", i8255_device, read, write)    // keyboard poll
ADDRESS_MAP_END

static INPUT_PORTS_START( mc1502 )
	PORT_INCLUDE( mc7007_3_keyboard )
INPUT_PORTS_END

static MACHINE_CONFIG_START( mc1502, mc1502_state )
	MCFG_CPU_ADD("maincpu", I8088, XTAL_16MHz/3)
	MCFG_CPU_PROGRAM_MAP(mc1502_map)
	MCFG_CPU_IO_MAP(mc1502_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	MCFG_MACHINE_START_OVERRIDE( mc1502_state, mc1502 )
	MCFG_MACHINE_RESET_OVERRIDE( mc1502_state, mc1502 )

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_16MHz/12) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_16MHz/12) /* serial port */
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(mc1502_state, mc1502_pit8253_out1_changed))
	MCFG_PIT8253_CLK2(XTAL_16MHz/12) /* pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(mc1502_state, mc1502_pit8253_out2_changed))

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_DEVICE_ADD("ppi8255n1", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(mc1502_state, mc1502_ppi_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(mc1502_state, mc1502_ppi_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mc1502_state, mc1502_ppi_portc_w))

	MCFG_DEVICE_ADD("ppi8255n2", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(mc1502_state, mc1502_kppi_porta_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(mc1502_state, mc1502_kppi_portb_w))
	MCFG_I8255_IN_PORTC_CB(DEVREAD8("cent_status_in", input_buffer_device, read))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mc1502_state, mc1502_kppi_portc_w))

	MCFG_DEVICE_ADD( "upd8251", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("irps", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("irps", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("irps", rs232_port_device, write_rts))
	/* XXX RxD data are accessible via PPI port C, bit 7 */
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w)) /* default handler does nothing */
	MCFG_I8251_TXRDY_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w))
	MCFG_I8251_SYNDET_HANDLER(WRITELINE(mc1502_state, mc1502_i8251_syndet))

	MCFG_RS232_PORT_ADD("irps", default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("upd8251", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("upd8251", i8251_device, write_dsr))

	MCFG_DEVICE_ADD("isa", ISA8, 0)
	MCFG_ISA8_CPU(":maincpu")
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE("pic8259", pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE("pic8259", pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE("pic8259", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE("pic8259", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE("pic8259", pic8259_device, ir7_w))

	MCFG_ISA8_SLOT_ADD("isa", "board0", mc1502_isa8_cards, "cga_mc1502", true)
	MCFG_ISA8_SLOT_ADD("isa", "isa1", mc1502_isa8_cards, "fdc", false)
	MCFG_ISA8_SLOT_ADD("isa", "isa2", mc1502_isa8_cards, "rom", false)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit6))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit7))
	MCFG_CENTRONICS_FAULT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit4))
	MCFG_CENTRONICS_PERROR_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit5))

	MCFG_DEVICE_ADD("cent_status_in", INPUT_BUFFER, 0)

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_SOFTWARE_LIST_ADD("flop_list","mc1502_flop")
//  MCFG_SOFTWARE_LIST_ADD("cass_list","mc1502_cass")

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("608K")                   /* 96 base + 512 on expansion card */
	MCFG_RAM_EXTRA_OPTIONS("96K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pk88, mc1502 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pk88_map)
MACHINE_CONFIG_END


/*
        Apparently there was a hardware revision with built-in floppy
        controller mapped to alternate set of ports; v531 and v533
        support this revision. v533 is possibly not an original BIOS, it
        supports autoboot which none of others do. v521h is a version
        with support for 3rd party hard disk controller (not emulated).
        v51 is designed for a different keyboard layout (JCUKEN, not
        QWERTY).
*/
ROM_START( mc1502 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)

	ROM_DEFAULT_BIOS("v52")
	ROM_SYSTEM_BIOS(0, "v50", "v5.0 10/05/89")
	ROMX_LOAD( "monitor_5_0.rom",  0xfc000, 0x4000, CRC(9e97c6a0) SHA1(16a304e8de69ec4d8b92acda6bf28454c361a24f),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v52", "v5.2 22/03/91")
	ROMX_LOAD( "monitor_5_2.rom",  0xfc000, 0x4000, CRC(0e65491e) SHA1(8a4d556473b5e0e59b05fab77c79c29f4d562412),ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v521", "v5.21 12/10/92")
	ROMX_LOAD( "monitor_5_21.rom", 0xfc000, 0x4000, CRC(28c8f653) SHA1(04b0b09e0b86d9648a83352cc1590eb8963833e0),ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "v531", "v5.31 12/10/92")
	ROMX_LOAD( "monitor_5_31.rom", 0xfc000, 0x4000, CRC(a48295d5) SHA1(6f38977c22f9cc6c2bc6f6e53edc4048ca6b6721),ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "v533", "v5.33 01/08/93")
	ROMX_LOAD( "0_(cbc0).bin", 0xfc000, 0x2000, CRC(9a55bc4f) SHA1(81da44eec2e52cf04b1fc7053502270f51270590),ROM_BIOS(5))
	ROMX_LOAD( "1_(dfe2).bin", 0xfe000, 0x2000, CRC(8dec077a) SHA1(d6f6d7cc2183abc77fbd9cd59132de5766f7c458),ROM_BIOS(5))

	// 5.21 + 3rd party HDC support. fails checksum test so marked BAD_DUMP.
	ROM_SYSTEM_BIOS(5, "v521h", "v5.21h 22/09/93")
	ROMX_LOAD( "mshbios0.pgm", 0xfc000, 0x2000, BAD_DUMP CRC(be447261) SHA1(b93c597c17dfa4b678f72c20a3f7119b73e6ba1c),ROM_BIOS(6))
	ROMX_LOAD( "mshbios1.pgm", 0xfe000, 0x2000, BAD_DUMP CRC(89e2eaf2) SHA1(37d6b225b5e35574fdac81219589407d925225be),ROM_BIOS(6))

	// 5.3
	ROM_SYSTEM_BIOS(6, "v53", "v5.3 10/11/91")
	ROMX_LOAD( "1502~3b0.pgm", 0xfc000, 0x2000, CRC(dc148763) SHA1(7a5e66438007b2de328ac680614f9c4ff60f6a75),ROM_BIOS(7))
	ROMX_LOAD( "1502~3b1.pgm", 0xfe000, 0x2000, CRC(17fc2af2) SHA1(a060d7b7302dfa639025f025106b50412cf26953),ROM_BIOS(7))
	// 5.1 -- JCUKEN keyboard
	ROM_SYSTEM_BIOS(7, "v51", "v5.1 10/12/90")
	ROMX_LOAD( "ms1502b0.pgm", 0xfc000, 0x2000, CRC(92fcc29a) SHA1(930a4cffcd6ec6110dd9a18bd389b78f0ccb110a),ROM_BIOS(8))
	ROMX_LOAD( "ms1502b1.pgm", 0xfe000, 0x2000, CRC(fe355a58) SHA1(b4ef7775045c6f2095e2b487fe19824986a4892c),ROM_BIOS(8))
	// 5.31
	ROM_SYSTEM_BIOS(8, "v531_92", "v5.31 12/10/92")
	ROMX_LOAD( "ms531b0.pgm", 0xfc000, 0x2000, CRC(d97157d1) SHA1(cb1a1e0e2d9a0fcc78f9b09bfb4814d408ee4fae),ROM_BIOS(9))
	ROMX_LOAD( "ms531b1.pgm", 0xfe000, 0x2000, CRC(b1368e1a) SHA1(286496d25dc0ac2d8fe1802caffc6c37b236d105),ROM_BIOS(9))
	// 5.2
	ROM_SYSTEM_BIOS(9, "v52_91", "v5.2 10/11/91")
	ROMX_LOAD( "msv5~2b0.pgm", 0xfc000, 0x2000, CRC(f7f370e9) SHA1(e069a35005581a02856853b57dd511ab8e10054b),ROM_BIOS(10))
	ROMX_LOAD( "msv5~2b1.pgm", 0xfe000, 0x2000, CRC(d50e1c43) SHA1(22724dec0052ee9e52f44f5914f2f5f3fae14612),ROM_BIOS(10))

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_LOAD( "symgen.rom", 0x0000, 0x2000, CRC(b2747a52) SHA1(6766d275467672436e91ac2997ac6b77700eba1e))
ROM_END

/*
        Predecessor of MC1502, same keyboard attachment but
        different video subsystem (not emulated).
*/
ROM_START( pk88 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)

	ROM_LOAD( "b0.064", 0xf0000, 0x2000, CRC(80d3cf5d) SHA1(64769b7a8b60ffeefa04e4afbec778069a2840c9))
	ROM_LOAD( "b1.064", 0xf2000, 0x2000, CRC(673a4acc) SHA1(082ae803994048e225150f771794ca305f73d731))
	ROM_LOAD( "b2.064", 0xf4000, 0x2000, CRC(1ee66152) SHA1(7ed8c4c6c582487e802beabeca5b86702e5083e8))
	ROM_LOAD( "b3.064", 0xf6000, 0x2000, CRC(3062b3fc) SHA1(5134dd64721cbf093d059ee5d3fd09c7f86604c7))
	ROM_LOAD( "pk88-0.064", 0xfc000, 0x2000, CRC(1e4666cf) SHA1(6364c5241f2792909ff318194161eb2c29737546))
	ROM_LOAD( "pk88-1.064", 0xfe000, 0x2000, CRC(6fa7e7ef) SHA1(d68bc273baa46ba733ac6ad4df7569dd70cf60dd))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR     NAME        PARENT      COMPAT  MACHINE     INPUT       INIT                COMPANY       FULLNAME */
COMP ( 1989,    mc1502,     ibm5150,    0,      mc1502,     mc1502,     mc1502_state, mc1502,   "NPO Microprocessor", "Elektronika MC-1502", 0)
COMP ( 1988,    pk88,       ibm5150,    0,      pk88,       mc1502,     mc1502_state, mc1502,   "NPO Microprocessor", "Elektronika PK-88", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
