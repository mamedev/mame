// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    ICD Advantage
    ICD AdSCSI 2000
    ICD AdSCSI 2080

    Zorro-II SCSI interface. Up to 8 MB Fast RAM with the 2080 model.

    Notes:
    - Advantage is the earlier name for the AdSCSI 2000 (might have slight
      hardware differences)
    - Holding the left mouse button disables autoboot
    - Holding the right mouse button inverts the cache jumper setting
    - Verions 2.2 and older have issues with Kickstart 3.1 (hang on reset).
      You can enable the commented ROMX_FILL to make 2.2 work better.

    TODO:
    - GAME jumper (disables card)
    - 6 MB BASE 20/40 jumper

***************************************************************************/

#include "emu.h"
#include "adscsi.h"

#include "bus/nscsi/devices.h"
#include "machine/autoconfig.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"

#define LOG_REG (1U << 1)
#define LOG_DMA (1U << 2)

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


namespace bus::amiga::zorro {

class adscsi2000_device : public device_t, public device_zorro2_card_interface
{
public:
	adscsi2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	adscsi2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void busrst_w(int state) override;
	virtual void cfgin_w(int state) override;

	void unmap(offs_t base, unsigned size);

private:
	uint8_t rom_r(offs_t offset);
	void autoconfig_w(offs_t offset, uint8_t data);
	uint8_t ncr_reg_r(offs_t offset);
	void ncr_reg_w(offs_t offset, uint8_t data);
	uint16_t ncr_dma_r(offs_t offset, uint16_t mem_mask);
	void ncr_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t scsi_status_r();

	void ncr_irq_w(int state);
	void ncr_drq_w(int state);

	required_device<nscsi_bus_device> m_scsi;
	required_device<ncr5380_device> m_ncr;
	required_region_ptr<uint8_t> m_rom;
	required_ioport m_jumpers;

	uint8_t m_board_address = 0;
	bool m_board_configured = false;

	uint8_t m_ncr_drq = 0;

	uint8_t m_holding_remaining = 0;
	uint16_t m_holding = 0;
	bool m_dma_in_progress = false;
	bool m_drq_completed = false;
	bool m_is_write = false;
};

class adscsi2080_device : public adscsi2000_device, public amiga_autoconfig
{
public:
	adscsi2080_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void busrst_w(int state) override;
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	required_ioport m_jumpers_ram;

	uint8_t m_ram_address[2] = { 0, 0 };
	bool m_ram_configured = false;

