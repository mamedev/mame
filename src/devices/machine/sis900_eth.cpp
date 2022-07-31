// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    SiS 900 Fast Ethernet Controller / Adapter

    TODO:
    - Stub interface, to be improved;
    - Sensible defaults for EEPROM;

**************************************************************************************************/

#include "emu.h"
#include "sis900_eth.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps
#define LOG_PMC    (1U << 4) // log PMC access

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP | LOG_PMC)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)
#define LOGPMC(...)    LOGMASKED(LOG_PMC,  __VA_ARGS__)

DEFINE_DEVICE_TYPE(SIS900_ETH, sis900_eth_device, "sis900_eth", "SiS 900 Fast Ethernet Adapter")

sis900_eth_device::sis900_eth_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, SIS900_ETH, tag, owner, clock)
	, m_eeprom(*this, "eeprom")
	, m_eth_rom(*this, "eth_rom")
{
	// 0x020000 - Network Ethernet controller
	// 0x80 - "Silicon revision" (sic)
	set_ids(0x10390900, 0x80, 0x020000, 0x00);
}

void sis900_eth_device::device_add_mconfig(machine_config &config)
{
	EEPROM_93C66_16BIT(config, m_eeprom); // NM93Cxx
}

void sis900_eth_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x40, 0x43).r(FUNC(sis900_eth_device::pmc_id_r));
//  map(0x44, 0x47).r(FUNC(sis900_eth_device::pmc_status_r), FUNC(sis900_eth_device::pmc_control_w));
}

void sis900_eth_device::memory_map(address_map &map)
{

}

void sis900_eth_device::io_map(address_map &map)
{
	map(0x08, 0x08).rw(FUNC(sis900_eth_device::eromar_r), FUNC(sis900_eth_device::eromar_w));
}

void sis900_eth_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
//  io_space->install_device(0, 0x03ff, *this, &sis900_eth_device::io_map);
}

ROM_START( sis900eth )
	ROM_REGION32_LE( 0x10000, "eth_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "pxe_m.19", 0x0000, 0xa000, BAD_DUMP CRC(c8da34a6) SHA1(f11b4f5398176b7d924c63c77a1951cb83020e48) )
	// structure:
	// [0x00] signature -> 0x55aa
	// [0x08] MAC address
	// [0x0b] checksum
	// PCIR signature+[0x00] vendor ID -> 0x1039
	// PCIR signature+[0x02] device ID -> 0x0900
ROM_END

const tiny_rom_entry *sis900_eth_device::device_rom_region() const
{
	return ROM_NAME(sis900eth);
}

void sis900_eth_device::device_start()
{
	pci_device::device_start();

	add_map(256, M_IO, FUNC(sis900_eth_device::io_map));
	add_map(4096, M_MEM, FUNC(sis900_eth_device::memory_map));
	add_rom((u8 *)m_eth_rom->base(), m_eth_rom->bytes());

	// INTC#
	intr_pin = 3;
	// TODO: "if auto load is enabled it is set by subvendor ID stored in EEPROM" (?)
	subsystem_id = 0x10390900;
}

void sis900_eth_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	status = 0x0290;

	m_eromar_mode = false;
}

uint8_t sis900_eth_device::capptr_r()
{
	return 0x40;
}

// TODO: move to specific interface
u32 sis900_eth_device::pmc_id_r()
{
	LOGPMC("Read PMC ID [$40]\n");
	// bits 31-16 PPMI v1.0 / v1.0a, D1 / D2 supported, PME# D0/D1/D2/D3hot & optional D3cold
	// bits 15-8 0x00 no NEXT_PTR (NULL terminates here)
	// bits 7-0 PM_CAP_ID (0x01 for PMC)
	// TODO: versioning may depend on ROM installed
	return 0xfe010001;
}

/*
 * I/O space
 */

u8 sis900_eth_device::eromar_r()
{
	// TODO: doc claims all bits are readable, looks unlikely?
	u8 res = m_eromar_mode << 7;
	if (m_eromar_mode)
	{
		// ...
	}
	else
		res |= m_eeprom->do_read() << 1;

	return res;
}

void sis900_eth_device::eromar_w(u8 data)
{
	m_eromar_mode = bool(BIT(data, 7));

	LOGIO("eromar_w %s selected %02x\n", m_eromar_mode ? "HomePHY" : "eeprom", data);

	if (m_eromar_mode)
	{
		// ...
	}
	else
	{
		m_eeprom->cs_write(BIT(data, 3));
		m_eeprom->clk_write(BIT(data, 2));
		m_eeprom->di_write(BIT(data, 0));
	}
}
