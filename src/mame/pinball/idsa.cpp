// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************************************************
PINBALL
IDSA pinballs

Games:
- Basketball
- Fantastic Car (1984, to be dumped)
- V1

Hardware listing and ROM definitions from PinMAME.

Hardware:
---------
CPU:     Z80 @ 4 MHz
    INT: IRQ @ 977 Hz (4MHz/2048/2) or 488 Hz (4MHz/2048/4)
DRIVERS: bsktbllp: 2 x 8255 driving lamps and coils, used as demultiplexers only (no read access)
DISPLAY: bsktbllp: 7-digit 7-segment panels with PROM-based 5-bit BCD data (allowing a simple alphabet)
         v1: 6-digit 7-segment panels with BCD decoding
SOUND:   2 x AY8910 @ 2 MHz (also used as output interface) plus SP0256 @ 3.12 MHz on board

Schematic is terrible, lots of important info left out. Need the V1 manual & schematic.

ToDO:
- Various PROMs are undumped.
- Speech is gibberish
- Display
- Save states
- Mechanical sounds
- The usual: identify lamps, solenoids, contactors
- Outputs

**********************************************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/sp0256.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "speaker.h"

namespace {

class idsa_state : public genpin_class
{
public:
	idsa_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speech(*this, "speech")
		, m_ppi(*this, "ppi%u", 1U)
		{ }

	void bsktbllp(machine_config &config);
	void idsa(machine_config &config);

private:
	void clock_w(int state);
	uint8_t portb0_r(offs_t offset);
	void port80_w(uint8_t data);
	void port90_w(uint8_t data);
	void ppi_control_w(uint8_t data);
	void ppi_data_w(uint8_t data);
	void ppi1_a_w(uint8_t data);
	void ppi1_b_w(uint8_t data);
	void ppi1_c_w(uint8_t data);
	void ppi2_a_w(uint8_t data);
	void ppi2_b_w(uint8_t data);
	void ppi2_c_w(uint8_t data);
	void ay1_a_w(uint8_t data);
	void ay1_b_w(uint8_t data);
	void ay2_a_w(uint8_t data);
	void ay2_b_w(uint8_t data);

	void maincpu_io_map(address_map &map) ATTR_COLD;
	void maincpu_map(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;

	uint16_t m_irqcnt = 0U;
	uint8_t m_ppi_data = 0U;
	required_device<cpu_device> m_maincpu;
	required_device<sp0256_device> m_speech;
	optional_device_array<i8255_device, 2> m_ppi;
};

void idsa_state::maincpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
}

void idsa_state::maincpu_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x0f).portr("X0");
	map(0x10, 0x1f).portr("X1");
	map(0x20, 0x2f).portr("X2");
	map(0x30, 0x3f).portr("X3");
	map(0x40, 0x4f).portr("X4");
	map(0x50, 0x5f).portr("X5");
	map(0x60, 0x6f).portr("X6");
	map(0x70, 0x7f).portr("X7");
	map(0x80, 0x8f).w(FUNC(idsa_state::port80_w));
	map(0x90, 0x9f).w(FUNC(idsa_state::port90_w));
	map(0xb0, 0xb3).r(FUNC(idsa_state::portb0_r));
	map(0xbd, 0xbd).portr("X8");
	map(0xd0, 0xdf).w(m_speech, FUNC(sp0256_device::ald_w));
	map(0xe0, 0xef).rw("aysnd1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xf0, 0xff).rw("aysnd2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
}


static INPUT_PORTS_START( idsa )
	PORT_START("X0") // 1-8
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP01")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP02")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP03")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP04")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP05")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP06")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP07")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP08")

	PORT_START("X1") // 9-16
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP16")

	PORT_START("X2") // 17-24
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP24")

	PORT_START("X3") // 25-32
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP32")

	PORT_START("X4") // 33-40
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP40")

	PORT_START("X5") // 41-48
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP48")

	PORT_START("X6")
	PORT_DIPNAME( 0x01, 0x01, "S01")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "S02")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x04, "S03")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x08, "S04")
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "S06")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "S07")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x00, "S08")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))

	PORT_START("X7")
	PORT_DIPNAME( 0x01, 0x01, "S09")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "S10")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x04, "S11")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x08, "S12")
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x10, "S13")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x00, "S14")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "S15")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x80, "S16")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))

	PORT_START("X8") // all bits here are unknown
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // this bit low makes V1 reboot
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START )
INPUT_PORTS_END

// This came from pinmame, even though there's no spb640 chip
uint8_t idsa_state::portb0_r(offs_t offset)
{
	uint16_t data = m_speech->spb640_r(offset / 2);
	return offset % 2 ? (uint8_t)(data >> 8) : (uint8_t)(data & 0xff);
}

// ports 80 & 90 for the display
void idsa_state::port80_w(uint8_t data)
{
}

void idsa_state::port90_w(uint8_t data)
{
}

// AY ports are for lamps and solenoids
void idsa_state::ppi_control_w(uint8_t data)
{
	//logerror("%s: AY1 port A = %02X\n", machine().describe_context(), data);
	if (!BIT(data, 2))
		m_ppi[0]->write(data & 0x03, m_ppi_data);
	if (!BIT(data, 3))
		m_ppi[1]->write(data & 0x03, m_ppi_data);
}

void idsa_state::ppi_data_w(uint8_t data)
{
	m_ppi_data = data;
}

