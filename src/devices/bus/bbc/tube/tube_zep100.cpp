// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Torch Z80 Communicator (ZEP100)

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Torch_Z802ndproc.html

**********************************************************************/

#include "emu.h"
#include "tube_zep100.h"

#include "cpu/z80/z80.h"
#include "machine/6522via.h"
#include "machine/i8255.h"

#include "softlist_dev.h"


namespace {

// ======================> bbc_tube_zep100_device

class bbc_tube_zep100_device : public device_t, public device_bbc_tube_interface
{
public:
	bbc_tube_zep100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_tube_zep100_device(mconfig, BBC_TUBE_ZEP100, tag, owner, clock)
	{
	}

protected:
	bbc_tube_zep100_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_bbc_tube_interface(mconfig, *this)
		, m_rom_enabled(true)
		, m_z80(*this, "z80")
		, m_via(*this, "via")
		, m_ppi(*this, "ppi")
		, m_rom(*this, "rom")
		, m_port_b(0)
	{
	}

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

	bool m_rom_enabled;

private:
	required_device<z80_device> m_z80;
	required_device<via6522_device> m_via;
	required_device<i8255_device> m_ppi;
	required_memory_region m_rom;

	std::unique_ptr<uint8_t[]> m_ram;

	uint8_t m_port_b;

	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	void via_pb_w(uint8_t data);
	uint8_t ppi_pb_r();
	void ppi_pc_w(uint8_t data);

