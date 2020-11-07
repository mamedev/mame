// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ASC-88 SCSI Adapter
    © 1985 Advanced Storage Concepts, Inc.

    This is a rather basic 8-bit SCSI host adapter on a quarter-size card.
    IRQ and DMA channels are software-selectable. There is a 2K HM6116P-2
    SRAM on board, but not all of it seems to be addressable.

    The BIOS was designed to accommodate either a NMOS 5380 or a 53C80-40.

***************************************************************************/

#include "emu.h"
#include "asc88.h"

#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"


DEFINE_DEVICE_TYPE(ASC88, asc88_device, "asc88", "ASC-88 SCSI Adapter")


asc88_device::asc88_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ASC88, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_scsic(*this, "scsi:7:scsic")
	, m_eeprom(*this, "eeprom")
	, m_baseaddr(*this, "BASEADDR")
	, m_control(0)
	, m_irq(false)
	, m_drq(false)
{
}

void asc88_device::device_start()
{
	set_isa_device();

	m_ram = make_unique_clear<u8[]>(0x800);

	save_item(NAME(m_control));
	save_item(NAME(m_irq));
	save_item(NAME(m_drq));
	save_pointer(NAME(m_ram), 0x800);
}

void asc88_device::device_reset()
{
	const offs_t baseaddr = 0xc0000 | (u32(m_baseaddr->read()) << 14);
	m_isa->install_rom(this, baseaddr, baseaddr | 0x37ff, "bios");
	m_isa->install_bank(baseaddr | 0x3800, baseaddr | 0x3fef, m_ram.get());
	m_isa->install_memory(baseaddr | 0x3ff0, baseaddr | 0x3ff7,
			read8sm_delegate(*m_scsic, FUNC(ncr5380n_device::read)),
			write8sm_delegate(*m_scsic, FUNC(ncr5380n_device::write)));
	m_isa->install_memory(baseaddr | 0x3ff8, baseaddr | 0x3ff8,
			read8smo_delegate(*this, FUNC(asc88_device::eeprom_r)),
			write8smo_delegate(*this, FUNC(asc88_device::control_w)));

	control_w(0);
}

WRITE_LINE_MEMBER(asc88_device::irq_w)
{
	if (m_irq != state)
	{
		m_irq = state;
		if (BIT(m_control, 3))
			m_isa->irq2_w(state);
		if (BIT(m_control, 4))
			m_isa->irq3_w(state);
		if (BIT(m_control, 5))
			m_isa->irq5_w(state);
	}
}

WRITE_LINE_MEMBER(asc88_device::drq_w)
{
	if (m_drq != state)
	{
		m_drq = state;
		if (BIT(m_control, 6))
			m_isa->drq1_w(state);
		if (BIT(m_control, 7))
			m_isa->drq3_w(state);
	}
}

u8 asc88_device::dack_r(int line)
{
	return m_scsic->dma_r();
}

void asc88_device::dack_w(int line, u8 data)
{
	m_scsic->dma_w(data);
}

void asc88_device::control_w(u8 data)
{
	const u8 changing_bits = data ^ m_control;
	m_control = data;

	m_eeprom->di_write(BIT(data, 0));
	m_eeprom->clk_write(BIT(data, 1));
	m_eeprom->cs_write(BIT(data, 2));

	if (m_irq)
	{
		if (BIT(changing_bits, 3))
			m_isa->irq2_w(BIT(data, 3));
		if (BIT(changing_bits, 4))
			m_isa->irq3_w(BIT(data, 4));
		if (BIT(changing_bits, 5))
			m_isa->irq5_w(BIT(data, 5));
	}

	if (m_drq)
	{
		if (BIT(changing_bits, 6))
			m_isa->drq1_w(BIT(data, 6));
		if (BIT(changing_bits, 7))
			m_isa->drq3_w(BIT(data, 7));
	}
}

u8 asc88_device::eeprom_r()
{
	return m_eeprom->do_read();
}

static INPUT_PORTS_START(asc88)
	PORT_START("BASEADDR")
	PORT_DIPNAME(0x7, 0x3, "BIOS Segment Address") PORT_DIPLOCATION("A16-A14:!1,!2,!3")
	// For each jumper, pins 1 & 2 closed = off (0); pins 2 & 3 closed = on (1)
	PORT_DIPSETTING(0x0, "C000h")
	PORT_DIPSETTING(0x1, "C400h")
	PORT_DIPSETTING(0x2, "C800h")
	PORT_DIPSETTING(0x3, "CC00h")
	PORT_DIPSETTING(0x4, "D000h")
	PORT_DIPSETTING(0x5, "D400h")
	PORT_DIPSETTING(0x6, "D800h")
	PORT_DIPSETTING(0x7, "DC00h")
INPUT_PORTS_END

ioport_constructor asc88_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(asc88);
}

void asc88_device::scsic_config(device_t *device)
{
	downcast<ncr5380n_device &>(*device).irq_handler().set("^^", FUNC(asc88_device::irq_w));
	downcast<ncr5380n_device &>(*device).drq_handler().set("^^", FUNC(asc88_device::drq_w));
}

void asc88_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", default_scsi_devices, "scsic", true)
		.option_add_internal("scsic", NCR5380N)
		.machine_config([this] (device_t *device) { scsic_config(device); });

	EEPROM_93C06_16BIT(config, m_eeprom); // NMC9306N
}

ROM_START(asc88)
	ROM_REGION(0x3800, "bios", ROMREGION_ERASE00)
	ROM_LOAD("asc88_rev._4.0n_14-nov-91.u14", 0x0000, 0x2000, CRC(8657d1bf) SHA1(49c44aafbb3f28478e4b5eeaa208b147fdf253b9)) // TMS27C64-IJL
ROM_END

const tiny_rom_entry *asc88_device::device_rom_region() const
{
	return ROM_NAME(asc88);
}
