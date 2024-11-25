// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    drivers/mc1502.cpp

    Driver file for Elektronika MS 1502

    To do:
    - fix video errors caused by 465caf8038a120b4c1ffad9df67a1dc7474e5bb1
      "cga: treat as fixed sync monitor (nw)"
    - debug video init in BIOS 7.2
    - pk88 (video, keyboard, etc.)

***************************************************************************/

#include "emu.h"
#include "kb_7007_3.h"

#include "bus/centronics/ctronics.h"
#include "bus/isa/isa.h"
#include "bus/isa/mc1502_fdc.h"
#include "bus/isa/xsu_cards.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "imagedev/cassette.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "softlist_dev.h"
#include "speaker.h"


#define LOG_KEYBOARD  (1U << 1)
#define LOG_PPI       (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_VRAM)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGKBD(...) LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)
#define LOGPPI(...) LOGMASKED(LOG_PPI, __VA_ARGS__)


namespace {

class mc1502_state : public driver_device
{
public:
	mc1502_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_upd8251(*this, "upd8251")
		, m_pic8259(*this, "pic8259")
		, m_pit8253(*this, "pit8253")
		, m_ppi8255n1(*this, "ppi8255n1")
		, m_ppi8255n2(*this, "ppi8255n2")
		, m_isabus(*this, "isa")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_centronics(*this, "centronics")
		, m_ram(*this, RAM_TAG)
		, m_kbdio(*this, "Y%u", 1)
	{ }

	void mc1502(machine_config &config);

	void init_mc1502();

	void fdc_config(device_t *device);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device>  m_maincpu;
	required_device<i8251_device> m_upd8251;
	required_device<pic8259_device>  m_pic8259;
	required_device<pit8253_device>  m_pit8253;
	required_device<i8255_device>  m_ppi8255n1;
	required_device<i8255_device>  m_ppi8255n2;
	required_device<isa8_device>  m_isabus;
	required_device<speaker_sound_device>  m_speaker;
	required_device<cassette_image_device>  m_cassette;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_ioport_array<12> m_kbdio;

	TIMER_CALLBACK_MEMBER(keyb_signal_callback);

	struct {
		uint8_t       pulsing = 0;
		uint16_t      mask = 0;       /* input lines */
		emu_timer   *keyb_signal_timer = nullptr;
	} m_kbd;

	uint8_t m_ppi_portb = 0;
	uint8_t m_ppi_portc = 0;
	uint8_t m_spkrdata = 0;

	void mc1502_pit8253_out1_changed(int state);
	void mc1502_pit8253_out2_changed(int state);
	void mc1502_speaker_set_spkrdata(int state);
	void mc1502_i8251_syndet(int state);

	void mc1502_ppi_portb_w(uint8_t data);
	void mc1502_ppi_portc_w(uint8_t data);
	uint8_t mc1502_ppi_portc_r();
	uint8_t mc1502_kppi_porta_r();
	void mc1502_kppi_portb_w(uint8_t data);
	void mc1502_kppi_portc_w(uint8_t data);

	void mc1502_io(address_map &map) ATTR_COLD;
	void mc1502_map(address_map &map) ATTR_COLD;

