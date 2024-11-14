// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Promise PDC20262 FastTrak66/UDMA66 IDE controller

No documentation, ATA4 compliant

TODO:
- how it sets compatible/native modes? Subvendor ID list suggests it can switch at will;
- Install win9x driver causes huge loading hiccups, eventually freezes by accessing drive with
  explorer.exe (enable UDMA?). For common use is **suggested** to not install them.
- Reportedly has issues with very big HDDs, pinpoint limit and assuming there isn't an issue here.
\- Tested with Seagate Barracuda ST380021A -chs=158816,16,63 (expected: 80GB, actual: 13655MB)
- Marketed as RAID card, verify;
\- Gets classified as SCSI controller in win9x device manager;
- ID and hookup Flash ROM type;
- PME 1.0 support (no low power states D1/D2, no PME#)

**************************************************************************************************/

#include "emu.h"
#include "pdc20262.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(PDC20262, pdc20262_device,   "pdc20262",   "Promise PDC20262 FastTrak66 EIDE controller")



pdc20262_device::pdc20262_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_ide1(*this, "ide1")
	, m_ide2(*this, "ide2")
	, m_irqs(*this, "irqs")
	// HACK: how to get to get_pci_busmaster_space()?
	, m_bus_master_space(*this, ":maincpu", 0)
	, m_bios_rom(*this, "bios_rom")
{
	// Subsystems:
	// 105a 4d30 Ultra Device on SuperTrak
	// 105a 4d33 Ultra66
	// 105a 4d39 FastTrak66
	// class code is trusted, bp 0xca09c
	// assume revision depending on BIOS
	set_ids(0x105a4d38, 0x02, 0x018000, 0x105a4d33);
}

pdc20262_device::pdc20262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pdc20262_device(mconfig, PDC20262, tag, owner, clock)
{
}

ROM_START( pdc20262 )
	ROM_REGION32_LE( 0x8000, "bios_rom", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("v200")

	ROM_SYSTEM_BIOS( 0, "v200", "Promise Ultra66 BIOS v2.00 (Build 18)" )
	ROMX_LOAD( "ul200b18.bin", 0x0000, 0x4000, CRC(71e48d73) SHA1(84d8c72118a3e26181573412e2cbb859691672de), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v114", "Promise Ultra66 BIOS v1.14 (Build 0728)" )
	ROMX_LOAD( "ul114b0728.bin", 0x0000, 0x4000, CRC(a71f0c3d) SHA1(ace4872c6060e9dd8458540c0f3193d1a9b4321a), ROM_BIOS(1) )

	// v1.12 known to exist
ROM_END

const tiny_rom_entry *pdc20262_device::device_rom_region() const
{
	return ROM_NAME(pdc20262);
}


void pdc20262_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set([this] (int state) {
		irq_pin_w(0, state);
	});

	BUS_MASTER_IDE_CONTROLLER(config, m_ide1).options(ata_devices, "hdd", nullptr, false);
	m_ide1->irq_handler().set([this] (int state) {
		m_irq_state &= ~0x4;
		m_irq_state |= (state << 2);
		m_irqs->in_w<0>(state);
	});
	m_ide1->set_bus_master_space(m_bus_master_space);

	BUS_MASTER_IDE_CONTROLLER(config, m_ide2).options(ata_devices, nullptr, nullptr, false);
	m_ide2->irq_handler().set([this] (int state) {
		m_irq_state &= ~0x40;
		m_irq_state |= (state << 6);
		m_irqs->in_w<1>(state);
	});
	m_ide2->set_bus_master_space(m_bus_master_space);
}

// $1f0
void pdc20262_device::ide1_command_map(address_map &map)
{
	map(0, 7).rw(FUNC(pdc20262_device::ide1_read32_cs0_r), FUNC(pdc20262_device::ide1_write32_cs0_w));
}

// $3f4
void pdc20262_device::ide1_control_map(address_map &map)
{
	map(2, 2).rw(FUNC(pdc20262_device::ide1_read_cs1_r), FUNC(pdc20262_device::ide1_write_cs1_w));
}

// $170
void pdc20262_device::ide2_command_map(address_map &map)
{
	map(0, 7).rw(FUNC(pdc20262_device::ide2_read32_cs0_r), FUNC(pdc20262_device::ide2_write32_cs0_w));
}

// $374
void pdc20262_device::ide2_control_map(address_map &map)
{
	map(2, 2).rw(FUNC(pdc20262_device::ide2_read_cs1_r), FUNC(pdc20262_device::ide2_write_cs1_w));
}

void pdc20262_device::bus_master_ide_control_map(address_map &map)
{
	map(0x00, 0x07).rw(m_ide1, FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
	map(0x08, 0x0f).rw(m_ide2, FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));

	map(0x11, 0x11).lrw8(
		NAME([this] () {
			return m_clock;
		}),
		NAME([this] (u8 data) {
			LOG("extra $11: Clock set %02x\n", data);
			m_clock = data;
		})
	);

//  map(0x1a, 0x1a) Primary Mode
//  map(0x1b, 0x1b) Secondary Mode
/*
 * upper nibble secondary, lower primary
 *
 * x--- error
 * -x-- irq
 * --x- FIFO full
 * ---x FIFO empty
 */
	map(0x1d, 0x1d).lr8(
		NAME([this] () {
			// FIXME: definitely requires a FIFO i/f
			return m_irq_state | 1;
		})
	);
//  map(0x1f, 0x1f) Ultra DMA speed flag
}

void pdc20262_device::extra_map(address_map &map)
{
	// TODO: should be memory mapped versions of above, *nix driver seems to use this
//  map(0x00, 0x07).m(*this, FUNC(pdc20262_device::ide1_command_map)));
// ...
}

void pdc20262_device::device_start()
{
	pci_card_device::device_start();

	add_map(8, M_IO, FUNC(pdc20262_device::ide1_command_map));
	add_map(4, M_IO, FUNC(pdc20262_device::ide1_control_map));
	add_map(8, M_IO, FUNC(pdc20262_device::ide2_command_map));
	add_map(4, M_IO, FUNC(pdc20262_device::ide2_control_map));
	add_map(32, M_IO, FUNC(pdc20262_device::bus_master_ide_control_map));
	// TODO: unknown size (a lot larger?), to be verified later thru PnP
	add_map(64, M_MEM, FUNC(pdc20262_device::extra_map));

	add_rom((u8 *)m_bios_rom->base(), 0x4000);
	expansion_rom_base = 0xc8000;

	// INTA#
	intr_pin = 1;
}

void pdc20262_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	command_mask = 7;
	status = 0x0210;

	remap_cb();
}

void pdc20262_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	// latency timer
	map(0x0d, 0x0d).lr8(NAME([] () { return 0x01; }));
	// TODO: everything, starting from capptr_r override
}

