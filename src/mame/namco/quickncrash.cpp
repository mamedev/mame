// license:BSD-3-Clause
// copyright-holders:AJR

/*

Quick & Crash by Namco (1999)

Hardware notes:
Main PCB has both 'Namco TY294-V-0 1331960101' and 'Tamura DNP0674A'
- KL5C80A12CFP (KC82-based Z80 with MMU, timers, parallel ports, KP51 USART)
- Oki M9810
- NEC D71055C-10 (8255 compatible PPI)
- 93C66N
- 6264BLP10L
- 20.000 MHz XTAL
- lots of connectors

Namco H-5 CPU PCB (8830970101 - 8830960101)
- TMPZ84C015BF-12
- D24OP8I XTAL
- Epson SED1351F0A
- 2x TC55257DFL-70L (also serve as the dot CPU's work RAM through the
  SED1351's cycle steal interface)

The two CPUs communicate through a serial link:
- main CPU KP51 USART (I/O 0x38 data, 0x39 status/control) <-> dot CPU SIO B
  (I/O 0x1a data, 0x1b control). The main CPU sends messages terminated with
  0xFE (0xFD/0xFC are framing commands), the dot CPU replies with a response
  byte (usually the command letter) followed by 0xFB.

Cool Gunman runs on similar hardware:
Main PCB has both 'Namco TY294V-0 1329960102' and 'Tamura GAME PCB'
- KL5C80A12CFP
- Oki M9810
- 93C46N
- 6264 (removed?)
- 20.000 MHz XTAL
- lots of connectors

It probably needs at least a LED driving PCB, too.
*/

#include "emu.h"

#include "cpu/z80/kl5c80a12.h"
#include "cpu/z80/tmpz84c015.h"
#include "machine/clock.h"
#include "machine/eepromser.h"
#include "machine/i8255.h"
#include "machine/ticket.h"
#include "sound/okim9810.h"

#include "screen.h"
#include "speaker.h"

#define LOG_PORTS (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PORTS)
#include "logmacro.h"


namespace {

class qncrash_state : public driver_device
{
public:
	qncrash_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dotcpu(*this, "dotcpu"),
		m_ppi(*this, "ppi"),
		m_eeprom(*this, "eeprom"),
		m_oki(*this, "oki"),
		m_screen(*this, "screen"),
		m_ticket(*this, "ticket"),
		m_dotbank(*this, "dotbank"),
		m_dotram(*this, "dotram", 0x8000, ENDIANNESS_LITTLE),
		m_dot_portb2(0)
	{ }

	void qncrash(machine_config &config) ATTR_COLD;
	void cgunman(machine_config &config) ATTR_COLD;

	int ticket_sensor_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<kl5c80a12_device> m_maincpu;
	required_device<tmpz84c015_device> m_dotcpu;
	optional_device<i8255_device> m_ppi;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<okim9810_device> m_oki;
	optional_device<screen_device> m_screen;
	optional_device<ticket_dispenser_device> m_ticket;
	required_memory_bank m_dotbank;
	memory_share_creator<u8> m_dotram;

	u8 m_dot_portb2;
	u8 m_sed_regs[16]{};

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void cgunman_main_io_map(address_map &map) ATTR_COLD;
	void dot_program_map(address_map &map) ATTR_COLD;
	void dot_io_map(address_map &map) ATTR_COLD;

	void dot_bank_w(u8 data);
	void sed_reg_w(offs_t offset, u8 data);
	u8 sed_reg_r(offs_t offset);
	u8 oki_r();
	void oki_w(u8 data);

	void main_porta0_w(u8 data);
	void main_portb0_w(u8 data);
	void main_portb2_w(u8 data);
};


void qncrash_state::machine_start()
{
	save_item(NAME(m_dot_portb2));
	save_item(NAME(m_sed_regs));

	// the dot CPU (and its ROM bank) only exist when it isn't disabled (cgunman)
	if (!m_dotcpu->disabled())
		m_dotbank->configure_entries(0, 8, memregion("dotcpu")->base(), 0x4000);
}

void qncrash_state::machine_reset()
{
	std::fill(std::begin(m_sed_regs), std::end(m_sed_regs), 0);
	m_dot_portb2 = 0;
}

void qncrash_state::main_program_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("maincpu", 0x0000);
	map(0xff800, 0xffdff).ram(); // external 6264
	// 0xffe00 - 0xfffff is internal RAM
}

void qncrash_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).lw8(NAME([this] (u8 data) { LOGMASKED(LOG_PORTS, "%s: port40 out %02X\n", machine().describe_context(), data); }));
	map(0x50, 0x50).rw(FUNC(qncrash_state::oki_r), FUNC(qncrash_state::oki_w));
	map(0x58, 0x5b).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	// 0x38-0x3b are the internal KP51 USART
}

