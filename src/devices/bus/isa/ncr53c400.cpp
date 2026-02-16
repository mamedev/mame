// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

NCR53C400 based ISA cards

'400 is a SCSI controller overlay for 5380/53C80, essentially a later iteration of the asc88 one

TODO:
- hangs when checking icmd with bit 6 high (IC_AIP);
- DMA;
- add remaining cards;

**************************************************************************************************/

#include "emu.h"
#include "ncr53c400.h"

#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"

DEFINE_DEVICE_TYPE(ISA8_RT1000B, isa8_rt1000b_device, "rt1000b", " Rancho Technology RT1000B SCSI controller")

isa8_rt1000b_device::isa8_rt1000b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_RT1000B, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_scsic(*this, "scsi:7:scsic")
	, m_config(*this, "CONFIG")
{
}

void isa8_rt1000b_device::scsic_config(device_t *device)
{
	downcast<ncr53c80_device &>(*device).irq_handler().set("^^", FUNC(isa8_rt1000b_device::irq_w));
	downcast<ncr53c80_device &>(*device).drq_handler().set("^^", FUNC(isa8_rt1000b_device::drq_w));
}

void isa8_rt1000b_device::device_add_mconfig(machine_config &config)
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
		.option_add_internal("scsic", NCR53C80)
		.machine_config([this] (device_t *device) { scsic_config(device); });

//	EEPROM_93C06_16BIT(config, m_eeprom);
}

ROM_START(rt1000b)
	ROM_REGION(0x4000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("rtbios_v8.20r_b256.u3", 0x0000, 0x4000, CRC(18d4d2f8) SHA1(4a5b2219b9ea5bfbeefaf6b958f4095c53c0e8b4))
	ROM_CONTINUE( 0x0000, 0x4000 )
ROM_END

const tiny_rom_entry *isa8_rt1000b_device::device_rom_region() const
{
	return ROM_NAME(rt1000b);
}


static INPUT_PORTS_START(rt1000b)
	PORT_START("CONFIG")
	// J1-J3
	PORT_CONFNAME(0x07, 0x06, "BIOS Segment Address")
	PORT_CONFSETTING(0x00, "C000h")
	PORT_CONFSETTING(0x01, "C400h")
	PORT_CONFSETTING(0x02, "C800h")
	PORT_CONFSETTING(0x03, "CC00h")
	PORT_CONFSETTING(0x04, "D000h")
	PORT_CONFSETTING(0x05, "D400h")
	PORT_CONFSETTING(0x06, "D800h")
	PORT_CONFSETTING(0x07, "DC00h")
	// J4
	// TODO: what's this exactly, is "floptical" a typo?
	PORT_CONFNAME(0x08, 0x08, "Drive A: setting")
	PORT_CONFSETTING( 0x00, "Floptical drive") // closed
	PORT_CONFSETTING( 0x08, "Other drive") // open
	// Connections
	PORT_CONFNAME( 0x10, 0x10, "SCSI connector")
	PORT_CONFSETTING( 0x00, "External") // J5
	PORT_CONFSETTING( 0x10, "Internal") // J10
	// J6-J7-J8-J9
	// NOTE: IRQ2/IRQ6 aren't implemented in this specific card
	PORT_CONFNAME( 0xe0, 0xe0, "Interrupt Select")
//	PORT_CONFSETTING( 0x00, "N/A")
//	PORT_CONFSETTING( 0x20, "N/A")
//	PORT_CONFSETTING( 0x40, "IRQ2")
	PORT_CONFSETTING( 0x60, "IRQ3")
	PORT_CONFSETTING( 0x80, "IRQ4")
	PORT_CONFSETTING( 0xa0, "IRQ5")
//	PORT_CONFSETTING( 0xc0, "IRQ6")
	PORT_CONFSETTING( 0xe0, "IRQ7")
INPUT_PORTS_END

ioport_constructor isa8_rt1000b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(rt1000b);
}

void isa8_rt1000b_device::device_start()
{
	set_isa_device();

	m_internal_sram = make_unique_clear<u8[]>(0x40);
	m_external_sram = make_unique_clear<u8[]>(0x600);

	save_item(NAME(m_control));
	save_item(NAME(m_irq));
	save_item(NAME(m_drq));
	save_item(NAME(m_block_counter));
	save_pointer(NAME(m_internal_sram), 0x40);
	save_pointer(NAME(m_external_sram), 0x600);

	// TODO: DMA
//	m_isa->set_dma_channel(1, this, false);
//	m_isa->set_dma_channel(5, this, false);
}