	int m_pit_out2 = 0;
};


/*
 * onboard devices:
 */

// Timer

/* check if any keys are pressed, raise IRQ1 if so */

TIMER_CALLBACK_MEMBER(mc1502_state::keyb_signal_callback)
{
	uint8_t key = 0;

	for (int i = 0; i < 12; i++)
	{
		key |= m_kbdio[i]->read();
	}

	LOGKBD("k_s_c = %02X (%d) %s\n", key, m_kbd.pulsing, (key || m_kbd.pulsing) ? " will IRQ" : "");

	/*
	   If a key is pressed and we're not pulsing yet, start pulsing the IRQ1;
	   keep pulsing while any key is pressed, and pulse one time after all keys
	   are released.
	 */
	if (key)
	{
		if (m_kbd.pulsing < 2)
		{
			m_kbd.pulsing += 2;
		}
	}

	if (m_kbd.pulsing)
	{
		m_pic8259->ir1_w(m_kbd.pulsing & 1);
		m_kbd.pulsing--;
	}
}

void mc1502_state::mc1502_ppi_portb_w(uint8_t data)
{
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
void mc1502_state::mc1502_ppi_portc_w(uint8_t data)
{
	m_ppi_portc = data & 15;
}

// 0x80 -- serial RxD (not emulated)
// 0x40 -- CASS IN, also loops back T2OUT (gated by CASWR)
// 0x20 -- T2OUT
// 0x10 -- SNDOUT
uint8_t mc1502_state::mc1502_ppi_portc_r()
{
	int data = 0xff;
	double tap_val = m_cassette->input();

	data = (data & ~0x40) | (tap_val < 0 ? 0x40 : 0x00) | ((BIT(m_ppi_portb, 7) && m_pit_out2) ? 0x40 : 0x00);
	data = (data & ~0x20) | (m_pit_out2 ? 0x20 : 0x00);
	data = (data & ~0x10) | ((BIT(m_ppi_portb, 1) && m_pit_out2) ? 0x10 : 0x00);

	LOGPPI("mc1502_ppi_portc_r = %02X (tap_val %f t2out %d) at %s\n",
		data, tap_val, m_pit_out2, machine().describe_context());
	return data;
}

uint8_t mc1502_state::mc1502_kppi_porta_r()
{
	uint8_t key = 0;

	for (int i = 0; i < 12; i++)
	{
		if (BIT(m_kbd.mask, i))
		{
			key |= m_kbdio[i]->read();
		}
	}

	key ^= 0xff;
	LOGPPI("mc1502_kppi_porta_r = %02X\n", key);
	return key;
}

void mc1502_state::mc1502_kppi_portb_w(uint8_t data)
{
	m_kbd.mask &= ~255;
	m_kbd.mask |= data ^ 255;
	if (!BIT(data, 0))
		m_kbd.mask |= 1 << 11;
	else
		m_kbd.mask &= ~(1 << 11);
	LOGPPI("mc1502_kppi_portb_w ( %02X -> %04X )\n", data, m_kbd.mask);
}

void mc1502_state::mc1502_kppi_portc_w(uint8_t data)
{
	m_kbd.mask &= ~(7 << 8);
	m_kbd.mask |= ((data ^ 7) & 7) << 8;
	LOGPPI("mc1502_kppi_portc_w ( %02X -> %04X )\n", data, m_kbd.mask);
}

void mc1502_state::mc1502_i8251_syndet(int state)
{
	if (!BIT(m_ppi_portc, 3))
		m_maincpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

void mc1502_state::mc1502_pit8253_out1_changed(int state)
{
	m_upd8251->write_txc(state);
	m_upd8251->write_rxc(state);
}

void mc1502_state::mc1502_pit8253_out2_changed(int state)
{
	m_pit_out2 = state;
	m_speaker->level_w(m_spkrdata & m_pit_out2);
	m_cassette->output(state ? 1 : -1);
}

void mc1502_state::mc1502_speaker_set_spkrdata(int state)
{
	m_spkrdata = state ? 1 : 0;
	m_speaker->level_w(m_spkrdata & m_pit_out2);
}

void mc1502_state::init_mc1502()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_ram(0, m_ram->size() - 1, m_ram->pointer());
}

void mc1502_state::machine_start()
{
	/*
	       Keyboard polling circuit holds IRQ1 high until a key is
	       pressed, then it starts a timer that pulses IRQ1 low each
	       40ms (check) for 20ms (check) until all keys are released.
	       Last pulse causes BIOS to write a 'break' scancode into port 60h.
	 */
	m_pic8259->ir1_w(1);
	m_kbd.pulsing = 0;
	m_kbd.mask = 0;
	m_kbd.keyb_signal_timer = timer_alloc(FUNC(mc1502_state::keyb_signal_callback), this);
	m_kbd.keyb_signal_timer->adjust(attotime::from_msec(20), 0, attotime::from_msec(20));
}

void mc1502_state::machine_reset()
{
	m_spkrdata = 0;
	m_pit_out2 = 1;
	m_ppi_portb = 0;
	m_ppi_portc = 0;
	m_speaker->level_w(0);
}

/*
 * macros
 */

void mc1502_state::fdc_config(device_t *device)
{
	mc1502_fdc_device &fdc = *downcast<mc1502_fdc_device *>(device);
	fdc.set_cpu(m_maincpu);
}

void mc1502_state::mc1502_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void mc1502_state::mc1502_io(address_map &map)
{
	map(0x0020, 0x0021).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0028, 0x0029).rw(m_upd8251, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x0040, 0x0043).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0060, 0x0063).rw(m_ppi8255n1, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0068, 0x006B).rw(m_ppi8255n2, FUNC(i8255_device::read), FUNC(i8255_device::write)); // keyboard poll
}