	void tube_zep100_io(address_map &map) ATTR_COLD;
	void tube_zep100_mem(address_map &map) ATTR_COLD;
};


// ======================> bbc_tube_zep100l_device

class bbc_tube_zep100l_device : public bbc_tube_zep100_device
{
public:
	bbc_tube_zep100l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_tube_zep100_device(mconfig, BBC_TUBE_ZEP100L, tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> bbc_tube_zep100w_device

class bbc_tube_zep100w_device : public bbc_tube_zep100_device
{
public:
	bbc_tube_zep100w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_tube_zep100_device(mconfig, BBC_TUBE_ZEP100W, tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> bbc_tube_zep100m_device

class bbc_tube_zep100m_device : public bbc_tube_zep100_device
{
public:
	bbc_tube_zep100m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_tube_zep100_device(mconfig, BBC_TUBE_ZEP100M, tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


//-------------------------------------------------
//  ADDRESS_MAP( tube_zep100_mem )
//-------------------------------------------------

void bbc_tube_zep100_device::tube_zep100_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(bbc_tube_zep100_device::mem_r), FUNC(bbc_tube_zep100_device::mem_w));
}

//-------------------------------------------------
//  ADDRESS_MAP( tube_zep100_io )
//-------------------------------------------------

void bbc_tube_zep100_device::tube_zep100_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x07).mirror(0xff00).rw(FUNC(bbc_tube_zep100_device::io_r), FUNC(bbc_tube_zep100_device::io_w));
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( tube_zep100 )
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_DEFAULT_BIOS("mcp120")
	ROM_SYSTEM_BIOS(0, "mcp120", "MCP v1.20 (CBL)")  // 1985
	ROMX_LOAD("mcp120cbl.rom", 0x0000, 0x4000, CRC(851d0879) SHA1(2e54ef15692ba7dd9fcfd1ef0d660464a772b156), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mcp041", "MCP v0.41 (CBL)")  // 1983
	ROMX_LOAD("mcp041cbl.rom", 0x0000, 0x4000, CRC(b36f07f4) SHA1(bd53f09bf73357845a6f97df1ee9e5aea5cdca90), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "cpn071", "CPN 0.71")         // 1982
	ROMX_LOAD("cpn71.rom", 0x0000, 0x2000, CRC(fcb1bdc8) SHA1(756e22f6d76eb26206765f92c78c7152944102b6), ROM_BIOS(2))
	ROM_RELOAD(            0x2000, 0x2000)

	ROM_REGION(0x2000, "rom", 0)
	ROMX_LOAD("cccp102.rom", 0x0000, 0x2000, CRC(2eb40a21) SHA1(e6ee738e5f2f8556002b79d18caa8ef21f14e08d), ROM_BIOS(0))
	ROMX_LOAD("cccp094.rom", 0x0000, 0x2000, CRC(49989bd4) SHA1(62b57c858a3baa4ff943c31f77d331c414772a61), ROM_BIOS(1))
	ROMX_LOAD("cccp094.rom", 0x0000, 0x2000, CRC(49989bd4) SHA1(62b57c858a3baa4ff943c31f77d331c414772a61), ROM_BIOS(2))
ROM_END

ROM_START( tube_zep100l )
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_SYSTEM_BIOS(0, "mcp121", "MCP v1.21 (ABL)") // 1985
	ROMX_LOAD("mcp121abl.rom", 0x0000, 0x4000, CRC(216f1074) SHA1(ec3d285f757b1b35bd0cc65155048a55cc1146ed), ROM_BIOS(0))

	ROM_REGION(0x2000, "rom", 0)
	ROM_LOAD("cccp102.rom", 0x0000, 0x2000, CRC(2eb40a21) SHA1(e6ee738e5f2f8556002b79d18caa8ef21f14e08d))
ROM_END

ROM_START( tube_zep100w )
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_SYSTEM_BIOS(0, "mcp122", "MCP v1.22 (ABW)") // 1986
	ROMX_LOAD("mcp122abw.rom", 0x0000, 0x4000, CRC(7124e9a6) SHA1(39cdd4367bfb2c8d6599b4a4420e6d311f8acaa9), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mcp121", "MCP v1.21 (ABW)") // 1985
	ROMX_LOAD("mcp121abw.rom", 0x0000, 0x4000, CRC(5e4a4a56) SHA1(27172e29f1020147c7ae52111b4234ee4a22e627), ROM_BIOS(1))

	ROM_REGION(0x2000, "rom", 0)
	ROM_LOAD("cccp102.rom", 0x0000, 0x2000, CRC(2eb40a21) SHA1(e6ee738e5f2f8556002b79d18caa8ef21f14e08d))
ROM_END

ROM_START( tube_zep100m )
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_SYSTEM_BIOS(0, "mcp202", "MCP v2.02 (CBM)") // 1987
	ROMX_LOAD("mcp202cbm.rom", 0x0000, 0x4000, CRC(237bd2d0) SHA1(064203ce44c764b189465fa93b71bcc6f0c66e3b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mcp122", "MCP v1.22 (ABM)") // 1986
	ROMX_LOAD("mcp122abm.rom", 0x0000, 0x4000, CRC(596be795) SHA1(35843ace281494ab40463a69a263244d0cb0ecd6), ROM_BIOS(1))

	ROM_REGION(0x2000, "rom", 0)
	ROM_LOAD("cccp102.rom", 0x0000, 0x2000, CRC(2eb40a21) SHA1(e6ee738e5f2f8556002b79d18caa8ef21f14e08d))
ROM_END

const tiny_rom_entry *bbc_tube_zep100_device::device_rom_region() const
{
	return ROM_NAME( tube_zep100 );
}

const tiny_rom_entry *bbc_tube_zep100l_device::device_rom_region() const
{
	return ROM_NAME( tube_zep100l );
}

const tiny_rom_entry *bbc_tube_zep100w_device::device_rom_region() const
{
	return ROM_NAME( tube_zep100w );
}

const tiny_rom_entry *bbc_tube_zep100m_device::device_rom_region() const
{
	return ROM_NAME( tube_zep100m );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_zep100_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_z80, 4_MHz_XTAL);
	m_z80->set_addrmap(AS_PROGRAM, &bbc_tube_zep100_device::tube_zep100_mem);
	m_z80->set_addrmap(AS_IO, &bbc_tube_zep100_device::tube_zep100_io);

	MOS6522(config, m_via, 4_MHz_XTAL / 2);
	m_via->writepb_handler().set(FUNC(bbc_tube_zep100_device::via_pb_w));
	m_via->cb2_handler().set(m_ppi, FUNC(i8255_device::pc2_w));
	m_via->ca2_handler().set(m_ppi, FUNC(i8255_device::pc6_w));
	m_via->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_tube_slot_device::irq_w));

	I8255A(config, m_ppi, 0);
	m_ppi->out_pa_callback().set(m_via, FUNC(via6522_device::write_pa));
	m_ppi->in_pb_callback().set(FUNC(bbc_tube_zep100_device::ppi_pb_r));
	m_ppi->out_pc_callback().set(FUNC(bbc_tube_zep100_device::ppi_pc_w));

	SOFTWARE_LIST(config, "flop_ls_torch").set_original("bbc_flop_torch").set_filter("Z80");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_zep100_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x10000);
	memset(m_ram.get(), 0xff, 0x10000);

	/* register for save states */
	save_pointer(NAME(m_ram), 0x10000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_zep100_device::device_reset()
{
	m_rom_enabled = true;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_tube_zep100_device::host_r(offs_t offset)
{
	return m_via->read(offset & 0x0f);
}

void bbc_tube_zep100_device::host_w(offs_t offset, uint8_t data)
{
	if (offset & 0x10)
		m_z80->reset();

	m_via->write(offset & 0x0f, data);
}


uint8_t bbc_tube_zep100_device::mem_r(offs_t offset)
{
	uint8_t data;

	if (m_rom_enabled)
		data = m_rom->base()[offset];
	else
		data = m_ram[offset];

	return data;
}

void bbc_tube_zep100_device::mem_w(offs_t offset, uint8_t data)
{
	m_ram[offset] = data;
}


uint8_t bbc_tube_zep100_device::io_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (!machine().side_effects_disabled())
		m_rom_enabled = !BIT(offset, 2);

	data = m_ppi->read(offset & 0x03);

	return data;
}

void bbc_tube_zep100_device::io_w(offs_t offset, uint8_t data)
{
	m_ppi->write(offset & 0x03, data);
}


void bbc_tube_zep100_device::via_pb_w(uint8_t data)
{
	m_port_b = data;
}

uint8_t bbc_tube_zep100_device::ppi_pb_r()
{
	return m_port_b;
}

void bbc_tube_zep100_device::ppi_pc_w(uint8_t data)
{
	m_via->write_ca1(BIT(data, 7));
	m_via->write_cb1(BIT(data, 1));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_TUBE_ZEP100, device_bbc_tube_interface, bbc_tube_zep100_device, "bbc_tube_zep100", "Torch Z80 Communicator")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_TUBE_ZEP100L, device_bbc_tube_interface, bbc_tube_zep100l_device, "bbc_tube_zep100l", "Torch Z80 Communicator (Model B)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_TUBE_ZEP100W, device_bbc_tube_interface, bbc_tube_zep100w_device, "bbc_tube_zep100w", "Torch Z80 Communicator (Model B+)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_TUBE_ZEP100M, device_bbc_tube_interface, bbc_tube_zep100m_device, "bbc_tube_zep100m", "Torch Z80 Communicator (Master)")