void isa8_rt1000b_device::device_reset()
{
	m_config_address = 0x18000;
	m_irq_line = (m_config->read() >> 5) & 7;
	remap(AS_PROGRAM, 0, 0xfffff);
}

void isa8_rt1000b_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		const offs_t baseaddr = 0xc0000 | m_config_address;
		m_isa->install_rom(this, baseaddr, baseaddr | 0x37ff, "bios");
		m_isa->install_bank(baseaddr | 0x3800, baseaddr | 0x383f, m_internal_sram.get());
		m_isa->install_memory(baseaddr | 0x3880, baseaddr | 0x3887,
				read8sm_delegate(*m_scsic, FUNC(ncr53c80_device::read)),
				write8sm_delegate(*m_scsic, FUNC(ncr53c80_device::write)));
		//m_isa->install_memory(baseaddr | 0x3900, baseaddr | 0x397f) Host buffer
		// Control/status section
		m_isa->install_memory(baseaddr | 0x3980, baseaddr | 0x3980,
				read8sm_delegate(*this, FUNC(isa8_rt1000b_device::control_r)),
				write8sm_delegate(*this, FUNC(isa8_rt1000b_device::control_w)));
		m_isa->install_memory(baseaddr | 0x3981, baseaddr | 0x3981,
				read8sm_delegate(*this, FUNC(isa8_rt1000b_device::block_counter_r)),
				write8sm_delegate(*this, FUNC(isa8_rt1000b_device::block_counter_w)));
		m_isa->install_memory(baseaddr | 0x3982, baseaddr | 0x3982,
				read8sm_delegate(*this, FUNC(isa8_rt1000b_device::switch_r)),
				write8sm_delegate(*this, FUNC(isa8_rt1000b_device::resume_xfer_w)));

		m_isa->install_bank(baseaddr | 0x3a00, baseaddr | 0x3fff, m_external_sram.get());
	}
}

void isa8_rt1000b_device::irq_w(int state)
{
	if (m_irq != state)
	{
		m_irq = state;
		m_control &= ~(1 << 0);
		m_control |= (state << 0);
		if (BIT(m_control, 4))
		{
			switch(m_irq_line)
			{
				case 2: m_isa->irq2_w(state); break;
				case 3: m_isa->irq3_w(state); break;
				case 4: m_isa->irq4_w(state); break;
				case 5: m_isa->irq5_w(state); break;
				case 6: m_isa->irq6_w(state); break;
				case 7: m_isa->irq7_w(state); break;
			}
		}
	}
}

void isa8_rt1000b_device::drq_w(int state)
{
	if (m_drq != state)
	{
		m_drq = state;
		// ...
	}
}

/*
 * x--- ---- (on read) 53c80 register access (on write) reset
 * -x-- ---- data transfer direction (1) read (0) write
 * --x- ---- enable SCSI buffer irq
 * ---x ---- enable 53C80 irq
 * ---- x--- Shared irq
 * ---- -x-- Host buffer not ready (r/o)
 * ---- --x- SCSI buffer ready (r/o)
 * ---- ---x Gate 53C80 IRQ
 */
u8 isa8_rt1000b_device::control_r(offs_t offset)
{
	return m_control;
}

void isa8_rt1000b_device::control_w(offs_t offset, u8 data)
{
	m_control = data;
	logerror("control_w %02x\n", data);
	if (BIT(data, 7))
		m_scsic->reset();
}

u8 isa8_rt1000b_device::block_counter_r(offs_t offset)
{
	return m_block_counter;
}

void isa8_rt1000b_device::block_counter_w(offs_t offset, u8 data)
{
	m_block_counter = data;
	logerror("block_counter_w %02x\n", data);
}

u8 isa8_rt1000b_device::switch_r(offs_t offset)
{
	// bits 7-3 "user defined", assume SCSI controller ID
	return ((m_config->read() >> 5) & 7) | 0xf8;
}

void isa8_rt1000b_device::resume_xfer_w(offs_t offset, u8 data)
{
	// dummy write
	(void)data;
	logerror("resume_xfer_w\n");
}