static INPUT_PORTS_START( mc1502 )
	PORT_INCLUDE( mc7007_3_keyboard )
INPUT_PORTS_END

void mc1502_state::mc1502(machine_config &config)
{
	I8088(config, m_maincpu, XTAL(16'000'000) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &mc1502_state::mc1502_map);
	m_maincpu->set_addrmap(AS_IO, &mc1502_state::mc1502_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));

	PIT8253(config, m_pit8253);
	m_pit8253->set_clk<0>(XTAL(16'000'000) / 12); /* heartbeat IRQ */
	m_pit8253->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_pit8253->set_clk<1>(XTAL(16'000'000) / 12); /* serial port */
	m_pit8253->out_handler<1>().set(FUNC(mc1502_state::mc1502_pit8253_out1_changed));
	m_pit8253->set_clk<2>(XTAL(16'000'000) / 12); /* pio port c pin 4, and speaker polling enough */
	m_pit8253->out_handler<2>().set(FUNC(mc1502_state::mc1502_pit8253_out2_changed));

	PIC8259(config, m_pic8259);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	I8255(config, m_ppi8255n1);
	m_ppi8255n1->out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_ppi8255n1->out_pb_callback().set(FUNC(mc1502_state::mc1502_ppi_portb_w));
	m_ppi8255n1->in_pc_callback().set(FUNC(mc1502_state::mc1502_ppi_portc_r));
	m_ppi8255n1->out_pc_callback().set(FUNC(mc1502_state::mc1502_ppi_portc_w));

	I8255(config, m_ppi8255n2);
	m_ppi8255n2->in_pa_callback().set(FUNC(mc1502_state::mc1502_kppi_porta_r));
	m_ppi8255n2->out_pb_callback().set(FUNC(mc1502_state::mc1502_kppi_portb_w));
	m_ppi8255n2->in_pc_callback().set("cent_status_in", FUNC(input_buffer_device::read));
	m_ppi8255n2->out_pc_callback().set(FUNC(mc1502_state::mc1502_kppi_portc_w));

	I8251(config, m_upd8251, 0);
	m_upd8251->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_upd8251->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_upd8251->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_upd8251->rxrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir7_w)); /* default handler does nothing */
	m_upd8251->txrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir7_w));
	m_upd8251->syndet_handler().set(FUNC(mc1502_state::mc1502_i8251_syndet));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_upd8251, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(m_upd8251, FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set(m_upd8251, FUNC(i8251_device::write_cts));

	isa8_device &isa(ISA8(config, "isa", 0));
	isa.set_memspace("maincpu", AS_PROGRAM);
	isa.set_iospace("maincpu", AS_IO);
	isa.irq2_callback().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	isa.irq3_callback().set(m_pic8259, FUNC(pic8259_device::ir3_w));
	isa.irq4_callback().set(m_pic8259, FUNC(pic8259_device::ir4_w));
	isa.irq5_callback().set(m_pic8259, FUNC(pic8259_device::ir5_w));
	isa.irq6_callback().set(m_pic8259, FUNC(pic8259_device::ir6_w));
	isa.irq7_callback().set(m_pic8259, FUNC(pic8259_device::ir7_w));
	isa.iochrdy_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	ISA8_SLOT(config, "board0", 0, "isa", mc1502_isa8_cards, "cga_mc1502", true); // FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa1", 0, "isa", mc1502_isa8_cards, "fdc", false).set_option_machine_config("fdc", [this](device_t *device) { fdc_config(device); });
	ISA8_SLOT(config, "isa2", 0, "isa", mc1502_isa8_cards, "rom", false).set_option_machine_config("fdc", [this](device_t* device) { fdc_config(device); });

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.80);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit6));
	m_centronics->busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit7));
	m_centronics->fault_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit4));
	m_centronics->perror_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit5));

	INPUT_BUFFER(config, "cent_status_in");

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	SOFTWARE_LIST(config, "flop_list").set_original("mc1502_flop");
//  SOFTWARE_LIST(config, "cass_list").set_original("mc1502_cass");

	RAM(config, RAM_TAG).set_default_size("608K").set_extra_options("96K"); /* 96 base + 512 on expansion card */
}


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
	ROM_REGION(0x10000,"bios", 0)

	ROM_DEFAULT_BIOS("v52")
	ROM_SYSTEM_BIOS(0, "v50", "v5.0 10/05/89")
	ROMX_LOAD("monitor_5_0.rom",  0xc000, 0x4000, CRC(9e97c6a0) SHA1(16a304e8de69ec4d8b92acda6bf28454c361a24f),ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v52", "v5.2 22/03/91")
	ROMX_LOAD("monitor_5_2.rom",  0xc000, 0x4000, CRC(0e65491e) SHA1(8a4d556473b5e0e59b05fab77c79c29f4d562412),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v521", "v5.21 12/10/92")
	ROMX_LOAD("monitor_5_21.rom", 0xc000, 0x4000, CRC(28c8f653) SHA1(04b0b09e0b86d9648a83352cc1590eb8963833e0),ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v531", "v5.31 12/10/92")
	ROMX_LOAD("monitor_5_31.rom", 0xc000, 0x4000, CRC(a48295d5) SHA1(6f38977c22f9cc6c2bc6f6e53edc4048ca6b6721),ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v533", "v5.33 01/08/93")
	ROMX_LOAD("0,cbc0.bin", 0xc000, 0x2000, CRC(9a55bc4f) SHA1(81da44eec2e52cf04b1fc7053502270f51270590),ROM_BIOS(4))
	ROMX_LOAD("1,dfe2.bin", 0xe000, 0x2000, CRC(8dec077a) SHA1(d6f6d7cc2183abc77fbd9cd59132de5766f7c458),ROM_BIOS(4))

	// 5.21 + 3rd party HDC support. fails checksum test so marked BAD_DUMP.
	ROM_SYSTEM_BIOS(5, "v521h", "v5.21h 22/09/93")
	ROMX_LOAD("mshbios0.pgm", 0xc000, 0x2000, BAD_DUMP CRC(be447261) SHA1(b93c597c17dfa4b678f72c20a3f7119b73e6ba1c),ROM_BIOS(5))
	ROMX_LOAD("mshbios1.pgm", 0xe000, 0x2000, BAD_DUMP CRC(89e2eaf2) SHA1(37d6b225b5e35574fdac81219589407d925225be),ROM_BIOS(5))

	// 5.3
	ROM_SYSTEM_BIOS(6, "v53", "v5.3 10/11/91")
	ROMX_LOAD("1502-3b0.pgm", 0xc000, 0x2000, CRC(dc148763) SHA1(7a5e66438007b2de328ac680614f9c4ff60f6a75),ROM_BIOS(6))
	ROMX_LOAD("1502-3b1.pgm", 0xe000, 0x2000, CRC(17fc2af2) SHA1(a060d7b7302dfa639025f025106b50412cf26953),ROM_BIOS(6))
	// 5.1 -- JCUKEN keyboard
	ROM_SYSTEM_BIOS(7, "v51", "v5.1 10/12/90")
	ROMX_LOAD("ms1502b0.pgm", 0xc000, 0x2000, CRC(92fcc29a) SHA1(930a4cffcd6ec6110dd9a18bd389b78f0ccb110a),ROM_BIOS(7))
	ROMX_LOAD("ms1502b1.pgm", 0xe000, 0x2000, CRC(fe355a58) SHA1(b4ef7775045c6f2095e2b487fe19824986a4892c),ROM_BIOS(7))
	// 5.31
	ROM_SYSTEM_BIOS(8, "v531_93", "v5.31 21/01/93")
	ROMX_LOAD("ms531b0.pgm", 0xc000, 0x2000, CRC(d97157d1) SHA1(cb1a1e0e2d9a0fcc78f9b09bfb4814d408ee4fae),ROM_BIOS(8))
	ROMX_LOAD("ms531b1.pgm", 0xe000, 0x2000, CRC(b1368e1a) SHA1(286496d25dc0ac2d8fe1802caffc6c37b236d105),ROM_BIOS(8))
	// 5.2
	ROM_SYSTEM_BIOS(9, "v52_91", "v5.2 10/11/91")
	ROMX_LOAD("msv5-2b0.pgm", 0xc000, 0x2000, CRC(f7f370e9) SHA1(e069a35005581a02856853b57dd511ab8e10054b),ROM_BIOS(9))
	ROMX_LOAD("msv5-2b1.pgm", 0xe000, 0x2000, CRC(d50e1c43) SHA1(22724dec0052ee9e52f44f5914f2f5f3fae14612),ROM_BIOS(9))

	// 7.2
	ROM_SYSTEM_BIOS(10, "v72", "v7.2 01/21/96")
	ROMX_LOAD("7.2_1.bin", 0xe000, 0x2000, CRC(80912ad4) SHA1(cc54b77b2db4cc5d614efafd04367d2f06400fc8),ROM_BIOS(10))

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_LOAD("symgen.rom", 0x0000, 0x2000, CRC(b2747a52) SHA1(6766d275467672436e91ac2997ac6b77700eba1e))
ROM_END

/*
        Predecessor of MC1502, same keyboard attachment but
        different video subsystem (not emulated).
*/
ROM_START( pk88 )
	ROM_REGION(0x10000, "bios", 0)

	// datecode 07.23.87
	ROM_LOAD( "b0.064", 0x0000, 0x2000, CRC(80d3cf5d) SHA1(64769b7a8b60ffeefa04e4afbec778069a2840c9))
	ROM_LOAD( "b1.064", 0x2000, 0x2000, CRC(673a4acc) SHA1(082ae803994048e225150f771794ca305f73d731))
	ROM_LOAD( "b2.064", 0x4000, 0x2000, CRC(1ee66152) SHA1(7ed8c4c6c582487e802beabeca5b86702e5083e8))
	ROM_LOAD( "b3.064", 0x6000, 0x2000, CRC(3062b3fc) SHA1(5134dd64721cbf093d059ee5d3fd09c7f86604c7))
	ROM_LOAD( "pk88-0.064", 0xc000, 0x2000, CRC(1e4666cf) SHA1(6364c5241f2792909ff318194161eb2c29737546))
	ROM_LOAD( "pk88-1.064", 0xe000, 0x2000, CRC(6fa7e7ef) SHA1(d68bc273baa46ba733ac6ad4df7569dd70cf60dd))
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//     YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT         COMPANY               FULLNAME               FLAGS
COMP ( 1989, mc1502, 0,      0,      mc1502,  mc1502,  mc1502_state, init_mc1502, "NPO Microprocessor", "Elektronika MS 1502", MACHINE_IMPERFECT_GRAPHICS )
COMP ( 1988, pk88,   0,      0,      mc1502,  mc1502,  mc1502_state, init_mc1502, "NPO Microprocessor", "Elektronika PK-88",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
