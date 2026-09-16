// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/********************************************************************************************************************

2013-07-31 Skeleton Driver [Curt Coder]
2013-07-31 Connected to terminal [Robbbert]
2016-07-11 After 10 seconds the monitor program will start [Robbbert]

Commands: (no spaces allowed)
B - Boot the disk
D - Dump memory to screen
F - Fill Memory
G - Go To
H - Help
P - Alter port values
S - Alter memory


The photos show 3 boards:
- A "SASI Interface" board (all 74-series TTL)
- CPU board (64k dynamic RAM, Z80A CPU, 2x Z80CTC, 2x Z80SIO/0, MB8877A, Z80DMA, 4x MC1488,
  4x MC1489, XTALS 1.8432MHz and 24MHz)
- ADES board (Adaptec Inc AIC-100, AIC-250, AIC-300, Intel D8085AH, unknown crystal)


********************************************************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80sio.h"


namespace {

class dsb46_state : public driver_device
{
public:
	dsb46_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
	{ }

	void dsb46(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void port1a_w(u8 data);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
};

void dsb46_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share(m_ram);
	map(0x0000, 0x07ff).bankr(m_bank1);
}

void dsb46_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x08, 0x0b).rw("ctc1", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x1a, 0x1a).w(FUNC(dsb46_state::port1a_w));
	//map(0x10, 0x10) disk related
	//map(0x14, 0x14) ?? (read after CTC1 TRG3)
	//map(0x18, 0x18) ??
	//map(0x1c, 0x1c) disk data
	//map(0x1d, 0x1d) disk status (FF = no fdc)
}

static INPUT_PORTS_START( dsb46 )
INPUT_PORTS_END

void dsb46_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);
}

void dsb46_state::machine_reset()
{
	m_bank1->set_entry(1);
}

void dsb46_state::port1a_w(u8 data)
{
	m_bank1->set_entry(BIT(~data, 0));
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc1" },
	{ "sio" },
	{ nullptr }
};


void dsb46_state::dsb46(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 24_MHz_XTAL / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &dsb46_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &dsb46_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	/* Devices */
	z80sio_device& sio(Z80SIO(config, "sio", 24_MHz_XTAL / 6));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));

	z80ctc_device &ctc1(Z80CTC(config, "ctc1", 24_MHz_XTAL / 6));
	ctc1.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc1.set_clk<0>(1.8432_MHz_XTAL);
	ctc1.zc_callback<0>().set("sio", FUNC(z80sio_device::rxca_w));
	ctc1.zc_callback<0>().append("sio", FUNC(z80sio_device::txca_w));
	ctc1.set_clk<2>(1.8432_MHz_XTAL);
	ctc1.zc_callback<2>().set("sio", FUNC(z80sio_device::rxcb_w));
	ctc1.zc_callback<2>().append("sio", FUNC(z80sio_device::txcb_w));
}

ROM_START( dsb46 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "1538a.bin", 0x0000, 0x0800, CRC(65b3e26e) SHA1(afe1f03f266b7d13fdb1f1bc6762df5e0aa5c764) )

	ROM_REGION( 0x4000, "ades", 0 )
	ROM_LOAD( "ades.bin", 0x0000, 0x4000, CRC(d374abf0) SHA1(331f51a2bb81375aeffbe63c1ebc1d7cd779b9c3) )
ROM_END

} // anonymous namespace


COMP( 198?, dsb46, 0, 0, dsb46, dsb46, dsb46_state, empty_init, "Davidge", "DSB-4/6",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
