// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for E-mu Emulator II synthesizer.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/adc0808.h"
#include "machine/bankdev.h"
#include "machine/pit8253.h"
#include "machine/scn_pci.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"


namespace {

class emu2_state : public driver_device
{
public:
	emu2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pages(*this, "pages")
		, m_scannercpu(*this, "scannercpu")
		, m_scan0(*this, "SCAN%u", 0U)
		, m_scan32(*this, "SCAN%u", 32U)
		, m_scan40(*this, "SCAN%u", 40U)
		, m_page_select{0, 0, 0, 0}
		, m_page_reset(true)
		, m_scan_select(0)
	{
	}

	void emu2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 main_paged_r(offs_t offset);
	void main_paged_w(offs_t offset, u8 data);
	u8 dma_paged_r(offs_t offset);
	void dma_paged_w(offs_t offset, u8 data);
	void page_select_w(offs_t offset, u8 data);
	void scan_select_w(u8 data);
	u8 scan_buffer_r();

	void mem_map(address_map &map) ATTR_COLD;
	void paged_mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void scanner_mem_map(address_map &map) ATTR_COLD;
	void scanner_io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<address_map_bank_device> m_pages;
	required_device<z80_device> m_scannercpu;

	required_ioport_array<16> m_scan0;
	required_ioport_array<2> m_scan32;
	required_ioport_array<2> m_scan40;

	u8 m_page_select[4];
	bool m_page_reset;
	u8 m_scan_select;
};

void emu2_state::machine_start()
{
	save_item(NAME(m_page_select));
	save_item(NAME(m_page_reset));
	save_item(NAME(m_scan_select));
}

void emu2_state::machine_reset()
{
	m_page_reset = true;
}


u8 emu2_state::main_paged_r(offs_t offset)
{
	return m_pages->read8((offset & 0x7fff) | u16(m_page_reset ? 0 : m_page_select[2 | BIT(offset, 15)]) << 15);
}

void emu2_state::main_paged_w(offs_t offset, u8 data)
{
	m_pages->write8((offset & 0x7fff) | u16(m_page_reset ? 0 : m_page_select[2 | BIT(offset, 15)]) << 15, data);
}

u8 emu2_state::dma_paged_r(offs_t offset)
{
	return m_pages->read8((offset & 0x7fff) | u16(m_page_reset ? 0 : m_page_select[BIT(offset, 15)]) << 15);
}

void emu2_state::dma_paged_w(offs_t offset, u8 data)
{
	m_pages->write8((offset & 0x7fff) | u16(m_page_reset ? 0 : m_page_select[BIT(offset, 15)]) << 15, data);
}

void emu2_state::page_select_w(offs_t offset, u8 data)
{
	if (!machine().side_effects_disabled())
		m_page_reset = false;
	m_page_select[offset] = data & 0x0f;
}

void emu2_state::scan_select_w(u8 data)
{
	m_scan_select = data;
}

u8 emu2_state::scan_buffer_r()
{
	if (!BIT(m_scan_select, 5))
		return m_scan0[m_scan_select & 0x0f]->read();
	else if (m_scan_select & 0x06)
		return (BIT(m_scan_select, 3) ? m_scan40 : m_scan32)[m_scan_select & 1]->read();
	else
		return 0xff;
}

void emu2_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(emu2_state::main_paged_r), FUNC(emu2_state::main_paged_w));
}

void emu2_state::paged_mem_map(address_map &map)
{
	map(0x00000, 0x01fff).rom().region("main_program", 0);
	map(0x02000, 0x027ff).ram();
}

void emu2_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).r("usart", FUNC(scn2651_device::read));
	map(0x04, 0x07).w("usart", FUNC(scn2651_device::write));
	map(0x08, 0x0b).mirror(4).w("t6x", FUNC(pit8254_device::write));
	map(0x10, 0x13).mirror(4).w("t35", FUNC(pit8254_device::write));
	map(0x18, 0x1b).mirror(4).w("t02", FUNC(pit8254_device::write));
	map(0x20, 0x20).mirror(7).rw("dma", FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x28, 0x2b).mirror(4).rw("mainctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x30, 0x33).mirror(4).rw("mainpio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x38, 0x3b).mirror(4).rw("disksio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x68, 0x6b).w(FUNC(emu2_state::page_select_w));
}

void emu2_state::scanner_mem_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x6000).rom().region("scanner_program", 0);
	map(0x8000, 0x87ff).mirror(0x7800).ram();
}

void emu2_state::scanner_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x34, 0x34).mirror(0x81).r("adc", FUNC(adc0809_device::data_r));
	map(0x36, 0x36).mirror(0x81).w("adc", FUNC(adc0809_device::address_data_start_w));
	map(0x38, 0x38).mirror(0x81).r(FUNC(emu2_state::scan_buffer_r));
	map(0x50, 0x53).mirror(0x8c).rw("scannerpio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x60, 0x63).mirror(0x8c).rw("scannerctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}


static INPUT_PORTS_START(emu2)
	PORT_START("SCAN0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN4")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN5")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN6")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN8")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN11")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN12")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN13")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN32")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN33")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN40")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN41")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

static const z80_daisy_config main_daisy_chain[] =
{
	{ "disksio" },
	{ "dma" },
	{ "mainctc" },
	{ "mainpio" },
	{ nullptr }
};

static const z80_daisy_config scanner_daisy_chain[] =
{
	{ "scannerctc" },
	{ "scannerpio" },
	{ nullptr }
};

