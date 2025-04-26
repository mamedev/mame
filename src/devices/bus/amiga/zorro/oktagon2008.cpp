// license:BSD-3-clause
// copyright-holders:AJR
/***************************************************************************

    BSC / Alfa Data Oktagon 2008 SCSI controller (2092/5/8)

***************************************************************************/

#include "emu.h"
#include "oktagon2008.h"

#include "bus/nscsi/devices.h"
#include "machine/autoconfig.h"
#include "machine/i2cmem.h"
#include "machine/ncr53c90.h"

#define VERBOSE (0)
#include "logmacro.h"


namespace bus::amiga::zorro {

class oktagon2008_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	oktagon2008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void busrst_w(int state) override;
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	u8 rom_r(offs_t offset);
	u8 int_control_r();
	void int_control_w(u8 data);
	u8 i2c_sda_r();
	void i2c_scl_w(u8 data);
	void i2c_sda_w(u8 data);
	void irq_w(int state);

	required_device<ncr53c94_device> m_scsic;
	required_device<i2cmem_device> m_eeprom;
	required_region_ptr<u8> m_rom;
	required_ioport m_jumpers;

	std::unique_ptr<u16[]> m_ram;
	bool m_ram_autoconfig_needed;
	bool m_scsic_int;
	bool m_scsic_int_enable;
};


oktagon2008_device::oktagon2008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AMIGA_OKTAGON2008, tag, owner, clock)
	, device_zorro2_card_interface(mconfig, *this)
	, m_scsic(*this, "scsi:7:scsic")
	, m_eeprom(*this, "eeprom")
	, m_rom(*this, "rom")
	, m_jumpers(*this, "JUMPERS")
	, m_ram_autoconfig_needed(false)
	, m_scsic_int(false)
	, m_scsic_int_enable(true)
{
}

u8 oktagon2008_device::rom_r(offs_t offset)
{
	return m_rom[offset];
}

u8 oktagon2008_device::int_control_r()
{
	return m_scsic_int_enable ? 0x80 : 0;
}

void oktagon2008_device::int_control_w(u8 data)
{
	if (m_scsic_int_enable != BIT(data, 7))
	{
		LOG("%s: 53C94 interrupt %sabled\n", machine().describe_context(), BIT(data, 7) ? "en" : "dis");
		m_scsic_int_enable = BIT(data, 7);
		m_zorro->int2_w(m_scsic_int && m_scsic_int_enable);
	}
}

u8 oktagon2008_device::i2c_sda_r()
{
	return m_eeprom->read_sda() << 7;
}

void oktagon2008_device::i2c_scl_w(u8 data)
{
	m_eeprom->write_scl(BIT(data, 7));
}

void oktagon2008_device::i2c_sda_w(u8 data)
{
	m_eeprom->write_sda(BIT(data, 7));
}

void oktagon2008_device::irq_w(int state)
{
	m_scsic_int = state;
	if (m_scsic_int_enable)
		m_zorro->int2_w(state);
}

void oktagon2008_device::device_start()
{
	m_ram = make_unique_clear<u16[]>(0x800000 / 2);

	save_pointer(NAME(m_ram), 0x800000 / 2);
	save_item(NAME(m_ram_autoconfig_needed));
	save_item(NAME(m_scsic_int));
}

void oktagon2008_device::busrst_w(int state)
{
	if (!state)
	{
		int_control_w(0);
		m_ram_autoconfig_needed = true;
	}
}