void qncrash_state::cgunman_main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).lw8(NAME([this] (u8 data) { LOGMASKED(LOG_PORTS, "%s: port40 out %02X\n", machine().describe_context(), data); }));
	map(0x50, 0x50).rw(FUNC(qncrash_state::oki_r), FUNC(qncrash_state::oki_w));
}

void qncrash_state::dot_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("dotcpu", 0x0000);
	map(0x4000, 0x7fff).bankr(m_dotbank);
	map(0x8000, 0xffff).ram().share("dotram");
}

void qncrash_state::dot_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).w(FUNC(qncrash_state::dot_bank_w));
	map(0x60, 0x60).lw8(NAME([this] (u8 data) { LOGMASKED(LOG_PORTS, "%s: port60 out %02X\n", machine().describe_context(), data); }));
	map(0xc0, 0xcf).rw(FUNC(qncrash_state::sed_reg_r), FUNC(qncrash_state::sed_reg_w));
}


//-------------------------------------------------
//  dot CPU helpers
//-------------------------------------------------

void qncrash_state::dot_bank_w(u8 data)
{
	// bit 3-5 of the latch select the 16KB ROM bank visible at 0x4000
	m_dotbank->set_entry((data >> 3) & 7);
}

void qncrash_state::sed_reg_w(offs_t offset, u8 data)
{
	m_sed_regs[offset & 0xf] = data;
}

u8 qncrash_state::sed_reg_r(offs_t offset)
{
	return m_sed_regs[offset & 0xf];
}


//-------------------------------------------------
//  M9810 interface
//-------------------------------------------------

u8 qncrash_state::oki_r()
{
	return m_oki->read_status();
}

void qncrash_state::oki_w(u8 data)
{
	// port B2 bit 0 selects between the command register and the TMP register
	LOGMASKED(LOG_PORTS, "%s: oki %s %02X\n", machine().describe_context(), BIT(m_dot_portb2, 0) ? "tmp" : "cmd", data);
	if (BIT(m_dot_portb2, 0))
		m_oki->write_tmp_register(data);
	else
		m_oki->write_command(data);
}


//-------------------------------------------------
//  main CPU port outputs
//-------------------------------------------------

void qncrash_state::main_porta0_w(u8 data)
{
	// A0.6 = coin counter drive
	LOGMASKED(LOG_PORTS, "%s: port A0 out %02X\n", machine().describe_context(), data);
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
}

void qncrash_state::main_portb0_w(u8 data)
{
	// B0.0-B0.3 = lamps, B0.4 = ticket dispenser auxiliary output,
	// B0.5 = ticket dispenser motor, B0.6/B0.7 = flat target U motor
	LOGMASKED(LOG_PORTS, "%s: port B0 out %02X\n", machine().describe_context(), data);
	if (m_ticket.found())
		m_ticket->motor_w(BIT(data, 5));
}


int qncrash_state::ticket_sensor_r()
{
	// ticket sensor is only present on Quick & Crash
	return m_ticket.found() ? m_ticket->line_r() : 0;
}

void qncrash_state::main_portb2_w(u8 data)
{
	// B2.0 = M9810 register select, B2.1 = M9810 read strobe
	m_dot_portb2 = data;
}


//-------------------------------------------------
//  dot matrix display (SED1351)
//-------------------------------------------------

u32 qncrash_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// The SED1351 scans a 2bpp display buffer out of its 64KB VRAM space.
	// The dot CPU's 0x8000-0xffff window maps to VRAM 0x2000-0x9fff, so the
	// display start address (registers 6/7) minus 0x6000 gives the CPU address.
	// Each display line is 25 bytes (24 bytes of visible 96 pixels + 1 pad).
	u16 const start = (u16(m_sed_regs[7]) << 8) | m_sed_regs[6];
	bitmap.fill(rgb_t::black(), cliprect);

	for (int y = 0; y < 16; y++)
	{
		u32 const row = start + y * 25;
		for (int x = 0; x < 96; x++)
		{
			u32 const cpu_addr = row + (x >> 2) + 0x6000;
			if ((cpu_addr < 0x8000) || (cpu_addr > 0xffff))
				continue;
			u8 const byte = m_dotram[cpu_addr - 0x8000];
			u8 const pix = (byte >> (6 - 2 * (x & 3))) & 3;
			// 4 gray shades
			u8 const level = pix * 0x55;
			bitmap.pix(y, x) = rgb_t(level, 0, 0); // red LEDs
		}
	}

	return 0;
}

