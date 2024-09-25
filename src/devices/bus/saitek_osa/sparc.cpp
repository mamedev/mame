// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Saitek OSA Module: Kasparov Sparc (1993)

The chess engine is by the Spracklen's. Their last, and also their strongest.

Hardware notes:
- Fujitsu MB86930-20 SPARClite @ 20MHz
- 256KB ROM (4*AMD AM27C512)
- 1MB DRAM (8*NEC 424256-60), expandable to 4MB
- ESAN 31A-5500 delay line (only on newer PCB)

Two PCB revisions are known, the older one is a lighter green PCB, it has
empty positions for more ICs (maybe expanded RAM?), and does not have a
delay line chip.

The module doesn't have its own LCD screen. It has a grill+fan underneath
at the front part, and a heatsink on the CPU.

About expanded RAM: The 4MB expansion mentioned in the manual works well,
but it doesn't look like the software was designed to work with other options.
At 2MB it doesn't work at all. At 8MB or 16MB it becomes very inefficient,
only using 5MB or 9MB of the available RAM for hash tables.

TODO:
- runs too slow? solving mate problems is around 60% slower than real device,
  maybe cpu cache related?
- opening book moves take 3 around seconds per ply, should be almost immediate

*******************************************************************************/

#include "emu.h"
#include "sparc.h"

#include "cpu/sparc/sparc.h"

namespace {

//-------------------------------------------------
//  initialization
//-------------------------------------------------

class saitekosa_sparc_device : public device_t, public device_saitekosa_expansion_interface
{
public:
	// construction/destruction
	saitekosa_sparc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// from host
	virtual u8 data_r() override;
	virtual void nmi_w(int state) override;
	virtual void ack_w(int state) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<mb86930_device> m_maincpu;
	required_region_ptr<u32> m_rom;
	required_shared_ptr<u32> m_ram;

	u32 m_data_out = 0;
	u32 m_rom_mask = 0;
	u32 m_ram_mask = 0;
	bool m_installed = false;

	void debugger_map(address_map &map) ATTR_COLD;

	u32 rom_r(offs_t offset, u32 mem_mask) { return m_rom[offset & m_rom_mask]; }
	u32 ram_r(offs_t offset, u32 mem_mask) { return m_ram[offset & m_ram_mask]; }
	void ram_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_ram[offset & m_ram_mask]); }
	u32 host_io_r(offs_t offset, u32 mem_mask = ~0U);
	void host_io_w(offs_t offset, u32 data, u32 mem_mask = ~0U);

	void set_ram_mask(u8 n) { m_ram_mask = ((1 << n) / 4) - 1; }
};

saitekosa_sparc_device::saitekosa_sparc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, OSA_SPARC, tag, owner, clock),
	device_saitekosa_expansion_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_rom(*this, "maincpu"),
	m_ram(*this, "ram")
{ }

void saitekosa_sparc_device::device_start()
{
	m_rom_mask = m_rom.length() - 1;
	set_ram_mask(20); // 1MB default

	// register for savestates
	save_item(NAME(m_data_out));
	save_item(NAME(m_rom_mask));
	save_item(NAME(m_ram_mask));
	save_item(NAME(m_installed));
}

void saitekosa_sparc_device::device_reset()
{
	if (!m_installed)
	{
		// MAME doesn't allow reading ioport at device_start
		set_ram_mask(ioport("RAM")->read() ? 22 : 20);
		m_installed = true;
	}

	host_io_w(0, 0x1c00);
}


//-------------------------------------------------
//  host i/o
//-------------------------------------------------

u8 saitekosa_sparc_device::data_r()
{
	return (m_data_out & 0x400) ? 0xff : (u8)m_data_out;
}

void saitekosa_sparc_device::nmi_w(int state)
{
	m_maincpu->set_input_line(SPARC_IRQ1, !state ? ASSERT_LINE : CLEAR_LINE);
}

void saitekosa_sparc_device::ack_w(int state)
{
	if (state != m_expansion->ack_state())
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
}


//-------------------------------------------------
//  internal i/o
//-------------------------------------------------

u32 saitekosa_sparc_device::host_io_r(offs_t offset, u32 mem_mask)
{
	// d0-d7: data input latch, d8: ACK-P
	return m_expansion->data_state() | (m_expansion->ack_state() ? 0 : 0x100);
}