void idsa_state::ppi1_a_w(uint8_t data)
{
	logerror("%s: PPI1 port A = %02X\n", machine().describe_context(), data);
}

void idsa_state::ppi1_b_w(uint8_t data)
{
	logerror("%s: PPI1 port B = %02X\n", machine().describe_context(), data);
}

void idsa_state::ppi1_c_w(uint8_t data)
{
	logerror("%s: PPI1 port C = %02X\n", machine().describe_context(), data);
}

void idsa_state::ppi2_a_w(uint8_t data)
{
	logerror("%s: PPI2 port A = %02X\n", machine().describe_context(), data);
}

void idsa_state::ppi2_b_w(uint8_t data)
{
	logerror("%s: PPI2 port B = %02X\n", machine().describe_context(), data);
}

void idsa_state::ppi2_c_w(uint8_t data)
{
	logerror("%s: PPI2 port C = %02X\n", machine().describe_context(), data);
}

void idsa_state::ay1_a_w(uint8_t data)
{
	//logerror("%s: AY1 port A = %02X\n", machine().describe_context(), data);
}

void idsa_state::ay1_b_w(uint8_t data)
{
	//logerror("%s: AY1 port B = %02X\n", machine().describe_context(), data);
}

void idsa_state::ay2_a_w(uint8_t data)
{
	//logerror("%s: AY2 port A = %02X\n", machine().describe_context(), data);
}

void idsa_state::ay2_b_w(uint8_t data)
{
	//logerror("%s: AY2 port B = %02X\n", machine().describe_context(), data);
}

void idsa_state::clock_w(int state)
{
	if (state)
	{
		m_irqcnt++;
		if (m_irqcnt == 1)
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		else
		if (m_irqcnt == 2048)
		{
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
			m_irqcnt = 0;
		}
	}
}

void idsa_state::machine_reset()
{
	m_irqcnt = 0;
}

void idsa_state::idsa(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &idsa_state::maincpu_map);
	m_maincpu->set_addrmap(AS_IO, &idsa_state::maincpu_io_map);

	clock_device &irqclock(CLOCK(config, "irqclock", XTAL(8'000'000) / 4));
	irqclock.signal_handler().set(FUNC(idsa_state::clock_w));

	/* video hardware */
	//config.set_default_layout()

	/* sound hardware */
	genpin_audio(config);
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	SP0256(config, m_speech, 3120000); // unknown variant
	m_speech->add_route(ALL_OUTPUTS, "lspeaker", 1.5);

	ay8910_device &aysnd1(AY8910(config, "aysnd1", 2000000));  // 2Mhz according to pinmame, schematic omits the clock line
	aysnd1.port_a_write_callback().set(FUNC(idsa_state::ay1_a_w));
	aysnd1.port_b_write_callback().set(FUNC(idsa_state::ay1_b_w));
	aysnd1.add_route(ALL_OUTPUTS, "lspeaker", 0.75);

	ay8910_device &aysnd2(AY8910(config, "aysnd2", 2000000));
	aysnd2.port_a_write_callback().set(FUNC(idsa_state::ay2_a_w));
	aysnd2.port_b_write_callback().set(FUNC(idsa_state::ay2_b_w));
	aysnd2.add_route(ALL_OUTPUTS, "rspeaker", 0.75);
}

void idsa_state::bsktbllp(machine_config &config)
{
	idsa(config);
	auto &aysnd1(*subdevice<ay8910_device>("aysnd1"));
	aysnd1.port_a_write_callback().set(FUNC(idsa_state::ppi_control_w));
	aysnd1.port_b_write_callback().set(FUNC(idsa_state::ppi_data_w));

	I8255(config, m_ppi[0]);
	m_ppi[0]->out_pa_callback().set(FUNC(idsa_state::ppi1_a_w));
	m_ppi[0]->out_pb_callback().set(FUNC(idsa_state::ppi1_b_w));
	m_ppi[0]->out_pc_callback().set(FUNC(idsa_state::ppi1_c_w));

	I8255(config, m_ppi[1]);
	m_ppi[1]->out_pa_callback().set(FUNC(idsa_state::ppi2_a_w));
	m_ppi[1]->out_pb_callback().set(FUNC(idsa_state::ppi2_b_w));
	m_ppi[1]->out_pc_callback().set(FUNC(idsa_state::ppi2_c_w));
}


ROM_START(v1)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("v1.128", 0x0000, 0x4000, CRC(4e08f7bc) SHA1(eb6ef00e489888dd9c53010c525840de06bcd0f3))

	ROM_REGION(0x10000, "speech", 0)
	ROM_LOAD( "sp0256a-al2.1b",   0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

ROM_START(bsktbllp)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("bsktball.256", 0x0000, 0x8000, CRC(d474e29b) SHA1(750cbacef34dde0b3dcb6c1e4679db78a73643fd))
	ROM_FILL(0x0cb8, 1, 0x3e) // patch suspicious LD that fetches a bad AY address

	ROM_REGION(0x10000, "speech", 0)
	ROM_LOAD( "sp0256a-al2.1b",   0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

} // Anonymous namespace

GAME( 1985, v1,       0, idsa,     idsa, idsa_state, empty_init, ROT0, "IDSA", "V.1",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, bsktbllp, 0, bsktbllp, idsa, idsa_state, empty_init, ROT0, "IDSA", "Basket Ball", MACHINE_IS_SKELETON_MECHANICAL )