static INPUT_PORTS_START( qncrash ) // no DIPs on PCB
	PORT_START("IN0") // KP65 port A0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // coin switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service switch (service credit / change setting)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) // select switch (advance menu item / operate test)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Switch") PORT_TOGGLE // slide switch, held = test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(qncrash_state::ticket_sensor_r)) // ticket dispenser sensor
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // coin door switch?
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // configured as output (coin counter)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START("IN1") // 8255 PPI port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Launcher Sensor") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Flat Target U Up/Down Sensor") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Flat Target U Movement Sensor") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Flat Target L Up/Down Sensor") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Flat Target L Movement Sensor") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2") // 8255 PPI port B
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Shutter Sensor") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Holster Sensor") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        // trigger switch
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3") // 8255 PPI port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Cup Target Light Receiver") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Flat Target U Light Receiver") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Flat Target L Light Receiver") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void qncrash_state::qncrash(machine_config &config)
{
	KL5C80A12(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &qncrash_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &qncrash_state::main_io_map);
	m_maincpu->in_p0_callback().set_ioport("IN0");
	m_maincpu->out_p0_callback().set(FUNC(qncrash_state::main_porta0_w));
	m_maincpu->out_p1_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(0);
	m_maincpu->out_p1_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(1);
	m_maincpu->out_p1_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(2);
	m_maincpu->out_p2_callback().set(FUNC(qncrash_state::main_portb0_w));
	m_maincpu->out_p3_callback().set([this] (u8 data) { LOGMASKED(LOG_PORTS, "%s: p3 out %02X\n", machine().describe_context(), data); });
	m_maincpu->out_p4_callback().set(FUNC(qncrash_state::main_portb2_w));

	tmpz84c015_device &dotcpu(TMPZ84C015(config, "dotcpu", 24'000'000 / 2)); // unknown D24OP8I XTAL, but TMPZ84C015BF-12 is rated for max 12 MHz, so guessed
	dotcpu.set_addrmap(AS_PROGRAM, &qncrash_state::dot_program_map);
	dotcpu.set_addrmap(AS_IO, &qncrash_state::dot_io_map);
	m_maincpu->txd_callback().set(dotcpu, FUNC(tmpz84c015_device::rxb_w));
	dotcpu.out_txdb_callback().set(m_maincpu, FUNC(kl5c80a12_device::rxd_w));

	// the dot CPU's SIO B clock must match the main CPU's USART clock: the KP51
	// is fed by KP63 timer B channel 1 (PWM, /4 prescale, count 0x070F -> 4*16
	// clocks at 10MHz = 6.4us) giving 156.25kHz, and the SIO divides by 16 (WR4
	// x16), so both ends run at 9765.6 baud
	clock_device &sioclk(CLOCK(config, "sioclk", 156'250)); // real clock source not identified
	sioclk.signal_handler().set(dotcpu, FUNC(tmpz84c015_device::txcb_w));
	sioclk.signal_handler().append(dotcpu, FUNC(tmpz84c015_device::rxcb_w));

	i8255_device &ppi(I8255(config, "ppi")); // NEC D71055C
	ppi.in_pa_callback().set_ioport("IN1");
	ppi.in_pb_callback().set_ioport("IN2");
	ppi.in_pc_callback().set_ioport("IN3");

	EEPROM_93C66_16BIT(config, "eeprom");

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(50)); // guessed; each phase must exceed the ROM's 8-sample (16ms) sensor debounce and leave the sensor low when the motor stops

	// video hardware
	// TODO:
	// - a 5 digit 7 segment display for time
	// - replace the dot matrix with lamps+layout?
	SCREEN(config, m_screen);
	m_screen->set_raw(96 * 16 * 120, 96, 0, 96, 16, 0, 16);
	m_screen->set_screen_update(FUNC(qncrash_state::screen_update));

	// sound hardware
	SPEAKER(config, "gun_speaker").front_center();
	SPEAKER(config, "target_speaker").front_center();

	okim9810_device &oki(OKIM9810(config, "oki", 4'096'000)); // XTAL on PCB not readable from the pic
	oki.add_route(0, "gun_speaker", 1.00);
	oki.add_route(1, "target_speaker", 1.00);
}

void qncrash_state::cgunman(machine_config &config)
{
	// basic machine hardware
	KL5C80A12(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &qncrash_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &qncrash_state::cgunman_main_io_map);
	m_maincpu->in_p0_callback().set_ioport("IN0");
	m_maincpu->out_p0_callback().set(FUNC(qncrash_state::main_porta0_w));
	m_maincpu->out_p1_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(0);
	m_maincpu->out_p1_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(1);
	m_maincpu->out_p1_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(2);
	m_maincpu->out_p2_callback().set([this] (u8 data) { logerror("%s: p2 out %02X\n", machine().describe_context(), data); });
	m_maincpu->out_p3_callback().set([this] (u8 data) { logerror("%s: p3 out %02X\n", machine().describe_context(), data); });
	m_maincpu->out_p4_callback().set([this] (u8 data) { logerror("%s: p4 out %02X\n", machine().describe_context(), data); });

	TMPZ84C015(config, "dotcpu", 24'000'000 / 2).set_disable();

	EEPROM_93C46_16BIT(config, "eeprom");

	// video hardware
	// no dot matrix, but a few 7 segment displays

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM9810(config, "oki", 4'096'000).add_route(ALL_OUTPUTS, "mono", 1.00); // XTAL on PCB not readable from the pic
}


ROM_START( qncrash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qc1-mpr0c.ic3", 0x00000, 0x10000, CRC(8e3f605d) SHA1(69a0da6286e250b92e47b66c9423bc5c793b350b) )

	ROM_REGION( 0x20000, "dotcpu", 0 )
	ROM_LOAD( "qc1-dot0.ic7", 0x00000, 0x20000, CRC(45dda645) SHA1(27efcef0e3a09390eec0d7859465a13caf52f9df) )

	ROM_REGION( 0x800000, "oki", 0 )
	ROM_LOAD( "qc1-sound00.ic12", 0x000000, 0x400000, CRC(d72713d2) SHA1(556a0be2bb08fc9b4a2476b0ce8a23aa66858809) )
	ROM_LOAD( "qc1-sound01.ic13", 0x400000, 0x400000, CRC(70e472a1) SHA1(df06270cede1d00e2ec231276e5e5466ab549794) ) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "93c66n.ic5", 0x000, 0x200, CRC(1ba66a58) SHA1(adb7f1685cf81585ed30613adc39e9091e63af84) )

	ROM_REGION( 0x600, "dot_plds", ROMREGION_ERASE00 ) // all 18CV8P
	ROM_LOAD( "j4153.ic2", 0x000, 0x155, NO_DUMP )
	ROM_LOAD( "j4154.ic3", 0x200, 0x155, NO_DUMP )
	ROM_LOAD( "j4155.ic5", 0x400, 0x155, NO_DUMP )
ROM_END

ROM_START( qncrasha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qc3_mpr0.ic3", 0x00000, 0x10000, CRC(42c54dec) SHA1(0f6ca4bec7ae4f60b1943dad756933d02cd660c4) )

	ROM_REGION( 0x20000, "dotcpu", 0 )
	ROM_LOAD( "qc3_dot0.bin", 0x00000, 0x20000, CRC(97d8c117) SHA1(61a8b52c61abae8cbeccf6dd23e3ee0b4c2e443d) )

	ROM_REGION( 0x800000, "oki", 0 )
	ROM_LOAD( "qc1_snd0.ic12", 0x000000, 0x400000, CRC(d72713d2) SHA1(556a0be2bb08fc9b4a2476b0ce8a23aa66858809) )
	ROM_LOAD( "qc1_snd1.ic13", 0x400000, 0x400000, CRC(70e472a1) SHA1(df06270cede1d00e2ec231276e5e5466ab549794) ) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "93c66n.ic5", 0x000, 0x200, CRC(1ba66a58) SHA1(adb7f1685cf81585ed30613adc39e9091e63af84) )

	ROM_REGION( 0x600, "dot_plds", ROMREGION_ERASE00 ) // all 18CV8P
	ROM_LOAD( "j4153.ic2", 0x000, 0x155, NO_DUMP )
	ROM_LOAD( "j4154.ic3", 0x200, 0x155, NO_DUMP )
	ROM_LOAD( "j4155.ic5", 0x400, 0x155, NO_DUMP )
ROM_END

ROM_START( cgunman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cgm1_mpro.ic3", 0x00000, 0x10000, CRC(0aed68fb) SHA1(d72a39c37c33369dae8a298fec2c8ef9065101d9) )

	ROM_REGION( 0x20000, "dotcpu", 0 )
	ROM_LOAD( "cgm1_dot0.bin", 0x00000, 0x20000, NO_DUMP ) // not 100% sure it exists

	ROM_REGION( 0x800000, "oki", 0 )
	ROM_LOAD( "cgm1_vo1.ic18", 0x000000, 0x800000, CRC(7ccae65c) SHA1(240a6c527c919fb9cc60901c2ebc5f3aab0df8d5) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46n.ic6", 0x00, 0x80, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1999, qncrash,  0,       qncrash, qncrash, qncrash_state, empty_init, ROT0, "Namco", "Quick & Crash (Japan, V2.200)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK ) // version listed at 0xa97a in program ROM
GAME( 1999, qncrasha, qncrash, qncrash, qncrash, qncrash_state, empty_init, ROT0, "Namco", "Quick & Crash (US)",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1998, cgunman,  0,       cgunman, qncrash, qncrash_state, empty_init, ROT0, "Namco", "Cool Gunman (V1.70)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