void saitekosa_sparc_device::host_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	// d0-d7: data output latch, d10: output latch enable
	COMBINE_DATA(&m_data_out);

	// d8: STB-P, d9: RTS-P
	m_expansion->stb_w(BIT(m_data_out, 8));
	m_expansion->rts_w(BIT(~m_data_out, 9));
}

void saitekosa_sparc_device::debugger_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom().region("maincpu", 0);
	map(0x01000000, 0x013fffff).ram().share("ram"); // 4MB
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( sparc )
	PORT_START("RAM")
	PORT_CONFNAME( 0x01, 0x00, "RAM Size" )
	PORT_CONFSETTING(    0x00, "1MB" )
	PORT_CONFSETTING(    0x01, "4MB" )
INPUT_PORTS_END

ioport_constructor saitekosa_sparc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sparc);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void saitekosa_sparc_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	MB86930(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(0x00, &saitekosa_sparc_device::debugger_map);
	m_maincpu->cs0_read_cb().set(FUNC(saitekosa_sparc_device::rom_r));
	m_maincpu->cs2_read_cb().set(FUNC(saitekosa_sparc_device::ram_r));
	m_maincpu->cs2_write_cb().set(FUNC(saitekosa_sparc_device::ram_w));
	m_maincpu->cs3_read_cb().set(FUNC(saitekosa_sparc_device::host_io_r));
	m_maincpu->cs3_write_cb().set(FUNC(saitekosa_sparc_device::host_io_w));
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

#define ROM_LOAD32_BYTE_BIOS(bios, name, offset, length, hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_SKIP(3) | ROM_BIOS(bios))

ROM_START( sparc )
	ROM_REGION32_BE(0x40000, "maincpu", 0)

	ROM_DEFAULT_BIOS("v518")

	ROM_SYSTEM_BIOS(0, "v512a", "Sparc (rev. 512A)")
	ROM_LOAD32_BYTE_BIOS(0, "sm16b_512.u5",  0x000000, 0x10000, CRC(96bca59d) SHA1(2c7e693d0cdf69b6e566c6dd03bd24d39e32aa82) )
	ROM_LOAD32_BYTE_BIOS(0, "sm16b_512.u4",  0x000001, 0x10000, CRC(15dd621d) SHA1(e8f7404e84fe027b086fcb918fbcaf2ce4203567) )
	ROM_LOAD32_BYTE_BIOS(0, "sm16b_512.u3",  0x000002, 0x10000, CRC(3201c6e4) SHA1(9a209219a0ab4b4f874381a16773bf33f8f7ba25) )
	ROM_LOAD32_BYTE_BIOS(0, "sm16b_512a.u2", 0x000003, 0x10000, CRC(56dedec7) SHA1(4f9d37e0ca639f892a574aa10a3fb42bba9b82c6) )

	ROM_SYSTEM_BIOS(1, "v518", "Sparc (rev. 518)")
	ROM_LOAD32_BYTE_BIOS(1, "sm16b_518.u5", 0x000000, 0x10000, CRC(cee97a8f) SHA1(541d9d3ee469bacb5a4c537f443836a684e5d168) )
	ROM_LOAD32_BYTE_BIOS(1, "sm16b_518.u4", 0x000001, 0x10000, CRC(7ac37299) SHA1(36f95520fbc9dfed2078d848bf6c3cc7c5823895) )
	ROM_LOAD32_BYTE_BIOS(1, "sm16b_518.u3", 0x000002, 0x10000, CRC(29426d9e) SHA1(ad14f6f2f3e6460d834382faeab78dca9ceb2a63) )
	ROM_LOAD32_BYTE_BIOS(1, "sm16b_518.u2", 0x000003, 0x10000, CRC(b2274cf5) SHA1(d04f41fa1f61439a4655003398f3ae27c2d06f82) )

	ROM_REGION(0x1000, "pals", 0)
	ROM_LOAD("palce16v8h.u23.jed", 0x0000, 0x0c25, CRC(de79fabc) SHA1(27e01ec405e261109dbe10c254b7127eda0f1886) )
	ROM_LOAD("palce16v8h.u32.jed", 0x0000, 0x0c25, CRC(422b66c8) SHA1(44b3394e0586c126ee95129c65e6692ffc01fa8e) )
ROM_END

const tiny_rom_entry *saitekosa_sparc_device::device_rom_region() const
{
	return ROM_NAME(sparc);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(OSA_SPARC, device_saitekosa_expansion_interface, saitekosa_sparc_device, "osa_sparc", "Saitek OSA Sparc")
