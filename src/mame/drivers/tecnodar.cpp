// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Automatics Pasqual darts with CRT display.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"
#include "screen.h"
#include "speaker.h"

class tecnodar_state : public driver_device
{
public:
	tecnodar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi")
		, m_eeprom(*this, "eeprom")
		, m_rombank(*this, "rombank")
	{
	}

	void tecnodar(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void bank_w(u8 data);
	u8 ppi_r(offs_t offset);
	void ppi_w(offs_t offset, u8 data);
	void ppi_pa_w(u8 data);
	void ppi_pb_w(u8 data);
	void ppi_pc_w(u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_memory_bank m_rombank;
};


void tecnodar_state::machine_start()
{
	m_rombank->configure_entries(0, 8, memregion("banked")->base(), 0x4000);
	m_rombank->set_entry(0);
}


void tecnodar_state::bank_w(u8 data)
{
	m_rombank->set_entry(data & 0x07);
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
}

void tecnodar_state::ppi_pb_w(u8 data)
{
	logerror("%s: Writing %02X to PPI port B\n", machine().describe_context(), data);
}

void tecnodar_state::ppi_pc_w(u8 data)
{
	logerror("%s: Writing %02X to PPI port C\n", machine().describe_context(), data);
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
	map(0x00, 0x00).w(FUNC(tecnodar_state::bank_w));
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
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown))
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
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

	ay8910_device &psg(AY8910(config, "psg", 10.245_MHz_XTAL / 6)); // Microchip AY38910A/P; divider not verified
	psg.port_a_read_callback().set_ioport("DSW");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START(tecnodar)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("1_100_333_27c128.bin", 0x0000, 0x4000, CRC(62cdac49) SHA1(7d3c013b14b5db1378c80e24e3e88ddea2d930ec))

	ROM_REGION(0x20000, "banked", 0)
	ROM_LOAD("2_100_tecno_27c512.bin", 0x00000, 0x10000, CRC(971c0c62) SHA1(0eb6a29a5e07e2ed85d9fc298077fa522213e624))
	ROM_LOAD("3_100_333_27c512.bin", 0x10000, 0x10000, CRC(f9bbbfe0) SHA1(505480188b4641cf48ca33f1600d4ec501122844))
	// 2 more ROM sockets are empty
ROM_END


GAME(1991, tecnodar, 0, tecnodar, tecnodar, tecnodar_state, empty_init, ROT0, "Automatics Pasqual", "Tecnodarts", MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
