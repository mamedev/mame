// license:BSD-3-Clause
// copyright-holders:AJR, Tomás García-Merás Capote

/*******************************************************************************
    Driver for Automatics Pasqual darts with CRT display.
  ________________________________________________________________________________
  |    _______                              _______  ______                       |
  |    |__CN__|                             |__CN__| |__CN_|                  ___ |
  |   ___  ___                                                                |  ||
  |   |F|  |F|                   __________                              ___  |  ||
  |   |U|  |U|                   |74LS373N_|                        ____ |  | |  ||
  |   |S|  |S|   __________     ___________                         |___||  | |  ||
  |   |E|  |E|  KS74HCTLS379N   | RAM      |        _________            |  | |C ||
  |                             |__________|       |_LM380N_|   ______   |__| |N ||
  |              _________     ____________         _________  |     |   ___  |  ||
  |              |SN74HC14N    | EMPTY     |       |MT4264-10  |     |   |  | |  ||
  |                            |___________|        _________  |     |   |  | |  ||
  |              _________     ____________        |MT4264-10 TMP82C55AP-2  | |  ||
  |              |74HC32AP|    | EMPTY     |        _________  |     |   |__| |__||
  |                            |___________|       |MT4264-10  |     |   ___  ___ |
  |        ___   _________     ____________         _________  |     |   |  | |C ||
  |        |F|   |GAL16V8_|    | ROM 3     |       |MT4264-10  |_____|   |  | |N ||
  |        |U|                 |___________|        _________    ______  |  | |  ||
  |        |S|   _________     ____________        |MT4264-10   |     |  |__|<-ULN2903A
  |        |E|   |74HC244AP    | ROM 2     |        _________   |     |       |__||
  |                            |___________|       |MT4264-10   |     |       ___ |
  |        ___   _________     ____________         _________   |     |       |CN||
  |        |F|   |74HC244AP    | ROM 1     |       |MT4264-10  AY38910A/P     |__||
  |        |U|                 |___________|        _________   |     |       ___ |
  |        |S|       _________    _________  _____ |MT4264-10   |     |       |  ||
  |        |E|      |74HC244AP   |74HC245AP |XTAL 10.245MHz     |_____|       |C ||
  |                        ________________    ________________  _____        |N ||
  |            _________   |Z8400A PS      |   |VIDEO          | |DIPS        |__||
  |           |_GD4011B|   |_______________|   |_______________|   _________      |
  |                                                               |CD40208E|      |
  |_______________________________________________________________________________|

  The program only writes "automatics PASQUAL S.A." to the EEPROM and reads it
  back on every boot, hanging if it doesn't match.

  The cabinet has three lamps on the left of the monitor and four buttons on the
  right. The lamps of the Up, Down and Select buttons blink while the game is
  waiting for them.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"
#include "screen.h"
#include "speaker.h"

#include "tecnodar.lh"


namespace {

class tecnodar_state : public driver_device
{
public:
	tecnodar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi")
		, m_eeprom(*this, "eeprom")
		, m_psg(*this, "psg")
		, m_rombank(*this, "rombank")
		, m_in1(*this, "IN1")
		, m_dart(*this, "DART%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
		, m_input_select(0xffff)
	{
	}

	void tecnodar(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void bank_w(offs_t offset, u8 data);
	u8 inputs_r();
	u8 ppi_r(offs_t offset);
	void ppi_w(offs_t offset, u8 data);
	void ppi_pa_w(u8 data);
	void ppi_pb_w(u8 data);
	void ppi_pc_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<ay8910_device> m_psg;
	required_memory_bank m_rombank;
	required_ioport m_in1;
	required_ioport_array<16> m_dart;
	output_finder<6> m_lamps;

	u16 m_input_select;
};


void tecnodar_state::machine_start()
{
	m_rombank->configure_entries(0, 8, memregion("banked")->base(), 0x4000);
	m_rombank->set_entry(0);

	save_item(NAME(m_input_select));
}


void tecnodar_state::bank_w(offs_t offset, u8 data)
{
	m_rombank->set_entry(BIT(offset, 2, 2) | (BIT(offset, 0) << 2));
}

u8 tecnodar_state::inputs_r()
{
	u8 ret = 0xf;
	for (int i = 0; i < 16; i++)
		if (!BIT(m_input_select, i))
			ret &= m_dart[i]->read();

	return (ret << 4) | m_in1->read();
}

u8 tecnodar_state::ppi_r(offs_t offset)
{
	return m_ppi->read(offset >> 5);
}

void tecnodar_state::ppi_w(offs_t offset, u8 data)
{
	m_ppi->write(offset >> 5, data);
}

void tecnodar_state::ppi_pa_w(u8 data)
{
	m_eeprom->cs_write(BIT(data, 0));
	m_eeprom->clk_write(BIT(data, 1));
	m_eeprom->di_write(BIT(data, 2));

	m_input_select = (m_input_select & 0xff00) | data;
}

void tecnodar_state::ppi_pb_w(u8 data)
{
	// pulses once for every 100 Pts
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));

	// lamp0-2: FINAL PARTIDA, RETIRE DARDOS and TIRE DARDOS
	// lamp3-5: Down, Select and Up buttons
	for (int i = 0; i < 6; i++)
		m_lamps[i] = BIT(data, i + 1);

	if (!BIT(data, 7))
		m_psg->reset_w(); // maybe
}

void tecnodar_state::ppi_pc_w(u8 data)
{
	m_input_select = u16(data) << 8 | (m_input_select & 0x00ff);
}

void tecnodar_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
	map(0x4000, 0x7fff).bankr(m_rombank);
	map(0x8002, 0x8002).select(0x60).rw(FUNC(tecnodar_state::ppi_r), FUNC(tecnodar_state::ppi_w));
	map(0x8004, 0x8004).w(m_psg, FUNC(ay8910_device::data_w));
	map(0x8008, 0x8008).r(m_psg, FUNC(ay8910_device::data_r));
	map(0x800c, 0x800c).w(m_psg, FUNC(ay8910_device::address_w));
	map(0x8010, 0x8011).rw("vdp", FUNC(tms9129_device::read), FUNC(tms9129_device::write));
	map(0xc000, 0xc7ff).ram();
}

void tecnodar_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).w(FUNC(tecnodar_state::bank_w));
}


static INPUT_PORTS_START(tecnodar)
	PORT_START("IN0")
	PORT_DIPNAME(0x01, 0x01, "Automatic Play") // throws random darts, and shows the bookkeeping instead of the records
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Coinage))
	PORT_DIPSETTING(0x00, DEF_STR(1C_1C))
	PORT_DIPSETTING(0x02, DEF_STR(1C_2C))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown)) // checked at boot: when on, RAM isn't cleared after an NMI
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "Rounds for x01 and Cricket")
	PORT_DIPSETTING(0x00, "20")
	PORT_DIPSETTING(0x08, "199")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Up")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Select")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Down")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_COIN1) PORT_NAME("Coin 1 (100 Pts)")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_COIN2) PORT_NAME("Coin 2 (500 Pts)")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Cancel")

	// the layout finds the targets by these names
	PORT_START("DART0")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 17")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 13")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 5")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 8")

	PORT_START("DART1")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 3")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 6")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 20")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 11")

	PORT_START("DART2")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 19")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 10")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 1")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 14")

	PORT_START("DART3")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 7")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 15")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 18")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 9")

	PORT_START("DART4")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 16")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 2")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 4")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Triple 12")

	PORT_START("DART5")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN) // scored as a 0 point dart
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double Bull")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single Bull")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN) // scored as a 0 point dart

	PORT_START("DART6")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 17")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 13")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 5")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 8")

	PORT_START("DART7")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 17")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 13")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 5")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 8")

	PORT_START("DART8")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 3")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 6")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 20")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 11")

	PORT_START("DART9")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 3")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 6")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 20")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 11")

	PORT_START("DART10")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 19")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 10")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 1")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 14")

	PORT_START("DART11")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 19")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 10")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 1")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 14")

	PORT_START("DART12")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 7")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 15")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 18")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 9")

	PORT_START("DART13")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 7")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 15")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 18")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 9")

	PORT_START("DART14")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 16")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 2")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 4")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Single 12")

	PORT_START("DART15")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 16")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 2")
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 4")
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dart Double 12")
INPUT_PORTS_END

static INPUT_PORTS_START(tecnodargr)
	PORT_INCLUDE(tecnodar)

	PORT_MODIFY("IN0")
	PORT_DIPNAME(0x02, 0x02, "Game Price")
	PORT_DIPSETTING(0x02, "100 Pts, 200 Pts for x01 Double, 501 and Cricket")
	PORT_DIPSETTING(0x00, "100 Pts")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Missed Dart Sensor") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END


void tecnodar_state::tecnodar(machine_config &config)
{
	Z80(config, m_maincpu, 10.245_MHz_XTAL / 3); // GoldStar Z8400APS; divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &tecnodar_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &tecnodar_state::io_map);
	// NMI is some sort of reset control

	I8255(config, m_ppi); // TMP82C55AP-2
	m_ppi->out_pa_callback().set(FUNC(tecnodar_state::ppi_pa_w));
	m_ppi->out_pb_callback().set(FUNC(tecnodar_state::ppi_pb_w));
	m_ppi->in_pc_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::do_read)).bit(0);
	m_ppi->out_pc_callback().set(FUNC(tecnodar_state::ppi_pc_w));

	EEPROM_93C46_8BIT(config, m_eeprom); // unknown 8-pin IC

	SCREEN(config, "screen");

	tms9129_device &vdp(TMS9129(config, "vdp", 10.245_MHz_XTAL)); // surface-scratched 40-pin DIP; exact type unknown
	vdp.set_screen("screen");
	vdp.set_vram_size(0x10000); // 8x MT4264-10
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_psg, 10.245_MHz_XTAL / 6); // Microchip AY38910A/P; divider not verified
	m_psg->port_a_read_callback().set_ioport("IN0");
	m_psg->port_b_read_callback().set(FUNC(tecnodar_state::inputs_r));
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START(tecnodar)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("1_100_333_27c128.bin", 0x0000, 0x4000, CRC(62cdac49) SHA1(7d3c013b14b5db1378c80e24e3e88ddea2d930ec))

	ROM_REGION(0x20000, "banked", 0)
	ROM_LOAD("2_100_tecno_27c512.bin", 0x00000, 0x10000, CRC(971c0c62) SHA1(0eb6a29a5e07e2ed85d9fc298077fa522213e624))
	ROM_LOAD("3_100_333_27c512.bin", 0x10000, 0x10000, CRC(f9bbbfe0) SHA1(505480188b4641cf48ca33f1600d4ec501122844))
	// 2 more ROM sockets are empty

	ROM_REGION(0x117, "plds", 0)
	ROM_LOAD("gal16v8.bin", 0x000, 0x117, NO_DUMP)
ROM_END

ROM_START(tecnodargr)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("15_100_tecno_27c128.bin", 0x0000, 0x4000, CRC(776f1c48) SHA1(90e659ca5339113113c621d8beddde0d478bbf4a))

	ROM_REGION(0x20000, "banked", 0)
	ROM_LOAD("2_100_gr_27c512.bin", 0x00000, 0x10000, CRC(fbcb5d7d) SHA1(1254ea7d4dec052aa29a51c1e8cf656e25849b13))
	ROM_LOAD("3_100_tecno_27c512.bin", 0x10000, 0x10000, CRC(f9bbbfe0) SHA1(505480188b4641cf48ca33f1600d4ec501122844))
	// 2 more ROM sockets are empty

	ROM_REGION(0x117, "plds", 0)
	ROM_LOAD("16as25hb1.bin", 0x000, 0x117, NO_DUMP)
ROM_END

} // anonymous namespace


GAMEL(1991, tecnodar,   0,        tecnodar, tecnodar,   tecnodar_state, empty_init, ROT0, "Automatics Pasqual",                    "Tecnodarts",                            MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_tecnodar)
GAMEL(1991, tecnodargr, tecnodar, tecnodar, tecnodargr, tecnodar_state, empty_init, ROT0, "Automatics Pasqual / Recreativos G.R.", "Tecnodarts (Recreativos G.R. license)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_tecnodar)