	std::unique_ptr<uint16_t[]> m_ram;
	uint8_t m_ram_size = 0;
	bool m_configure_6mb = false;
};

adscsi2000_device::adscsi2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	device_t(mconfig, type, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_scsi(*this, "scsi"),
	m_ncr(*this, "ncr"),
	m_rom(*this, "rom"),
	m_jumpers(*this, "jumpers")
{
}

adscsi2000_device::adscsi2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	adscsi2000_device(mconfig, tag, owner, clock, AMIGA_ADSCSI2000)
{
}

adscsi2080_device::adscsi2080_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	adscsi2000_device(mconfig, tag, owner, clock, AMIGA_ADSCSI2080),
	m_jumpers_ram(*this, "jumpers_ram")
{
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( adscsi2000 )
	PORT_START("jumpers")
	PORT_CONFNAME(0x07, 0x07, "J3: SCSI ID")
	PORT_CONFSETTING(0x00, "0")
	PORT_CONFSETTING(0x01, "1")
	PORT_CONFSETTING(0x02, "2")
	PORT_CONFSETTING(0x03, "3")
	PORT_CONFSETTING(0x04, "4")
	PORT_CONFSETTING(0x05, "5")
	PORT_CONFSETTING(0x06, "6")
	PORT_CONFSETTING(0x07, "7")
	PORT_CONFNAME(0x08, 0x00, "J3: GAME (Disable board)")
	PORT_CONFSETTING(0x00, DEF_STR( Off ))
	PORT_CONFSETTING(0x08, DEF_STR( On ))
	PORT_CONFNAME(0x10, 0x00, "J3: B (Caching)")
	PORT_CONFSETTING(0x00, DEF_STR( Off ))
	PORT_CONFSETTING(0x10, DEF_STR( On ))
	PORT_CONFNAME(0x20, 0x00, "J3: A (Disable Boot ROM)")
	PORT_CONFSETTING(0x00, DEF_STR( Off ))
	PORT_CONFSETTING(0x20, DEF_STR( On ))
	PORT_CONFNAME(0x40, 0x40, "Spin-Up Delay") // TODO: What jumper is this?
	PORT_CONFSETTING(0x00, "20 Seconds")
	PORT_CONFSETTING(0x40, "8 Seconds")

	// TODO: Jumper J5, J6
INPUT_PORTS_END

ioport_constructor adscsi2000_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( adscsi2000 );
}

static INPUT_PORTS_START( adscsi2080 )
	PORT_INCLUDE(adscsi2000)

	PORT_START("jumpers_ram")
	PORT_CONFNAME(0x03, 0x03, "J0/J1: Installed RAM")
	PORT_CONFSETTING(0x00, "2")
	PORT_CONFSETTING(0x01, "4")
	PORT_CONFSETTING(0x02, "6")
	PORT_CONFSETTING(0x03, "8")
	PORT_CONFNAME(0x04, 0x04, "J2: 6 MB BASE")
	PORT_CONFSETTING(0x00, "40")
	PORT_CONFSETTING(0x04, "20")
INPUT_PORTS_END

ioport_constructor adscsi2080_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( adscsi2080 );
}


//**************************************************************************
//  ZORRO / AUTOCONFIG
//**************************************************************************

void adscsi2000_device::unmap(offs_t base, unsigned size)
{
	LOG("Unmapping from %06x to %06x\n", base, base + size - 1);
	zorro_space().unmap_readwrite(base, base + size - 1);
}

void adscsi2000_device::busrst_w(int state)
{
	if (state)
		return;

	if (m_board_configured)
	{
		LOG("Removing AdSCSI\n");

		unmap(m_board_address << 16, 0x10000);
		m_board_address = 0;
		m_board_configured = false;
	}

	m_holding = 0xffff;
	m_holding_remaining = 0;
	m_dma_in_progress = false;
	m_drq_completed = false;

	m_ncr->reset();
	m_ncr_drq = 0;

	xrdy_w(1);
}

void adscsi2080_device::busrst_w(int state)
{
	adscsi2000_device::busrst_w(state);

	if (state)
		return;

	if (m_ram_configured)
	{
		LOG("Removing AdSCSI RAM\n");

		if (m_ram_size == 6)
		{
			if (m_ram_address[0])
				unmap(m_ram_address[0] << 16, 0x400000);
			if (m_ram_address[1])
				unmap(m_ram_address[1] << 16, 0x200000);
		}
		else
		{
			if (m_ram_address[0])
				unmap(m_ram_address[0] << 16, m_ram_size << 20);
		}

		m_ram_address[0] = 0;
		m_ram_address[1] = 0;
		m_ram_configured = false;
	}

	m_configure_6mb = false;
}

void adscsi2000_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state == 0)
	{
		// install autoconfig handler
		zorro_space().install_readwrite_handler(0xe80000, 0xe8007f,
			read8sm_delegate(*this, FUNC(adscsi2000_device::rom_r)),
			write8sm_delegate(*this, FUNC(adscsi2000_device::autoconfig_w)), 0xff00);
	}
}

void adscsi2080_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state)
		return;

	// setup autoconfig
	autoconfig_board_type(BOARD_TYPE_ZORRO2);

	if (m_configure_6mb)
	{
		// part two of the 6 mb ram configuration
		autoconfig_board_size(BOARD_SIZE_2M);
	}
	else
	{
		// setup ram from jumper
		switch (m_jumpers_ram->read() & 3)
		{
		case 0:
			autoconfig_board_size(BOARD_SIZE_2M);
			m_ram_size = 2;
			break;
		case 1:
			autoconfig_board_size(BOARD_SIZE_4M);
			m_ram_size = 4;
			break;
		case 2:
			autoconfig_board_size(BOARD_SIZE_4M);
			m_ram_size = 6;
			break;
		case 3:
			autoconfig_board_size(BOARD_SIZE_8M);
			m_ram_size = 8;
			break;
		}
	}

	autoconfig_product(4);
	autoconfig_manufacturer(2071);
	autoconfig_serial(0x00000000);

	autoconfig_link_into_memory(true);
	autoconfig_rom_vector_valid(false);
	autoconfig_multi_device(false);
	autoconfig_8meg_preferred(false);
	autoconfig_can_shutup(false); // ?

	// install autoconfig handler
	zorro_space().install_readwrite_handler(0xe80000, 0xe8007f,
		read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
		write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
}

