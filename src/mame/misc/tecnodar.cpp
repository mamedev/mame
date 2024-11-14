// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************
    Skeleton driver for Automatics Pasqual darts with CRT display.
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

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"
#include "screen.h"
#include "speaker.h"


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
		, m_coins(*this, "COINS")
		, m_inputs(*this, "IN%u", 0U)
		, m_input_select(0xffff)
	{
	}

	void tecnodar(machine_config &config);

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
	required_ioport m_coins;
	required_ioport_array<16> m_inputs;

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
	m_rombank->set_entry((offset & 0x0c) >> 2 | (offset & 0x01) << 2);
}

u8 tecnodar_state::inputs_r()
{
	u8 ret = 0xf;
	for (int i = 0; i < 16; i++)
		if (!BIT(m_input_select, i))
			ret &= m_inputs[i]->read();

	return (ret << 4) | m_coins->read();
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
	if (!BIT(data, 7))
		m_psg->reset(); // maybe

	logerror("%s: Writing %02X to PPI port B\n", machine().describe_context(), data);
}

void tecnodar_state::ppi_pc_w(u8 data)
{
	m_input_select = u16(data) << 8 | (m_input_select & 0x00ff);
}

void tecnodar_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x8002, 0x8002).select(0x60).rw(FUNC(tecnodar_state::ppi_r), FUNC(tecnodar_state::ppi_w));
	map(0x8004, 0x8004).w("psg", FUNC(ay8910_device::data_w));
	map(0x8008, 0x8008).r("psg", FUNC(ay8910_device::data_r));
	map(0x800c, 0x800c).w("psg", FUNC(ay8910_device::address_w));
	map(0x8010, 0x8011).rw("vdp", FUNC(tms9129_device::read), FUNC(tms9129_device::write));
	map(0xc000, 0xc7ff).ram();
}

void tecnodar_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).w(FUNC(tecnodar_state::bank_w));
}


static INPUT_PORTS_START(tecnodar)
	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown))
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown))
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown))
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown))
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown))
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown))
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unused))
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("COINS")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN0")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN1")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN3")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN4")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN5")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN6")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN7")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN8")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN9")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN10")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN11")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN12")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN13")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN14")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN15")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN)
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

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	tms9129_device &vdp(TMS9129(config, "vdp", 10.245_MHz_XTAL)); // surface-scratched 40-pin DIP; exact type unknown
	vdp.set_screen("screen");
	vdp.set_vram_size(0x10000); // 8x MT4264-10
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_psg, 10.245_MHz_XTAL / 6); // Microchip AY38910A/P; divider not verified
	m_psg->port_a_read_callback().set_ioport("DSW");
	m_psg->port_b_read_callback().set(FUNC(tecnodar_state::inputs_r));
	m_psg->add_route(ALL_OUTPUTS, "mono", 1.0);
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


GAME(1991, tecnodar,   0,        tecnodar, tecnodar, tecnodar_state, empty_init, ROT0, "Automatics Pasqual",                    "Tecnodarts",                            MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME(1991, tecnodargr, tecnodar, tecnodar, tecnodar, tecnodar_state, empty_init, ROT0, "Automatics Pasqual / Recreativos G.R.", "Tecnodarts (Recreativos G.R. license)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