void oktagon2008_device::autoconfig_base_address(offs_t address)
{
	LOG("Autoconfig base address (%s) = $%06X\n", m_ram_autoconfig_needed ? "RAM" : "controller", address);

	if (m_ram_autoconfig_needed)
	{
		int memory_size = m_jumpers->read() & 0x03;
		LOG("%u bytes of RAM installed\n", (memory_size + 1) * 0x200000);
		m_zorro->space().install_ram(address, address + (memory_size + 1) * 0x200000 - 1, m_ram.get());

		m_ram_autoconfig_needed = false;
		cfgin_w(0);
	}
	else
	{
		// Diag init read from $Ex0001-$Ex0FFF; device drivers read from $Ex2001-$ExFFFF
		m_zorro->space().install_read_handler(address + 0x0000, address + 0xffff,
			read8sm_delegate(*this, FUNC(oktagon2008_device::rom_r)), 0x00ff);

		m_zorro->space().install_readwrite_handler(address + 0x1000, address + 0x1fff,
			read16smo_delegate(*m_scsic, FUNC(ncr53c94_device::dma16_swap_r)),
			write16smo_delegate(*m_scsic, FUNC(ncr53c94_device::dma16_swap_w)), 0xffff);
		m_zorro->space().install_device(address + 0x3000, address + 0x301f, *m_scsic, &ncr53c94_device::map, 0xff00);

		m_zorro->space().install_readwrite_handler(address + 0x8000, address + 0x8001,
				read8smo_delegate(*this, FUNC(oktagon2008_device::int_control_r)),
				write8smo_delegate(*this, FUNC(oktagon2008_device::int_control_w)), 0xff00);
		m_zorro->space().install_readwrite_handler(address + 0x8010, address + 0x8011,
				read8smo_delegate(*this, FUNC(oktagon2008_device::i2c_sda_r)),
				write8smo_delegate(*this, FUNC(oktagon2008_device::i2c_scl_w)), 0xff00);
		m_zorro->space().install_readwrite_handler(address + 0x8018, address + 0x8019,
				read8smo_delegate(*this, FUNC(oktagon2008_device::i2c_sda_r)),
				write8smo_delegate(*this, FUNC(oktagon2008_device::i2c_sda_w)), 0xff00);

		m_zorro->space().unmap_readwrite(0xe80000, 0xe8007f);
		m_zorro->cfgout_w(0);
	}
}

void oktagon2008_device::cfgin_w(int state)
{
	if (!state)
	{
		LOG("CFGIN received for %s\n", m_ram_autoconfig_needed ? "RAM" : "controller");

		if (m_ram_autoconfig_needed)
		{
			int memory_size = m_jumpers->read() & 0x03;

			autoconfig_board_type(BOARD_TYPE_ZORRO2);
			if (memory_size == 0)
				autoconfig_board_size(BOARD_SIZE_2M);
			else if (memory_size == 1)
				autoconfig_board_size(BOARD_SIZE_4M);
			else
				autoconfig_board_size(BOARD_SIZE_8M);
			autoconfig_link_into_memory(false);
			autoconfig_rom_vector_valid(false);
			autoconfig_multi_device(false);
			autoconfig_8meg_preferred(false);
			autoconfig_can_shutup(true);
			autoconfig_product(8);
			autoconfig_manufacturer(2092);
			autoconfig_serial(0x00000000);
			autoconfig_rom_vector(0x0000);

			m_zorro->space().install_readwrite_handler(0xe80000, 0xe8007f,
				read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
				write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
		}
		else
		{
			autoconfig_board_type(BOARD_TYPE_ZORRO2);
			autoconfig_board_size(BOARD_SIZE_64K);
			autoconfig_link_into_memory(false);
			autoconfig_rom_vector_valid(true);
			autoconfig_multi_device(false);
			autoconfig_8meg_preferred(false);
			autoconfig_can_shutup(true);
			autoconfig_product(5);
			autoconfig_manufacturer(2092);
			autoconfig_serial(0x00000000);
			autoconfig_rom_vector(0x0001);
		}
	}
}

static INPUT_PORTS_START(oktagon2008)
	PORT_START("JUMPERS")
	PORT_CONFNAME(0x03, 0x03, "Memory Size")
	PORT_CONFSETTING(0x00, "2 MB")
	PORT_CONFSETTING(0x01, "4 MB")
	PORT_CONFSETTING(0x02, "6 MB")
	PORT_CONFSETTING(0x03, "8 MB")
INPUT_PORTS_END

ioport_constructor oktagon2008_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(oktagon2008);
}

void oktagon2008_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("scsic", NCR53C94).machine_config([this](device_t *device) {
		device->set_clock(25_MHz_XTAL);
		downcast<ncr53c94_device &>(*device).set_busmd(ncr53c94_device::BUSMD_1);
		downcast<ncr53c94_device &>(*device).irq_handler_cb().set(*this, FUNC(oktagon2008_device::irq_w));
	});

	I2C_24C04(config, m_eeprom);
}

ROM_START(oktagon2008)
	ROM_REGION(0x8000, "rom", 0)
	ROM_LOAD("oktagon 2008 rev7 bs rl-v6.12.bin", 0x0000, 0x8000, CRC(bb0d2f6a) SHA1(56c441fa37d193393081b2e8ceae823bc7e97e49)) // Mitsubishi M5L27256K,54810C
ROM_END

const tiny_rom_entry *oktagon2008_device::device_rom_region() const
{
	return ROM_NAME(oktagon2008);
}

} // bus::amiga::zorro

DEFINE_DEVICE_TYPE_PRIVATE(AMIGA_OKTAGON2008, device_zorro2_card_interface, bus::amiga::zorro::oktagon2008_device, "amiga_oktagon2008", "Oktagon 2008 SCSI controller")