void adscsi2080_device::autoconfig_base_address(offs_t address)
{
	LOG("Autoconfig address received: 0x%06x\n", address);

	// stop responding to default autoconfig
	zorro_space().unmap_readwrite(0xe80000, 0xe8007f);

	if (address == 0)
	{
		LOG("Autoconfig Shut-Up: Not supported\n");
		adscsi2000_device::cfgin_w(0);
		return;
	}

	LOG("-> Installing AdSCSI RAM\n");

	// install ram
	if (m_ram_size == 6 && !m_configure_6mb)
	{
		// the 6 MB option is special, we need to configure twice (4 MB + 2 MB)
		zorro_space().install_ram(address, address + 0x400000 - 1, m_ram.get());
		m_ram_address[0] = address >> 16;

		// restart memory config
		m_configure_6mb = true;
		cfgin_w(0);
	}
	else
	{
		if (m_configure_6mb)
		{
			zorro_space().install_ram(address, address + 0x200000 - 1, m_ram.get() + (0x400000 / sizeof(uint16_t)));
			m_ram_address[1] = address >> 16;
		}
		else
		{
			zorro_space().install_ram(address, address + (m_ram_size << 20) - 1, m_ram.get());
			m_ram_address[0] = address >> 16;
		}

		m_ram_configured = true;

		// tell the scsi controller to start configuration
		adscsi2000_device::cfgin_w(0);
	}
}

uint8_t adscsi2000_device::rom_r(offs_t offset)
{
	return m_rom[offset];
}

void adscsi2000_device::autoconfig_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x24:
			m_board_address = (m_board_address & 0x0f) | ((data & 0xf0) >> 0);
			{
				offs_t addr = m_board_address << 16;

				LOG("Autoconfig address received: 0x%06x\n", addr);
				LOG("-> Installing AdSCSI\n");

				// map rom (if not disabled)
				if ((m_jumpers->read() & 0x20) == 0)
					zorro_space().install_read_handler(addr + 0x0000, addr + 0xffff,
						read8sm_delegate(*this, FUNC(adscsi2000_device::rom_r)), 0xff00);

				zorro_space().install_readwrite_handler(addr + 0x00, addr + 0x3f,
					read8sm_delegate(*this, FUNC(adscsi2000_device::ncr_reg_r)),
					write8sm_delegate(*this, FUNC(adscsi2000_device::ncr_reg_w)), 0xff00);

				zorro_space().install_write_handler(addr + 0x20, addr + 0x21,
					write16s_delegate(*this, FUNC(adscsi2000_device::ncr_dma_w)), 0xffff);

				zorro_space().install_read_handler(addr + 0x38, addr + 0x39,
					read16s_delegate(*this, FUNC(adscsi2000_device::ncr_dma_r)), 0xffff);

				zorro_space().install_read_handler(addr + 0x40, addr + 0x41,
					read8smo_delegate(*this, FUNC(adscsi2000_device::scsi_status_r)), 0xff00);

				m_board_configured = true;

				// stop responding to default autoconfig and lower cfgout
				zorro_space().unmap_readwrite(0xe80000, 0xe8007f);
				cfgout_w(0);
			}
			break;

		case 0x25:
			m_board_address = (m_board_address & 0xf0) | ((data & 0xf0) >> 4);
			break;

		default:
			LOG("Autoconfig: Write to unsupported register %02x\n", offset);
			break;
	}
}


//**************************************************************************
//  SCSI / DMA
//**************************************************************************

uint8_t adscsi2000_device::ncr_reg_r(offs_t offset)
{
	unsigned const reg = (offset >> 1) & 7;
	return m_ncr->read(reg);
}

void adscsi2000_device::ncr_reg_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_REG, "ncr5380_reg_w: %02x = %02x\n", offset, data);

	unsigned const reg = (offset >> 1) & 7;

	if ((reg == 5) || (reg == 7))
	{
		LOGMASKED(LOG_DMA, "Rearming DMA engine\n");

		m_dma_in_progress = true;
		m_drq_completed = false;
		m_is_write = (reg == 5);
	}

	// address 0x3c
	if ((reg == 7) && BIT(offset, 4))
	{
		// some kind of reset port - might also reset the ncr
		m_holding = 0xffff;
		m_holding_remaining = 0;
	}

	m_ncr->write(reg, data);
}