/*
 * Start of legacy handling, to be moved out
 */

uint32_t pdc20262_device::ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask)
{
	if (!(command & 1))
		return 0xffffffff;
	return m_ide1->read_cs0(offset, mem_mask);
}

void pdc20262_device::ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!(command & 1))
		return;
	m_ide1->write_cs0(offset, data, mem_mask);
}

uint32_t pdc20262_device::ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask)
{
	if (!(command & 1))
		return 0xffffffff;
	return m_ide2->read_cs0(offset, mem_mask);
}

void pdc20262_device::ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!(command & 1))
		return;
	m_ide2->write_cs0(offset, data, mem_mask);
}

uint8_t pdc20262_device::ide1_read_cs1_r()
{
	if (!(command & 1))
		return 0xff;
	return m_ide1->read_cs1(1, 0xff0000) >> 16;
}

void pdc20262_device::ide1_write_cs1_w(uint8_t data)
{
	if (!(command & 1))
		return;
	m_ide1->write_cs1(1, data << 16, 0xff0000);
}

uint8_t pdc20262_device::ide2_read_cs1_r()
{
	if (!(command & 1))
		return 0xff;
	return m_ide2->read_cs1(1, 0xff0000) >> 16;
}

void pdc20262_device::ide2_write_cs1_w(uint8_t data)
{
	if (!(command & 1))
		return;
	m_ide2->write_cs1(1, data << 16, 0xff0000);
}