void emu2_state::emu2(machine_config &config)
{
	Z80(config, m_maincpu, 20_MHz_XTAL / 5); // Z80A CPU (IC21)
	m_maincpu->set_addrmap(AS_PROGRAM, &emu2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &emu2_state::io_map);
	m_maincpu->set_daisy_config(main_daisy_chain);

	ADDRESS_MAP_BANK(config, m_pages);
	m_pages->set_endianness(ENDIANNESS_LITTLE);
	m_pages->set_data_width(8);
	m_pages->set_addr_width(19);
	m_pages->set_addrmap(0, &emu2_state::paged_mem_map);

	z80ctc_device &mainctc(Z80CTC(config, "mainctc", 20_MHz_XTAL / 5)); // Z80A CTC (IC60)
	mainctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dma_device &dma(Z80DMA(config, "dma", 20_MHz_XTAL / 5)); // Z80A DMA
	dma.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	dma.in_mreq_callback().set(FUNC(emu2_state::dma_paged_r));
	dma.out_mreq_callback().set(FUNC(emu2_state::dma_paged_w));

	z80pio_device &mainpio(Z80PIO(config, "mainpio", 20_MHz_XTAL / 5)); // Z80A PIO (IC24)
	mainpio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	mainpio.out_pa_callback().set("scannerpio", FUNC(z80pio_device::port_a_write));

	z80sio_device &disksio(Z80SIO(config, "disksio", 20_MHz_XTAL / 5)); // Z80A SIO/2 (IC126)
	disksio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	scn2651_device &usart(SCN2651(config, "usart", 20_MHz_XTAL / 4));
	usart.rxrdy_handler().set("mainctc", FUNC(z80ctc_device::trg1)).invert();

	pit8254_device &t02(PIT8254(config, "t02")); // 8254-2 (IC3)
	t02.set_clk<0>(20_MHz_XTAL / 2);
	t02.set_clk<1>(20_MHz_XTAL / 2);
	t02.set_clk<2>(20_MHz_XTAL / 2);

	pit8254_device &t35(PIT8254(config, "t35")); // 8254-2 (IC2)
	t35.set_clk<0>(20_MHz_XTAL / 2);
	t35.set_clk<1>(20_MHz_XTAL / 2);
	t35.set_clk<2>(20_MHz_XTAL / 2);

	pit8254_device &t6x(PIT8254(config, "t6x")); // 8254-2 (IC1)
	t6x.set_clk<0>(20_MHz_XTAL / 2);
	t6x.set_clk<1>(20_MHz_XTAL / 2);
	t6x.set_clk<2>(20_MHz_XTAL / 2);

	Z80(config, m_scannercpu, 20_MHz_XTAL / 5); // Z80A CPU (IC66)
	m_scannercpu->set_addrmap(AS_PROGRAM, &emu2_state::scanner_mem_map);
	m_scannercpu->set_addrmap(AS_IO, &emu2_state::scanner_io_map);
	m_scannercpu->set_daisy_config(scanner_daisy_chain);

	z80ctc_device &scannerctc(Z80CTC(config, "scannerctc", 20_MHz_XTAL / 5)); // Z80A CTC (IC45)
	scannerctc.intr_callback().set_inputline(m_scannercpu, INPUT_LINE_IRQ0);
	scannerctc.set_clk<2>(20_MHz_XTAL / 10);

	adc0809_device &adc(ADC0809(config, "adc", 20_MHz_XTAL / 40));
	adc.eoc_callback().set("scannerctc", FUNC(z80ctc_device::trg3));

	z80pio_device &scannerpio(Z80PIO(config, "scannerpio", 20_MHz_XTAL / 5)); // Z80A PIO (IC46)
	scannerpio.out_int_callback().set_inputline(m_scannercpu, INPUT_LINE_IRQ0);
	scannerpio.out_pa_callback().set("mainpio", FUNC(z80pio_device::port_a_write));
	scannerpio.out_pb_callback().set(FUNC(emu2_state::scan_select_w));
}

ROM_START(emu2)
	ROM_REGION(0x2000, "main_program", 0)
	ROM_SYSTEM_BIOS(0, "v23", "Version 2.3")
	ROMX_LOAD("e2newermain_hdmb2_3_main.ic42", 0x0000, 0x2000, CRC(c8303551) SHA1(4ec4bf6df95638d4503ab8ff7e859c420ec0720e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v21", "Version 2.1")
	ROMX_LOAD("e2plus_mainboot_2_1_dm_2764.ic42", 0x0000, 0x2000, CRC(91e60278) SHA1(a2332266d6a12eade584b5da85817f3b3ed9037c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "diag", "Diagnostics")
	ROMX_LOAD("e-mu_systems_1986_e2plus_debug_rom_ip341.ic42", 0x0000, 0x2000, CRC(7cd30c30) SHA1(d572d88c8788e25154a3bb8fd2ba3ff8a300ae40), ROM_BIOS(2)) // 2764 chip

	ROM_REGION(0x2000, "scanner_program", 0) // TODO: make these selectable))
	ROM_LOAD("eiiscanv2-1.ic28", 0x0000, 0x2000, CRC(b7913026) SHA1(4485e5423606014be73f121c2ee1f053a6eb148a))
	ROM_LOAD("eiiscanv3-1.ic28", 0x0000, 0x2000, CRC(1544b11d) SHA1(e354388cd694a1ce55a5b7d89c546308628090c7))

	ROM_REGION(0x400, "ucontroller", 0)
	ROM_LOAD("74s472.ic137", 0x000, 0x200, NO_DUMP)
	ROM_LOAD("74s472.ic135", 0x200, 0x200, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1984, emu2, 0, 0, emu2, emu2, emu2_state, empty_init, "E-mu Systems", "Emulator II", MACHINE_IS_SKELETON)