uint16_t adscsi2000_device::ncr_dma_r(offs_t offset, uint16_t mem_mask)
{
	if (machine().side_effects_disabled())
		return 0xffff;

	if (mem_mask != 0xffff)
		fatalerror("8-bit transfer not implemented\n");

	if (m_drq_completed)
	{
		// read was completed in the drq handler, return result
		LOGMASKED(LOG_DMA, "DMA read complete: %04x\n", m_holding);
		m_drq_completed = false;
		return m_holding;
	}

	if (!m_dma_in_progress)
	{
		LOGMASKED(LOG_DMA, "DMA read with no DMA in progress!\n");
		return 0xffff;
	}

	// new word
	m_holding = 0xffff;
	m_holding_remaining = 2;

	LOGMASKED(LOG_DMA, "DMA read new word\n");

	while (m_ncr_drq && (m_holding_remaining > 0))
	{
		m_holding <<= 8;
		m_holding |= m_ncr->dma_r();
		m_holding_remaining--;

		LOGMASKED(LOG_DMA, "DMA read %02x, remaining %d\n", (m_holding >> 0) & 0xff, m_holding_remaining);
	}

	if (m_holding_remaining == 0)
	{
		// we're done, return the word
		return m_holding;
	}
	else
	{
		// we need to wait for more data, suspend cpu until we get it
		xrdy_w(0);
		return 0xffff;
	}
}

void adscsi2000_device::ncr_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask != 0xffff)
		fatalerror("8-bit transfer not implemented\n");

	if (m_drq_completed)
	{
		// write was completed in the drq handler, return
		m_drq_completed = false;
		return;
	}

	if (!m_dma_in_progress)
	{
		LOGMASKED(LOG_DMA, "DMA write with no DMA in progress!\n");
		return;
	}

	// new word
	m_holding = data;
	m_holding_remaining = 2;

	LOGMASKED(LOG_DMA, "DMA write new word: %04x\n", data);

	while (m_ncr_drq && (m_holding_remaining > 0))
	{
		LOGMASKED(LOG_DMA, "DMA write %02x, remaining %d\n", m_holding >> 8, m_holding_remaining);

		m_ncr->dma_w(m_holding >> 8);
		m_holding <<= 8;
		m_holding_remaining--;
	}

	// if needed suspend cpu until we can write the remaining data
	if (m_holding_remaining > 0)
		xrdy_w(0);
}

uint8_t adscsi2000_device::scsi_status_r()
{
	// 7-------  pseudo-dma ready (latch empty)
	// -6------  ncr drq
	// --5-----  spin-up delay (0 = 20s delay, 1 = 8s delay)
	// ---4----  caching jumper
	// ----3---  not used?
	// -----210  scsi id (inverted)

	uint8_t data = 0;

	data |= ((m_jumpers->read() & 7) ^ 7);
	data |= ((m_jumpers->read() & 0x10) ? 1 : 0) << 4;
	data |= ((m_jumpers->read() & 0x40) ? 1 : 0) << 5;
	data |= (m_ncr_drq ? 1 : 0) << 6;
	data |= ((m_holding_remaining == 0) ? 1 : 0) << 7;

	return data;
}

void adscsi2000_device::ncr_irq_w(int state)
{
	if (state)
	{
		m_dma_in_progress = false;
		m_holding_remaining = 0;
		xrdy_w(1); // safe-guard, unusual dma could leave the cpu suspended
	}
}

void adscsi2000_device::ncr_drq_w(int state)
{
	LOGMASKED(LOG_DMA, "NCR drq %d -> %d, remaining %d (write %d)\n", m_ncr_drq, state, m_holding_remaining, m_is_write);

	bool const rising = (m_ncr_drq == 0) && (state == 1);
	m_ncr_drq = state;

	if (rising && (m_holding_remaining > 0))
	{
		if (m_is_write)
		{
			LOGMASKED(LOG_DMA, "DMA write %02x (drq)\n", (m_holding >> 8) & 0xff);

			m_ncr->dma_w(m_holding >> 8);
			m_holding <<= 8;
			m_holding_remaining--;
		}
		else
		{
			m_holding <<= 8;
			m_holding |= m_ncr->dma_r();
			m_holding_remaining--;

			LOGMASKED(LOG_DMA, "DMA read %02x (drq)\n", (m_holding >> 0) & 0xff);
		}

		LOGMASKED(LOG_DMA, "-> %d remaining\n", m_holding_remaining);

		if (m_holding_remaining == 0)
		{
			// word fully handled, resume cpu
			m_drq_completed = true;
			xrdy_w(1);
		}
	}
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void adscsi2000_device::device_start()
{
	// register for save states
	save_item(NAME(m_board_address));
	save_item(NAME(m_board_configured));
	save_item(NAME(m_ncr_drq));
	save_item(NAME(m_holding_remaining));
	save_item(NAME(m_holding));
	save_item(NAME(m_dma_in_progress));
	save_item(NAME(m_drq_completed));
	save_item(NAME(m_is_write));
}

void adscsi2080_device::device_start()
{
	adscsi2000_device::device_start();

	// allocate ram
	m_ram = make_unique_clear<uint16_t[]>(0x800000 / 2);

	// register for save states
	save_item(NAME(m_ram_configured));
	save_item(NAME(m_ram_address));
	save_pointer(NAME(m_ram), 0x800000 / 2);
	save_item(NAME(m_ram_size));
	save_item(NAME(m_configure_6mb));
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void adscsi2000_device::device_add_mconfig(machine_config &config)
{
	NCR5380(config, m_ncr, DERIVED_CLOCK(1, 1)); // LOGIC L5380PC-2 (clock assumed)
	m_ncr->irq_handler().set(FUNC(adscsi2000_device::ncr_irq_w));
	m_ncr->drq_handler().set(FUNC(adscsi2000_device::ncr_drq_w));

	NSCSI_BUS(config, m_scsi);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	m_scsi->set_external_device(7, m_ncr);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( adscsi )
	ROM_REGION(0x8000, "rom", 0)
	ROM_DEFAULT_BIOS("35r1")
	ROM_SYSTEM_BIOS(0, "16",   "AdSCSI 1.6") // icddisk.device V1.0
	ROMX_LOAD("adscsi_16.u28",   0x0000, 0x8000, CRC(7dba3e1f) SHA1(1e05f284d59a1e5d4e4de44e6f075175625cd6c0), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "201",  "AdSCSI 2.01") // icddisk.device V32.1
	ROMX_LOAD("adscsi_201.u28",  0x0000, 0x8000, CRC(0860a46d) SHA1(f6bd052006d504494efd48dae6a58d7b774c9eef), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "21",   "AdSCSI 2.1") // icddisk.device V33.0 (is this "AdSCSI 2.10" from photos?)
	ROMX_LOAD("adscsi_21.u28",   0x0000, 0x8000, CRC(3184ec04) SHA1(f8ab84b04853404f840085ce5fe992e35dc443da), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "22",   "AdSCSI 2.2") // icddisk.device V34.0
	ROMX_LOAD("adscsi_22.u28",   0x0000, 0x8000, CRC(6881cd2f) SHA1(482fab24df39446ba8662c7e67f11854895a46d9), ROM_BIOS(3))
	// ROMX_FILL(0x7065, 1, 0x05, ROM_BIOS(3)) // use MEMF_REVERSE for allocating memory
	ROM_SYSTEM_BIOS(4, "35r1", "AdSCSI 3.5r1") // icddisk.device V53.1
	ROMX_LOAD("adscsi_35r1.u28", 0x0000, 0x8000, CRC(378a7d85) SHA1(6a78364ebf14e735a150ba6890a617a2b72f3f42), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "421", "AdSCSI 4.21") // 1993
	ROMX_LOAD("adscsi_421.u28",  0x0000, 0x8000, NO_DUMP, ROM_BIOS(5))
ROM_END

const tiny_rom_entry *adscsi2000_device::device_rom_region() const
{
	return ROM_NAME( adscsi );
}

} // bus::amiga::zorro


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(AMIGA_ADSCSI2000, device_zorro2_card_interface, bus::amiga::zorro::adscsi2000_device, "amiga_adscsi2000", "ICD Advantage/AdSCSI 2000")
DEFINE_DEVICE_TYPE_PRIVATE(AMIGA_ADSCSI2080, device_zorro2_card_interface, bus::amiga::zorro::adscsi2080_device, "amiga_adscsi2080", "ICD AdSCSI 2080")
